#include <QCoreApplication>
#include <QJsonDocument>

#include "testobj.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    // Enumeration serialize
    qDebug() << QASEnumType<TestEnum>::toString(TestEnum::Alice);

    // Enumeration deserialize
    qDebug() << QASEnumType<TestEnum>::fromString("Bob");

    std::set<int> s;

    TestClass foo;
    foo.Alice = "1";
    foo.Bob = "2";
    foo.Mark = "3";

    // Json serialize
    qDebug() << QJsonDocument(QASJsonType<TestClass>::toObject(foo)).toJson();

    return 0;
}