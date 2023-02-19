#include <QCoreApplication>
#include <QJsonDocument>

#include "classroom.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    Rectangle rect{1, 2};
    qDebug() << qAsAnyToJson(rect);

    QJsonObject obj{
            {"a", 2},
            {"b", 5},
    };
    rect = _qAsJsonGetAny(obj);
    qDebug() << qAsAnyToJson(rect);

    return 0;
}
