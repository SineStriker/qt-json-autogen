#ifndef QAS_JSON_TYPES_H
#define QAS_JSON_TYPES_H

#include <QJsonArray>
#include <QJsonObject>

#include <QHash>
#include <QMap>

#include <map>
#include <unordered_map>

#include "qas_basic.h"

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
                QAS_SET_OK(ok, false);                                                             \
                return TYPE{};                                                                     \
            }                                                                                      \
            QAS_SET_OK(ok, true);                                                                  \
            return val.to##IDENTIFIER();                                                           \
        }                                                                                          \
                                                                                                   \
        static QJsonValue toValue(const TYPE &val) {                                               \
            return QJsonValue(val);                                                                \
        }                                                                                          \
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
                QAS_SET_OK(ok, false);                                                             \
                return TYPE{};                                                                     \
            }                                                                                      \
            QAS_SET_OK(ok, true);                                                                  \
            return tmp.value<TYPE>();                                                              \
        }                                                                                          \
                                                                                                   \
        static QJsonValue toValue(const TYPE &val) {                                               \
            return QJsonValue(val);                                                                \
        }                                                                                          \
    };

#define QAS_JSON_QT_TEMPLATE_MAP_DECLARE(TYPE)                                                     \
    template <class T>                                                                             \
    struct QASJsonType<TYPE<QString, T>> {                                                         \
        enum {                                                                                     \
            Defined = 1,                                                                           \
        };                                                                                         \
                                                                                                   \
        static TYPE<QString, T> fromValue(const QJsonValue &val, bool *ok = nullptr) {             \
            if (!val.isObject()) {                                                                 \
                qasDebug() << #TYPE "<QString, T>: expect object, but get" << val.type();          \
                QAS_SET_OK(ok, false);                                                             \
                return {};                                                                         \
            }                                                                                      \
            QAS_SET_OK(ok, true);                                                                  \
            return fromObject(val.toObject(), ok);                                                 \
        }                                                                                          \
                                                                                                   \
        static QJsonValue toValue(const TYPE<QString, T> &val) {                                   \
            return toObject(val);                                                                  \
        }                                                                                          \
                                                                                                   \
        static TYPE<QString, T> fromObject(const QJsonObject &obj, bool *ok = nullptr) {           \
            TYPE<QString, T> res;                                                                  \
            bool ok2 = true;                                                                       \
            for (auto it = obj.begin(); it != obj.end(); ++it) {                                   \
                auto tmp = QASJsonType<T>::fromValue(it.value(), &ok2);                            \
                if (!ok2) {                                                                        \
                    qasDebug() << #TYPE "<QString, T>: fail at key" << it.key();                   \
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

#define QAS_JSON_QT_SPECIFIED_MAP_DECLARE(TYPE)                                                    \
    template <>                                                                                    \
    struct QASJsonType<TYPE> {                                                                     \
        enum {                                                                                     \
            Defined = 1,                                                                           \
        };                                                                                         \
                                                                                                   \
        static TYPE fromValue(const QJsonValue &val, bool *ok = nullptr) {                         \
            if (!val.isObject()) {                                                                 \
                qasDebug() << #TYPE ": expect object, but get" << val.type();                      \
                QAS_SET_OK(ok, false);                                                             \
                return {};                                                                         \
            }                                                                                      \
            QAS_SET_OK(ok, true);                                                                  \
            return fromObject(val.toObject(), ok);                                                 \
        }                                                                                          \
                                                                                                   \
        static QJsonValue toValue(const TYPE &val) {                                               \
            return toObject(val);                                                                  \
        }                                                                                          \
                                                                                                   \
        static TYPE fromObject(const QJsonObject &obj, bool *ok = nullptr) {                       \
            TYPE res;                                                                              \
            bool ok2 = true;                                                                       \
            for (auto it = obj.begin(); it != obj.end(); ++it) {                                   \
                auto tmp = QASJsonType<TYPE::mapped_type>::fromValue(it.value(), &ok2);            \
                if (!ok2) {                                                                        \
                    qasDebug() << #TYPE ": fail at key" << it.key();                               \
                    res.clear();                                                                   \
                    break;                                                                         \
                }                                                                                  \
                res.insert(it.key(), tmp);                                                         \
            }                                                                                      \
            ok ? (*ok = ok2) : false;                                                              \
            return res;                                                                            \
        }                                                                                          \
                                                                                                   \
        static QJsonObject toObject(const TYPE &obj) {                                             \
            QJsonObject res;                                                                       \
            for (auto it = obj.begin(); it != obj.end(); ++it) {                                   \
                res.insert(it.key(), QASJsonType<TYPE::mapped_type>::toValue(it.value()));         \
            }                                                                                      \
            return res;                                                                            \
        };                                                                                         \
    };

#define QAS_JSON_STL_TEMPLATE_MAP_DECLARE(TYPE)                                                    \
    template <class T>                                                                             \
    struct QASJsonType<TYPE<QString, T>> {                                                         \
        enum {                                                                                     \
            Defined = 1,                                                                           \
        };                                                                                         \
                                                                                                   \
        static TYPE<QString, T> fromValue(const QJsonValue &val, bool *ok = nullptr) {             \
            if (!val.isObject()) {                                                                 \
                qasDebug() << #TYPE "<QString, T>: expect object, but get" << val.type();          \
                QAS_SET_OK(ok, false);                                                             \
                return {};                                                                         \
            }                                                                                      \
            QAS_SET_OK(ok, true);                                                                  \
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
                    qasDebug() << #TYPE "<QString, T>: fail at key" << it.key();                   \
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

#define QAS_JSON_STL_SPECIFIED_MAP_DECLARE(TYPE)                                                   \
    template <>                                                                                    \
    struct QASJsonType<TYPE> {                                                                     \
        enum {                                                                                     \
            Defined = 1,                                                                           \
        };                                                                                         \
                                                                                                   \
        static TYPE fromValue(const QJsonValue &val, bool *ok = nullptr) {                         \
            if (!val.isObject()) {                                                                 \
                qasDebug() << #TYPE ": expect object, but get" << val.type();                      \
                QAS_SET_OK(ok, false);                                                             \
                return {};                                                                         \
            }                                                                                      \
            QAS_SET_OK(ok, true);                                                                  \
            return fromObject(val.toObject(), ok);                                                 \
        }                                                                                          \
                                                                                                   \
        static QJsonValue toValue(const TYPE &obj) {                                               \
            return toObject(obj);                                                                  \
        }                                                                                          \
                                                                                                   \
        static TYPE fromObject(const QJsonObject &obj, bool *ok = nullptr) {                       \
            TYPE res;                                                                              \
            bool ok2 = true;                                                                       \
            for (auto it = obj.begin(); it != obj.end(); ++it) {                                   \
                auto tmp = QASJsonType<TYPE::mapped_type>::fromValue(it.value(), &ok2);            \
                if (!ok2) {                                                                        \
                    qasDebug() << #TYPE ": fail at key" << it.key();                               \
                    res.clear();                                                                   \
                    break;                                                                         \
                }                                                                                  \
                res.insert(std::make_pair(it.key(), tmp));                                         \
            }                                                                                      \
            ok ? (*ok = ok2) : false;                                                              \
            return res;                                                                            \
        }                                                                                          \
                                                                                                   \
        static QJsonObject toObject(const TYPE &obj) {                                             \
            QJsonObject res;                                                                       \
            for (auto it = obj.begin(); it != obj.end(); ++it) {                                   \
                res.insert(it->first, QASJsonType<TYPE::mapped_type>::toValue(it->second));        \
            }                                                                                      \
            return res;                                                                            \
        };                                                                                         \
    };

#define QAS_JSON_TEMPLATE_LIST_DECLARE(TYPE)                                                       \
    template <class T>                                                                             \
    struct QASJsonType<TYPE<T>> {                                                                  \
        enum {                                                                                     \
            Defined = 1,                                                                           \
        };                                                                                         \
                                                                                                   \
        static TYPE<T> fromValue(const QJsonValue &val, bool *ok = nullptr) {                      \
            if (!val.isArray()) {                                                                  \
                qasDebug() << #TYPE "<T>: expect array, but get" << val.type();                    \
                QAS_SET_OK(ok, false);                                                             \
                return {};                                                                         \
            }                                                                                      \
            QAS_SET_OK(ok, true);                                                                  \
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
            int idx = 0;                                                                           \
            for (const auto &item : arr) {                                                         \
                auto tmp = QASJsonType<T>::fromValue(item, &ok2);                                  \
                if (!ok2) {                                                                        \
                    qasDebug() << #TYPE "<T>: fail at index" << idx;                               \
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

#define QAS_JSON_SPECIFIED_LIST_DECLARE(TYPE)                                                      \
    template <>                                                                                    \
    struct QASJsonType<TYPE> {                                                                     \
        enum {                                                                                     \
            Defined = 1,                                                                           \
        };                                                                                         \
                                                                                                   \
        static TYPE fromValue(const QJsonValue &val, bool *ok = nullptr) {                         \
            if (!val.isArray()) {                                                                  \
                qasDebug() << #TYPE ": expect array, but get" << val.type();                       \
                QAS_SET_OK(ok, false);                                                             \
                return {};                                                                         \
            }                                                                                      \
            QAS_SET_OK(ok, true);                                                                  \
            return fromArray(val.toArray(), ok);                                                   \
        }                                                                                          \
                                                                                                   \
        static QJsonValue toValue(const TYPE &val) {                                               \
            return toArray(val);                                                                   \
        }                                                                                          \
                                                                                                   \
        static TYPE fromArray(const QJsonArray &arr, bool *ok = nullptr) {                         \
            TYPE res;                                                                              \
            bool ok2 = true;                                                                       \
            int idx = 0;                                                                           \
            for (const auto &item : arr) {                                                         \
                auto tmp = QASJsonType<TYPE::value_type>::fromValue(item, &ok2);                   \
                if (!ok2) {                                                                        \
                    qasDebug() << #TYPE ": fail at index" << idx;                                  \
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
        static QJsonArray toArray(const TYPE &arr) {                                               \
            QJsonArray res;                                                                        \
            for (const auto &item : arr) {                                                         \
                res.append(QASJsonType<TYPE::value_type>::toValue(item));                          \
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
                QAS_SET_OK(ok, false);                                                             \
                return {};                                                                         \
            }                                                                                      \
            QAS_SET_OK(ok, true);                                                                  \
            return fromObject(val.toObject(), ok);                                                 \
        }                                                                                          \
                                                                                                   \
        static QJsonValue toValue(const TYPE &val) {                                               \
            return toObject(val);                                                                  \
        }                                                                                          \
                                                                                                   \
        static TYPE fromObject(const QJsonObject &obj, bool *ok = nullptr);                        \
                                                                                                   \
        static QJsonObject toObject(const TYPE &cls);                                              \
    };

template <>
struct QASJsonType<QJsonValue> {
    enum {
        Defined = 1,
    };

    static QJsonValue fromValue(const QJsonValue &val, bool *ok = nullptr) {
        QAS_SET_OK(ok, true);
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
QAS_JSON_QT_TEMPLATE_MAP_DECLARE(QMap)

QAS_JSON_QT_TEMPLATE_MAP_DECLARE(QHash)

QAS_JSON_STL_TEMPLATE_MAP_DECLARE(std::map)

QAS_JSON_STL_TEMPLATE_MAP_DECLARE(std::unordered_map)

// List Types
QAS_JSON_TEMPLATE_LIST_DECLARE(QList)

#if QT_VERSION_MAJON <= 5 // Qt 6 treat QVector as QList
QAS_JSON_TEMPLATE_LIST_DECLARE(QVector)
#endif

QAS_JSON_TEMPLATE_LIST_DECLARE(std::list)

QAS_JSON_TEMPLATE_LIST_DECLARE(std::vector)

// Specified List Types

#if QT_VERSION_MAJON <= 5 // Qt 6 treat QStringList as QList<QString>
QAS_JSON_SPECIFIED_LIST_DECLARE(QStringList)
#endif

// Specified Map Types
// ...

#endif // QAS_JSON_TYPES_H
