/*
 * Copyright © 2015-2016 Andrew Penkrat
 *
 * This file is part of TwitchTube.
 *
 * TwitchTube is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TwitchTube is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TwitchTube.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef IRCCHAT_H
#define IRCCHAT_H

#include <QObject>
#include <QTcpSocket>
#include <QNetworkReply>
#include <QMap>
#include <QRegExp>
#include <QColor>
#include <QQmlListProperty>
#include <QImage>
#include <QFile>
#include <QHash>
#include <QDir>
//#include "messagelistmodel.h"
//#include "message.h"

const qint16 PORT = 6667;
const QString HOST = "irc.twitch.tv";

//#define TWITCH_EMOTE_URI "https://static-cdn.jtvnw.net/emoticons/v1/%d/1.0"

struct ChatMessage {
    QString name;
    QVariantList messageList;
    QString color;
    bool subscriber;
    bool turbo;
};

// Handles state for an individual download
class DownloadHandler : public QObject
{
    Q_OBJECT
public:
    DownloadHandler(QString filename);
private:
    QString filename;
    QFile _file;

signals:
    void downloadComplete(QString filename);

public slots:
    void dataAvailable();
    void replyFinished();
};

class CachedImageProvider : public QQuickImageProvider {
public:
    CachedImageProvider(QHash<QString, QImage*> & imageTable);
    QImage requestImage(const QString &id, QSize * size, const QSize & requestedSize);
private:
    QHash<QString, QImage*> & imageTable;
};

// Backend for chat
class IrcChat : public QObject
{
    Q_OBJECT
public:
    IrcChat(QObject *parent = 0);
    ~IrcChat();

    Q_PROPERTY(QString name MEMBER username)
    Q_PROPERTY(QString password MEMBER userpass)
    Q_PROPERTY(bool anonymous READ anonymous WRITE setAnonymous NOTIFY anonymousChanged)
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(bool inRoom READ inRoom)
    Q_PROPERTY(QHash<QString, QImage*> emoteTable READ emoteTable NOTIFY emoteTableChanged)
    Q_PROPERTY(QString emoteDirPath MEMBER emoteDirPathImpl)

    Q_INVOKABLE void join(const QString channel);
    Q_INVOKABLE void leave();
    Q_INVOKABLE void disconnect();
    Q_INVOKABLE void reopenSocket();

    //# User
    QString username, userpass;
    inline bool anonymous() { return anonym; }
    void setAnonymous(bool newAnonymous);
    bool anonym;
    QStringList userSpecs;
    QString userDisplayName;

    //# Network
    bool connected();
    inline bool inRoom() { return !room.isEmpty(); }

    //emote download
    QDir emoteDir;
    QString emoteDirPathImpl;
    bool download_emotes(QString);
    QHash<QString, QImage*> emoteTable();
    void loadEmoteImageFile(QString filename);

signals:
    void errorOccured(QString errorDescription);
    void connectedChanged();
    void anonymousChanged();
    void messageReceived(QString user, QVariantList message, QString chatColor, bool subscriber, bool turbo);
    void noticeReceived(QString message);
    void emoteTableChanged();

    //emotes
    void downloadComplete();
    bool downloadError();
    
public slots:
    void sendMessage(const QString &msg);
    void onSockStateChanged();
    void login();
    void individualDownloadComplete(QString filename);

private slots:
    void createConnection();
    void receive();
    void processError(QAbstractSocket::SocketError socketError);

private:
    //some kind of emote table
    //downloader for emotes
    QByteArray _data;
    QNetworkAccessManager _manager;
    QList<QNetworkReply *> currentDownloads; //??...
    
    QHash<QString, QImage*> _emoteTable;
    QList<ChatMessage> msgQueue;

    void parseCommand(QString cmd);
    QString getParamValue(QString params, QString param);
    QTcpSocket *sock;
    QString room;
    QMap<QString, QString> badges;
    bool logged_in;
    int activeDownloadCount;
};

#endif // IRCCHAT_H
