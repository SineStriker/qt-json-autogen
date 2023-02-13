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

struct QMetaObject;

struct Type {
    enum ReferenceType { NoReference, Reference, RValueReference, Pointer };

    inline Type()
        : isVolatile(false), isScoped(false), firstToken(NOTOKEN), referenceType(NoReference) {
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
struct EnumDef {
    QByteArray name;
    QByteArray enumName;
    QVector<bool> ignoreList;
    QVector<QByteArray> attrList;
    QVector<QByteArray> values;
    bool isEnumClass; // c++11 enum class
    Type enumType;
    EnumDef() : isEnumClass(false) {
    }
    QJsonObject toJson(const ClassDef &cdef) const;
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


struct ClassInfoDef {
    QByteArray name;
    QByteArray value;
};
Q_DECLARE_TYPEINFO(ClassInfoDef, Q_MOVABLE_TYPE);

struct BaseDef {
    QByteArray classname;
    QByteArray qualified;
    QMap<QByteArray, bool> enumDeclarations;
    QVector<EnumDef> enumList;
    int begin = 0;
    int end = 0;
};

struct ClassDef : BaseDef {
    QVector<QPair<QByteArray, FunctionDef::Access>> superclassList;

    QVector<ArgumentDef> memberVars;
    QVector<QByteArray> memberVarAttrs;

    struct Interface {
        Interface() {
        } // for QVector, don't use
        inline explicit Interface(const QByteArray &_className) : className(_className) {
        }
        QByteArray className;
        QByteArray interfaceId;
    };
    QVector<QVector<Interface>> interfaceList;

    QVector<FunctionDef> constructorList;
    QVector<FunctionDef> signalList, slotList, methodList, publicList;

    QJsonObject toJson() const;
};
Q_DECLARE_TYPEINFO(ClassDef, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(ClassDef::Interface, Q_MOVABLE_TYPE);

struct NamespaceDef : BaseDef {
};
Q_DECLARE_TYPEINFO(NamespaceDef, Q_MOVABLE_TYPE);

struct EnvironmentDef : BaseDef {
    bool isNamespace;
};

class Moc : public Parser {
public:
    Moc() : noInclude(false), mustIncludeQPluginH(false) {
    }

    QByteArray filename;

    BaseDef enumDeclarations;
    BaseDef jsonDeclarations;
    QVector<EnumDef> globalEnums;

    bool noInclude;
    bool mustIncludeQPluginH;
    QByteArray includePath;
    QVector<QByteArray> includeFiles;
    QVector<ClassDef> classList;

    QList<EnvironmentDef> environments;

    void parse();
    void generate(FILE *out, FILE *jsonOutput);

    bool parseClassHead(ClassDef *def);
    inline bool inClass(const ClassDef *def) const {
        return index > def->begin && index < def->end - 1;
    }

    inline bool inNamespace(const NamespaceDef *def) const {
        return index > def->begin && index < def->end - 1;
    }

    Type parseType();

    bool parseEnum(EnumDef *def);
    bool parseFunction(FunctionDef *def, bool inMacro = false);
    bool parseMaybeFunction(const ClassDef *cdef, FunctionDef *def);

    void parseEnumOrFlag(BaseDef *def, bool isFlag);

    void parseFunctionArguments(FunctionDef *def);
    bool parseMemberVariable(ArgumentDef *def);

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
