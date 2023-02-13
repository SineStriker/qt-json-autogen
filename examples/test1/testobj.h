#ifndef TESTOBJ_H
#define TESTOBJ_H

#include "qas.h"

#include <set>

namespace NS {

    struct NSClass {
        enum TestNestClass2 {
            Microsoft,
            Apple,
            Google,
        };
    };

    enum NSEnum { Amazon, Facebook, Netflix };

} // namespace NS

enum TestEnum {
    // Member using explict specified property name
    QAS_ATTRIBUTE(alice) Alice,

    // Member using default property name
    Bob,

    // Member ignored
    QAS_IGNORE Mark,
};

QAS_ENUM_DECLARE(TestEnum)

class TestClass {
public:
    TestClass() {
    }

    ~TestClass() {
    }

    class NestClass {
    public:
        QString ZheJiang;
        QString ShangHai;
    };

    enum TestClassEnum {
        Apple,
        Banana,
        Lemon,
    };

public:
    // Member using explict specified property name
    QAS_ATTRIBUTE(alice)
    QString Alice;

    // Member using default property name
    QString Bob;

    // Member ignored
    QAS_IGNORE
    QString Mark;

    std::function<void()> abc;

    QMap<QString, QHash<int, QVariant>> mm;

protected:
    int KK;
};

QAS_JSON_DECLARE(TestClass)

#endif // TESTOBJ_H
