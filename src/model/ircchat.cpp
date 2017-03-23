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

	activeDownloadCount = 0;
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
        //TODO need the user's status info to show here
        emit messageReceived(username, message, "", false, false);
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

        QString displayName = "";
        QString color = "";
        bool subscriber = false;
        bool turbo = false;
        bool downloading = false;
        QString emotes = "";

        if (cmd.at(0) == QChar('@')) {
            // tags are present
            int tagsEnd = cmd.indexOf(" ");
            QString tags = cmd.mid(1, tagsEnd - 1);
            foreach(const QString & tag, tags.split(";")) {
                int assignPos = tag.indexOf("=");
                if (assignPos == -1) continue;
                QString key = tag.left(assignPos);
                QString value = tag.mid(assignPos + 1);
                if (key == "display-name") {
                    displayName = value;
                }
                else if (key == "color") {
                    color = value;
                }
                else if (key == "subscriber") {
                    subscriber = (value == "1");
                }
                else if (key == "turbo") {
                    turbo = (value == "1");
                }
				else if (key == "emotes") {
					emotes = value;
				}
            }
        }

        QString params = cmd.left(cmd.indexOf("PRIVMSG"));
        QString nickname = params.left(params.lastIndexOf('!')).remove(0, params.lastIndexOf(':') + 1);
        QString message = cmd.remove(0, cmd.indexOf(':', cmd.indexOf("PRIVMSG")) + 1);
        QString oldmessage = cmd.remove(0, cmd.indexOf(':', cmd.indexOf("PRIVMSG")) + 1);
        //qDebug() << "emotes " << emotes;
        //qDebug() << "oldmessage " << oldmessage;

        QMap<int, QPair<int, int>> emotePositionsMap;

        if(emotes != "") {
          auto emoteList = emotes.split('/');

          for(auto emote : emoteList) {
            auto key = emote.left(emote.indexOf(':'));
            emote.remove(0, emote.indexOf(':')+1);
            qDebug() << "key " << key;
            if (download_emotes(key)) {
                activeDownloadCount += 1;
			}
            for(auto emotePlc : emote.split(',')) {
              auto firstAndLast = emotePlc.split('-');
              int first = firstAndLast[0].toInt();
              int last = firstAndLast.length() > 1 ? firstAndLast[1].toInt() : first;

              emotePositionsMap.insert(first, qMakePair(last, key.toInt()));
            }
          }
        }

        QVariantList messageList;

        int cur = 0;
        for (auto i = emotePositionsMap.constBegin(); i != emotePositionsMap.constEnd(); i++) {
            auto emoteStart = i.key();
            if (emoteStart > cur) {
                messageList.append(message.mid(cur, emoteStart - cur).toHtmlEscaped());
            }
            auto emoteEnd = i.value().first;
            auto emoteId = i.value().second;
            messageList.append(emoteId);
            cur = emoteEnd + 1;
        }
        if (cur < message.length()) {
            messageList.append(message.mid(cur, message.length() - cur).toHtmlEscaped());
        }

        qDebug() << "messageList " << messageList;
        if (displayName.length() > 0) {
            nickname = displayName;
        }

        if(activeDownloadCount == 0) {
          emit messageReceived(nickname, messageList, color, subscriber, turbo);
        }
        else {
          // queue message to be shown when downloads are complete
		  msgQueue.push_back({nickname, messageList, color, subscriber, turbo});
        }
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

    QUrl url = QString("https://static-cdn.jtvnw.net/emoticons/v1/") + QString(key) + QString("/1.0");
    emoteDir.mkpath(".");

    QString filename = emoteDir.absoluteFilePath(key + ".png");

    if(emoteDir.exists(key + ".png")) {
        qDebug() << "local file already exists";
		loadEmoteImageFile(filename);
        return false;
    }
	qDebug() << "downloading";

    QNetworkRequest request(url);
	QNetworkReply* _reply = nullptr;
    _reply = _manager.get(request);

	DownloadHandler * dh = new DownloadHandler(filename);

    connect(_reply, &QNetworkReply::readyRead,
      dh, &DownloadHandler::dataAvailable);
    connect(_reply, &QNetworkReply::finished,
      dh, &DownloadHandler::replyFinished);
	connect(dh, &DownloadHandler::downloadComplete,
		this, &IrcChat::individualDownloadComplete);

    return true;
}

DownloadHandler::DownloadHandler(QString filename) : filename(filename) {
    _file.setFileName(filename);
    _file.open(QFile::WriteOnly);
	qDebug() << "starting download of " << filename;
}

void DownloadHandler::dataAvailable() {
  QNetworkReply* _reply = qobject_cast<QNetworkReply*>(sender());
  auto buffer = _reply->readAll();
  _file.write(buffer.data(), buffer.size());
}

void DownloadHandler::replyFinished() {
  QNetworkReply* _reply = qobject_cast<QNetworkReply*>(sender());
  if(_reply) {
    _reply->deleteLater();
	_file.close();
    //qDebug() << _file.fileName();
    //might need something for windows for the forwardslash..
    qDebug() << "download of " << _file.fileName() << "complete";

    emit downloadComplete(_file.fileName());
  }
}

void IrcChat::loadEmoteImageFile(QString filename) {
    QImage* emoteImg = new QImage();
    emoteImg->load(filename);
	QString emoteKey = filename.left(filename.indexOf(".png")).remove(0, filename.lastIndexOf('/') + 1);
    _emoteTable.insert(emoteKey, emoteImg);
}

void IrcChat::individualDownloadComplete(QString filename) {
    DownloadHandler * dh = qobject_cast<DownloadHandler*>(sender());
    delete dh;
    
	loadEmoteImageFile(filename);
    
	if (activeDownloadCount > 0) {
		activeDownloadCount--;
		qDebug() << activeDownloadCount << " active downloads remaining";
	}

	if (activeDownloadCount == 0) {
		qDebug() << "Download queue complete; posting pending messages";
		while (!msgQueue.empty()) {
			ChatMessage tmpMsg = msgQueue.first();
			emit messageReceived(tmpMsg.name, tmpMsg.messageList, tmpMsg.color, tmpMsg.subscriber, tmpMsg.turbo);
			msgQueue.pop_front();
		}
	}

	//msgQueue.pop_front();
	emit downloadComplete();
}

QHash<QString, QImage*> IrcChat::emoteTable() {
  return _emoteTable;
}
