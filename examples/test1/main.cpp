#include <QCoreApplication>
#include <QJsonDocument>

#include "testobj.h"

#include "Model/QDspxModel.h"

#include <QFile>

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
            } else {
                qDebug() << "Failed";
            }
        }
    }
    out:

    QJsonObject obj{
            {"a", QJsonObject({{"app", QJsonArray{1, 2}}})},
            {"b", QJsonObject{}},
    };

    bool ok1;
    auto map = QASJsonType<QMap<QString, QJsonObject>>::fromObject(obj, &ok1);
    qDebug() << ok1;

    return 0;
}