#ifndef NAMEUTIL_H
#define NAMEUTIL_H

#include <QByteArrayList>

class EnumDef;
class Environment;

namespace NameUtil {

    void error(const QByteArray &msg, const QByteArray &filename, int lineNum, bool isError = true);

    QByteArrayList splitScopes(const QByteArray &name, bool ignoreTemplate = true);

    Environment *exploreEnv(Environment *curEnv, const QByteArrayList &names,
                            bool createIfNotFound = false);

    QByteArray combineNames(const QByteArray &l, const QByteArray &r);

    extern bool considerPredefinedClass;

    struct FindResult {
        enum Type {
            Scope,
            ImportedClass,
            Enumeration,
            PredefinedClass,
        };

        Environment *env;
        Type type;
        QByteArray name;

        FindResult(Environment *env) : FindResult(env, Scope){};
        FindResult(Environment *env, Type type, const QByteArray &name = {})
            : env(env), type(type), name(name){};
    };

    FindResult getScope(Environment *rootEnv, Environment *fromEnv, const QByteArray &allName,
                        bool force);

    QByteArrayList getQualifiedNameList(Environment *env);

    QByteArray getQualifiedName(Environment *env);

} // namespace NameUtil

#endif // NAMEUTIL_H
