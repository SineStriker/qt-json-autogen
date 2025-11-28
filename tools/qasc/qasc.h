/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#ifndef QASC_H
#define QASC_H

#include "parser.h"

#include <ctype.h>
#include <qjsonarray.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qmap.h>
#include <qpair.h>
#include <qstringlist.h>
#include <stdio.h>

QT_BEGIN_NAMESPACE

struct Type {
    enum ReferenceType { NoReference, Reference, RValueReference, Pointer };

    inline Type() : isVolatile(false), isScoped(false), firstToken(NOTOKEN), referenceType(NoReference) {
    }
    inline explicit Type(const QByteArray &_name)
        : name(_name), rawName(name), isVolatile(false), isScoped(false), firstToken(NOTOKEN),
          referenceType(NoReference) {
    }
    QByteArray name;
    // When used as a return type, the type name may be modified to remove the references.
    //  rawName is the type as found in the function signature
    QByteArray rawName;
    uint isVolatile : 1;
    uint isScoped : 1;
    Token firstToken;
    ReferenceType referenceType;
};
Q_DECLARE_TYPEINFO(Type, Q_MOVABLE_TYPE);

struct ClassDef;

struct JsonAttributes;

struct EnumDef {
    QByteArray name;
    QByteArray enumName;

    QVector<JsonAttributes> values;

    bool isEnumClass; // c++11 enum class
    Type enumType;
    EnumDef() : isEnumClass(false) {
    }
    QJsonObject toJson() const;
};
Q_DECLARE_TYPEINFO(EnumDef, Q_MOVABLE_TYPE);

struct ArgumentDef {
    ArgumentDef() : isDefault(false) {
    }
    Type type;
    QByteArray rightType, normalizedType, name;
    QByteArray typeNameForCast; // type name to be used in cast from void * in metacall
    bool isDefault;

    QJsonObject toJson() const;
};
Q_DECLARE_TYPEINFO(ArgumentDef, Q_MOVABLE_TYPE);

struct FunctionDef {
    Type type;
    QVector<ArgumentDef> arguments;
    QByteArray normalizedType;
    QByteArray tag;
    QByteArray name;
    QByteArray inPrivateClass;

    enum Access { Private, Protected, Public };
    Access access = Private;

    bool isConst = false;
    bool isVirtual = false;
    bool isStatic = false;
    bool inlineCode = false;
    bool wasCloned = false;

    bool returnTypeIsVolatile = false;

    bool isCompat = false;
    bool isConstructor = false;
    bool isDestructor = false;
    bool isAbstract = false;

    QJsonObject toJson() const;
    static void accessToJson(QJsonObject *obj, Access acs);
};
Q_DECLARE_TYPEINFO(FunctionDef, Q_MOVABLE_TYPE);

// Constraint types enumeration for JSON validation
enum class ConstraintType {
    MINIMUM,                // x >= value
    MAXIMUM,               // x <= value  
    EXCLUSIVE_MINIMUM,     // x > value
    EXCLUSIVE_MAXIMUM,     // x < value
    CONST,                 // x === value
    ENUM,                  // x in [value1, value2, ...]
    MIN_LENGTH,            // x.length >= value
    MAX_LENGTH,            // x.length <= value
    PATTERN                // x matches regex pattern
};

// Single constraint definition
struct Constraint {
    ConstraintType type;
    QJsonValue value;      // Constraint value stored as JSON for type flexibility
    
    // Convert constraint type to string for error reporting
    static QByteArray typeToString(ConstraintType type);
    
    // Validate if the given value satisfies this constraint
    bool validate(const QJsonValue& input, QString* errorMsg = nullptr) const;
};
Q_DECLARE_TYPEINFO(Constraint, Q_MOVABLE_TYPE);

// Group of constraints (AND relationship within group)
struct ConstraintGroup {
    QVector<Constraint> constraints;
    
    // Validate if input satisfies ALL constraints in this group
    bool validate(const QJsonValue& input, QString* errorMsg = nullptr) const;
};
Q_DECLARE_TYPEINFO(ConstraintGroup, Q_MOVABLE_TYPE);

struct JsonAttributes {
    FunctionDef::Access access = FunctionDef::Access::Public;
    int lineNum = 0;
    QByteArray filename;

    QByteArray itemName;
    QByteArray attr;
    bool exclude = false;
    bool include = false;
    
    // Constraint groups (OR relationship between groups)
    QVector<ConstraintGroup> constraintGroups;
    
    // Check if any constraint group is satisfied
    bool validateConstraints(const QJsonValue& input, QString* errorMsg = nullptr) const;
};
struct MemberVariableDef : ArgumentDef, JsonAttributes {
    FunctionDef::Access access = FunctionDef::Public;
    MemberVariableDef() {
    }
};

struct ClassInfoDef {
    QByteArray name;
    QByteArray value;
};
Q_DECLARE_TYPEINFO(ClassInfoDef, Q_MOVABLE_TYPE);

struct BaseDef {
    QByteArray classname;
    QByteArray qualified;
    QVector<EnumDef> enumList;
    int begin = 0;
    int end = 0;
};

struct ClassDef : BaseDef {
    QVector<QPair<QByteArray, JsonAttributes>> superclassList;

    QVector<MemberVariableDef> memberVars;

    QJsonObject toJson() const;
};
Q_DECLARE_TYPEINFO(ClassDef, Q_MOVABLE_TYPE);

struct NamespaceDef : BaseDef {};

Q_DECLARE_TYPEINFO(NamespaceDef, Q_MOVABLE_TYPE);

// struct TemplateDef : BaseDef {
//     QByteArrayList typeNames;
// };

struct DeclareItem {
    QByteArray token;
    int lineNum;
    QByteArray filename;
    bool gen;
};

struct Environment {
    bool isRoot;
    bool isNamespace;
    FunctionDef::Access access = FunctionDef::Public;
    bool templateClass = false;

    QSharedPointer<NamespaceDef> ns;
    QSharedPointer<ClassDef> cl;

    inline QByteArray name() const {
        return ns.isNull() ? (cl.isNull() ? QByteArrayLiteral("@root") : cl->classname) : ns->classname;
    }

    QSet<QByteArray> usedNamespaces;
    QHash<QByteArray, QByteArray> usedClasses;
    QHash<QByteArray, QByteArray> aliasNamespaces;
    QHash<QByteArray, QByteArray> aliasClasses;

    Environment *parent;
    QHash<QByteArray, QSharedPointer<Environment>> children;
    QHash<QByteArray, EnumDef> enums;
    QSet<QByteArray> predeclaredClasses;

    QList<DeclareItem> classToGen;

    Environment() : isRoot(true), isNamespace(false), parent(nullptr) {
    }

    Environment(NamespaceDef *ns, Environment *parent) : isRoot(false), isNamespace(true), ns(ns), parent(parent) {
    }

    Environment(ClassDef *cl, Environment *parent) : isRoot(false), isNamespace(false), cl(cl), parent(parent) {
    }
};

class Moc : public Parser {
public:
    Moc() : noInclude(false), declareCount(0) {
    }

    QByteArray filename;

    bool noInclude;
    QByteArray includePath;
    QVector<QByteArray> includeFiles;

    Environment rootEnv;
    int declareCount;

    void parse();
    void parseEnv(Environment *env);

    void generate(FILE *out, FILE *jsonOutput);

    bool parseClassHead(ClassDef *def);
    inline bool inClass(const ClassDef *def) const {
        return index > def->begin && index < def->end - 1;
    }

    inline bool inNamespace(const NamespaceDef *def) const {
        return index > def->begin && index < def->end - 1;
    }

    inline bool inScope(const BaseDef *def) const {
        return index > def->begin && index < def->end - 1;
    }

    inline bool inEnv(const Environment *env) const {
        return env->isRoot || (env->isNamespace ? inNamespace(env->ns.data()) : inClass(env->cl.data()));
    }

    Type parseType();
    QByteArray parseNamespace();
    // TemplateDef parseTemplate();

    bool parseEnum(EnumDef *def);
    bool parseMaybeFunction(const ClassDef *cdef, FunctionDef *def);

    //    bool parseMaybeMemberVarOrSkip(MemberVariableDef *mdef);

    void parseFunctionArguments(FunctionDef *def);
    bool parseMemberVariable(ArgumentDef *def);
    void parseDeclareType(QByteArray *name);
    
    // Parse constraint expressions from __qas_constraint__ annotation
    bool parseConstraints(ConstraintGroup *group);
    ConstraintType parseConstraintType(const QByteArray &typeStr);
    bool parseConstraintValue(ConstraintType type, QJsonValue *value);
    bool parseJsonValue(QJsonValue *value);

    QByteArray lexemUntil(Token);
    bool until(Token);

    bool skipCxxAttributes();
};

inline QByteArray noRef(const QByteArray &type) {
    if (type.endsWith('&')) {
        if (type.endsWith("&&"))
            return type.left(type.length() - 2);
        return type.left(type.length() - 1);
    }
    return type;
}

QT_END_NAMESPACE


#endif // QASC_H
