#include <QCoreApplication>
#include <QJsonDocument>

#include "classroom.h"
#include "foo.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    Class cls;
    cls.className = "F114514";
    cls.students = {
            {"1", {"alice", Class::Student::Female, 18}},
            {"2", {"bob",   Class::Student::Male,   17}},
            {"3", {"mark",  Class::Student::Male,   19}},
    };
    cls.otherInfo = "PHP is the best programming language.";
    qDebug().noquote() << QJsonDocument(qAsClassToJson(cls)).toJson();

    qDebug().noquote() << QJsonDocument(qAsClassToJson(Messed())).toJson();

    return 0;
}
