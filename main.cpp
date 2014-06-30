#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QTextCodec>
#include "iqtempmainwindow.h"

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    IQTempMainWindow *w = IQTempMainWindow::instance();
    if (w)
        w->writeToLog(type, context, msg);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setApplicationName("iqTemp");
    QApplication::setOrganizationName("itQuasar");
    QApplication::setOrganizationDomain("itquasar.ru");
    QApplication::setApplicationVersion("1.0");

    QTextCodec *utfCodec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(utfCodec);


    QTranslator qtTranslator;
    if (!qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        qtTranslator.load(QCoreApplication::applicationDirPath() + "/qt_" + QLocale::system().name() + ".qm");

    QTranslator qtBaseTranslator;
    if (!qtBaseTranslator.load("qtbase_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        qtBaseTranslator.load(QCoreApplication::applicationDirPath() + "/qtbase_" + QLocale::system().name() + ".qm");


    QTranslator appTranslator;
    appTranslator.load(QCoreApplication::applicationDirPath() + "/iqtemp_" + QLocale::system().name() + ".qm");

    QTranslator qmlTranslator;
    qmlTranslator.load(QCoreApplication::applicationDirPath() + "/MimicPanel_" + QLocale::system().name() + ".qm");


    a.installTranslator(&qtTranslator);
    a.installTranslator(&qtBaseTranslator);
    a.installTranslator(&appTranslator);
    a.installTranslator(&qmlTranslator);

    IQTempMainWindow w;

    qInstallMessageHandler(myMessageOutput);

    QString t = QLibraryInfo::location(QLibraryInfo::TranslationsPath);

    w.show();

    return a.exec();
}
