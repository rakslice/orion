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

#include "power.h"
#include <QtGlobal>
#include <QProcess>
#include <QDebug>
#include <QWindow>

#ifdef Q_OS_LINUX
    #include <QtDBus>
#elif defined(Q_OS_MAC)
    #include <CoreServices/CoreServices.h>
#endif

Power::Power(QApplication *app) :
    app(app), cookie(0)
{
    timer = new QTimer();
    connect(timer, &QTimer::timeout, this, &Power::onTimerProc);
}

Power::~Power()
{
    setScreensaver(true);
    delete timer;
}

void Power::setScreensaver(bool enabled)
{

#ifdef Q_OS_LINUX

    if (!enabled) {
        if (cookie == 0) {
            QDBusInterface dbus("org.freedesktop.ScreenSaver", "/org/freedesktop/ScreenSaver", "org.freedesktop.ScreenSaver");
            QDBusMessage reply = dbus.call("Inhibit", APP_NAME, "Playing video");

            if(reply.type() != QDBusMessage::ErrorMessage) {
                cookie = reply.arguments().at(0).toInt();
                qDebug() << "Disabled screensaver with cookie " << cookie;
            }
        }
    } else {
        if (cookie > 0) {
            QDBusInterface dbus("org.freedesktop.ScreenSaver", "/org/freedesktop/ScreenSaver", "org.freedesktop.ScreenSaver");
            QDBusMessage reply = dbus.call("UnInhibit", QVariant(cookie));

            if(reply.type() != QDBusMessage::ErrorMessage) {
                cookie = 0;
                qDebug() << "Enabled screensaver";
            }
        }
    }

//    QWindow *win = app->allWindows().at(0);

//    if (!win)
//        return;

//    quint32 wid = win->winId();

//    qDebug() << "Screensaver change event, wid" << wid;

//    QStringList args;
//    args << (enabled ? "resume" : "suspend") << QString::number(wid);

//    QProcess::startDetached("xdg-screensaver",args);

//    // Also set timer to send nudges to xserver
//    if (enabled){
//        timer->stop();
//    } else {
//        if (!timer->isActive()){
//            timer->start(25000);
//        }
//    }

#elif defined(Q_OS_WIN)
    if (!enabled)
        SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);
    else
        SetThreadExecutionState(ES_CONTINUOUS);

#endif

#ifdef Q_OS_MAC
    if (enabled){
        timer->stop();
    } else {
        if (!timer->isActive()){
            timer->start(25000);
        }
    }
#endif
}

void Power::onTimerProc()
{
#ifdef Q_OS_LINUX
    QProcess::startDetached("xdg-screensaver reset");

#endif

#ifdef Q_OS_MAC
    UpdateSystemActivity(OverallAct);
    qDebug() << "Sent nudge to osx screensaver";
#endif

}
