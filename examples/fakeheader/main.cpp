#include <QCoreApplication>

#include "real.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    NS1::Foo foo;
    foo << 1;

    return 0;
}