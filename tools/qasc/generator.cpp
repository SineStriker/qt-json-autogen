#include "generator.h"

static QByteArrayList splitScopes(const QByteArray &name, bool ignoreTemplate = true) {
    int cnt = 0;
    QByteArrayList res;
    QByteArray cur;
    for (const auto &ch : name) {
        switch (ch) {
            case '<':
                cnt++;
                if (!ignoreTemplate) {
                    cur += ch;
                }
                break;
            case '>':
                cnt--;
                if (!ignoreTemplate) {
                    cur += ch;
                }
                break;
            default:
                if (cnt > 0) {
                    if (!ignoreTemplate) {
                        cur += ch;
                    }
                    break;
                }
                if (ch == ':') {
                    if (!cur.isEmpty()) {
                        res += cur;
                        cur.clear();
                    }
                } else {
                    cur += ch;
                }
                break;
        }
    }
    if (!cur.isEmpty()) {
        res += cur;
    }
    return res;
}

static bool searchScope(const QByteArrayList &scopeNames, Environment *root, Environment *&out) {
    Environment *env = root;
    QByteArrayList names = scopeNames;

    while (!names.isEmpty()) {
        auto name = names.first();
        names.pop_front();

        while (env) {
            // Search children
            {
                auto it = env->children.find(name);
                if (it != env->children.end()) {
                    const auto &inners = it.value();
                    for (const auto &inner : inners) {
                        if (searchScope(names, inner.data(), env)) {
                            names.clear();
                            break;
                        }
                    }
                    break;
                }
            }

            // Search namespace alias
            {
                auto it = env->aliasNamespaces.find(name);
                if (it != env->aliasNamespaces.end()) {
                    QByteArrayList aliasNames = splitScopes(it.value());
                    names = aliasNames + names;
                    break;
                }
            }

            // Search class alias
            {
                auto it = env->aliasClasses.find(name);
                if (it != env->aliasClasses.end()) {
                    QByteArrayList aliasNames = splitScopes(it.value().name);
                    names = aliasNames + names;
                    break;
                }
            }

            // Go upper if not in current layer
            env = env->parent;
        }

        if (env)
            continue;

        return false;
    }

    out = env;

    return true;
}

QByteArray resolveSuperClass(const QByteArray &classQualified, const QByteArray &superName,
                             Environment *rootEnv) {
    QByteArray res;

    QByteArrayList layerNames = splitScopes(classQualified);
    layerNames.removeLast();

    QByteArrayList layerFullNames = splitScopes(classQualified, false);
    layerFullNames.removeLast();

    QByteArrayList superNames = splitScopes(superName);
    int i;
    for (i = layerFullNames.size(); i >= 0; --i) {
        Environment *env = nullptr;
        auto scopeNames = layerNames.mid(0, i) + superNames;
        if (!searchScope(scopeNames, rootEnv, env) || env->cl.isNull()) {
            continue;
        }
        break;
    }

    if (i >= 0) {
        res = layerFullNames.mid(0, i).join("::") + "::" + superName;
    }
    return res;
}

void Generator::generateCode() {
    // Generate all enums
    for (const auto &q : qAsConst(rootEnv->enumToGen)) {
        auto scopeNames = splitScopes(q);
        Environment *env = nullptr;
        if (!searchScope(scopeNames.mid(0, scopeNames.size() - 1), rootEnv, env)) {
            goto enum_not_found;
        }

        {
            auto it = env->enums.find(scopeNames.back());
            if (it == env->enums.end()) {
                goto enum_not_found;
            }
            generateEnums(q, it.value());
            continue;
        }

    enum_not_found:
        qDebug().noquote() << QString::asprintf("Cannot found the correct scope of enum : %s",
                                                q.data());
    }

    // Generate all classes
    for (const auto &q : qAsConst(rootEnv->classToGen)) {
        const auto &name = q.first;
        auto scopeNames = splitScopes(name);
        Environment *env = nullptr;
        if (!searchScope(scopeNames, rootEnv, env) || env->cl.isNull()) {
            qDebug().noquote() << QString::asprintf("Cannot found the correct scope of class: %s",
                                                    name.data());
            continue;
        }

        auto &cl = *env->cl.data();
        QByteArrayList qualifiedSuperNames;
        for (const auto &item : qAsConst(cl.superclassList)) {
            const auto &superName = item.first;
            auto res = resolveSuperClass(name, superName, rootEnv);
            if (res.isEmpty()) {
                qDebug().noquote() << QString::asprintf(
                    "Cannot found the correct scope of class %s derived by class %s",
                    superName.data(), name.data());
                continue;
            }
            qualifiedSuperNames.append(res);
        }

        generateClass(name, qualifiedSuperNames, cl);
    }
}

void Generator::generateEnums(const QByteArray &qualified, const EnumDef &def) {
    const char *fmt;
    const char *type_str = qualified.data();

    // Title
    {
        QByteArray title = "// Deserializer and serializer for enumeration " + QByteArray(type_str);
        QByteArray line = "//" + QByteArray(title.size() + 10, '=');
        fprintf(fp, "%s\n%s\n%s\n\n", line.data(), title.data(), line.data());
    }

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
                "    QAS_SET_OK(ok, ok2);\n"
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

    fprintf(fp, "\n\n");
}

void Generator::generateClass(const QByteArray &qualified, const QByteArrayList &supers,
                              const ClassDef &def) {
    const char *fmt;
    const char *type_str = qualified.data();

    // Title
    {
        QByteArray title = "// Deserializer and serializer for class " + QByteArray(type_str);
        QByteArray line = "//" + QByteArray(title.size() + 10, '=');
        fprintf(fp, "%s\n%s\n%s\n\n", line.data(), title.data(), line.data());
    }

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
    for (const auto &super : supers) {
        fmt = "    *reinterpret_cast<%s *>(&res) = QASJsonType<%s>::fromObject(obj, &ok2);\n"
              "    if (!ok2) {\n"
              "        goto over;\n"
              "    }\n";
        const char *name_str = super.data();
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
                "    QAS_SET_OK(ok, ok2);\n"
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
    for (const auto &super : supers) {
        fmt = "    tmp = QASJsonType<%s>::toObject(cls);\n"
              "    for (auto it = tmp.begin(); it != tmp.end(); ++it) {\n"
              "        res.insert(it.key(), it.value());\n"
              "    }\n";
        const char *name_str = super.data();
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

    fprintf(fp, "\n\n");
}
