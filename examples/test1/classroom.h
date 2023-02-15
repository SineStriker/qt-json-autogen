#ifndef EXAMPLES_CLASSROOM_H
#define EXAMPLES_CLASSROOM_H

#include <QList>
#include <QString>

#include "qas.h"

#include <set>

namespace School {

    class Classroom {
    public:
        class Student {
        public:
            Student() = default;

            enum Gender {
                Male,
                Female,
            };

            QString name;
            Gender gender;
            int height;
        };

        class SmallStudent : public Student {
        public:
            int iq;
        };

        Classroom() = default;

        QString className;
        QString slogan;
        QList<Student> students;
    };

    class BigClassroom : public Classroom {
    public:
        QString address;
    };
}; // namespace School


namespace A {
    namespace B {

        template <class A, class B>
        class Pair {
        public:
            A a;
            B b;
        };


        namespace C {
            namespace F = B;
        }
    } // namespace B
} // namespace A

namespace A {
    namespace E = School;
    namespace B {
        namespace C {
            namespace D = E;
        };
    } // namespace B
} // namespace A

class CA : public A::B::C::D::Classroom::Student {
public:
    QString str;
};

QAS_ENUM_DECLARE(A::B::C::D::Classroom::Student::Gender)

QAS_JSON_DECLARE(A::B::C::D::Classroom::Student)

QAS_JSON_DECLARE(A::B::C::D::Classroom::SmallStudent)

QAS_JSON_DECLARE(School::Classroom)

QAS_JSON_DECLARE(School::BigClassroom)

typedef A::B::C::F::Pair<int, QString> NestPair;

QAS_JSON_DECLARE(NestPair)

QAS_JSON_DECLARE(CA)

#endif // EXAMPLES_CLASSROOM_H
