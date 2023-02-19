#include "nameutil.h"

#include "qasc.h"

#ifdef Q_CC_MSVC
#define ErrorFormatString "%s(%d): "
#else
#define ErrorFormatString "%s:%d: "
#endif

void NameUtil::error(const QByteArray &msg, const QByteArray &filename, int lineNum, bool isError) {
    if (isError) {
        fprintf(stderr, ErrorFormatString "Error: %s\n", filename.constData(), lineNum,
                msg.constData());
        exit(EXIT_FAILURE);
    } else {
        fprintf(stderr, ErrorFormatString "Warning: %s\n", filename.constData(), lineNum,
                msg.constData());
    }
}

QByteArrayList NameUtil::splitScopes(const QByteArray &name, bool ignoreTemplate) {
    int cnt = 0;
    QByteArrayList res;
    QByteArray cur;
    for (const auto &ch: name) {
        switch (ch) {
            case ' ':
                break;
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

Environment *NameUtil::exploreEnv(Environment *curEnv, const QByteArrayList &names,
                                  bool createIfNotFound) {
    auto env = curEnv;

    auto it0 = names.begin();
    while (it0 != names.end()) {
        auto curName = *it0;
        it0++;

        auto it = env->children.find(curName);
        if (it != env->children.end()) {
            env = it->data();
            continue;
        }

        if (!createIfNotFound) {
            return nullptr;
        }

        auto def = new NamespaceDef();
        def->classname = curName;
        def->qualified = curName;

        auto newEnv = QSharedPointer<Environment>::create(def, env);
        env->children.insert(curName, newEnv);
        env = newEnv.data();
    }
    return env;
}

QByteArray NameUtil::combineNames(const QByteArray &l, const QByteArray &r) {
    if (l.isEmpty())
        return r;
    if (r.isEmpty())
        return l;
    return l + "::" + r;
}

int stackDepth = 0;

QHash<Environment *, QSet<QByteArray>> notFounds;

// Used as a fallback
struct StackGuard {
    StackGuard() {
        stackDepth++;
    }

    ~StackGuard() {
        stackDepth--;
    }
};

struct FindArguments {
    bool qualified;
    bool force;
};

static NameUtil::FindResult findScope(Environment *rootEnv, Environment *fromEnv,
                                      const QByteArray &allName, FindArguments args);

static NameUtil::FindResult findNextScope(Environment *rootEnv, Environment *fromEnv,
                                          const QByteArray &token, const QByteArray &nextAllName,
                                          FindArguments args) {
    // Get namespace scope (Force)
    auto nextScope = findScope(rootEnv, fromEnv, token, {false, true});
    if (!nextScope.env || nextScope.type == NameUtil::FindResult::Enumeration ||
        nextScope.type == NameUtil::FindResult::PredefinedClass) {
        // Namespace not found
        // There are mistakes in the code, but ignore
        return nullptr;
    }

    // if (nextScope.env->templateClass) {
    //     return nullptr;
    // }

    return findScope(rootEnv, nextScope.env, nextAllName, {true, args.force});
}

static QByteArray findFirstNamespace(Environment *env, QByteArray ns) {
    QSet<QByteArray> nss;

    // Resolve alias
    while (true) {
        if (nss.contains(ns)) {
            // Alias loop
            return {};
        }
        nss.insert(ns);

        auto it = env->aliasNamespaces.find(ns);
        if (it == env->aliasNamespaces.end()) {
            break;
        }
        ns = it.value();
    }

    return ns;
}

NameUtil::FindResult findScope(Environment *rootEnv, Environment *fromEnv,
                               const QByteArray &allName, FindArguments args) {
    StackGuard guard;
    if (stackDepth > 10) {
        fprintf(stderr, "Fatal: Stack overflow!!!\n");
        fflush(stderr);
        return nullptr;
    }

    if (!fromEnv) {
        return nullptr;
    }

    if (allName.isEmpty()) {
        return fromEnv;
    }

    if (allName.startsWith("::")) {
        fromEnv = rootEnv;
    }

    QByteArrayList names = NameUtil::splitScopes(allName);
    auto nameFrom = [&names](int i) { return names.mid(i).join("::"); };

    // Search enum
    if (names.size() == 1) {
        if (fromEnv->enums.contains(allName)) {
            return {fromEnv, NameUtil::FindResult::Enumeration, allName};
        }

        if (NameUtil::considerPredefinedClass) {
            if (fromEnv->predeclaredClasses.contains(names.front())) {
                return {fromEnv, NameUtil::FindResult::PredefinedClass, allName};
            }
        }

        if (!args.force) {
            if (fromEnv->aliasClasses.contains(allName) || fromEnv->usedClasses.contains(allName)) {
                return {fromEnv, NameUtil::FindResult::ImportedClass, allName};
            }
        }
    }

    QByteArray firstName = names.front();

    // Search children
    {
        auto it = fromEnv->children.find(firstName);
        if (it != fromEnv->children.end()) {
            return findScope(rootEnv, it->data(), nameFrom(1), {true, args.force});
        }
    }

    // Search alias
    {
        auto it = fromEnv->aliasNamespaces.find(firstName);
        if (it != fromEnv->aliasNamespaces.end()) {
            auto ns = findFirstNamespace(fromEnv, it.value());
            if (ns.isEmpty()) {
                return nullptr;
            }
            return findNextScope(rootEnv, fromEnv, ns, nameFrom(1), args);
        }
    }

    {
        auto it = fromEnv->aliasClasses.find(firstName);
        if (it != fromEnv->aliasClasses.end()) {
            return findNextScope(rootEnv, fromEnv, it.value(), nameFrom(1), args);
        }
    }

    // Search in super class scope
    if (fromEnv->parent && !fromEnv->cl.isNull()) {
        for (const auto &super: qAsConst(fromEnv->cl->superclassList)) {
            auto tmp = findNextScope(rootEnv, fromEnv->parent, super.first, allName, args);
            if (tmp.env) {
                return tmp;
            }
        }
    }

    // Search class imported by "using"
    {
        auto it = fromEnv->usedClasses.find(firstName);
        if (it != fromEnv->usedClasses.end()) {
            auto tmp = findNextScope(rootEnv, fromEnv, it.value(), nameFrom(1), args);
            if (tmp.env) {
                return tmp;
            }
        }
    }

    // Search namespace imported by "using"
    for (auto ns: qAsConst(fromEnv->usedNamespaces)) {
        ns = findFirstNamespace(fromEnv, ns);
        if (ns.isEmpty()) {
            return nullptr;
        }

        if (notFounds[fromEnv].contains(ns)) {
            continue;
        }

        notFounds[fromEnv].insert(ns);
        auto tmp = findNextScope(rootEnv, fromEnv, ns, allName, args);
        notFounds[fromEnv].remove(ns);

        if (tmp.env) {
            return tmp;
        }
    }

    // Search parent
    if (!args.qualified) {
        auto tmp = findScope(rootEnv, fromEnv->parent, allName, args);
        if (tmp.env) {
            return tmp;
        }
    }

    return nullptr;
}

bool NameUtil::considerPredefinedClass = false;

NameUtil::FindResult NameUtil::getScope(Environment *rootEnv, Environment *fromEnv,
                                        const QByteArray &allName, bool force) {
    return findScope(rootEnv, fromEnv, allName, {false, force});
}

QByteArrayList NameUtil::getQualifiedNameList(Environment *env) {
    QByteArrayList names;
    while (env && !env->isRoot) {
        names.prepend(env->name());
        env = env->parent;
    }
    return names;
}

QByteArray NameUtil::getQualifiedName(Environment *env) {
    return getQualifiedNameList(env).join("::");
}
