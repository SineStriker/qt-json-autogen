#include "generator.h"

static QByteArray removeTemplateSpec(const QByteArray &name) {
    QByteArray nonTemplate = name;
    int cnt = 0;
    if (nonTemplate.endsWith('>')) {
        int i = nonTemplate.size() - 1;
        while (i >= 0) {
            switch (nonTemplate.at(i)) {
                case '<':
                    cnt--;
                    break;
                case '>':
                    cnt++;
                    break;
                default:
                    break;
            }
            if (cnt == 0) {
                break;
            }
            i--;
        }
        if (i > 0) {
            nonTemplate = nonTemplate.left(i);
        }
    }
    return nonTemplate;
}

void Generator::generateCode() {
    std::list<QPair<QByteArrayList, Environment *>> stack;
    stack.push_back(qMakePair(QByteArrayList(), rootEnv));

    QHash<QByteArray, QByteArray> classToGen2;
    for (const auto &name : qAsConst(rootEnv->classToGen)) {
        auto nameWithoutSpec = removeTemplateSpec(name);
        if (nameWithoutSpec.isEmpty()) {
            continue;
        }
        classToGen2.insert(nameWithoutSpec, name);
    }

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
            auto it = classToGen2.find(qualified);
            if (it != classToGen2.end()) {
                generateClass(it.value(), cl);
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

    fprintf(fp, " {\n");

    // Debug mode
    if (debug) {
        fmt = "        qasDebug() << \"%s: unexpected token\" << s;\n";
        fprintf(fp, fmt, type_str);
    }

    // Last and end
    fprintf(fp, "        ok2 = false;\n"
                "    }\n"
                "    ok ? (*ok = ok2) : false;\n"
                "    return res;\n"
                "}\n");

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

    // Super classes
    for (const auto &super : def.superclassList) {
        fmt = "    *reinterpret_cast<%s *>(&res) = QASJsonType<%s>::fromObject(obj, &ok2);\n"
              "    if (!ok2) {\n"
              "        goto over;\n"
              "    }\n";
        const char *name_str = super.first.data();
        fprintf(fp, fmt, name_str, name_str);
    }

    // Start branches
    for (const auto &item : def.memberVars) {
        if (item.ignore || item.access != FunctionDef::Public) {
            continue;
        }
        QByteArray attr = item.attr.isEmpty() ? item.name : item.attr;
        fmt = "    it = obj.find(\"%s\");\n"
              "    if (it != obj.end()) {\n"
              "        res.%s = QASJsonType<decltype(res.%s)>::fromValue(it.value(), &ok2);\n"
              "        if (!ok2) {\n";
        const char *name_str = item.name.data();
        fprintf(fp, fmt, attr.data(), name_str, name_str);

        if (debug) {
            fmt = "            qasDebug() << \"%s: parse value failed, key: %s\";\n";
            fprintf(fp, fmt, type_str, attr.data());
        }

        fprintf(fp, "            goto over;\n"
                    "        }\n"
                    "    }\n");
    }

    // Last and end
    fprintf(fp, "over:\n"
                "    ok ? (*ok = ok2) : false;\n"
                "    return res;\n"
                "}\n");

    fprintf(fp, "\n");

    // Generate serializer
    // Declaration head
    fmt = "QJsonObject QASJsonType<%s>::toObject(const %s &cls) {\n";
    fprintf(fp, fmt, type_str, type_str);

    // Define res
    fprintf(fp, "    QJsonObject res, tmp;\n");

    // Super classes
    for (const auto &super : def.superclassList) {
        fmt = "    tmp = QASJsonType<%s>::toObject(cls);\n"
              "    for (auto it = tmp.begin(); it != tmp.end(); ++it) {\n"
              "        res.insert(it.key(), it.value());\n"
              "    }\n";
        const char *name_str = super.first.data();
        fprintf(fp, fmt, name_str);
    }

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
