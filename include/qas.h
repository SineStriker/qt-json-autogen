#ifndef QAS_H
#define QAS_H

#include "qas_enum_types.h"
#include "qas_json_types.h"

// ======================================= Json Macros =======================================

#define QAS_JSON_PROPERTY(T)

#define QAS_JSON_IGNORE

#define QAS_JSON_TYPE_DECLARE(TYPE)                                                                \
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
        static QJsonValue toValue(const TYPE &val) {                                               \
            return toObject(val);                                                                  \
        }                                                                                          \
                                                                                                   \
        QAS_JSON_TYPE_DESERIALIZER                                                                 \
        static TYPE fromObject(const QJsonObject &obj, bool *ok = nullptr);                        \
                                                                                                   \
        QAS_JSON_TYPE_SERIALIZER                                                                   \
        static QJsonObject toObject(const TYPE &obj);                                              \
    };



// ======================================= Enum Macros =======================================

#define QAS_ENUM_PROPERTY(T)

#define QAS_ENUM_IGNORE

#define QAS_ENUM_TYPE_DECLARE(TYPE)                                                                \
    template <>                                                                                    \
    struct QASEnumType<TYPE> {                                                                     \
        enum {                                                                                     \
            Defined = 1,                                                                           \
        };                                                                                         \
                                                                                                   \
        QAS_ENUM_TYPE_DESERIALIZER                                                                 \
        static TYPE fromString(const QString &s, bool *ok = nullptr);                              \
                                                                                                   \
        QAS_ENUM_TYPE_SERIALIZER                                                                   \
        static QString toString(TYPE e);                                                           \
    };


#endif // QAS_H
