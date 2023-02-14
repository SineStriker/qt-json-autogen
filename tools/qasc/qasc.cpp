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
    QByteArray result = normalizeTypeInternal(buf, d, fixScope);
    if (buf != stackbuf)
        delete[] buf;
    return result;
}

bool Moc::parseClassHead(ClassDef *def) {
    // figure out whether this is a class declaration, or only a
    // forward or variable declaration.
    int i = 0;
    Token token;
    do {
        token = lookup(i++);
        if (token == COLON || token == LBRACE)
            break;
        if (token == SEMIC || token == RANGLE)
            return false;
    } while (token);

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
            const QByteArray type = parseType().name;
            // ignore the 'class Foo : BAR(Baz)' case
            if (test(LPAREN)) {
                until(RPAREN);
            } else {
                def->superclassList += qMakePair(type, access);
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
                ;
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
    while (test(CONST) || test(VOLATILE) || test(SIGNED) || test(UNSIGNED) || test(STAR) ||
           test(AND) || test(ANDAND)) {
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
    if (test(COLON)) {               // C++11 strongly typed enum
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
            bool ignore = false;
            QByteArray attr;

            auto t = next();
            switch (t) {
                case QAS_ATTRIBUTE_TOKEN: {
                    parseDeclareType(&attr);
                    attr.replace("\"", "");
                    next(IDENTIFIER);
                    break;
                }

                case QAS_IGNORE_TOKEN:
                    ignore = true;
                    next(IDENTIFIER);
                    break;

                case IDENTIFIER:
                    break;

                default:
                    error();
                    break;
            }

            def->values += {lexem(), attr, ignore};
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
        arg.typeNameForCast =
            normalizeType(QByteArray(noRef(arg.type.name) + "(*)" + arg.rightType));
        if (test(EQ))
            arg.isDefault = true;
        def->arguments += arg;
        if (!until(COMMA))
            break;
    }
}

bool Moc::parseMemberVariable(ArgumentDef *def) {
    ArgumentDef &arg = *def;
    arg.type = parseType();
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

// returns false if the function should be ignored
bool Moc::parseFunction(FunctionDef *def, bool inMacro) {
    def->isVirtual = false;
    def->isStatic = false;
    // skip modifiers and attributes
    while (test(INLINE) || (test(STATIC) && (def->isStatic = true) == true) ||
           (test(VIRTUAL) && (def->isVirtual = true) == true) // mark as virtual
           || skipCxxAttributes()) {
    }
    bool templateFunction = (lookup() == TEMPLATE);
    def->type = parseType();
    if (def->type.name.isEmpty()) {
        if (templateFunction)
            error("Template function as signal or slot");
        else
            error();
    }
    bool scopedFunctionName = false;
    if (test(LPAREN)) {
        def->name = def->type.name;
        scopedFunctionName = def->type.isScoped;
        def->type = Type("int");
    } else {
        Type tempType = parseType();
        ;
        while (!tempType.name.isEmpty() && lookup() != LPAREN) {
            if (def->type.firstToken == Q_SIGNALS_TOKEN)
                error();
            else if (def->type.firstToken == Q_SLOTS_TOKEN)
                error();
            else {
                if (!def->tag.isEmpty())
                    def->tag += ' ';
                def->tag += def->type.name;
            }
            def->type = tempType;
            tempType = parseType();
        }
        next(LPAREN, "Not a signal or slot declaration");
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
        next(RPAREN);
    }

    // support optional macros with compiler specific options
    while (test(IDENTIFIER))
        ;

    def->isConst = test(CONST);

    while (test(IDENTIFIER))
        ;

    if (inMacro) {
        next(RPAREN);
        prev();
    } else {
        if (test(THROW)) {
            next(LPAREN);
            until(RPAREN);
        }
        if (test(SEMIC))
            ;
        else if ((def->inlineCode = test(LBRACE)))
            until(RBRACE);
        else if ((def->isAbstract = test(EQ)))
            until(SEMIC);
        else if (skipCxxAttributes())
            until(SEMIC);
        else
            error();
    }
    if (scopedFunctionName) {
        const QByteArray msg = "Function declaration " + def->name +
                               " contains extra qualification. Ignoring as signal or slot.";
        warning(msg.constData());
        return false;
    }
    return true;
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
    return true;
}

void Moc::parse() {
    parseEnv(&rootEnv);
}

void Moc::parseEnv(Environment *env) {
    auto access = env->access;
    while (inEnv(env) && hasNext()) {
        Token t = next();
        if (env->isNamespace || env->isRoot) {
            switch (t) {
                case NAMESPACE: {
                    if (test(IDENTIFIER)) {
                        QByteArray nsName = lexem();
                        QByteArrayList nested;
                        nested.append(nsName);
                        while (test(SCOPE)) {
                            next(IDENTIFIER);
                            nested.append(nsName);
                            nsName = lexem();
                        }
                        if (test(EQ)) {
                            // namespace Foo = Bar::Baz;
                            until(SEMIC);
                        } else if (test(LPAREN)) {
                            // Ignore invalid code such as: 'namespace __identifier("x")'
                            // (QTBUG-56634)
                            until(RPAREN);
                        } else if (!test(SEMIC)) {
                            NamespaceDef def;
                            def.classname = nsName;
                            def.qualified = nested.join("::");

                            next(LBRACE);
                            def.begin = index - 1;
                            until(RBRACE);
                            def.end = index;
                            index = def.begin + 1;

                            auto newNamespace = new NamespaceDef(std::move(def));
                            auto newEnv = QSharedPointer<Environment>::create(newNamespace, env);
                            parseEnv(newEnv.data());
                            env->children.append(newEnv);
                        }
                    }
                    break;
                }
                case SEMIC:
                case RBRACE:
                case TEMPLATE:
                case Q_DECLARE_INTERFACE_TOKEN:
                case Q_DECLARE_METATYPE_TOKEN:
                    break;
                case MOC_INCLUDE_BEGIN:
                    currentFilenames.push(symbol().unquotedLexem());
                    break;
                case MOC_INCLUDE_END:
                    currentFilenames.pop();
                    break;
                case USING:
                    if (test(NAMESPACE)) {
                        while (test(SCOPE) || test(IDENTIFIER))
                            ;
                        // Ignore invalid code such as: 'using namespace __identifier("x")'
                        // (QTBUG-63772)
                        if (test(LPAREN))
                            until(RPAREN);
                        next(SEMIC);
                    }
                    //  else if (test(IDENTIFIER)) {
                    //     next(EQ);
                    // }
                    break;
                case ENUM: {
                    EnumDef enumDef;
                    if (parseEnum(&enumDef)) {
                        env->enums += enumDef;
                    }
                    break;
                }
                case QAS_ENUM_DECLARE_TOKEN: {
                    if (env->isRoot) {
                        QByteArray typeName;
                        parseDeclareType(&typeName);
                        env->enumToGen.append(typeName);
                    }
                    break;
                }
                case QAS_JSON_DECLARE_TOKEN: {
                    if (env->isRoot) {
                        QByteArray typeName;
                        parseDeclareType(&typeName);
                        QByteArrayList types = typeName.split(',');
                        for (auto &type : types) {
                            type = type.trimmed();
                        }
                        env->classToGen.append(qMakePair(types.front(), types.mid(1)));
                    }
                    break;
                }
                case CLASS:
                case STRUCT: {
                    ClassDef def;
                    if (parseClassHead(&def)) {
                        if (currentFilenames.size() <= 1) {
                            auto newClass = new ClassDef(std::move(def));
                            auto newEnv = QSharedPointer<Environment>::create(newClass, env);
                            env->access = t == CLASS ? FunctionDef::Private : FunctionDef::Public;

                            parseEnv(newEnv.data());
                            env->children.append(newEnv);
                        } else {
                            while (inClass(&def) && hasNext()) {
                                next();
                            }
                        }
                        break;
                    }
                }
                default:
                    break;
            }
        } else {
            switch (t) {
                case PRIVATE:
                    access = FunctionDef::Private;
                    break;
                case PROTECTED:
                    access = FunctionDef::Protected;
                    break;
                case PUBLIC:
                    access = FunctionDef::Public;
                    break;
                case STRUCT:
                case CLASS: {
                    ClassDef def;
                    if (parseClassHead(&def)) {
                        if (currentFilenames.size() <= 1) {
                            auto newClass = new ClassDef(std::move(def));
                            auto newEnv = QSharedPointer<Environment>::create(newClass, env);
                            env->access = t == CLASS ? FunctionDef::Private : FunctionDef::Public;

                            parseEnv(newEnv.data());
                            env->children.append(newEnv);
                        } else {
                            while (inClass(&def) && hasNext()) {
                                next();
                            }
                        }
                        break;
                    }
                    break;
                }
                case ENUM: {
                    EnumDef enumDef;
                    if (parseEnum(&enumDef)) {
                        env->enums += enumDef;
                    }
                    break;
                }
                case Q_SIGNALS_TOKEN:
                case Q_SLOTS_TOKEN:
                case Q_OBJECT_TOKEN:
                case Q_GADGET_TOKEN:
                case Q_PROPERTY_TOKEN:
                case Q_PLUGIN_METADATA_TOKEN:
                case Q_ENUMS_TOKEN:
                case Q_ENUM_TOKEN:
                case Q_ENUM_NS_TOKEN:
                case Q_FLAGS_TOKEN:
                case Q_FLAG_TOKEN:
                case Q_FLAG_NS_TOKEN:
                case Q_DECLARE_FLAGS_TOKEN:
                case Q_CLASSINFO_TOKEN:
                case Q_INTERFACES_TOKEN:
                case Q_PRIVATE_SLOT_TOKEN:
                case Q_PRIVATE_PROPERTY_TOKEN:
                case SEMIC:
                case COLON:
                    break;
                case RBRACE:
                    break;
                default:
                    bool ignore = false;
                    QByteArray attr;
                    switch (t) {
                        case QAS_ATTRIBUTE_TOKEN: {
                            parseDeclareType(&attr);
                            if (!hasNext()) {
                                error("QAS_ATTRIBUTE has no following member.");
                            }
                            attr.replace("\"", "");
                            next();
                            break;
                        }
                        case QAS_IGNORE_TOKEN:
                            ignore = true;
                            if (!hasNext()) {
                                error("QAS_IGNORE has no following member.");
                            }
                            next();
                            break;

                        default:
                            break;
                    }

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
                            arg.ignore = ignore;
                            env->cl->memberVars += arg;
                        } else {
                            index = rewind;
                        }
                    }
            }
        }
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
    } else {
        QJsonArray enumNames;
        for (const auto &item : qAsConst(env->enumToGen)) {
            enumNames.append(QString::fromUtf8(item));
        }
        envObj.insert("declaredEnums", enumNames);

        QJsonArray classNames;
        for (const auto &item : qAsConst(env->classToGen)) {
            classNames.append(QString::fromUtf8(item.first));
        }
        envObj.insert("declaredClasses", classNames);
    }

    // Children
    QJsonArray childArr;
    for (const auto &child : qAsConst(env->children)) {
        childArr.append(encodeEnv(child.data()));
    }
    envObj.insert("children", childArr);

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
    fprintf(out, "** Created by: The Qt Auto Serialization Compiler version %s (Qt %s)\n**\n",
            APP_VERSION, QT_VERSION_STR);
    fprintf(out,
            "** WARNING! All changes made in this file will be lost!\n"
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

    Generator generator(&rootEnv, debugMode, out);
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

        if (braceCount < 0 || brackCount < 0 || parenCount < 0 ||
            (target == RANGLE && angleCount < 0)) {
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
        obj.insert("attr", QString::fromUtf8(member.attr));
        obj.insert("ignore", member.ignore);
        obj.insert("access", member.access);
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
        const auto access = super.second;
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
        obj.insert("name", QString::fromUtf8(val.name));
        obj.insert("attr", QString::fromUtf8(val.attr));
        obj.insert("ignore", val.ignore);
        valueArr.append(obj);
    }
    if (!valueArr.isEmpty()) {
        def[QLatin1String("values")] = valueArr;
    }

    return def;
}

QT_END_NAMESPACE
