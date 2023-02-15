#ifndef QASSTREAM_H
#define QASSTREAM_H

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

#include <QHash>
#include <QList>
#include <QMap>
#include <QVector>

#include <list>
#include <map>
#include <unordered_map>
#include <vector>

#ifndef QASSTREAM_DISABLE_DEBUG
#define qasDebug2 qDebug
#else
#define qasDebug2 QT_NO_QDEBUG_MACRO
#endif

class QASStream {
public:
    enum Status {
        Ok = 1,
        KeyNotFound = 2,
        TypeNotMatch = 4,
        Success = Ok | KeyNotFound,
        Failed = ~Success,
    };

    QASStream() = default;
    ~QASStream() = default;

    QASStream(const qint8 &sc);
    QASStream(const quint8 &c);
    QASStream(const qint16 &s);
    QASStream(const quint16 &us);
    QASStream(const qint32 &i);
    QASStream(const quint32 &u);
    QASStream(const qint64 &l);
    QASStream(const quint64 &ul);
    QASStream(const bool &b);
    QASStream(const float &f);
    QASStream(const double &d);
    QASStream(const char *&s);
    QASStream(const QString &s);
    QASStream(const QJsonValue &val);
    QASStream(const QJsonArray &arr);
    QASStream(const QJsonObject &obj);

    inline QJsonValue data() const {
        return q_val;
    };

    Status status() const;
    void setStatus(Status status);
    void resetStatus();

public:
    QASStream &operator>>(qint8 &sc);
    QASStream &operator>>(quint8 &c);
    QASStream &operator>>(qint16 &s);
    QASStream &operator>>(quint16 &us);
    QASStream &operator>>(qint32 &i);
    QASStream &operator>>(quint32 &u);
    QASStream &operator>>(qint64 &l);
    QASStream &operator>>(quint64 &ul);
    QASStream &operator>>(bool &b);
    QASStream &operator>>(float &f);
    QASStream &operator>>(double &d);
    QASStream &operator>>(char *&s);
    QASStream &operator>>(QString &s);
    QASStream &operator>>(QJsonValue &val);
    QASStream &operator>>(QJsonArray &arr);
    QASStream &operator>>(QJsonObject &obj);

    QASStream &operator<<(qint8 sc);
    QASStream &operator<<(quint8 c);
    QASStream &operator<<(qint16 s);
    QASStream &operator<<(quint16 us);
    QASStream &operator<<(qint32 i);
    QASStream &operator<<(quint32 u);
    QASStream &operator<<(qint64 l);
    QASStream &operator<<(quint64 ul);
    QASStream &operator<<(bool b);
    QASStream &operator<<(float f);
    QASStream &operator<<(double d);
    QASStream &operator<<(const char *s);
    QASStream &operator<<(const QString &s);
    QASStream &operator<<(const QJsonValue &val);
    QASStream &operator<<(const QJsonArray &arr);
    QASStream &operator<<(const QJsonObject &obj);

protected:
    QJsonValue q_val;

    Status q_status;
};

#define QASSTREAM_REGISTER_LIST(LIST)                                                              \
    template <class T>                                                                             \
    QASStream &operator>>(QASStream &stream, LIST<T> &list) {                                      \
        const auto &data = stream.data();                                                          \
        if (!data.isArray()) {                                                                     \
            qasDebug2() << #LIST "<T>: expect array, but get" << data.type();                      \
            stream.setStatus(QASStream::TypeNotMatch);                                             \
            return stream;                                                                         \
        }                                                                                          \
                                                                                                   \
        QJsonArray arr = data.toArray();                                                           \
        for (const auto &item : qAsConst(arr)) {                                                   \
            T tmp;                                                                                 \
            QASStream tmpStream(item);                                                             \
            tmpStream >> tmp;                                                                      \
            if (tmpStream.status() & QASStream::Failed) {                                          \
                qasDebug2() << #LIST "<T>: fail at index" << list.size();                          \
                stream.setStatus(tmpStream.status());                                              \
                break;                                                                             \
            }                                                                                      \
            list.append(tmp);                                                                      \
        }                                                                                          \
        return stream;                                                                             \
    }                                                                                              \
                                                                                                   \
    template <class T>                                                                             \
    QASStream &operator<<(QASStream &stream, const LIST<T> &list) {                                \
        QJsonArray arr;                                                                            \
        for (const auto &item : qAsConst(list)) {                                                  \
            QASStream tmpStream;                                                                   \
            tmpStream << item;                                                                     \
            arr += tmpStream.data();                                                               \
        }                                                                                          \
        stream << arr;                                                                             \
        return stream;                                                                             \
    }

#define QASSTREAM_REGISTER_MAP(MAP)                                                                \
    template <class T>                                                                             \
    QASStream &operator>>(QASStream &stream, MAP<QString, T> &map) {                               \
        const auto &data = stream.data();                                                          \
        if (!data.isObject()) {                                                                    \
            qasDebug2() << #MAP "<QString, T>: expect object, but get" << data.type();             \
            stream.setStatus(QASStream::TypeNotMatch);                                             \
            return stream;                                                                         \
        }                                                                                          \
                                                                                                   \
        QJsonObject obj = data.toObject();                                                         \
        for (auto it = obj.begin(); it != obj.end(); ++it) {                                       \
            T tmp;                                                                                 \
            QASStream tmpStream(it.value());                                                       \
            tmpStream >> tmp;                                                                      \
            if (tmpStream.status() & QASStream::Failed) {                                          \
                qasDebug2() << #MAP "<QString, T>: fail at key" << it.key();                       \
                stream.setStatus(tmpStream.status());                                              \
                break;                                                                             \
            }                                                                                      \
            map.insert(it.key(), tmp);                                                             \
        }                                                                                          \
        return stream;                                                                             \
    }                                                                                              \
                                                                                                   \
    template <class T>                                                                             \
    QASStream &operator<<(QASStream &stream, const MAP<QString, T> &map) {                         \
        QJsonObject obj;                                                                           \
        for (auto it = map.begin(); it != map.end(); ++it) {                                       \
            QASStream tmpStream;                                                                   \
            tmpStream << it.value();                                                               \
            obj.insert(it.key(), tmpStream.data());                                                \
        }                                                                                          \
        stream << obj;                                                                             \
        return stream;                                                                             \
    }

#define QASSTREAM_DECLARE_CLASS_IMPL(TYPE)                                                         \
    friend QASStream &operator>>(QASStream &stream, TYPE &c);                                      \
    friend QASStream &operator<<(QASStream &stream, const TYPE &c);

#define QASSTREAM_DECLARE_ENUM_IMPL(TYPE)                                                          \
    QASStream &operator>>(QASStream &stream, TYPE &e);                                             \
    QASStream &operator<<(QASStream &stream, TYPE e);

#ifdef QASC_RUN
#define QAS_ATTRIBUTE(T) QAS_ATTRIBUTE(T)
#define QAS_EXCLUDE QAS_EXCLUDE
#define QAS_INCLUDE QAS_INCLUDE
#define QAS_DECLARE_CLASS(T, ...) QAS_DECLARE_CLASS(T, ...)
#define QAS_DECLARE_ENUM(T) QAS_DECLARE_CLASS(T)
#else
#define QAS_ATTRIBUTE(T)
#define QAS_EXCLUDE
#define QAS_INCLUDE
#define QAS_DECLARE_CLASS(T) QASSTREAM_DECLARE_CLASS_IMPL(T)
#define QAS_DECLARE_ENUM(T) QASSTREAM_DECLARE_ENUM_IMPL(T)
#endif

QASSTREAM_REGISTER_LIST(QList)
QASSTREAM_REGISTER_LIST(QVector)
QASSTREAM_REGISTER_MAP(QMap)
QASSTREAM_REGISTER_MAP(QHash)

#endif // QASSTREAM_H
