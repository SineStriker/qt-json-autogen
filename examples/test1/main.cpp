#include <QCoreApplication>
#include <QJsonDocument>

#include "testobj.h"
#include "classroom.h"

#include "Model/QDspxModel.h"

#include <QFile>

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    Classroom cr;
    cr.className = "201";
    cr.slogan = "Keep calm and carry on.";

    Classroom::Student alice;
    alice.name = "Alice";
    alice.gender = Classroom::Student::Female;
    alice.Height = 165;

    Classroom::Student bob;
    bob.name = "Bob";
    bob.gender = Classroom::Student::Male;
    bob.Height = 180;

    cr.students = {alice, bob};

    qDebug().noquote() << QJsonDocument(QASJsonType<Classroom>::toObject(cr)).toJson();

    // Enumeration serialize
    qDebug() << QASEnumType<TestEnum>::toString(TestEnum::Alice);

    // Enumeration deserialize
    qDebug() << QASEnumType<TestEnum>::fromString("Bob");

    qDebug() << QASJsonType<QStringList>::fromValue("1");

    std::set<int> s;

    TestClass foo;
    foo.Alice = "1";
    foo.Bob = "2";
    foo.Mark = "3";

    // Json serialize
    qDebug().noquote() << QJsonDocument(QASJsonType<TestClass>::toObject(foo)).toJson();

    qDebug().noquote() << QASJsonType<QList<TestClass>>::toArray({foo, foo});

//    QFile file("empty.dspx");
//    if (file.open(QIODevice::WriteOnly)) {
//        file.write(QJsonDocument(QASJsonType<QDspxModel>::toObject({})).toJson());
//    }
//    file.close();

    if (argc > 1) {
        QFile file(a.arguments().at(1));
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray data = file.readAll();
            file.close();
            QJsonParseError parseError{};
            QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
            if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
                qDebug() << "Read file error.";
                goto out;
            }

            bool ok;
            QDspxModel model = QASJsonType<QDspxModel>::fromObject(doc.object(), &ok);
            if (ok) {
                qDebug() << model.metadata.name;
                QFile file("2.json");
                if (file.open(QIODevice::WriteOnly)) {
                    file.write(QJsonDocument(QASJsonType<QDspxModel>::toObject(model)).toJson());
                }
                file.close();
            } else {
                qDebug() << "Failed";
            }
        }
    }
    out:
    return 0;
}