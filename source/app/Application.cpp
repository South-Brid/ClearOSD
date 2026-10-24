#include "Application.h"
#include "Controller.h"
#include "brightness/BrightnessController.h"
#include "platform/WindowsPlatform.h"
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QPainter>
#include <QDateTime>
#include <QFile>
#include <QLockFile>
#include <QLocalSocket>
#include <QLocalServer>
#include <QJsonDocument>
#include <QJsonObject>
using namespace WindowsPlatform;
static QIcon appIcon(){QPixmap p(64,64);p.fill(Qt::transparent);QPainter paint(&p);paint.setRenderHint(QPainter::Antialiasing);paint.setBrush(QColor("#141d2b"));paint.setPen(Qt::NoPen);paint.drawRoundedRect(1,1,62,62,16,16);paint.setPen(QColor("#8ee3b2"));paint.setFont(QFont("Segoe UI",25,QFont::Bold));paint.drawText(p.rect(),Qt::AlignCenter,"O");return QIcon(p);}
static void logMessage(QtMsgType type,const QMessageLogContext&,const QString &message){QFile f(dataDir()+"/app.log");if(f.open(QIODevice::Append))f.write((QDateTime::currentDateTime().toString(Qt::ISODate)+" "+QString::number(type)+" "+message+"\n").toUtf8());}

int Application::run(int argc,char **argv){
    QApplication app(argc,argv);app.setApplicationName("ClearOSD");app.setOrganizationName("ClearOSD");app.setQuitOnLastWindowClosed(false);qInstallMessageHandler(logMessage);QQuickStyle::setStyle("Basic");
    if(app.arguments().contains("--clear-legacy-osd"))return RtssClient::clearLegacyOsd()?0:3;
    QLocalSocket existing;existing.connectToServer("ClearOSD.Native.Control");
    if(existing.waitForConnected(200)){existing.write(app.arguments().contains("--quit")?"quit":"show");existing.waitForBytesWritten(300);return 0;}
    if(app.arguments().contains("--quit"))return 0;
    QLockFile lock(dataDir()+"/instance.lock");if(!lock.tryLock(0))return 0;
    QLocalServer server;QLocalServer::removeServer("ClearOSD.Native.Control");server.listen("ClearOSD.Native.Control");
    RtssClient::clearLegacyOsd();
    app.setWindowIcon(appIcon());Controller controller;BrightnessController brightness;QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("brightnessBackend",&brightness);
    engine.rootContext()->setContextProperty("backend",&controller);engine.load(QUrl("qrc:/Main.qml"));
    if(engine.rootObjects().isEmpty()){controller.close();return 1;}
    auto overlay=engine.rootObjects().first()->findChild<QQuickWindow*>("overlayWindow");controller.attach(overlay);
    if(app.arguments().contains("--background"))
        qobject_cast<QQuickWindow*>(engine.rootObjects().first())->hide();
    QObject::connect(&server,&QLocalServer::newConnection,&controller,[&]{while(server.hasPendingConnections()){auto s=server.nextPendingConnection();QObject::connect(s,&QLocalSocket::readyRead,&controller,[&,s]{auto command=s->readAll();if(command=="quit")app.quit();else if(command=="diagnostics")s->write(QJsonDocument(QJsonObject::fromVariantMap(controller.diagnostics())).toJson(QJsonDocument::Compact));else controller.openSettings();s->disconnectFromServer();});QObject::connect(s,&QLocalSocket::disconnected,s,&QObject::deleteLater);}});
    QSystemTrayIcon tray(appIcon());QMenu menu;menu.addAction(QStringLiteral("打开设置"),&controller,&Controller::openSettings);menu.addAction(QStringLiteral("显示 / 隐藏"),&controller,&Controller::toggleHidden);
    menu.addAction(QStringLiteral("锁定 / 解锁拖动"),&controller,[&]{controller.change("locked",!controller.config()["locked"].toBool());});menu.addSeparator();menu.addAction(QStringLiteral("恢复全部显示器亮度"),&brightness,&BrightnessController::resetAll);menu.addSeparator();menu.addAction(QStringLiteral("退出 ClearOSD"),&app,&QApplication::quit);
    tray.setContextMenu(&menu);tray.setToolTip(QStringLiteral("ClearOSD · 右键打开设置"));tray.show();
    QObject::connect(&tray,&QSystemTrayIcon::activated,&controller,[&](auto r){if(r==QSystemTrayIcon::DoubleClick)controller.openSettings();});
    QObject::connect(&app,&QApplication::aboutToQuit,&controller,&Controller::close);
    return app.exec();
}
