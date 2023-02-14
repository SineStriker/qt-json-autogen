//
// Created by Functioner on 2023/2/14.
//

#ifndef EXAMPLES_CLASSROOM_H
#define EXAMPLES_CLASSROOM_H

#include <QString>
#include <QList>

#include "qas.h"

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
        int Height;
    };

    Classroom() = default;

    QString className;
    QString slogan;
    QList<Student> students;
};

QAS_ENUM_DECLARE(Classroom::Student::Gender)
QAS_JSON_DECLARE(Classroom::Student)
QAS_JSON_DECLARE(Classroom)

#endif //EXAMPLES_CLASSROOM_H
