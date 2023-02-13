#ifndef QAS_H
#define QAS_H

#include "qas_enum_types.h"
#include "qas_json_types.h"

#ifndef QT_AUTO_SERIALIZATION_COMPILER_RUN

#define QAS_ATTRIBUTE(ATTR)

#define QAS_IGNORE

#define QAS_JSON_DECLARE(TYPE)                                                                     \
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
        static TYPE fromObject(const QJsonObject &obj, bool *ok = nullptr);                        \
                                                                                                   \
        static QJsonObject toObject(const TYPE &obj);                                              \
    };


#define QAS_ENUM_DECLARE(TYPE)                                                                     \
    template <>                                                                                    \
    struct QASEnumType<TYPE> {                                                                     \
        enum {                                                                                     \
            Defined = 1,                                                                           \
        };                                                                                         \
                                                                                                   \
        static TYPE fromString(const QString &s, bool *ok = nullptr);                              \
                                                                                                   \
        static QString toString(TYPE e);                                                           \
    };

#else
#define QAS_ATTRIBUTE(ATTR) QAS_ATTRIBUTE(ATTR)
#define QAS_IGNORE QAS_IGNORE
#define QAS_JSON_DECLARE(TYPE) QAS_JSON_DECLARE(TYPE)
#define QAS_ENUM_DECLARE(TYPE) QAS_ENUM_DECLARE(TYPE)
#endif


#endif // QAS_H
