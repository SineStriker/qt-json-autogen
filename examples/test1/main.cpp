#include <QCoreApplication>
#include <QJsonDocument>

#include "qas.h"

#include <set>

enum TestEnum {
    // Member using explict specified property name
    QAS_ENUM_PROPERTY(alice) Alice,

    // Member using default property name
    Bob,

    // Member ignored
    QAS_ENUM_IGNORE Mark,
};

QAS_ENUM_TYPE_DECLARE(TestEnum)

class TestClass {
public:
    TestClass() {
    }

    ~TestClass() {
    }

public:
    // Member using explict specified property name
    QAS_JSON_PROPERTY(alice)
    QString Alice;

    // Member using default property name
    QString Bob;

    // Member ignored
    QAS_JSON_IGNORE
    QString Mark;
};

QAS_JSON_TYPE_DECLARE(TestClass)

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