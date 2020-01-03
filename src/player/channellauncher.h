#ifndef CHANNELLAUNCHER_H
#define CHANNELLAUNCHER_H

#include <QVariant>

class ChannelLauncher: public QObject {
    Q_OBJECT
public:
    ChannelLauncher(QObject *parent = 0) : QObject(parent) {}

    Q_INVOKABLE void launchChannel(QVariant channelId);
};

#endif // CHANNELLAUNCHER_H
