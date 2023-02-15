#include "qasstream.h"

class Base {
    QAS_DECLARE_CLASS(Base)
public:
    Base();

    QString name;
    int age;
};

QASStream &operator>>(QASStream &stream, Base &c) {
    return stream;
}

QASStream &operator<<(QASStream &stream, const Base &c) {
    return stream;
}