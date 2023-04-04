#include <QDebug>

#include "common.h"

namespace NS1 {

    struct Foo : Bar {

        using Super = Bar;

        Foo &operator<<(int n);
    };

    Foo &Foo::operator<<(int n) {
        qDebug() << n;
        return *this;
    }

}
