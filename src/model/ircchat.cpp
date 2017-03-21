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

#include "ircchat.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QStandardPaths>
#include <QImage>

IrcChat::IrcChat(QObject *parent) :
    QObject(parent) {

    logged_in = false;

    // Open socket
    sock = new QTcpSocket(this);
    if(sock) {
        emit errorOccured("Error opening socket");
    }

    //createConnection();
    connect(sock, SIGNAL(readyRead()), this, SLOT(receive()));
    connect(sock, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(processError(QAbstractSocket::SocketError)));
    connect(sock, SIGNAL(connected()), this, SLOT(login()));
    connect(sock, SIGNAL(connected()), this, SLOT(onSockStateChanged()));
    connect(sock, SIGNAL(disconnected()), this, SLOT(onSockStateChanged()));


    //download_emotes();
    room = "";

	emoteDir = QDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QString("/emotes"));
	emoteDirPathImpl = emoteDir.absolutePath();
}

IrcChat::~IrcChat() { disconnect(); }

void IrcChat::join(const QString channel) {

    if (inRoom())
        leave();

    // Save channel name for later use
    room = channel;

    if (!connected()) {
        reopenSocket();
    }

    // Join channel's chat room
    sock->write(("JOIN #" + channel + "\r\n").toStdString().c_str());



    qDebug() << "Joined channel " << channel;
}

void IrcChat::leave()
{
    sock->write(("PART #" + room + "\r\n").toStdString().c_str());
    room = "";
}

void IrcChat::disconnect() {
    leave();
    sock->close();
}

void IrcChat::reopenSocket() {
    qDebug() << "Reopening socket";
    if(sock->isOpen())
        sock->close();
    sock->open(QIODevice::ReadWrite);
    sock->connectToHost(HOST, PORT);
    if(!sock->isOpen()) {
        errorOccured("Error opening socket");
    }
}

void IrcChat::setAnonymous(bool newAnonymous) {
    if(newAnonymous != anonym) {
        if(newAnonymous) {
            qsrand(QTime::currentTime().msec());
            username = "";
            username.sprintf("justinfan%06d", (qrand() % (1000000 - 100000)) + 100000);
            userpass = "blah";
        }
        anonym = newAnonymous;

        //login();

        emit anonymousChanged();
    }
}

bool IrcChat::connected() {
    return sock->state() == QTcpSocket::ConnectedState;
}

void IrcChat::sendMessage(const QString &msg) {
    if (inRoom() && connected()) {
        sock->write(("PRIVMSG #" + room + " :" + msg + "\r\n").toStdString().c_str());
        QVariantList message;
        message.append(msg);
        emit messageReceived(username, message);
    }
}

void IrcChat::onSockStateChanged() {
    // We don't check if connected property actually changed because this slot should only be awaken when it did
    emit connectedChanged();
}

void IrcChat::createConnection()
{
    sock->connectToHost(HOST, PORT);
}

void IrcChat::login()
{
    if (userpass.isEmpty() || username.isEmpty())
        setAnonymous(true);
    else
        setAnonymous(false);

    // Tell server that we support twitch-specific commands
    sock->write("CAP REQ :twitch.tv/commands\r\n");
    sock->write("CAP REQ :twitch.tv/tags\r\n");

    // Login
    sock->write(("PASS " + userpass + "\r\n").toStdString().c_str());
    sock->write(("NICK " + username + "\r\n").toStdString().c_str());

    logged_in = true;

    //Join room automatically, if given
    if (!room.isEmpty())
        join(room);
}

void IrcChat::receive() {
    QString msg;
    while (sock->canReadLine()) {
        msg = sock->readLine();
        msg = msg.remove('\n').remove('\r');
        parseCommand(msg);
    }
}

void IrcChat::processError(QAbstractSocket::SocketError socketError) {
    QString err;
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        err = "Server closed connection.";
        break;
    case QAbstractSocket::HostNotFoundError:
        err = "Host not found.";
        break;
    case QAbstractSocket::ConnectionRefusedError:
        err = "Connection refused.";
        break;
    default:
        err = "Unknown error.";
    }

    errorOccured(err);
}

void IrcChat::parseCommand(QString cmd) {
    if(cmd.startsWith("PING ")) {
        sock->write("PONG\r\n");
        return;
    }
    if(cmd.contains("PRIVMSG")) {

        // Structure of message: '@color=#HEX;display-name=NicK;emotes=id:start-end,start-end/id:start-end;subscriber=0or1;turbo=0or1;user-type=type :nick!nick@nick.tmi.twitch.tv PRIVMSG #channel :message'
        QString params = cmd.left(cmd.indexOf("PRIVMSG"));
        QString nickname = params.left(params.lastIndexOf('!')).remove(0, params.lastIndexOf(':') + 1);
        QString emotes = params.left(params.indexOf("id") - 1).remove(0, params.indexOf("tes=")+4);
        QString message = cmd.remove(0, cmd.indexOf(':', cmd.indexOf("PRIVMSG")) + 1);
        QString oldmessage = cmd.remove(0, cmd.indexOf(':', cmd.indexOf("PRIVMSG")) + 1);
        QVariantList messageList;
        qDebug() << oldmessage;
        if(emotes != "") {
          auto emoteList = emotes.split('/');
          for(auto emote : emoteList) {
            auto key = emote.left(emote.indexOf(':'));
            emote.remove(0, emote.indexOf(':')+1);
            qDebug() << "the emote and stuff" << key;
            download_emotes(key);
            auto replaceIndexs = emote.split(',');
            int prev = 0;
            int first = 0;
            //TODO: don't add empty values at start
            for(auto emotePlc : replaceIndexs) {
              auto firstAndLast = emotePlc.split('-');
              first = firstAndLast[0].toInt();
              int last = firstAndLast[1].toInt();
              if(first >  0) {
                messageList.append(message.left(first));
              }
              else {
                messageList.append(message.mid(prev + 1, first));
              }
              messageList.append(key);
              prev = last;
            }
            if(prev+1 < message.size()) {
              messageList.append(message.mid(prev+1, message.size()-1));
            }
          }
        }
        else {
          messageList.append(message);
        }
        qDebug() << messageList;
        emit messageReceived(nickname, messageList);
        return;
    }
    if(cmd.contains("NOTICE")) {
        QString text = cmd.remove(0, cmd.indexOf(':', cmd.indexOf("NOTICE")) + 1);
        emit noticeReceived(text);
    }
    if(cmd.contains("GLOBALUSERSTATE")) {
        // We are not interested in this one, it only exists because otherwise USERSTATE would be trigged instead
        return;
    }
    //qDebug() << "Unrecognized chat command:" << cmd;
}

QString IrcChat::getParamValue(QString params, QString param) {
    QString paramValue = params.remove(0, params.indexOf(param + "="));
    paramValue = paramValue.left(paramValue.indexOf(';')).remove(0, paramValue.indexOf('=') + 1);
    return paramValue;
}

bool IrcChat::download_emotes(QString key) {
    if(_emoteTable.contains(key)) {
      qDebug() << "already in the table";
        return false;
    }
    qDebug() << "downloading";

    QUrl url = QString("https://static-cdn.jtvnw.net/emoticons/v1/") + QString(key) + QString("/1.0");
    emoteDir.mkpath(".");

    _file.setFileName(emoteDir.absoluteFilePath(key + ".png"));
    _file.open(QFile::WriteOnly);

    QNetworkRequest request(url);
    _reply = _manager.get(request);

    //emote_table.insert(key, );
    
    connect(_reply, &QNetworkReply::readyRead,
      this, &IrcChat::dataAvailable);
    connect(_reply, &QNetworkReply::finished,
      this, &IrcChat::replyFinished);

    return true;
}

void IrcChat::dataAvailable() {
  auto buffer = _reply->readAll();
  _file.write(buffer.data(), buffer.size());
}

void IrcChat::replyFinished() {
  if(_reply) {
    _reply->deleteLater();
    QImage* emoteImg = new QImage();
    emoteImg->load(_file.fileName());
    qDebug() << _file.fileName();
    //might need something for windows for the forwardslash..
    _emoteTable.insert(_file.fileName().left(_file.fileName().indexOf(".png")).remove(0, _file.fileName().lastIndexOf('/') + 1), emoteImg);

    _file.close();
    _reply = nullptr;
    emit downloadComplete();
  }
}

QHash<QString, QImage*> IrcChat::emoteTable() {
  return _emoteTable;
}
