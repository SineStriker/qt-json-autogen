/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Copyright (C) 2016 Olivier Goffart <ogoffart@woboq.com>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qasc.h"

#include "generator.h"
#include "outputrevision.h"
#include "qdatetime.h"
#include "utils.h"
#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qjsondocument.h>

// for normalizeTypeInternal
#include <private/qmetaobject_moc_p.h>

#include "nameutil.h"

QT_BEGIN_NAMESPACE

// only moc needs this function
static QByteArray normalizeType(const QByteArray &ba, bool fixScope = false) {
    const char *s = ba.constData();
    int len = ba.size();
    char stackbuf[64];
    char *buf = (len >= 64 ? new char[len + 1] : stackbuf);
    char *d = buf;
    char last = 0;
    while (*s && is_space(*s))
        s++;
    while (*s) {
        while (*s && !is_space(*s))
            last = *d++ = *s++;
        while (*s && is_space(*s))
            s++;
        if (*s && ((is_ident_char(*s) && is_ident_char(last)) || ((*s == ':') && (last == '<')))) {
            last = *d++ = ' ';
        }
    }
    *d = '\0';
#if QT_VERSION_MAJOR > 5
    QByteArray result = normalizeTypeInternal(buf, d);
#else
    QByteArray result = normalizeTypeInternal(buf, d, fixScope);
#endif
    if (buf != stackbuf)
        delete[] buf;
    return result;
}

bool Moc::parseClassHead(ClassDef *def) {
    // figure out whether this is a class declaration, or only a
    // forward or variable declaration.

    //    int i = 0;
    //    Token token;
    //    do {
    //        token = lookup(i++);
    //        if (token == COLON || token == LBRACE)
    //            break;
    //        if (token == SEMIC || token == RANGLE)
    //            return false;
    //    } while (token);

    if (!test(IDENTIFIER)) // typedef struct { ... }
        return false;
    QByteArray name = lexem();

    // support "class IDENT name" and "class IDENT(IDENT) name"
    // also support "class IDENT name (final|sealed|Q_DECL_FINAL)"
    if (test(LPAREN)) {
        until(RPAREN);
        if (!test(IDENTIFIER))
            return false;
        name = lexem();
    } else if (test(IDENTIFIER)) {
        const QByteArray lex = lexem();
        if (lex != "final" && lex != "sealed" && lex != "Q_DECL_FINAL")
            name = lex;
    }

    def->qualified += name;
    while (test(SCOPE)) {
        def->qualified += lexem();
        if (test(IDENTIFIER)) {
            name = lexem();
            def->qualified += name;
        }
    }
    def->classname = name;

    if (test(IDENTIFIER)) {
        const QByteArray lex = lexem();
        if (lex != "final" && lex != "sealed" && lex != "Q_DECL_FINAL")
            return false;
    }

    if (test(COLON)) {
        do {
            test(VIRTUAL);
            FunctionDef::Access access = FunctionDef::Public;
            if (test(PRIVATE))
                access = FunctionDef::Private;
            else if (test(PROTECTED))
                access = FunctionDef::Protected;
            else
                test(PUBLIC);
            test(VIRTUAL);

            bool exclude = test(QAS_EXCLUDE_TOKEN);
            bool include = test(QAS_INCLUDE_TOKEN);

            const QByteArray type = parseType().name;
            // ignore the 'class Foo : BAR(Baz)' case
            if (test(LPAREN)) {
                until(RPAREN);
            } else {
                JsonAttributes ja;
                ja.access = access;
                ja.lineNum = symbol().lineNum;
                ja.filename = currentFilenames.top();
                ja.exclude = exclude;
                ja.include = include;
                def->superclassList += qMakePair(type, ja);
            }
        } while (test(COMMA));
    }
    if (!test(LBRACE))
        return false;
    def->begin = index - 1;
    bool foundRBrace = until(RBRACE);
    def->end = index;
    index = def->begin + 1;
    return foundRBrace;
}

Type Moc::parseType() {
    Type type;
    bool hasSignedOrUnsigned = false;
    bool isVoid = false;
    type.firstToken = lookup();
    for (;;) {
        skipCxxAttributes();
        switch (next()) {
            case SIGNED:
            case UNSIGNED:
                hasSignedOrUnsigned = true;
                Q_FALLTHROUGH();
            case CONST:
            case VOLATILE:
                type.name += lexem();
                type.name += ' ';
                if (lookup(0) == VOLATILE)
                    type.isVolatile = true;
                continue;
            case Q_MOC_COMPAT_TOKEN:
            case Q_INVOKABLE_TOKEN:
            case Q_SCRIPTABLE_TOKEN:
            case Q_SIGNALS_TOKEN:
            case Q_SLOTS_TOKEN:
            case Q_SIGNAL_TOKEN:
            case Q_SLOT_TOKEN:
                type.name += lexem();
                return type;
            case NOTOKEN:
                return type;
            default:
                prev();
                break;
        }
        break;
    }

    skipCxxAttributes();
    test(ENUM) || test(CLASS) || test(STRUCT);
    for (;;) {
        skipCxxAttributes();
        switch (next()) {
            case IDENTIFIER:
                // void mySlot(unsigned myArg)
                if (hasSignedOrUnsigned) {
                    prev();
                    break;
                }
                Q_FALLTHROUGH();
            case CHAR:
            case SHORT:
            case INT:
            case LONG:
                type.name += lexem();
                // preserve '[unsigned] long long', 'short int', 'long int', 'long double'
                if (test(LONG) || test(INT) || test(DOUBLE)) {
                    type.name += ' ';
                    prev();
                    continue;
                }
                break;
            case FLOAT:
            case DOUBLE:
            case VOID:
            case BOOL:
                type.name += lexem();
                isVoid |= (lookup(0) == VOID);
                break;
            case NOTOKEN:
                return type;
            default:
                prev();
                break;
        }
        if (test(LANGLE)) {
            if (type.name.isEmpty()) {
                // '<' cannot start a type
                return type;
            }
            type.name += lexemUntil(RANGLE);
        }
        if (test(SCOPE)) {
            type.name += lexem();
            type.isScoped = true;
        } else {
            break;
        }
    }
    while (test(CONST) || test(VOLATILE) || test(SIGNED) || test(UNSIGNED) || test(STAR) || test(AND) || test(ANDAND)) {
        type.name += ' ';
        type.name += lexem();
        if (lookup(0) == AND)
            type.referenceType = Type::Reference;
        else if (lookup(0) == ANDAND)
            type.referenceType = Type::RValueReference;
        else if (lookup(0) == STAR)
            type.referenceType = Type::Pointer;
    }
    type.rawName = type.name;
    // transform stupid things like 'const void' or 'void const' into 'void'
    if (isVoid && type.referenceType == Type::NoReference) {
        type.name = "void";
    }
    return type;
}

QByteArray Moc::parseNamespace() {
    QByteArray ns;
    bool tmp;
    while ((ns += (tmp = test(SCOPE)) ? QByteArrayLiteral("::") : QByteArrayLiteral(""), tmp) ||
           (ns += (tmp = test(IDENTIFIER)) ? lexem() : QByteArrayLiteral(""), tmp)) {
    }
    return ns;
}

// TemplateDef Moc::parseTemplate() {
//     TemplateDef def;
//     next(LANGLE);
//     def.begin = index - 1;
//     until(RANGLE);
//     def.end = index;
//     index = def.begin + 1;

//     int idx = 0;
//     int angleCnt = 0;
//     while (inScope(&def) && hasNext()) {
//         if (test(LANGLE)) {
//             angleCnt++;
//             continue;
//         } else if (test(RANGLE)) {
//             angleCnt--;
//             continue;
//         }
//         if (angleCnt > 0) {
//             next();
//             continue;
//         }

//         if (test(CLASS) || test(TYPENAME)) {
//             if (lookup(1) == IDENTIFIER) {
//                 next();

//             }
//         }

//         if (!test(CLASS) && !test(TYPENAME)) {
//             parseType();
//         } else {
//             if (test(IDENTIFIER)) {
//                 def.typeNames.append(lexem());
//             } else {
//                 def.typeNames.append(QByteArray());
//             }
//         }
//         if (!until(COMMA))
//             break;
//     }
//     return def;
// }

bool Moc::parseEnum(EnumDef *def) {
    bool isTypdefEnum = false; // typedef enum { ... } Foo;

    if (test(CLASS) || test(STRUCT))
        def->isEnumClass = true;

    if (test(IDENTIFIER)) {
        def->name = lexem();
    } else {
        if (lookup(-1) != TYPEDEF)
            return false; // anonymous enum
        isTypdefEnum = true;
    }
    if (test(COLON)) { // C++11 strongly typed enum
        // enum Foo : unsigned long { ... };
        def->enumType = parseType(); // ignore the result
    }
    if (!test(LBRACE))
        return false;
    auto handleInclude = [this]() {
        if (test(MOC_INCLUDE_BEGIN))
            currentFilenames.push(symbol().unquotedLexem());
        if (test(NOTOKEN)) {
            next(MOC_INCLUDE_END);
            currentFilenames.pop();
        }
    };
    do {
        if (lookup() == RBRACE) // accept trailing comma
            break;
        handleInclude();
        {
            bool exclude = false;
            bool include = false;
            QByteArray attr;
            ConstraintGroup constraintGroup;

            auto t = next();
            switch (t) {
                case QAS_ATTR_TOKEN: {
                    parseDeclareType(&attr);
                    attr.replace("\"", "");
                    next(IDENTIFIER);
                    break;
                }

                case QAS_EXCLUDE_TOKEN:
                    exclude = true;
                    next(IDENTIFIER);
                    break;

                case QAS_INCLUDE_TOKEN:
                    include = true;
                    next(IDENTIFIER);
                    break;
                    
                case QAS_CONSTRAINT_TOKEN:
                    if (!parseConstraints(&constraintGroup)) {
                        error("Failed to parse constraint expression");
                        return false;
                    }
                    next(IDENTIFIER);
                    break;

                case IDENTIFIER:
                    break;

                default:
                    error();
                    break;
            }

            JsonAttributes ja;
            ja.itemName = lexem();
            ja.attr = attr;
            ja.exclude = exclude;
            ja.include = include;
            
            // Add constraint group if it has constraints
            if (!constraintGroup.constraints.isEmpty()) {
                ja.constraintGroups.append(constraintGroup);
            }

            def->values += ja;
        }
        handleInclude();
        skipCxxAttributes();
    } while (test(EQ) ? until(COMMA) : test(COMMA));
    next(RBRACE);
    if (isTypdefEnum) {
        if (!test(IDENTIFIER))
            return false;
        def->name = lexem();
    }
    return true;
}

void Moc::parseFunctionArguments(FunctionDef *def) {
    Q_UNUSED(def);
    while (hasNext()) {
        ArgumentDef arg;
        arg.type = parseType();
        if (arg.type.name == "void")
            break;
        if (test(IDENTIFIER))
            arg.name = lexem();
        while (test(LBRACK)) {
            arg.rightType += lexemUntil(RBRACK);
        }
        if (test(CONST) || test(VOLATILE)) {
            arg.rightType += ' ';
            arg.rightType += lexem();
        }
        arg.normalizedType = normalizeType(QByteArray(arg.type.name + ' ' + arg.rightType));
        arg.typeNameForCast = normalizeType(QByteArray(noRef(arg.type.name) + "(*)" + arg.rightType));
        if (test(EQ))
            arg.isDefault = true;
        def->arguments += arg;
        if (!until(COMMA))
            break;
    }
}

bool Moc::parseMemberVariable(ArgumentDef *def) {
    // Skip when meet function identifiers
    while (test(EXPLICIT) || test(INLINE) || test(STATIC) || test(VIRTUAL) || test(TILDE) || skipCxxAttributes()) {
        return false;
    }
    ArgumentDef &arg = *def;
    arg.type = parseType();
    if (arg.type.name.isEmpty())
        return false;
    if (test(IDENTIFIER))
        arg.name = lexem();
    while (test(LBRACK)) {
        arg.rightType += lexemUntil(RBRACK);
    }
    if (test(CONST) || test(VOLATILE)) {
        arg.rightType += ' ';
        arg.rightType += lexem();
    }
    arg.normalizedType = normalizeType(QByteArray(arg.type.name + ' ' + arg.rightType));
    arg.typeNameForCast = normalizeType(QByteArray(noRef(arg.type.name) + "(*)" + arg.rightType));

    // int a = 1;
    if (test(EQ)) {
        return until(SEMIC);
    }

    // int a{};
    if (test(LBRACE)) {
        return until(RBRACE);
    }

    return test(SEMIC);
}

void Moc::parseDeclareType(QByteArray *name) {
    next(LPAREN);
    QByteArray typeName = lexemUntil(RPAREN);
    typeName.remove(0, 1);
    typeName.chop(1);
    *name = typeName;
}

bool Moc::skipCxxAttributes() {
    auto rewind = index;
    if (test(LBRACK) && test(LBRACK) && until(RBRACK) && test(RBRACK))
        return true;
    index = rewind;
    return false;
}

// like parseFunction, but never aborts with an error
bool Moc::parseMaybeFunction(const ClassDef *cdef, FunctionDef *def) {
    def->isVirtual = false;
    def->isStatic = false;
    // skip modifiers and attributes
    while (test(EXPLICIT) || test(INLINE) || (test(STATIC) && (def->isStatic = true) == true) ||
           (test(VIRTUAL) && (def->isVirtual = true) == true) // mark as virtual
           || skipCxxAttributes()) {
    }
    bool tilde = test(TILDE);
    def->type = parseType();
    if (def->type.name.isEmpty()) {
        return false;
    }
    bool scopedFunctionName = false;
    if (test(LPAREN)) {
        def->name = def->type.name;
        scopedFunctionName = def->type.isScoped;
        if (def->name == cdef->classname) {
            def->isDestructor = tilde;
            def->isConstructor = !tilde;
            def->type = Type();
        } else {
            def->type = Type("int");
        }
    } else {
        Type tempType = parseType();
        while (!tempType.name.isEmpty() && lookup() != LPAREN) {
            if (!def->tag.isEmpty())
                def->tag += ' ';
            def->tag += def->type.name;

            def->type = tempType;
            tempType = parseType();
        }
        if (!test(LPAREN))
            return false;
        def->name = tempType.name;
        scopedFunctionName = tempType.isScoped;
    }

    // we don't support references as return types, it's too dangerous
    if (def->type.referenceType == Type::Reference) {
        QByteArray rawName = def->type.rawName;
        def->type = Type("void");
        def->type.rawName = rawName;
    }

    def->normalizedType = normalizeType(def->type.name);

    if (!test(RPAREN)) {
        parseFunctionArguments(def);
        if (!test(RPAREN))
            return false;
    }
    def->isConst = test(CONST);

    if (def->isVirtual) {
        if (test(EQ)) {
            return until(SEMIC);
        }
    }

    return true;
}

// bool Moc::parseMaybeMemberVarOrSkip(MemberVariableDef *mdef) {
//     if (test(EXPLICIT) || test(INLINE) || test(STATIC) || test(VIRTUAL) // mark as virtual
//         || skipCxxAttributes() || test(TILDE)) {
//         return false;
//     }
//     ArgumentDef &arg = *mdef;
//     arg.type = parseType();
//     if (test(IDENTIFIER))
//         arg.name = lexem();
//     while (test(LBRACK)) {
//         arg.rightType += lexemUntil(RBRACK);
//     }
//     if (test(CONST) || test(VOLATILE)) {
//         arg.rightType += ' ';
//         arg.rightType += lexem();
//     }
//     arg.normalizedType = normalizeType(QByteArray(arg.type.name + ' ' + arg.rightType));
//     arg.typeNameForCast = normalizeType(QByteArray(noRef(arg.type.name) + "(*)" +
//     arg.rightType));
//
//     // void a();
//     if (test(LPAREN)) {
//         qDebug() << "Func" << arg.type.name << arg.name;
//         until(RPAREN);
//         if (test(LBRACE)) {
//             until(RBRACE);
//         }
//         return false;
//     }
//
//     // QString a = "1";
//     if (test(EQ)) {
//         qDebug() << "Inited" << arg.type.name << arg.name;
//         until(SEMIC);
//         return true;
//     }
//
//     // QString a{};
//     if (test(LBRACE)) {
//         qDebug() << "Default" << arg.type.name << arg.name;
//         until(RBRACE);
//         until(SEMIC);
//         return true;
//     }
//
//     return test(SEMIC);
// }

void Moc::parse() {
    parseEnv(&rootEnv);
}

void Moc::parseEnv(Environment *env) {
    auto access = env->access;
    bool templateClass = false;

    bool isNamespace = env->isNamespace;
    bool isClass = !isNamespace && !env->isRoot;

    while (inEnv(env) && hasNext()) {
        Token t = next();
        if (!isClass) {
            switch (t) {
                case NAMESPACE: {
                    if (test(IDENTIFIER)) {
                        QByteArray nsName = lexem();
                        QByteArrayList nested;
                        while (test(SCOPE)) {
                            next(IDENTIFIER);
                            nested.append(nsName);
                            nsName = lexem();
                        }
                        if (test(EQ)) {
                            // namespace Foo = Bar::Baz;
                            env->aliasNamespaces.insert(nsName, parseNamespace());
                        } else if (test(LPAREN)) {
                            // Ignore invalid code such as: 'namespace __identifier("x")'
                            // (QTBUG-56634)
                            until(RPAREN);
                        } else if (!test(SEMIC)) {
                            NamespaceDef def;
                            def.classname = nsName;
                            def.qualified = (QByteArrayList(nested) << nsName).join("::");

                            next(LBRACE);
                            def.begin = index - 1;
                            until(RBRACE);
                            def.end = index;
                            index = def.begin + 1;

                            // Find target child env(maybe env itself)
                            auto targetEnv = NameUtil::exploreEnv(env, nested, true);

                            auto it = targetEnv->children.find(nsName);
                            if (it == targetEnv->children.end()) {
                                auto newNamespace = new NamespaceDef(std::move(def));
                                auto newEnv = QSharedPointer<Environment>::create(newNamespace, targetEnv);
                                targetEnv->children.insert(nsName, newEnv);
                                parseEnv(newEnv.data());
                            } else {
                                auto newEnv = it.value();
                                if (!newEnv->isNamespace) {
                                    error(QByteArray(nsName + " has been defined as a class!").constData());
                                }
                                *newEnv->ns.data() = std::move(def);
                                parseEnv(newEnv.data());
                            }
                        }
                    }
                    goto end;
                }
                case MOC_INCLUDE_BEGIN:
                    currentFilenames.push(symbol().unquotedLexem());
                    goto end;
                case MOC_INCLUDE_END:
                    currentFilenames.pop();
                    goto end;
                case USING:
                    if (test(NAMESPACE)) {
                        auto ns = parseNamespace();
                        env->usedNamespaces.insert(ns);
                        // Ignore invalid code such as: 'using namespace __identifier("x")'
                        // (QTBUG-63772)
                        if (test(LPAREN))
                            until(RPAREN);
                        next(SEMIC);
                        goto end;
                    }
                    break;
                default:
                    break;
            }
        } else {
            switch (t) {
                case PRIVATE:
                    access = FunctionDef::Private;
                    goto end;
                case PROTECTED:
                    access = FunctionDef::Protected;
                    goto end;
                case PUBLIC:
                    access = FunctionDef::Public;
                    goto end;
                default:
                    break;
            }
        }

        // Common
        switch (t) {
            case QAS_JSON_TOKEN:
            case QAS_JSON_NS_TOKEN: {
                declareCount += currentFilenames.size() <= 1;

                next(LPAREN);
                Type type = parseType();
                if (type.name.isEmpty()) {
                    error();
                }
                next(RPAREN);
                env->classToGen.append(
                    {type.name, symbol().lineNum, currentFilenames.top(), currentFilenames.size() <= 1});
                goto end;
            }
            case USING: {
                bool tmp;
                QByteArray name;
                int rewind = index;
                if ((name = (tmp = test(IDENTIFIER)) ? lexem() : QByteArrayLiteral(""), tmp) && test(EQ) &&
                    !test(TYPENAME)) {
                    // using string = std::string;
                    QByteArray className = parseType().name;
                    if (className.isEmpty()) {
                        error();
                    }
                    env->aliasClasses.insert(name, className);
                } else {
                    // using std::string;
                    index = rewind;

                    if (!test(TYPENAME)) {
                        QByteArray className = parseType().name;
                        if (className.isEmpty()) {
                            error();
                        }
                        env->usedClasses.insert(QString(className).split("::").back().toUtf8(), className);
                    }
                }
                goto end;
            }
            case TYPEDEF: {
                auto type = parseType();
                if (test(IDENTIFIER)) {
                    env->aliasClasses.insert(lexem(), type.name);
                } else {
                    // Ignore function pointers
                }
                until(SEMIC);
                break;
            }
            case ENUM: {
                if (currentFilenames.size() <= 1) {
                    EnumDef enumDef;
                    if (parseEnum(&enumDef)) {
                        env->enums.insert(enumDef.name, enumDef);
                    }
                }
                break;
            }
            case LBRACE:
                until(RBRACE);
                templateClass = false;
                break;
            case SEMIC:
            case RBRACE:
                templateClass = false;
                break;
            case TEMPLATE:
                templateClass = true;
                break;
            case STRUCT:
            case CLASS: {
                ClassDef def;
                if (parseClassHead(&def)) {
                    auto name = def.classname;

                    auto names = NameUtil::splitScopes(def.qualified);
                    Environment *targetEnv = nullptr;

                    if (names.size() > 1) {
                        // Search forward declaration
                        NameUtil::considerPredefinedClass = true;
                        auto res = NameUtil::getScope(&rootEnv, env, def.qualified, true);
                        NameUtil::considerPredefinedClass = false;

                        if (res.env && res.type == NameUtil::FindResult::PredefinedClass) {
                            targetEnv = res.env;
                        }
                    } else {
                        targetEnv = env;
                    }

                    if (targetEnv) {
                        auto newClass = new ClassDef(std::move(def));
                        auto newEnv = QSharedPointer<Environment>::create(newClass, targetEnv);
                        newEnv->access = t == CLASS ? FunctionDef::Private : FunctionDef::Public;
                        newEnv->templateClass = templateClass;

                        targetEnv->children.insert(name, newEnv);

                        parseEnv(newEnv.data());
                    }

                } else {
                    env->predeclaredClasses.insert(def.classname);
                }
                break;
            }
            default:
                if (!isClass || !(currentFilenames.size() <= 1))
                    break;

                QByteArray attr;
                bool exclude = false;
                bool include = false;
                QByteArray annotation;
                QVector<ConstraintGroup> constraintGroups;
                if (currentFilenames.size() <= 1) {
                    auto tt = t;
                    while (true) {
                        switch (tt) {
                            case QAS_ATTR_TOKEN: {
                                annotation = lexem();
                                parseDeclareType(&attr);
                                attr.replace("\"", "");
                                tt = lookup();
                                next();
                                break;
                            }
                            case QAS_EXCLUDE_TOKEN:
                                annotation = lexem();
                                exclude = true;
                                include = false;
                                tt = lookup();
                                next();
                                break;
                            case QAS_INCLUDE_TOKEN:
                                annotation = lexem();
                                include = true;
                                exclude = false;
                                tt = lookup();
                                next();
                                break;
                            case QAS_CONSTRAINT_TOKEN:
                                annotation = lexem();
                                {
                                    ConstraintGroup constraintGroup;
                                    if (!parseConstraints(&constraintGroup)) {
                                        error(QByteArray("Failed to parse constraint expression in " + annotation).constData());
                                        return;
                                    }
                                    if (!constraintGroup.constraints.isEmpty()) {
                                        constraintGroups.append(constraintGroup);
                                    }
                                }
                                tt = lookup();
                                next();
                                break;
                            default:
                                goto end_qas_loop;
                        }
                    }
                    end_qas_loop:;
                }

                bool parseAnnotated = false;
                FunctionDef funcDef;
                funcDef.access = access;
                int rewind = index--;
                if (parseMaybeFunction(env->cl.data(), &funcDef)) {
                } else {
                    index = rewind - 1;
                    MemberVariableDef arg;
                    if (parseMemberVariable(&arg)) {
                        arg.access = access;
                        arg.attr = attr;
                        arg.exclude = exclude;
                        arg.include = include;
                        
                        arg.constraintGroups = constraintGroups;
                        
                        env->cl->memberVars += arg;
                        parseAnnotated = true;
                    } else {
                        index = rewind;
                    }
                }
                if (!annotation.isEmpty() && !parseAnnotated) {
                    error(QByteArray(annotation + " has no following member variable.").constData());
                }
                break;
        }
    end:;
    }
}

static QJsonObject encodeEnv(Environment *env) {
    QJsonObject envObj;

    // Type
    envObj.insert("type", env->isRoot ? "root" : (env->isNamespace ? "namespace" : "class"));

    // Enums
    QJsonArray enumArr;
    for (const auto &item : qAsConst(env->enums)) {
        enumArr.append(item.toJson());
    }
    envObj.insert("enums", enumArr);

    // Properties
    if (!env->isRoot) {
        QJsonObject classObj;
        if (!env->isNamespace) {
            classObj = env->cl->toJson();
        } else {
            ClassDef def;
            static_cast<BaseDef &>(def) = static_cast<BaseDef>(*env->ns.data());
            classObj = def.toJson();
        }
        for (auto it = classObj.begin(); it != classObj.end(); ++it) {
            envObj.insert(it.key(), it.value());
        }
    }
    {
        QJsonArray classNames;
        for (const auto &item : qAsConst(env->classToGen)) {
            classNames.append(QString::fromUtf8(item.token));
        }
        envObj.insert("declaredItems", classNames);
    }

    // Children
    QJsonArray childArr;
    for (const auto &child : qAsConst(env->children)) {
        childArr.append(encodeEnv(child.data()));
    }
    envObj.insert("children", childArr);

    // using
    {
        QJsonArray nsArr;
        for (auto it = env->usedNamespaces.begin(); it != env->usedNamespaces.end(); ++it) {
            nsArr.append(QString::fromUtf8(*it));
        }
        QJsonObject clObj;
        for (auto it = env->usedClasses.begin(); it != env->usedClasses.end(); ++it) {
            clObj.insert(it.key(), QString::fromUtf8(it.value()));
        }
        envObj.insert("using", QJsonObject({
                                   {"namespaces", nsArr},
                                   {"classes",    clObj}
        }));
    }

    // Alias
    {
        QJsonObject nsObj;
        for (auto it = env->aliasNamespaces.begin(); it != env->aliasNamespaces.end(); ++it) {
            nsObj.insert(it.key(), QString::fromUtf8(it.value()));
        }
        QJsonObject clObj;
        for (auto it = env->aliasClasses.begin(); it != env->aliasClasses.end(); ++it) {
            clObj.insert(it.key(), QString::fromUtf8(it.value()));
        }
        envObj.insert("alias", QJsonObject({
                                   {"namespaces", nsObj},
                                   {"classes",    clObj}
        }));
    }

    return envObj;
}

void Moc::generate(FILE *out, FILE *jsonOutput) {
    QByteArray fn = filename;
    int i = filename.length() - 1;
    while (i > 0 && filename.at(i - 1) != '/' && filename.at(i - 1) != '\\')
        --i; // skip path
    if (i >= 0)
        fn = filename.mid(i);
    fprintf(out,
            "/****************************************************************************\n"
            "** Auto serialization code from reading C++ file '%s'\n**\n",
            fn.constData());
    fprintf(out, "** Created by: The Qt Auto Serialization Compiler version %s (Qt %s)\n**\n", APP_VERSION,
            QT_VERSION_STR);
    fprintf(out, "** WARNING! All changes made in this file will be lost!\n"
                 "*****************************************************************************/\n\n");

    //    fprintf(out, "#include <memory>\n"); // For std::addressof
    if (!noInclude) {
        if (includePath.size() && !includePath.endsWith('/'))
            includePath += '/';
        for (auto inc : includeFiles) {
            if (inc.at(0) != '<' && inc.at(0) != '"') {
                if (includePath.size() && includePath != "./")
                    inc.prepend(includePath);
                inc = '\"' + inc + '\"';
            }
            fprintf(out, "#include %s\n", inc.constData());
        }
    }

    //    fprintf(out, "#include <QtCore/qbytearray.h>\n"); // For QByteArrayData
    //    fprintf(out, "#include <QtCore/qmetatype.h>\n");  // For QMetaType::Type
    //
    //    fprintf(out,
    //            "#if !defined(Q_MOC_OUTPUT_REVISION)\n"
    //            "#error \"The header file '%s' doesn't include <QObject>.\"\n",
    //            fn.constData());
    //    fprintf(out, "#elif Q_MOC_OUTPUT_REVISION != %d\n", mocOutputRevision);
    //    fprintf(out,
    //            "#error \"This file was generated using the moc from %s."
    //            " It\"\n#error \"cannot be used with the include files from"
    //            " this version of Qt.\"\n#error \"(The moc has changed too"
    //            " much.)\"\n",
    //            QT_VERSION_STR);
    //    fprintf(out, "#endif\n\n");

    fprintf(out, "\n\n");

    Generator generator(&rootEnv, out);
    generator.generateCode();

    if (jsonOutput) {
        QJsonObject mocData;
        mocData[QLatin1String("outputRevision")] = APP_VERSION;
        mocData[QLatin1String("inputFile")] = QLatin1String(fn.constData());

        mocData.insert("environment", encodeEnv(&rootEnv));

        QJsonDocument jsonDoc(mocData);
        fputs(jsonDoc.toJson().constData(), jsonOutput);
    }
}

QByteArray Moc::lexemUntil(Token target) {
    int from = index;
    until(target);
    QByteArray s;
    while (from <= index) {
        QByteArray n = symbols.at(from++ - 1).lexem();
        if (s.size() && n.size()) {
            char prev = s.at(s.size() - 1);
            char next = n.at(0);
            if ((is_ident_char(prev) && is_ident_char(next)) || (prev == '<' && next == ':') ||
                (prev == '>' && next == '>'))
                s += ' ';
        }
        s += n;
    }
    return s;
}

bool Moc::until(Token target) {
    int braceCount = 0;
    int brackCount = 0;
    int parenCount = 0;
    int angleCount = 0;
    if (index) {
        switch (symbols.at(index - 1).token) {
            case LBRACE:
                ++braceCount;
                break;
            case LBRACK:
                ++brackCount;
                break;
            case LPAREN:
                ++parenCount;
                break;
            case LANGLE:
                ++angleCount;
                break;
            default:
                break;
        }
    }

    // when searching commas within the default argument, we should take care of template depth
    // (anglecount)
    //  unfortunatelly, we do not have enough semantic information to know if '<' is the
    //  operator< or the beginning of a template type. so we just use heuristics.
    int possible = -1;

    while (index < symbols.size()) {
        Token t = symbols.at(index++).token;
        switch (t) {
            case LBRACE:
                ++braceCount;
                break;
            case RBRACE:
                --braceCount;
                break;
            case LBRACK:
                ++brackCount;
                break;
            case RBRACK:
                --brackCount;
                break;
            case LPAREN:
                ++parenCount;
                break;
            case RPAREN:
                --parenCount;
                break;
            case LANGLE:
                if (parenCount == 0 && braceCount == 0)
                    ++angleCount;
                break;
            case RANGLE:
                if (parenCount == 0 && braceCount == 0)
                    --angleCount;
                break;
            case GTGT:
                if (parenCount == 0 && braceCount == 0) {
                    angleCount -= 2;
                    t = RANGLE;
                }
                break;
            default:
                break;
        }
        if (t == target && braceCount <= 0 && brackCount <= 0 && parenCount <= 0 &&
            (target != RANGLE || angleCount <= 0)) {
            if (target != COMMA || angleCount <= 0)
                return true;
            possible = index;
        }

        if (target == COMMA && t == EQ && possible != -1) {
            index = possible;
            return true;
        }

        if (braceCount < 0 || brackCount < 0 || parenCount < 0 || (target == RANGLE && angleCount < 0)) {
            --index;
            break;
        }

        if (braceCount <= 0 && t == SEMIC) {
            // Abort on semicolon. Allow recovering bad template parsing (QTBUG-31218)
            break;
        }
    }

    if (target == COMMA && angleCount != 0 && possible != -1) {
        index = possible;
        return true;
    }

    return false;
}

QJsonObject ClassDef::toJson() const {
    QJsonObject cls;
    cls[QLatin1String("className")] = QString::fromUtf8(classname.constData());
    cls[QLatin1String("qualifiedClassName")] = QString::fromUtf8(qualified.constData());

    QJsonArray memberInfos;
    for (const auto &member : qAsConst(memberVars)) {
        QJsonObject obj;
        obj.insert("name", QString::fromUtf8(member.name));
        obj.insert("typeName", QString::fromUtf8(member.type.name));
        memberInfos.append(obj);
    }
    if (!memberInfos.isEmpty()) {
        cls.insert("memberInfos", memberInfos);
    }

    //    const auto appendFunctions = [&cls](const QString &type, const QVector<FunctionDef>
    //    &funcs) {
    //        QJsonArray jsonFuncs;
    //
    //        for (const FunctionDef &fdef : funcs)
    //            jsonFuncs.append(fdef.toJson());
    //
    //        if (!jsonFuncs.isEmpty())
    //            cls[type] = jsonFuncs;
    //    };
    //
    //    appendFunctions(QLatin1String("signals"), signalList);
    //    appendFunctions(QLatin1String("slots"), slotList);
    //    appendFunctions(QLatin1String("constructors"), constructorList);
    //    appendFunctions(QLatin1String("methods"), methodList);

    QJsonArray superClasses;

    for (const auto &super : qAsConst(superclassList)) {
        const auto name = super.first;
        const auto access = super.second.access;
        QJsonObject superCls;
        superCls[QLatin1String("name")] = QString::fromUtf8(name);
        FunctionDef::accessToJson(&superCls, access);
        superClasses.append(superCls);
    }

    if (!superClasses.isEmpty())
        cls[QLatin1String("superClasses")] = superClasses;

    //    QJsonArray ifaces;
    //    for (const QVector<Interface> &ifaceList : interfaceList) {
    //        QJsonArray jsonList;
    //        for (const Interface &iface : ifaceList) {
    //            QJsonObject ifaceJson;
    //            ifaceJson[QLatin1String("id")] = QString::fromUtf8(iface.interfaceId);
    //            ifaceJson[QLatin1String("className")] = QString::fromUtf8(iface.className);
    //            jsonList.append(ifaceJson);
    //        }
    //        ifaces.append(jsonList);
    //    }
    //    if (!ifaces.isEmpty())
    //        cls[QLatin1String("interfaces")] = ifaces;

    return cls;
}

QJsonObject FunctionDef::toJson() const {
    QJsonObject fdef;
    fdef[QLatin1String("name")] = QString::fromUtf8(name);
    if (!tag.isEmpty())
        fdef[QLatin1String("tag")] = QString::fromUtf8(tag);
    fdef[QLatin1String("returnType")] = QString::fromUtf8(normalizedType);

    QJsonArray args;
    for (const ArgumentDef &arg : arguments)
        args.append(arg.toJson());

    if (!args.isEmpty())
        fdef[QLatin1String("arguments")] = args;

    accessToJson(&fdef, access);

    return fdef;
}

void FunctionDef::accessToJson(QJsonObject *obj, FunctionDef::Access acs) {
    switch (acs) {
        case Private:
            (*obj)[QLatin1String("access")] = QLatin1String("private");
            break;
        case Public:
            (*obj)[QLatin1String("access")] = QLatin1String("public");
            break;
        case Protected:
            (*obj)[QLatin1String("access")] = QLatin1String("protected");
            break;
    }
}

QJsonObject ArgumentDef::toJson() const {
    QJsonObject arg;
    arg[QLatin1String("type")] = QString::fromUtf8(normalizedType);
    if (!name.isEmpty())
        arg[QLatin1String("name")] = QString::fromUtf8(name);
    return arg;
}

QJsonObject EnumDef::toJson() const {
    QJsonObject def;
    def[QLatin1String("name")] = QString::fromUtf8(name);
    if (!enumName.isEmpty())
        def[QLatin1String("alias")] = QString::fromUtf8(enumName);
    def[QLatin1String("isClass")] = isEnumClass;
    if (isEnumClass) {
        def.insert("typeName", QString::fromUtf8(enumType.name));
    }

    QJsonArray valueArr;
    for (const auto &val : qAsConst(values)) {
        QJsonObject obj;
        obj.insert("name", QString::fromUtf8(val.itemName));
        obj.insert("attr", QString::fromUtf8(val.attr));
        valueArr.append(obj);
    }
    if (!valueArr.isEmpty()) {
        def[QLatin1String("values")] = valueArr;
    }

    return def;
}

// Implementation of constraint parsing functions
bool Moc::parseConstraints(ConstraintGroup *group) {
    // Expect: __qas_constraint__(MINIMUM 10 MAXIMUM 20)
    // Current position should be after QAS_CONSTRAINT_TOKEN
    
    if (!test(LPAREN)) {
        error("Expected '(' after __qas_constraint__");
        return false;
    }
    
    // Parse constraint pairs within parentheses
    while (!test(RPAREN)) {
        if (lookup() == RPAREN) {
            break; // Empty constraint list is allowed
        }
        
        // Parse constraint type (e.g., MINIMUM, MAXIMUM, etc.)
        if (!test(IDENTIFIER)) {
            error("Expected constraint type identifier");
            return false;
        }
        
        QByteArray typeStr = lexem();
        ConstraintType type = parseConstraintType(typeStr);
        
        if (type == static_cast<ConstraintType>(-1)) {
            error(QByteArray("Unsupported constraint type: " + typeStr).constData());
            return false;
        }
        
        // Parse constraint value
        QJsonValue value;
        if (!parseConstraintValue(type, &value)) {
            error(QByteArray("Failed to parse constraint value for " + typeStr).constData());
            return false;
        }
        
        // Create constraint and add to group
        Constraint constraint;
        constraint.type = type;
        constraint.value = value;
        group->constraints.append(constraint);
    }
    
    return true;
}

ConstraintType Moc::parseConstraintType(const QByteArray &typeStr) {
    // Map string to constraint type enum
    if (typeStr == "MINIMUM") return ConstraintType::MINIMUM;
    if (typeStr == "MAXIMUM") return ConstraintType::MAXIMUM;
    if (typeStr == "EXCLUSIVE_MINIMUM") return ConstraintType::EXCLUSIVE_MINIMUM;
    if (typeStr == "EXCLUSIVE_MAXIMUM") return ConstraintType::EXCLUSIVE_MAXIMUM;
    if (typeStr == "CONST") return ConstraintType::CONST;
    if (typeStr == "ENUM") return ConstraintType::ENUM;
    if (typeStr == "MIN_LENGTH") return ConstraintType::MIN_LENGTH;
    if (typeStr == "MAX_LENGTH") return ConstraintType::MAX_LENGTH;
    if (typeStr == "PATTERN") return ConstraintType::PATTERN;
    
    return static_cast<ConstraintType>(-1); // Invalid type
}

bool Moc::parseConstraintValue(ConstraintType type, QJsonValue *value) {
    switch (type) {
        case ConstraintType::MINIMUM:
        case ConstraintType::MAXIMUM:
        case ConstraintType::EXCLUSIVE_MINIMUM:
        case ConstraintType::EXCLUSIVE_MAXIMUM:
        case ConstraintType::MIN_LENGTH:
        case ConstraintType::MAX_LENGTH: {
            // Expect numeric value
            Token t = lookup();
            if (t != INTEGER_LITERAL && t != FLOATING_LITERAL) {
                error("Expected numeric value for range constraint");
                return false;
            }
            
            next(); // consume the token
            QByteArray numberStr = lexem();
            bool ok;
            
            // Try to parse as integer first, then as double
            int intVal = numberStr.toInt(&ok);
            if (ok) {
                *value = QJsonValue(intVal);
            } else {
                double doubleVal = numberStr.toDouble(&ok);
                if (ok) {
                    *value = QJsonValue(doubleVal);
                } else {
                    error("Invalid numeric value");
                    return false;
                }
            }
            break;
        }
        
        case ConstraintType::CONST: {
            // Parse JSON value (string, number, boolean, null)
            return parseJsonValue(value);
        }
        
        case ConstraintType::ENUM: {
            // Parse JSON array
            if (!test(LBRACK)) {
                error("Expected '[' for ENUM constraint value");
                return false;
            }
            
            QJsonArray arrayValue;
            
            // Parse array elements
            while (lookup() != RBRACK) {
                QJsonValue itemValue;
                if (!parseJsonValue(&itemValue)) {
                    return false;
                }
                arrayValue.append(itemValue);
                
                // Check for comma or end of array
                if (lookup() == COMMA) {
                    next(); // consume comma
                } else if (lookup() != RBRACK) {
                    error("Expected ',' or ']' in ENUM array");
                    return false;
                }
            }
            
            // Consume the closing bracket
            if (!test(RBRACK)) {
                error("Expected ']' after ENUM array elements");
                return false;
            }
            
            *value = QJsonValue(arrayValue);
            break;
        }
        
        case ConstraintType::PATTERN: {
            // Parse string literal
            if (!test(STRING_LITERAL)) {
                error("Expected string literal for PATTERN constraint");
                return false;
            }
            
            QByteArray patternStr = lexem();
            // Remove quotes
            if (patternStr.length() >= 2 && patternStr.startsWith('"') && patternStr.endsWith('"')) {
                patternStr = patternStr.mid(1, patternStr.length() - 2);
            }
            
            *value = QJsonValue(QString::fromUtf8(patternStr));
            break;
        }
        
        default:
            error("Unsupported constraint type");
            return false;
    }
    
    return true;
}

bool Moc::parseJsonValue(QJsonValue *value) {
    // Parse various JSON value types
    Token t = lookup();
    bool isNegative = false;
    
    switch (t) {
        case STRING_LITERAL: {
            next(); // consume token
            QByteArray strValue = lexem();
            // Remove quotes
            if (strValue.length() >= 2 && strValue.startsWith('"') && strValue.endsWith('"')) {
                strValue = strValue.mid(1, strValue.length() - 2);
            }
            *value = QJsonValue(QString::fromUtf8(strValue));
            return true;
        }

        case MINUS:
            isNegative = true;
            next();
            t = lookup();
            if (t != INTEGER_LITERAL && t != FLOATING_LITERAL) {
                error("Expected numeric value after '-'");
            }
        
        case INTEGER_LITERAL:
        case FLOATING_LITERAL: {
            next(); // consume token
            QByteArray numberStr = lexem();
            bool ok;
            
            // Try integer first
            int intVal = numberStr.toInt(&ok);
            if (ok) {
                *value = QJsonValue(intVal * (isNegative ? -1 : 1));
            } else {
                double doubleVal = numberStr.toDouble(&ok);
                if (ok) {
                    *value = QJsonValue(doubleVal * (isNegative ? -1 : 1));
                } else {
                    error("Invalid numeric value");
                    return false;
                }
            }
            return true;
        }
        
        case BOOLEAN_LITERAL: {
            next(); // consume token
            QByteArray boolStr = lexem();
            if (boolStr == "true") {
                *value = QJsonValue(true);
            } else if (boolStr == "false") {
                *value = QJsonValue(false);
            } else {
                error("Invalid boolean value");
                return false;
            }
            return true;
        }
        
        case IDENTIFIER: {
            // Check for null
            QByteArray identStr = lexem();
            next(); // consume token
            if (identStr == "null") {
                *value = QJsonValue(QJsonValue::Null);
                return true;
            }
            // Fall through to error
        }
        
        default:
            error("Expected JSON value (string, number, boolean, or null)");
            return false;
    }
}

// Implementation of constraint-related methods
QByteArray Constraint::typeToString(ConstraintType type) {
    switch (type) {
        case ConstraintType::MINIMUM: return "MINIMUM";
        case ConstraintType::MAXIMUM: return "MAXIMUM";
        case ConstraintType::EXCLUSIVE_MINIMUM: return "EXCLUSIVE_MINIMUM";
        case ConstraintType::EXCLUSIVE_MAXIMUM: return "EXCLUSIVE_MAXIMUM";
        case ConstraintType::CONST: return "CONST";
        case ConstraintType::ENUM: return "ENUM";
        case ConstraintType::MIN_LENGTH: return "MIN_LENGTH";
        case ConstraintType::MAX_LENGTH: return "MAX_LENGTH";
        case ConstraintType::PATTERN: return "PATTERN";
        default: return "UNKNOWN";
    }
}

bool Constraint::validate(const QJsonValue& input, QString* errorMsg) const {
    // Implementation will be added in the runtime library
    // This is just a placeholder for compilation
    Q_UNUSED(input);
    Q_UNUSED(errorMsg);
    return true;
}

bool ConstraintGroup::validate(const QJsonValue& input, QString* errorMsg) const {
    // All constraints in the group must be satisfied (AND relationship)
    for (const auto& constraint : constraints) {
        if (!constraint.validate(input, errorMsg)) {
            return false;
        }
    }
    return true;
}

bool JsonAttributes::validateConstraints(const QJsonValue& input, QString* errorMsg) const {
    // At least one constraint group must be satisfied (OR relationship)
    if (constraintGroups.isEmpty()) {
        return true; // No constraints means validation passes
    }
    
    for (const auto& group : constraintGroups) {
        if (group.validate(input, errorMsg)) {
            return true; // Found a satisfied group
        }
    }
    
    // No group was satisfied
    if (errorMsg) {
        *errorMsg = "Value violates all constraint groups";
    }
    return false;
}

QT_END_NAMESPACE
