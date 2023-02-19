#ifndef CLASSROOM_H
#define CLASSROOM_H

#include "qjsonstream.h"

// Source
class Class {
public:
    class Student {
    public:
        enum Gender {
            Male,
            Female,
        };
        QAS_JSON(Gender);

        QString name;
        Gender gender;
        int age;
    };
    QAS_JSON(Student);

    __qas_attr__("Class")
    QString className;

    QMap<QString, Student> students; // id -> student

    __qas_exclude__
    QString otherInfo;
};

QAS_JSON_NS(Class)

#endif // CLASSROOM_H
