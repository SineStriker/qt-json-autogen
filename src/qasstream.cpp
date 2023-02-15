#include "qasstream.h"

QASStream::QASStream(const qint8 &sc) {
    *this << sc;
}

QASStream::QASStream(const quint8 &c) {
    *this << c;
}

QASStream::QASStream(const qint16 &s) {
    *this << s;
}

QASStream::QASStream(const quint16 &us) {
    *this << us;
}

QASStream::QASStream(const qint32 &i) {
    *this << i;
}

QASStream::QASStream(const quint32 &u) {
    *this << u;
}

QASStream::QASStream(const qint64 &l) {
    *this << l;
}

QASStream::QASStream(const quint64 &ul) {
    *this << ul;
}

QASStream::QASStream(const bool &b) {
    *this << b;
}

QASStream::QASStream(const float &f) {
    *this << f;
}

QASStream::QASStream(const double &d) {
    *this << d;
}

QASStream::QASStream(const char *&s) {
    *this << QString::fromUtf8(s);
}

QASStream::QASStream(const QString &s) {
    *this << s;
}

QASStream::QASStream(const QJsonValue &val) {
    *this << val;
}

QASStream::QASStream(const QJsonArray &arr) {
    *this << arr;
}

QASStream::QASStream(const QJsonObject &obj) {
    *this << obj;
}

QASStream::Status QASStream::status() const {
    return q_status;
}

void QASStream::setStatus(QASStream::Status status) {
    if (q_status == Ok)
        q_status = status;
}

void QASStream::resetStatus() {
    q_status = Ok;
}

#define QASSTREAM_OUTPUT(VAL, TYPE)                                                                \
    setStatus(q_val.is##TYPE() ? (VAL = q_val.to##TYPE(), Ok) : TypeNotMatch);

QASStream &QASStream::operator>>(qint8 &sc) {
    QASSTREAM_OUTPUT(sc, Double);
    return *this;
}

QASStream &QASStream::operator>>(quint8 &c) {
    QASSTREAM_OUTPUT(c, Double);
    return *this;
}

QASStream &QASStream::operator>>(qint16 &s) {
    QASSTREAM_OUTPUT(s, Double);
    return *this;
}

QASStream &QASStream::operator>>(quint16 &us) {
    QASSTREAM_OUTPUT(us, Double);
    return *this;
}

QASStream &QASStream::operator>>(qint32 &i) {
    QASSTREAM_OUTPUT(i, Double);
    return *this;
}

QASStream &QASStream::operator>>(quint32 &u) {
    QASSTREAM_OUTPUT(u, Double);
    return *this;
}

QASStream &QASStream::operator>>(qint64 &l) {
    QASSTREAM_OUTPUT(l, Double);
    return *this;
}

QASStream &QASStream::operator>>(quint64 &ul) {
    QASSTREAM_OUTPUT(ul, Double);
    return *this;
}

QASStream &QASStream::operator>>(bool &b) {
    QASSTREAM_OUTPUT(b, Bool);
    return *this;
}

QASStream &QASStream::operator>>(float &f) {
    QASSTREAM_OUTPUT(f, Double);
    return *this;
}
QASStream &QASStream::operator>>(double &d) {
    QASSTREAM_OUTPUT(d, Double);
    return *this;
}

QASStream &QASStream::operator>>(char *&s) {
    QString tmp;
    QASSTREAM_OUTPUT(tmp, String);
    if (q_status == Ok) {
        auto bytes = tmp.toUtf8();
        s = new char[bytes.size()];
        memcpy(s, bytes.data(), bytes.size());
    }
    return *this;
}

QASStream &QASStream::operator>>(QString &s) {
    QASSTREAM_OUTPUT(s, String);
    return *this;
}

QASStream &QASStream::operator>>(QJsonValue &val) {
    q_val = val;
    return *this;
}

QASStream &QASStream::operator>>(QJsonArray &arr) {
    QASSTREAM_OUTPUT(arr, Array);
    return *this;
}

QASStream &QASStream::operator>>(QJsonObject &obj) {
    QASSTREAM_OUTPUT(obj, Object);
    return *this;
}

#undef QASSTREAM_OUTPUT

QASStream &QASStream::operator<<(qint8 sc) {
    q_val = QJsonValue(sc);
    return *this;
}
QASStream &QASStream::operator<<(quint8 c) {
    q_val = QJsonValue(c);
    return *this;
}
QASStream &QASStream::operator<<(qint16 s) {
    q_val = QJsonValue(s);
    return *this;
}
QASStream &QASStream::operator<<(quint16 us) {
    q_val = QJsonValue(us);
    return *this;
}
QASStream &QASStream::operator<<(qint32 i) {
    q_val = QJsonValue(i);
    return *this;
}
QASStream &QASStream::operator<<(quint32 u) {
    q_val = QJsonValue((qint64) u);
    return *this;
}
QASStream &QASStream::operator<<(qint64 l) {
    q_val = QJsonValue(l);
    return *this;
}
QASStream &QASStream::operator<<(quint64 ul) {
    q_val = QJsonValue((qint64) ul);
    return *this;
}
QASStream &QASStream::operator<<(bool b) {
    q_val = QJsonValue(b);
    return *this;
}
QASStream &QASStream::operator<<(float f) {
    q_val = QJsonValue(f);
    return *this;
}
QASStream &QASStream::operator<<(double d) {
    q_val = QJsonValue(d);
    return *this;
}
QASStream &QASStream::operator<<(const char *s) {
    q_val = QJsonValue(QString::fromUtf8(s));
    return *this;
}
QASStream &QASStream::operator<<(const QString &s) {
    q_val = QJsonValue(s);
    return *this;
}
QASStream &QASStream::operator<<(const QJsonValue &val) {
    q_val = val;
    return *this;
}
QASStream &QASStream::operator<<(const QJsonArray &arr) {
    q_val = arr;
    return *this;
}
QASStream &QASStream::operator<<(const QJsonObject &obj) {
    q_val = obj;
    return *this;
}