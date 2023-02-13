#include "generator.h"

void Generator::generateCode() {
    std::list<QPair<QByteArrayList, Environment *>> stack;
    stack.push_back(qMakePair(QByteArrayList(), rootEnv));

    while (!stack.empty()) {
        auto pair = stack.front();
        stack.pop_front();

        const auto &names = pair.first;
        const auto &env = pair.second;

        // Handle current enumerations
        for (const auto &item : qAsConst(env->enums)) {
            QByteArray qualified = (QByteArrayList(names) << item.name).join("::");
            if (rootEnv->enumToGen.contains(qualified)) {
                generateEnums(qualified, item);
            }
        }

        // Handle current classes
        if (!env->cl.isNull()) {
            const auto &cl = *env->cl;
            QByteArray qualified = names.join("::");
            if (rootEnv->classToGen.contains(qualified)) {
                generateClass(qualified, cl);
            }
        }

        // Push all children
        for (const auto &child : qAsConst(env->children)) {
            QByteArrayList newNames =
                QByteArrayList(names)
                << (child->isNamespace ? child->ns->classname : child->cl->classname);
            stack.push_back(qMakePair(newNames, child.data()));
        }
    }
}

void Generator::generateEnums(const QByteArray &qualified, const EnumDef &def) {
    const char *fmt;
    const char *type_str = qualified.data();

    // Generate deserializer
    // Declaration head
    fmt = "%s QASEnumType<%s>::fromString(const QString &s, bool *ok) {\n";
    fprintf(fp, fmt, type_str, type_str);

    // Define res
    fmt = "    %s res{};\n";
    fprintf(fp, fmt, type_str);
    fprintf(fp, "    bool ok2 = true;\n");

    // Start branches
    fprintf(fp, "    ");
    for (const auto &item : def.values) {
        if (item.ignore) {
            continue;
        }
        QByteArray attr = item.attr.isEmpty() ? item.name : item.attr;
        fmt = "if (s == \"%s\") {\n        res = %s::%s;\n    } else ";
        fprintf(fp, fmt, attr.data(), type_str, item.name.data());
    }

    // Last and end
    fprintf(fp,
            " {\n        ok2 = false;\n    }\n    ok ? (*ok = ok2) : false;\n    return res;\n}\n");

    fprintf(fp, "\n");

    // Generate serializer
    // Declaration head
    fmt = "QString QASEnumType<%s>::toString(%s e) {\n";
    fprintf(fp, fmt, type_str, type_str);

    // Define res
    fprintf(fp, "    QString res;\n"
                "    switch (e) {\n");

    // Start switch
    for (const auto &item : def.values) {
        if (item.ignore) {
            continue;
        }
        QByteArray attr = item.attr.isEmpty() ? item.name : item.attr;
        fmt = "        case %s::%s:\n"
              "            res = \"%s\";\n"
              "            break;\n";
        fprintf(fp, fmt, type_str, item.name.data(), attr.data());
    }

    // Last and end
    fprintf(fp, "        default:\n"
                "            break;\n"
                "    }\n"
                "    return res;\n"
                "}\n");

    fprintf(fp, "\n");
}

void Generator::generateClass(const QByteArray &qualified, const ClassDef &def) {
    const char *fmt;
    const char *type_str = qualified.data();

    // Generate deserializer
    // Declaration head
    fmt = "%s QASJsonType<%s>::fromObject(const QJsonObject &obj, bool *ok) {\n";
    fprintf(fp, fmt, type_str, type_str);

    // Define res
    fmt = "    %s res;\n";
    fprintf(fp, fmt, type_str);
    fprintf(fp, "    QJsonObject::ConstIterator it;\n"
                "    bool ok2 = true;\n");

    // Start branches
    for (const auto &item : def.memberVars) {
        if (item.ignore || item.access != FunctionDef::Public) {
            continue;
        }
        QByteArray attr = item.attr.isEmpty() ? item.name : item.attr;
        fmt = "    it = obj.find(\"%s\");\n    if (it != obj.end()) {\n"
              "        res.%s = QASJsonType<decltype(res.%s)>::fromValue(it.value(), &ok2);\n"
              "        if (!ok2) {\n"
              "            goto over;\n"
              "        }\n"
              "    }\n";
        const char *name_str = item.name.data();
        fprintf(fp, fmt, attr.data(), name_str, name_str);
    }

    // Last and end
    fprintf(fp, "over:\n"
                "    ok ? (*ok = ok2) : false;\n"
                "    return res;\n"
                "}");

    fprintf(fp, "\n");

    // Generate serializer
    // Declaration head
    fmt = "QJsonObject QASJsonType<%s>::toObject(const %s &cls) {\n";
    fprintf(fp, fmt, type_str, type_str);

    // Define res
    fprintf(fp, "    QJsonObject res;\n");

    // Start switch
    for (const auto &item : def.memberVars) {
        if (item.ignore || item.access != FunctionDef::Public) {
            continue;
        }
        QByteArray attr = item.attr.isEmpty() ? item.name : item.attr;
        fmt = "    res.insert(\"%s\", QASJsonType<decltype(cls.%s)>::toValue(cls.%s));\n";
        const char *name_str = item.name.data();
        fprintf(fp, fmt, attr.data(), name_str, name_str);
    }

    // Last and end
    fprintf(fp, "    return res;\n"
                "}\n");

    fprintf(fp, "\n");
}
