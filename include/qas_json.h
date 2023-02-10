#ifndef QAS_GLOBAL_H
#define QAS_GLOBAL_H

#include <QJsonObject>

#define QAS_JSON(T)                                                                                \
    static T fromJsonObject(const QJsonObject &obj, bool *ok = nullptr);                           \
    QJsonObject toJsonObject() const;

#define QAS_JSON2(T, DESERIALIZE, SERIALIZE)                                                       \
    static T DESERIALIZE(const QJsonObject &obj, bool *ok = nullptr);                              \
    QJsonObject SERIALIZE() const;

#endif // QAS_GLOBAL_H