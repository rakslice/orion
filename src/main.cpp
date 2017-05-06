/*
 * Copyright © 2015-2016 Antti Lamminsalo
 *
 * This file is part of Orion.
 *
 * Orion is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with Orion.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QQmlApplicationEngine>
#include <QtQml>
#include <QQmlComponent>
#include <QQuickView>
#include <QScreen>
#include <QMainWindow>
#include <QQmlContext>
#include <QString>
#include <QtSvg/QGraphicsSvgItem>
#include <QFontDatabase>
#include <QResource>
#include "util/runguard.h"
#include "model/channelmanager.h"
#include "network/networkmanager.h"
#include "power/power.h"
#include "systray.h"
#include "customapp.h"
#include "notification/notificationmanager.h"
#include "model/vodmanager.h"
#include "model/ircchat.h"
#include "network/httpserver.h"
#include <QFont>
#include <QDesktopWidget>
#include "main.h"

#ifdef MPV_PLAYER
#include "player/mpvrenderer.h"
#endif

inline void noisyFailureMsgHandler(QtMsgType /*type*/, const QMessageLogContext &/*context*/, const QString &/*msg*/)
{

}

void ScreenChangeHandler::screenChanged(QScreen * screen) {

    qreal dpiMultiplier = screen->logicalDotsPerInch();
    qDebug() << "screen changed; new logical dpi" << dpiMultiplier;
    
    qreal devicePixelRatio = screen->devicePixelRatio();
    qDebug() << "device pixel ratio" << devicePixelRatio;
    dpiMultiplier *= devicePixelRatio;

#ifdef Q_OS_WIN
    dpiMultiplier /= 96;

#elif defined(Q_OS_LINUX)
    dpiMultiplier /= 96;

#elif defined(Q_OS_MAC)
    dpiMultiplier /= 72;

#endif

    //Small adjustment to sizing overall
    dpiMultiplier *= .8;

    qDebug() << "setting new dpiMultiplier" << dpiMultiplier;

    _rootContext->setContextProperty("dpiMultiplier", dpiMultiplier);
}

int main(int argc, char *argv[])
{
    CustomApp app(argc, argv);

    //Single application solution
    RunGuard guard("wz0dPKqHv3vX0BBsUFZt");
    if ( !guard.tryToRun() ){
        guard.sendWakeup();
        return -1;
    }

    //Init engine
    QQmlApplicationEngine engine;

    QIcon appIcon = QIcon(":/icon/orion.ico");

    //Setup default font
    if (QFontDatabase::addApplicationFont(":/fonts/NotoSans-Regular.ttf") == -1)
        qDebug() << "Can't open application font!";
    else
        app.setFont(QFont(":/fonts/NotoSans-Regular.ttf", 10, QFont::Normal, false));

#ifndef  QT_DEBUG
    qInstallMessageHandler(noisyFailureMsgHandler);
#endif
    app.setWindowIcon(appIcon);

    SysTray *tray = new SysTray();
    tray->setIcon(appIcon);

    QObject::connect(tray, SIGNAL(closeEventTriggered()), &app, SLOT(quit()));

    //Prime network manager
    QNetworkProxyFactory::setUseSystemConfiguration(true);
    NetworkManager *netman = new NetworkManager(engine.networkAccessManager());

    //Create channels manager
    ChannelManager *cman = new ChannelManager(netman);

    //Screensaver mngr
    Power *power = new Power(static_cast<QApplication *>(&app));

    //Create vods manager
    VodManager *vod = new VodManager(netman);

    //Http server used for auth
    HttpServer *httpserver = new HttpServer(&app);
    QObject::connect(httpserver, SIGNAL(codeReceived(QString)), cman, SLOT(setAccessToken(QString)));
    //-------------------------------------------------------------------------------------------------------------------//

    QQmlContext *rootContext = engine.rootContext();

    ScreenChangeHandler screenChangeHandler(rootContext);

    qreal screens = 0;
    qreal totalDevicePixelRatio = 0;

    qDebug() << "Screens:";
    for (const auto & screen : QGuiApplication::screens()) {
        qreal curPixelRatio = screen->devicePixelRatio();
        totalDevicePixelRatio += curPixelRatio;
        screens++;
        qDebug() << "  Screen #" << screens << screen->name() << ": devicePixelRatio" << curPixelRatio;
    }
    qreal pixelRatio = screens? (totalDevicePixelRatio / screens) : QGuiApplication::primaryScreen()->devicePixelRatio();
    qDebug() << "Pixel ratio " << pixelRatio;
    //qDebug() <<"DPI mult: "<< dpiMultiplier;

    //rootContext->setContextProperty("dpiMultiplier", dpiMultiplier);
    rootContext->setContextProperty("netman", netman);
    rootContext->setContextProperty("g_cman", cman);
    rootContext->setContextProperty("g_guard", &guard);
    rootContext->setContextProperty("g_powerman", power);
    rootContext->setContextProperty("g_favourites", cman->getFavouritesProxy());
    rootContext->setContextProperty("g_results", cman->getResultsModel());
    rootContext->setContextProperty("g_featured", cman->getFeaturedProxy());
    rootContext->setContextProperty("g_games", cman->getGamesModel());
    rootContext->setContextProperty("g_tray", tray);
    rootContext->setContextProperty("g_vodmgr", vod);
    rootContext->setContextProperty("vodsModel", vod->getModel());
    rootContext->setContextProperty("app_version", APP_VERSION);
    rootContext->setContextProperty("httpServer", httpserver);

#ifdef MPV_PLAYER
    rootContext->setContextProperty("player_backend", "mpv");
    qmlRegisterType<MpvObject>("mpv", 1, 0, "MpvObject");

#elif defined (QTAV_PLAYER)
    rootContext->setContextProperty("player_backend", "qtav");

#elif defined (MULTIMEDIA_PLAYER)
    rootContext->setContextProperty("player_backend", "multimedia");
#endif

    qmlRegisterType<IrcChat>("aldrog.twitchtube.ircchat", 1, 0, "IrcChat");

    engine.load(QUrl("qrc:/main.qml"));

    qDebug() << "getting current screen info";

    QWindow * window = nullptr;

    auto rootObjects = engine.rootObjects();
    if (rootObjects.length() > 0) {
        QObject * qmlApplicationWindow = rootObjects.first();
        if (qmlApplicationWindow != nullptr) {
            qDebug() << "root object is a" << qmlApplicationWindow->metaObject()->className();

            window = dynamic_cast<QWindow *>(qmlApplicationWindow);
            if (window) {
                // future screen updates
                QObject::connect(window, &QWindow::screenChanged, &screenChangeHandler, &ScreenChangeHandler::screenChanged);

                // current screen update
                QDesktopWidget * desktop = QApplication::desktop();
                if (desktop != nullptr) {
                    int screenNumber = desktop->screenNumber(dynamic_cast<QWidget *>(window));
                    const auto screens = QApplication::screens();
                    if (screenNumber >= 0 && screenNumber < screens.length()) {
                        QScreen * screen = screens[screenNumber];
                        screenChangeHandler.screenChanged(screen);
                    }
                    else {
                        qDebug() << "QDesktopWidget reported window screen number" << screenNumber << "but there is no screen by that number";
                    }
                }
                else {
                    qDebug() << "can't get desktop widget";
                }
            }
            else {
                qDebug() << "couldn't cast to QWindow";
            }
        }
        else {
            qDebug() << "can't get root object - null ptr";
        }
    }
    else {
        qDebug() << "can't get root object - empty rootobjects list";
    }

    //Set up notifications
    NotificationManager *notificationManager = new NotificationManager(&engine, engine.networkAccessManager());
    QObject::connect(cman, SIGNAL(pushNotification(QString,QString,QString)), notificationManager, SLOT(pushNotification(QString,QString,QString)));

    qDebug() << "Starting window...";
    tray->show();

    app.exec();

    //-------------------------------------------------------------------------------------------------------------------//

    //Cleanup
    delete vod;
    delete tray;
    delete netman;
    delete cman;
    delete notificationManager;

    if (window) {
        QObject::disconnect(window, &QWindow::screenChanged, &screenChangeHandler, &ScreenChangeHandler::screenChanged);
    }

    qDebug() << "Closing application...";
    return 0;
}
