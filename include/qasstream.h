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

// Implementations

inline QASStream::QASStream(const qint8 &sc) {
    *this << sc;
}

inline QASStream::QASStream(const quint8 &c) {
    *this << c;
}

inline QASStream::QASStream(const qint16 &s) {
    *this << s;
}

inline QASStream::QASStream(const quint16 &us) {
    *this << us;
}

inline QASStream::QASStream(const qint32 &i) {
    *this << i;
}

inline QASStream::QASStream(const quint32 &u) {
    *this << u;
}

inline QASStream::QASStream(const qint64 &l) {
    *this << l;
}

inline QASStream::QASStream(const quint64 &ul) {
    *this << ul;
}

inline QASStream::QASStream(const bool &b) {
    *this << b;
}

inline QASStream::QASStream(const float &f) {
    *this << f;
}

inline QASStream::QASStream(const double &d) {
    *this << d;
}

inline QASStream::QASStream(const char *&s) {
    *this << QString::fromUtf8(s);
}

inline QASStream::QASStream(const QString &s) {
    *this << s;
}

inline QASStream::QASStream(const QJsonValue &val) {
    *this << val;
}

inline QASStream::QASStream(const QJsonArray &arr) {
    *this << arr;
}

inline QASStream::QASStream(const QJsonObject &obj) {
    *this << obj;
}

inline QASStream::Status QASStream::status() const {
    return q_status;
}

inline void QASStream::setStatus(QASStream::Status status) {
    if (q_status == Ok)
        q_status = status;
}

inline void QASStream::resetStatus() {
    q_status = Ok;
}

#define QASSTREAM_OUTPUT(VAL, TYPE)                                                                \
    setStatus(q_val.is##TYPE() ? (VAL = q_val.to##TYPE(), Ok) : TypeNotMatch);

inline QASStream &QASStream::operator>>(qint8 &sc) {
    QASSTREAM_OUTPUT(sc, Double);
    return *this;
}

inline QASStream &QASStream::operator>>(quint8 &c) {
    QASSTREAM_OUTPUT(c, Double);
    return *this;
}

inline QASStream &QASStream::operator>>(qint16 &s) {
    QASSTREAM_OUTPUT(s, Double);
    return *this;
}

inline QASStream &QASStream::operator>>(quint16 &us) {
    QASSTREAM_OUTPUT(us, Double);
    return *this;
}

inline QASStream &QASStream::operator>>(qint32 &i) {
    QASSTREAM_OUTPUT(i, Double);
    return *this;
}

inline QASStream &QASStream::operator>>(quint32 &u) {
    QASSTREAM_OUTPUT(u, Double);
    return *this;
}

inline QASStream &QASStream::operator>>(qint64 &l) {
    QASSTREAM_OUTPUT(l, Double);
    return *this;
}

inline QASStream &QASStream::operator>>(quint64 &ul) {
    QASSTREAM_OUTPUT(ul, Double);
    return *this;
}

inline QASStream &QASStream::operator>>(bool &b) {
    QASSTREAM_OUTPUT(b, Bool);
    return *this;
}

inline QASStream &QASStream::operator>>(float &f) {
    QASSTREAM_OUTPUT(f, Double);
    return *this;
}
inline QASStream &QASStream::operator>>(double &d) {
    QASSTREAM_OUTPUT(d, Double);
    return *this;
}

inline QASStream &QASStream::operator>>(char *&s) {
    QString tmp;
    QASSTREAM_OUTPUT(tmp, String);
    if (q_status == Ok) {
        auto bytes = tmp.toUtf8();
        s = new char[bytes.size()];
        memcpy(s, bytes.data(), bytes.size());
    }
    return *this;
}

inline QASStream &QASStream::operator>>(QString &s) {
    QASSTREAM_OUTPUT(s, String);
    return *this;
}

inline QASStream &QASStream::operator>>(QJsonValue &val) {
    q_val = val;
    return *this;
}

inline QASStream &QASStream::operator>>(QJsonArray &arr) {
    QASSTREAM_OUTPUT(arr, Array);
    return *this;
}

inline QASStream &QASStream::operator>>(QJsonObject &obj) {
    QASSTREAM_OUTPUT(obj, Object);
    return *this;
}

#undef QASSTREAM_OUTPUT

inline QASStream &QASStream::operator<<(qint8 sc) {
    q_val = QJsonValue(sc);
    return *this;
}
inline QASStream &QASStream::operator<<(quint8 c) {
    q_val = QJsonValue(c);
    return *this;
}
inline QASStream &QASStream::operator<<(qint16 s) {
    q_val = QJsonValue(s);
    return *this;
}
inline QASStream &QASStream::operator<<(quint16 us) {
    q_val = QJsonValue(us);
    return *this;
}
inline QASStream &QASStream::operator<<(qint32 i) {
    q_val = QJsonValue(i);
    return *this;
}
inline QASStream &QASStream::operator<<(quint32 u) {
    q_val = QJsonValue((qint64) u);
    return *this;
}
inline QASStream &QASStream::operator<<(qint64 l) {
    q_val = QJsonValue(l);
    return *this;
}
inline QASStream &QASStream::operator<<(quint64 ul) {
    q_val = QJsonValue((qint64) ul);
    return *this;
}
inline QASStream &QASStream::operator<<(bool b) {
    q_val = QJsonValue(b);
    return *this;
}
inline QASStream &QASStream::operator<<(float f) {
    q_val = QJsonValue(f);
    return *this;
}
inline QASStream &QASStream::operator<<(double d) {
    q_val = QJsonValue(d);
    return *this;
}
inline QASStream &QASStream::operator<<(const char *s) {
    q_val = QJsonValue(QString::fromUtf8(s));
    return *this;
}
inline QASStream &QASStream::operator<<(const QString &s) {
    q_val = QJsonValue(s);
    return *this;
}
inline QASStream &QASStream::operator<<(const QJsonValue &val) {
    q_val = val;
    return *this;
}
inline QASStream &QASStream::operator<<(const QJsonArray &arr) {
    q_val = arr;
    return *this;
}
inline QASStream &QASStream::operator<<(const QJsonObject &obj) {
    q_val = obj;
    return *this;
}

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
#define QAS_DECLARE_CLASS(T, ...) QASSTREAM_DECLARE_CLASS_IMPL(T)
#define QAS_DECLARE_ENUM(T) QASSTREAM_DECLARE_ENUM_IMPL(T)
#endif

QASSTREAM_REGISTER_LIST(QList)
QASSTREAM_REGISTER_LIST(QVector)
QASSTREAM_REGISTER_MAP(QMap)
QASSTREAM_REGISTER_MAP(QHash)

#endif // QASSTREAM_H
