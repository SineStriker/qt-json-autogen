#ifndef QAS_JSON_TYPES_H
#define QAS_JSON_TYPES_H

#include <QJsonArray>
#include <QJsonObject>

#include <QHash>
#include <QMap>

#include <map>
#include <unordered_map>

#ifndef QAS_DISABLE_DEBUG
#define qasDebug qDebug
#else
#define qasDebug QT_NO_QDEBUG_MACRO
#endif

template<class T>
struct QASJsonTypeDefault {
    enum {
        Defined = 0,
    };
};

template<class T>
struct QASJsonType : public QASJsonTypeDefault<T> //
{                                                 //
};

#define QAS_JSON_VALUE_TYPE_DECLARE(TYPE, IDENTIFIER)                                              \
    template <>                                                                                    \
    struct QASJsonType<TYPE> {                                                                     \
        enum {                                                                                     \
            Defined = 1,                                                                           \
        };                                                                                         \
                                                                                                   \
        static TYPE fromValue(const QJsonValue &val, bool *ok = nullptr) {                         \
            if (!val.is##IDENTIFIER()) {                                                           \
                qasDebug() << #TYPE ": expect " #TYPE ", but get" << val.type();                   \
                ok ? (*ok = false) : false;                                                        \
                return TYPE{};                                                                     \
            }                                                                                      \
            ok ? (*ok = true) : false;                                                             \
            return val.to##IDENTIFIER();                                                           \
        }                                                                                          \
                                                                                                   \
        static QJsonValue toValue(const TYPE &val) { return QJsonValue(val); }                     \
    };

#define QAS_JSON_QT_BASIC_TYPE_DECLARE(TYPE)                                                       \
    template <>                                                                                    \
    struct QASJsonType<TYPE> {                                                                     \
        enum {                                                                                     \
            Defined = 1,                                                                           \
        };                                                                                         \
                                                                                                   \
        static TYPE fromValue(const QJsonValue &val, bool *ok = nullptr) {                         \
            QVariant tmp = val.toVariant();                                                        \
            if (tmp.type() != qMetaTypeId<TYPE>()) {                                               \
                qasDebug() << #TYPE ": expect " #TYPE ", but get" << tmp.type();                   \
                ok ? (*ok = false) : false;                                                        \
                return TYPE{};                                                                     \
            }                                                                                      \
            ok ? (*ok = true) : false;                                                             \
            return tmp.value<TYPE>();                                                              \
        }                                                                                          \
                                                                                                   \
        static QJsonValue toValue(const TYPE &val) { return QJsonValue(val); }                     \
    };

#define QAS_JSON_QT_MAP_TYPE_DECLARE(TYPE)                                                         \
    template <class T>                                                                             \
    struct QASJsonType<TYPE<QString, T>> {                                                         \
        enum {                                                                                     \
            Defined = 1,                                                                           \
        };                                                                                         \
                                                                                                   \
        static TYPE<QString, T> fromValue(const QJsonValue &val, bool *ok = nullptr) {             \
            if (!val.isObject()) {                                                                 \
                qasDebug() << #TYPE "<QString, T>: expect object, but get" << val.type();          \
                ok ? (*ok = false) : false;                                                        \
                return {};                                                                         \
            }                                                                                      \
            ok ? (*ok = true) : false;                                                             \
            return fromObject(val.toObject(), ok);                                                 \
        }                                                                                          \
                                                                                                   \
        static QJsonValue toValue(const TYPE<QString, T> &val) { return toObject(val); }           \
                                                                                                   \
        static TYPE<QString, T> fromObject(const QJsonObject &obj, bool *ok = nullptr) {           \
            TYPE<QString, T> res;                                                                  \
            bool ok2 = true;                                                                       \
            for (auto it = obj.begin(); it != obj.end(); ++it) {                                   \
                auto tmp = QASJsonType<T>::fromValue(it.value(), &ok2);                            \
                if (!ok2) {                                                                        \
                    qasDebug() << #TYPE "<QString, T>: fail at key" << it.key();                 \
                    res.clear();                                                                   \
                    break;                                                                         \
                }                                                                                  \
                res.insert(it.key(), tmp);                                                         \
            }                                                                                      \
            ok ? (*ok = ok2) : false;                                                              \
            return res;                                                                            \
        }                                                                                          \
                                                                                                   \
        static QJsonObject toObject(const TYPE<QString, T> &obj) {                                 \
            QJsonObject res;                                                                       \
            for (auto it = obj.begin(); it != obj.end(); ++it) {                                   \
                res.insert(it.key(), QASJsonType<T>::toValue(it.value()));                         \
            }                                                                                      \
            return res;                                                                            \
        };                                                                                         \
    };

#define QAS_JSON_STL_MAP_TYPE_DECLARE(TYPE)                                                        \
    template <class T>                                                                             \
    struct QASJsonType<TYPE<QString, T>> {                                                         \
        enum {                                                                                     \
            Defined = 1,                                                                           \
        };                                                                                         \
                                                                                                   \
        static TYPE<QString, T> fromValue(const QJsonValue &val, bool *ok = nullptr) {             \
            if (!val.isObject()) {                                                                 \
                qasDebug() << #TYPE "<QString, T>: expect object, but get" << val.type();        \
                ok ? (*ok = false) : false;                                                        \
                return {};                                                                         \
            }                                                                                      \
            ok ? (*ok = true) : false;                                                             \
            return fromObject(val.toObject(), ok);                                                 \
        }                                                                                          \
                                                                                                   \
        static QJsonValue toValue(const TYPE<QString, T> &obj) { return toObject(obj); }           \
                                                                                                   \
        static TYPE<QString, T> fromObject(const QJsonObject &obj, bool *ok = nullptr) {           \
            TYPE<QString, T> res;                                                                  \
            bool ok2 = true;                                                                       \
            for (auto it = obj.begin(); it != obj.end(); ++it) {                                   \
                auto tmp = QASJsonType<T>::fromValue(it.value(), &ok2);                            \
                if (!ok2) {                                                                        \
                    qasDebug() << #TYPE "<QString, T>: fail at key" << it.key();                 \
                    res.clear();                                                                   \
                    break;                                                                         \
                }                                                                                  \
                res.insert(std::make_pair(it.key(), tmp));                                         \
            }                                                                                      \
            ok ? (*ok = ok2) : false;                                                              \
            return res;                                                                            \
        }                                                                                          \
                                                                                                   \
        static QJsonObject toObject(const TYPE<QString, T> &obj) {                                 \
            QJsonObject res;                                                                       \
            for (auto it = obj.begin(); it != obj.end(); ++it) {                                   \
                res.insert(it->first, QASJsonType<T>::toValue(it->second));                        \
            }                                                                                      \
            return res;                                                                            \
        };                                                                                         \
    };

#define QAS_JSON_LIST_TYPE_DECLARE(TYPE)                                                           \
    template <class T>                                                                             \
    struct QASJsonType<TYPE<T>> {                                                                  \
        enum {                                                                                     \
            Defined = 1,                                                                           \
        };                                                                                         \
                                                                                                   \
        static TYPE<T> fromValue(const QJsonValue &val, bool *ok = nullptr) {                      \
            if (!val.isArray()) {                                                                  \
                qasDebug() << #TYPE "<T>: expect array, but get" << val.type();                  \
                ok ? (*ok = false) : false;                                                        \
                return {};                                                                         \
            }                                                                                      \
            ok ? (*ok = true) : false;                                                             \
            return fromArray(val.toArray(), ok);                                                   \
        }                                                                                          \
                                                                                                   \
        static QJsonValue toValue(const TYPE<T> &val) { return toArray(val); }                     \
                                                                                                   \
        static TYPE<T> fromArray(const QJsonArray &arr, bool *ok = nullptr) {                      \
            TYPE<T> res;                                                                           \
            bool ok2 = true;                                                                       \
            int idx = 0;                                                                           \
            for (const auto &item : arr) {                                                         \
                auto tmp = QASJsonType<T>::fromValue(item, &ok2);                                  \
                if (!ok2) {                                                                        \
                    qasDebug() << #TYPE "<T>: fail at index" << idx;                             \
                    res.clear();                                                                   \
                    break;                                                                         \
                }                                                                                  \
                res.push_back(tmp);                                                                \
                idx++;                                                                             \
            }                                                                                      \
            ok ? (*ok = ok2) : false;                                                              \
            return res;                                                                            \
        }                                                                                          \
                                                                                                   \
        static QJsonArray toArray(const TYPE<T> &arr) {                                            \
            QJsonArray res;                                                                        \
            for (const auto &item : arr) {                                                         \
                res.append(QASJsonType<T>::toValue(item));                                         \
            }                                                                                      \
            return res;                                                                            \
        };                                                                                         \
    };

#define QAS_JSON_DECLARE_IMPL(TYPE)                                                                \
    template <>                                                                                    \
    struct QASJsonType<TYPE> {                                                                     \
        enum {                                                                                     \
            Defined = 1,                                                                           \
        };                                                                                         \
                                                                                                   \
        static TYPE fromValue(const QJsonValue &val, bool *ok = nullptr) {                         \
            if (!val.isObject()) {                                                                 \
                ok ? (*ok = false) : false;                                                        \
                return {};                                                                         \
            }                                                                                      \
            ok ? (*ok = true) : false;                                                             \
            return fromObject(val.toObject(), ok);                                                 \
        }                                                                                          \
                                                                                                   \
        static QJsonValue toValue(const TYPE &val) { return toObject(val); }                       \
                                                                                                   \
        static TYPE fromObject(const QJsonObject &obj, bool *ok = nullptr);                        \
                                                                                                   \
        static QJsonObject toObject(const TYPE &cls);                                              \
    };

template<>
struct QASJsonType<QJsonValue> {
    enum {
        Defined = 1,
    };

    static QJsonValue fromValue(const QJsonValue &val, bool *ok = nullptr) {
        ok ? (*ok = true) : false;
        return val;
    }

    static QJsonValue toValue(const QJsonValue &val) {
        return val;
    }
};


// Basic Types
QAS_JSON_VALUE_TYPE_DECLARE(double, Double)

QAS_JSON_VALUE_TYPE_DECLARE(float, Double)

QAS_JSON_VALUE_TYPE_DECLARE(qint32, Double)

QAS_JSON_VALUE_TYPE_DECLARE(qint64, Double)

QAS_JSON_VALUE_TYPE_DECLARE(bool, Bool)

QAS_JSON_VALUE_TYPE_DECLARE(QString, String)

QAS_JSON_VALUE_TYPE_DECLARE(QJsonArray, Array)

QAS_JSON_VALUE_TYPE_DECLARE(QJsonObject, Object)

// Map Types
QAS_JSON_QT_MAP_TYPE_DECLARE(QMap)

QAS_JSON_QT_MAP_TYPE_DECLARE(QHash)

QAS_JSON_STL_MAP_TYPE_DECLARE(std::map)

QAS_JSON_STL_MAP_TYPE_DECLARE(std::unordered_map)

// List Types
QAS_JSON_LIST_TYPE_DECLARE(QList)

QAS_JSON_LIST_TYPE_DECLARE(QVector)

QAS_JSON_LIST_TYPE_DECLARE(std::list)

QAS_JSON_LIST_TYPE_DECLARE(std::vector)

#endif // QAS_JSON_TYPES_H