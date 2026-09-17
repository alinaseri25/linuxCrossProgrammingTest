#include <QCoreApplication>
#include <QTextStream>
#include <QDebug>
#include <QFile>

int main(int argc, char *argv[])
{
    // Initialize the core event loop / platform abstraction
    QCoreApplication app(argc, argv);

    // Standard output via QTextStream with proper encoding handling
    QTextStream cout(stdout);
    cout << "=========================================" << Qt::endl;
    cout << " Hello, World! from Qt " << QT_VERSION_STR << Qt::endl;
    cout << " Target Architecture: " << QSysInfo::currentCpuArchitecture() << Qt::endl;
    cout << " OS: " << QSysInfo::prettyProductName() << Qt::endl;
    cout << "=========================================" << Qt::endl;

    // Structured debug logging
    qDebug() << "Application name:" << QCoreApplication::applicationName();
    qDebug() << "Application path:" << QCoreApplication::applicationDirPath();

    QFile file("/etc/asterisk/pjsip.conf");
    if(file.open(QIODeviceBase::ReadWrite | QIODeviceBase::Text))
    {
        QTextStream iostream(&file);
        QString content = iostream.readAll();
        content.prepend("Ali\n");
        content.append("\nAli");
        file.seek(0);
        iostream << content;
        iostream.flush();
        file.close();
    }
    else
    {
        cout << "failed open /etc/asterisk/pjsip.conf";
    }

    // Return directly for a simple run-and-exit CLI tool:
    return 0;

    // Note: If you need an active event loop (QTimer, QSerialPort, QNetworkAccessManager, signals/slots):
    // return app.exec();
}