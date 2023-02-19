#include <QCoreApplication>
#include <QJsonDocument>
#include <QFile>

#include "Model/QDspxModel.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

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

            QDspxModel model;
            if (qAsJsonTryGetClass(doc.object(), &model)) {
                qDebug() << model.metadata.name;
                file.setFileName("2.json");
                if (file.open(QIODevice::WriteOnly)) {
                    file.write(QJsonDocument(qAsClassToJson(model)).toJson());
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