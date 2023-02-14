#ifndef EXAMPLES_CLASSROOM_H
#define EXAMPLES_CLASSROOM_H

#include <QString>
#include <QList>

#include "qas.h"

#include<set>

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

        Classroom() = default;

        QString className;
        QString slogan;
        QList<Student> students;
    };

    class BigClassroom : public Classroom{
    public:
        QString address;
    };
};

QAS_ENUM_DECLARE(School::Classroom::Student::Gender)

QAS_JSON_DECLARE(School::Classroom::Student)

QAS_JSON_DECLARE(School::Classroom)

QAS_JSON_DECLARE(School::BigClassroom, School::Classroom)

#endif //EXAMPLES_CLASSROOM_H
