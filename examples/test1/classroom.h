#ifndef EXAMPLES_CLASSROOM_H
#define EXAMPLES_CLASSROOM_H

#include <QString>
#include <QList>

#include "qas.h"
#include "../test2/Model/QDspxBase.h"

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
};

namespace A {
    namespace B {
        namespace C {
            namespace F = B;
        }
    }
}

namespace A {
    namespace E = School;
    namespace B {
        namespace C {
            namespace D = E;
        };
    }
}

QAS_ENUM_DECLARE(A::B::C::D::Classroom::Student::Gender)

QAS_JSON_DECLARE(A::B::C::D::Classroom::Student)

QAS_JSON_DECLARE(A::B::C::D::Classroom::SmallStudent)

QAS_JSON_DECLARE(School::Classroom)

QAS_JSON_DECLARE(School::BigClassroom)

#endif //EXAMPLES_CLASSROOM_H
