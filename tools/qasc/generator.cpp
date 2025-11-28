#include "generator.h"

#include "nameutil.h"

static QByteArray fixClassName(Environment *env, const QByteArray &unqualified) {
    auto qualified = NameUtil::getQualifiedNameList(env);

    auto names = NameUtil::splitScopes(unqualified, false);
    if (env->templateClass) {
        qualified.removeLast();
        qualified.append(names.back());
    }

    return qualified.join("::");
}

void Generator::generateCode() {
    QList<Environment *> envsToProcess;

    std::list<Environment *> stack;
    stack.push_back(rootEnv);
    while (!stack.empty()) {
        auto env = stack.front();
        stack.pop_front();

        if (!env->classToGen.isEmpty()) {
            envsToProcess.append(env);
        }

        for (const auto &child: qAsConst(env->children)) {
            stack.push_back(child.data());
        }
    }

    // Generate header
    //    QSet<Environment *> hasUsing;
    //    for (auto env : qAsConst(envsToProcess)) {
    //        // Back to nearest namespace
    //        while (!env->isNamespace && !env->isRoot) {
    //            if (env->templateClass) {
    //                auto first = env->classToGen.front();
    //                NameUtil::error("Q_JSON cannot be declared in template class!",
    //                first.filename,
    //                                first.lineNum);
    //            }
    //            env = env->parent;
    //        }

    //        // using foo::bar::operator<<;
    //        if (env->isNamespace && !hasUsing.contains(env)) {
    //            hasUsing.insert(env);
    //            generateUsing(NameUtil::getQualifiedName(env));
    //        }
    //    }

    //    fprintf(fp, "\n");

    // Generate implementations
    QSet<QByteArray> classes;
    QSet<QByteArray> enums;
    for (auto env: qAsConst(envsToProcess)) {
        QByteArray prefix;
        // Get namespace
        {
            auto curEnv = env;
            // Back to nearest namespace
            while (!curEnv->isNamespace && !curEnv->isRoot) {
                if (curEnv->templateClass) {
                    auto first = curEnv->classToGen.front();
                    NameUtil::error("Q_JSON cannot be declared in template class!", first.filename,
                                    first.lineNum);
                }
                curEnv = curEnv->parent;
            }

            if (curEnv->isNamespace) {
                prefix = NameUtil::getQualifiedName(curEnv);
            }
        }

        for (const auto &item: qAsConst(env->classToGen)) {
            if (!item.gen) {
                continue;
            }

            const QByteArray &classToken = item.token;

            auto res = NameUtil::getScope(rootEnv, env, classToken, true);
            if (!res.env) {
                NameUtil::error("Class " + classToken + " not found!", item.filename, item.lineNum);
                return;
            }
            auto classDefEnv = res.env;

            // Enumeration
            if (res.type == NameUtil::FindResult::Enumeration) {
                auto it = classDefEnv->enums.find(res.name);
                if (it == classDefEnv->enums.end()) {
                    NameUtil::error("Enumeration " + classToken + " not found!", item.filename,
                                    item.lineNum);
                }

                auto enumName =
                        NameUtil::combineNames(NameUtil::getQualifiedName(classDefEnv), res.name);
                if (enums.contains(enumName)) {
                    NameUtil::error("Enumeration " + enumName + " has duplicated declarations!",
                                    item.filename, item.lineNum);
                }
                enums.insert(enumName);

                generateEnums(prefix.isEmpty() ? QByteArray() : prefix + "::", enumName, it.value());
                continue;
            }

            if (res.env->isNamespace || !res.env->cl) {
                NameUtil::error(classToken + " is a namespace!", item.filename, item.lineNum);
            }
            const auto &classDef = *res.env->cl;

            QByteArray className = fixClassName(classDefEnv, classToken);

            // Search again to find a alias
            {
                auto res2 = NameUtil::getScope(rootEnv, env, classToken, false);
                if (res2.type == NameUtil::FindResult::ImportedClass) {
                    className =
                            NameUtil::combineNames(NameUtil::getQualifiedName(res2.env), res2.name);
                }
            }

            // This check won't work in intermediate scope which is a template class
            if (classes.contains(className)) {
                NameUtil::error("Class " + className + " has duplicated declarations!",
                                item.filename, item.lineNum);
            }
            classes.insert(className);

            // Class
            QByteArrayList superNameList;

            for (const auto &super: qAsConst(classDef.superclassList)) {
                // Collect super class
                const auto &superToken = super.first;
                const auto &info = super.second;

                // Check access
                if (info.access == FunctionDef::Public) {
                    if (info.exclude)
                        continue;
                } else {
                    if (!info.include)
                        continue;
                }

                auto superRes = NameUtil::getScope(rootEnv, classDefEnv, superToken, false);
                if (!superRes.env) {
                    NameUtil::error("Base class " + super.first + " not found!", info.filename,
                                    info.lineNum);
                    return;
                }

                QByteArray superName;
                switch (superRes.type) {
                    case NameUtil::FindResult::Enumeration:
                        NameUtil::error("Base class " + superToken + " is a enumeration!",
                                        info.filename, info.lineNum);
                        break;
                    case NameUtil::FindResult::ImportedClass:
                        superName = NameUtil::combineNames(NameUtil::getQualifiedName(superRes.env),
                                                           superRes.name);
                        break;
                    case NameUtil::FindResult::Scope:
                        if (superRes.env->isNamespace) {
                            NameUtil::error("Base class " + superToken + " is a namespace!",
                                            info.filename, info.lineNum);
                        }
                        superName = fixClassName(superRes.env, superToken);
                        break;
                }
                superNameList.append(superName);
            }

            generateClass(prefix.isEmpty() ? QByteArray() : prefix + "::", className, superNameList, classDef);
        }
    }
}

// void Generator::generateUsing(const QByteArray &qualified) {
//     fprintf(fp, "using %s::operator<<;\n", qualified.constData());
//     fprintf(fp, "using %s::operator>>;\n", qualified.constData());
//     fprintf(fp, "\n");
// }

void Generator::generateEnums(const QByteArray &ns, const QByteArray &qualified,
                              const EnumDef &def) {
    const char *fmt;
    const char *type_str = qualified.data();
    const char *ns_str = ns.data();

    // Title
    {
        QByteArray title = "// Deserializer and serializer for enumeration " + QByteArray(type_str);
        QByteArray line = "//" + QByteArray(title.size() + 10, '=');
        fprintf(fp, "%s\n%s\n%s\n\n", line.data(), title.data(), line.data());
    }

    // Generate deserializer
    // Declaration head
    fmt = "QAS::JsonStream &%soperator>>(QAS::JsonStream &_stream, %s &_var) {\n";
    fprintf(fp, fmt, ns_str, type_str);

    // Convert to string
    fprintf(fp, "    QString _str;\n"
                "    if (!QAS::JsonStreamUtils::parseAsString(_stream, typeid(_var).name(), &_str).good()) {\n"
                "        return _stream;\n"
                "    }\n\n");

    // Define res
    fmt = "    %s _tmp{};\n";
    fprintf(fp, fmt, type_str);

    // Start branches
    fprintf(fp, "    ");
    for (const auto &item: def.values) {
        if (item.exclude) {
            continue;
        }
        QByteArray attr = item.attr.isEmpty() ? item.itemName : item.attr;
        fmt = "if (_str == \"%s\") {\n"
              "        _tmp = %s::%s;\n"
              "    } else ";
        fprintf(fp, fmt, attr.data(), type_str, item.itemName.data());
    }

    fprintf(fp, " {\n");

    // Last and end
    fprintf(fp, "        _stream.setStatus(QAS::JsonStream::UnlistedValue);\n"
                "    }\n"
                "    _var = _tmp;\n"
                "\n"
                "    return _stream;\n"
                "}\n");

    fprintf(fp, "\n");

    // Generate serializer
    // Declaration head
    fmt = "QAS::JsonStream &%soperator<<(QAS::JsonStream &_stream, const %s &_var) {\n";
    fprintf(fp, fmt, ns_str, type_str);

    // Define res
    fprintf(fp, "    _stream.resetStatus();\n"
                "\n"
                "    QString _tmp;\n"
                "    switch (_var) {\n");

    // Start switch
    for (const auto &item: def.values) {
        if (item.exclude) {
            continue;
        }
        QByteArray attr = item.attr.isEmpty() ? item.itemName : item.attr;
        fmt = "        case %s::%s:\n"
              "            _tmp = \"%s\";\n"
              "            break;\n";
        fprintf(fp, fmt, type_str, item.itemName.data(), attr.data());
    }

    // Last and end
    fprintf(fp, "        default:\n"
                "            break;\n"
                "    }\n"
                "    _stream << _tmp;\n"
                "\n"
                "    return _stream;\n"
                "}\n");

    fprintf(fp, "\n\n");
}

void Generator::generateClass(const QByteArray &ns, const QByteArray &qualified,
                              const QByteArrayList &supers, const ClassDef &def) {
    const char *fmt;
    const char *type_str = qualified.data();
    const char *ns_str = ns.data();

    // Title
    {
        QByteArray title = "// Deserializer and serializer for class " + QByteArray(type_str);
        QByteArray line = "//" + QByteArray(title.size() + 10, '=');
        fprintf(fp, "%s\n%s\n%s\n\n", line.data(), title.data(), line.data());
    }

    // Generate deserializer
    // Declaration head
    fmt = "QAS::JsonStream &%soperator>>(QAS::JsonStream &_stream, %s &_var) {\n";
    fprintf(fp, fmt, ns_str, type_str);

    // Convert to object
    fprintf(fp, "    QJsonObject _obj;\n"
                "    if (!QAS::JsonStreamUtils::parseAsObject(_stream, typeid(_var).name(), &_obj).good()) {\n"
                "        return _stream;\n"
                "    }\n\n");

    // Define res
    fmt = "    %s _tmpVar{};\n"
          "\n"
          "    QAS::JsonStream _tmpStream;\n";
    fprintf(fp, fmt, type_str);

    // Super classes
    for (const auto &super: supers) {
        const char *name_str = super.data();
        fmt = "    _stream >> *static_cast<%s *>(&_tmpVar);\n"
              "    if (!_stream.good()) {\n"
              "        return _stream;\n"
              "    }\n";
        fprintf(fp, fmt, name_str);
    }

    // Start branches
    for (const auto &item: def.memberVars) {
        if (item.access == FunctionDef::Public) {
            if (item.exclude)
                continue;
        } else {
            if (!item.include)
                continue;
        }
        QByteArray attr = item.attr.isEmpty() ? item.name : item.attr;
        const char *name_str = item.name.data();
        fmt = "    if (!(_tmpStream = QAS::JsonStreamUtils::parseObjectMember(_obj, \"%s\", typeid(_tmpVar).name(), &_tmpVar.%s)).good()) {\n"
              "        _stream.setStatus(_tmpStream.status());\n"
              "        return _stream;\n"
              "    }\n";
        fprintf(fp, fmt, attr.data(), name_str);
        
        // Generate constraint validation if constraints exist
        if (!item.constraintGroups.isEmpty()) {
            generateConstraintValidation(item.name, item.constraintGroups);
        }
    }

    // Last and end
    fprintf(fp, "    _var = std::move(_tmpVar);\n"
                "\n"
                "    return _stream;\n"
                "}\n");

    fprintf(fp, "\n");

    // Generate serializer
    // Declaration head
    fmt = "QAS::JsonStream &%soperator<<(QAS::JsonStream &_stream, const %s &_var) {\n";
    fprintf(fp, fmt, ns_str, type_str);

    // Define res
    fprintf(fp, "    _stream.resetStatus();\n"
                "\n"
                "    QJsonObject _obj;\n");

    // Super classes
    for (const auto &super: supers) {
        fmt =
                "    {\n"
                "        QJsonObject _tmpObj = qAsClassToJson(*static_cast<const %s *>(&_var));\n"
                "        for (auto it = _tmpObj.begin(); it != _tmpObj.end(); ++it) {\n"
                "            _obj.insert(it.key(), it.value());\n"
                "        }\n"
                "    }\n";
        const char *name_str = super.data();
        fprintf(fp, fmt, name_str);
    }

    // Start switch
    for (const auto &item: def.memberVars) {
        if (item.access == FunctionDef::Public) {
            if (item.exclude)
                continue;
        } else {
            if (!item.include)
                continue;
        }
        QByteArray attr = item.attr.isEmpty() ? item.name : item.attr;
        fmt = "    _obj.insert(\"%s\", QAS::JsonStream::fromValue(_var.%s).data());\n";
        const char *name_str = item.name.data();
        fprintf(fp, fmt, attr.data(), name_str);
    }

    // Last and end
    fprintf(fp, "    _stream << _obj;\n"
                "\n"
                "    return _stream;\n"
                "}\n");

    fprintf(fp, "\n\n");
}

void Generator::generateConstraintValidation(const QByteArray &fieldName, 
                                            const QVector<ConstraintGroup> &constraintGroups) {
    if (constraintGroups.isEmpty()) {
        return;
    }
    
    const char *field_str = fieldName.data();
    
    fprintf(fp, "    // Validate constraints for field '%s'\n", field_str);
    fprintf(fp, "    {\n");
    fprintf(fp, "        bool _constraintSatisfied = false;\n");
    fprintf(fp, "        QJsonValue _fieldValue = QAS::JsonStream::fromValue(_tmpVar.%s).data();\n", field_str);
    
    // Generate validation for each constraint group (OR relationship)
    for (int groupIdx = 0; groupIdx < constraintGroups.size(); ++groupIdx) {
        const auto &group = constraintGroups[groupIdx];
        
        if (groupIdx == 0) {
            fprintf(fp, "        if (");
        } else {
            fprintf(fp, "        } else if (");
        }
        
        // Generate validation for each constraint in the group (AND relationship)
        for (int constraintIdx = 0; constraintIdx < group.constraints.size(); ++constraintIdx) {
            const auto &constraint = group.constraints[constraintIdx];
            
            if (constraintIdx > 0) {
                fprintf(fp, " && ");
            }
            
            // Generate specific constraint validation based on type
            generateSingleConstraintCheck(constraint);
        }
        
        fprintf(fp, ") {\n");
        fprintf(fp, "            _constraintSatisfied = true;\n");
    }
    
    if (!constraintGroups.isEmpty()) {
        fprintf(fp, "        }\n");
    }
    
    fprintf(fp, "        if (!_constraintSatisfied) {\n");
    fprintf(fp, "            _stream.setStatus(QAS::JsonStream::ConstraintViolation);\n");
    fprintf(fp, "            return _stream;\n");
    fprintf(fp, "        }\n");
    fprintf(fp, "    }\n");
}

void Generator::generateSingleConstraintCheck(const Constraint &constraint) {
    QJsonValue value = constraint.value;
    
    switch (constraint.type) {
        case ConstraintType::MINIMUM:
            fprintf(fp, "QAS::ConstraintValidator::validateMinimum(_fieldValue, QJsonValue(%g))", value.toDouble());
            break;
            
        case ConstraintType::MAXIMUM:
            fprintf(fp, "QAS::ConstraintValidator::validateMaximum(_fieldValue, QJsonValue(%g))", value.toDouble());
            break;
            
        case ConstraintType::EXCLUSIVE_MINIMUM:
            fprintf(fp, "QAS::ConstraintValidator::validateExclusiveMinimum(_fieldValue, QJsonValue(%g))", value.toDouble());
            break;
            
        case ConstraintType::EXCLUSIVE_MAXIMUM:
            fprintf(fp, "QAS::ConstraintValidator::validateExclusiveMaximum(_fieldValue, QJsonValue(%g))", value.toDouble());
            break;
            
        case ConstraintType::CONST:
            if (value.isString()) {
                fprintf(fp, "QAS::ConstraintValidator::validateConst(_fieldValue, QJsonValue(QString(\"%s\")))", 
                       value.toString().toUtf8().constData());
            } else if (value.isDouble()) {
                fprintf(fp, "QAS::ConstraintValidator::validateConst(_fieldValue, QJsonValue(%g))", value.toDouble());
            } else if (value.isBool()) {
                fprintf(fp, "QAS::ConstraintValidator::validateConst(_fieldValue, QJsonValue(%s))", 
                       value.toBool() ? "true" : "false");
            } else if (value.isNull()) {
                fprintf(fp, "QAS::ConstraintValidator::validateConst(_fieldValue, QJsonValue(QJsonValue::Null))");
            }
            break;
            
        case ConstraintType::ENUM: {
            fprintf(fp, "QAS::ConstraintValidator::validateEnum(_fieldValue, QJsonValue(QJsonArray({");
            QJsonArray arr = value.toArray();
            for (int i = 0; i < arr.size(); ++i) {
                if (i > 0) fprintf(fp, ", ");
                QJsonValue item = arr[i];
                if (item.isString()) {
                    fprintf(fp, "QString(\"%s\")", item.toString().toUtf8().constData());
                } else if (item.isDouble()) {
                    fprintf(fp, "%g", item.toDouble());
                } else if (item.isBool()) {
                    fprintf(fp, "%s", item.toBool() ? "true" : "false");
                } else if (item.isNull()) {
                    fprintf(fp, "QJsonValue(QJsonValue::Null)");
                }
            }
            fprintf(fp, "})))");
            break;
        }
        
        case ConstraintType::MIN_LENGTH:
            fprintf(fp, "QAS::ConstraintValidator::validateMinLength(_fieldValue, QJsonValue(%d))", value.toInt());
            break;
            
        case ConstraintType::MAX_LENGTH:
            fprintf(fp, "QAS::ConstraintValidator::validateMaxLength(_fieldValue, QJsonValue(%d))", value.toInt());
            break;
            
        case ConstraintType::PATTERN:
            fprintf(fp, "QAS::ConstraintValidator::validatePattern(_fieldValue, QJsonValue(QString(\"%s\")))", 
                   value.toString().toUtf8().constData());
            break;
            
        default:
            fprintf(fp, "true /* unsupported constraint type */");
            break;
    }
}
