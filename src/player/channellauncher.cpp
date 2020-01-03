#include "channellauncher.h"

#include <QProcess>


Q_INVOKABLE void ChannelLauncher::launchChannel(QVariant channelId) {
    QStringList args;
    args << "/C";
    args << "C:\\Users\\Andrew Tonner\\Documents\\kodicontrol\\kodicontrol.cmd";
    args << channelId.toString();

    QProcess process(this);
    process.startDetached("cmd.exe", args);
}
