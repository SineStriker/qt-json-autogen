#ifndef QAS_ENUM_TYPES_H
#define QAS_ENUM_TYPES_H

#include <QMetaEnum>

#include "qas_json_types.h"

template <class T>
struct QASEnumTypeDefault {
    enum {
        Defined = 0,
    };
};

template <class T>
struct QASEnumType : public QASEnumTypeDefault<T> //
{                                                 //
};

#define QAS_ENUM_JSON_DECLARE_IMPL(TYPE)                                                           \
    template <>                                                                                    \
    struct QASJsonType<TYPE> {                                                                     \
        enum {                                                                                     \
            Defined = 1,                                                                           \
        };                                                                                         \
                                                                                                   \
        static TYPE fromValue(const QJsonValue &val, bool *ok = nullptr) {                         \
            if (val.isNull()) {                                                                    \
                return TYPE{};                                                                     \
            } else if (!val.isString()) {                                                          \
                QAS_SET_OK(ok, false);                                                             \
                return TYPE{};                                                                     \
            }                                                                                      \
            return QASEnumType<TYPE>::fromString(val.toString(), ok);                              \
        }                                                                                          \
                                                                                                   \
        static QJsonValue toValue(const TYPE &val) {                                               \
            return QJsonValue(QASEnumType<TYPE>::toString(val));                                   \
        }                                                                                          \
    };

#define QAS_ENUM_DECLARE_IMPL(TYPE)                                                                \
    template <>                                                                                    \
    struct QASEnumType<TYPE> {                                                                     \
        enum {                                                                                     \
            Defined = 1,                                                                           \
        };                                                                                         \
                                                                                                   \
        static TYPE fromString(const QString &s, bool *ok = nullptr);                              \
                                                                                                   \
        static QString toString(TYPE e);                                                           \
    };                                                                                             \
                                                                                                   \
    QAS_ENUM_JSON_DECLARE_IMPL(TYPE)

#endif // QAS_ENUM_TYPES_H
