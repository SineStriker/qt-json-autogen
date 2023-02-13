#ifndef QAS_JSON_TYPES_H
#define QAS_JSON_TYPES_H

#include <QJsonArray>
#include <QJsonObject>

#include <QHash>
#include <QMap>

#include <map>
#include <unordered_map>

#define QAS_JSON_TYPE_SERIALIZER

#define QAS_JSON_TYPE_DESERIALIZER

template <class T>
struct QASJsonTypeDefault {
    enum {
        Defined = 0,
    };
};

template <class T>
struct QASJsonType : public QASJsonTypeDefault<T> //
{                                                 //
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
                ok ? (*ok = false) : false;                                                        \
                return TYPE{};                                                                     \
            }                                                                                      \
            ok ? (*ok = true) : false;                                                             \
            return tmp.value<TYPE>();                                                              \
        }                                                                                          \
                                                                                                   \
        static QJsonValue toValue(const TYPE &val) {                                               \
            return QJsonValue(val);                                                                \
        }                                                                                          \
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
                ok ? (*ok = false) : false;                                                        \
                return {};                                                                         \
            }                                                                                      \
            ok ? (*ok = true) : false;                                                             \
            return fromObject(val.toObject(), ok);                                                 \
        }                                                                                          \
                                                                                                   \
        static QJsonValue toValue(const TYPE<QString, T> &val) {                                   \
            return toObject(val);                                                                  \
        }                                                                                          \
                                                                                                   \
        static TYPE<QString, T> fromObject(const QJsonObject &obj, bool *ok = nullptr) {           \
            TYPE<QString, T> res;                                                                  \
            for (auto it = obj.begin(); it != obj.end(); ++it) {                                   \
                auto tmp = QASJsonType<T>::fromValue(it.value(), ok);                              \
                if (ok && !(*ok)) {                                                                \
                    return {};                                                                     \
                }                                                                                  \
                res.insert(it.key(), tmp);                                                         \
            }                                                                                      \
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
                ok ? (*ok = false) : false;                                                        \
                return {};                                                                         \
            }                                                                                      \
            ok ? (*ok = true) : false;                                                             \
            return fromObject(val.toObject(), ok);                                                 \
        }                                                                                          \
                                                                                                   \
        static QJsonValue toValue(const TYPE<QString, T> &obj) {                                   \
            return toObject(obj);                                                                  \
        }                                                                                          \
                                                                                                   \
        static TYPE<QString, T> fromObject(const QJsonObject &obj, bool *ok = nullptr) {           \
            TYPE<QString, T> res;                                                                  \
            bool ok2 = true;                                                                       \
            for (auto it = obj.begin(); it != obj.end(); ++it) {                                   \
                auto tmp = QASJsonType<T>::fromValue(it.value(), &ok2);                            \
                if (!ok2) {                                                                        \
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
                ok ? (*ok = false) : false;                                                        \
                return {};                                                                         \
            }                                                                                      \
            ok ? (*ok = true) : false;                                                             \
            return fromArray(val.toArray(), ok);                                                   \
        }                                                                                          \
                                                                                                   \
        static QJsonValue toValue(const TYPE<T> &val) {                                            \
            return toArray(val);                                                                   \
        }                                                                                          \
                                                                                                   \
        static TYPE<T> fromArray(const QJsonArray &arr, bool *ok = nullptr) {                      \
            TYPE<T> res;                                                                           \
            bool ok2 = true;                                                                       \
            for (const auto &item : arr) {                                                         \
                auto tmp = QASJsonType<T>::fromValue(item, &ok2);                                  \
                if (!ok2) {                                                                        \
                    res.clear();                                                                   \
                    break;                                                                         \
                }                                                                                  \
                res.push_back(tmp);                                                                \
            }                                                                                      \
            ok ? (*ok = ok2) : false;                                                              \
            return res;                                                                            \
        }                                                                                          \
                                                                                                   \
        static QJsonArray toArray(const TYPE<T> &arr) {                                            \
            QJsonArray res;                                                                        \
            for (const auto &item : arr) {                                                         \
                res.insert(item);                                                                  \
            }                                                                                      \
            return res;                                                                            \
        };                                                                                         \
    };


// Basic Types
QAS_JSON_QT_BASIC_TYPE_DECLARE(bool)

QAS_JSON_QT_BASIC_TYPE_DECLARE(double)

QAS_JSON_QT_BASIC_TYPE_DECLARE(qint32)

QAS_JSON_QT_BASIC_TYPE_DECLARE(qint64)

QAS_JSON_QT_BASIC_TYPE_DECLARE(QString)

QAS_JSON_QT_BASIC_TYPE_DECLARE(QJsonValue)

QAS_JSON_QT_BASIC_TYPE_DECLARE(QJsonArray)

QAS_JSON_QT_BASIC_TYPE_DECLARE(QJsonObject)

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