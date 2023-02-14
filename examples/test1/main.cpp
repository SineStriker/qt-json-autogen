#include <QCoreApplication>
#include <QJsonDocument>
#include <QFile>

#include "classroom.h"

using namespace School;

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    BigClassroom cr;
    cr.address = "Washington D.C.";
    cr.className = "201";
    cr.slogan = "Keep calm and carry on.";

    Classroom::Student alice;
    alice.name = "Alice";
    alice.gender = Classroom::Student::Female;
    alice.height = 165;

    Classroom::Student bob;
    bob.name = "Bob";
    bob.gender = Classroom::Student::Male;
    bob.height = 180;

    cr.students = {alice, bob};

    QJsonDocument doc(QASJsonType<BigClassroom>::toObject(cr));
    qDebug().noquote() << doc.toJson();

    return 0;
}
