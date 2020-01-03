#include "channellauncher.h"

#include <QProcess>
#include <QStandardPaths>
#include <QDebug>


Q_INVOKABLE void ChannelLauncher::launchChannel(QVariant channelId) {

    QStringList args;
    QString prog;
    QString script = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first() + "/open_twitch_stream";

#ifdef Q_OS_WIN32
    prog = "cmd.exe";
    args << "/C";
    script += ".cmd";
#else
    prog = "bash";
    args << "-e";
#endif

    qDebug() << "Using external script at" << script;
    args << script;
    args << channelId.toString();

    QProcess process(this);
    process.startDetached(prog, args);
}
