#include <QCommandLineParser>
#include <QCoreApplication>
#include <QStringLiteral>

#include "config.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    QCommandLineParser parser;

    // Basic information
    parser.setApplicationDescription(
        QStringLiteral("Qt Auto Serialization Compiler version %1 (Qt %2)")
            .arg(QAS_JSON_VERSION)
            .arg(QString::fromLatin1(QT_VERSION_STR)));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);

    // Output option
    QCommandLineOption outputOption(QStringLiteral("o"));
    outputOption.setDescription(QStringLiteral("Write output to file rather than stdout."));
    outputOption.setValueName(QStringLiteral("file"));
    outputOption.setFlags(QCommandLineOption::ShortOptionStyle);
    parser.addOption(outputOption);

    return 0;
}