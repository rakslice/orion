#include "prefetchstream.h"

#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>
#include <QPointer>

PrefetchStream::PrefetchStream(QTcpSocket * socket, QString streamUrl, QNetworkAccessManager *operation, QObject * parent) : QObject(parent),
            socket(socket), streamUrl(QUrl(streamUrl)), operation(operation),
            alive(true), consecutiveEmptyPlaylists(0), readBlockSize(4096), currentFragmentBytesSoFar(0)
{
    nextPlaylistTimer.setInterval(1900);
    nextPlaylistTimer.setSingleShot(true);

    connect(&nextPlaylistTimer, &QTimer::timeout, this, &PrefetchStream::timeForNextPlaylist);
}

PrefetchStream::~PrefetchStream() {
}

void PrefetchStream::start() {
    requestPlaylist(true);
}

void PrefetchStream::requestPlaylist(bool first)
{
    QNetworkRequest request;

    qDebug() << "fetching playlist"; // << streamUrl;

    request.setUrl(streamUrl);

    // squirrel away a bit of state
    request.setAttribute(QNetworkRequest::User, first? 1 : 0);

    QNetworkReply *reply = operation->get(request);

    connect(reply, &QNetworkReply::finished, this, &PrefetchStream::handlePlaylistResponse);
}

void PrefetchStream::handlePlaylistResponse() {
    QNetworkReply* reply = qobject_cast<QNetworkReply *>(sender());

    qDebug() << "playlist response";

    if (reply->error() != QNetworkReply::NoError) {

        qDebug() << "playlist request got HTTP error" << reply->error();
        stop();
        return;
    }

    QByteArray data = reply->readAll();

    bool firstRequest = reply->request().attribute(QNetworkRequest::User).toULongLong() == 1;

    reply->deleteLater();

    QStringList fragments = getPrefetchUrls(data);

    if (firstRequest) {
        /* We still need to decide whether this is a valid request or not and serve headers */
        if (fragments.length() > 0) {
            /* Looks like a realtime stream, start the merry-go-round */
            QString response = "HTTP/1.1 200 OK\r\n";
            response += "Content-Type: video/MP2T\r\n";
            response += "Connection: Closed\r\n";
            response += "\r\n";
            trySendData(response.toUtf8());
        }
        else {
            qDebug() << "giving not found error";
            /* This isn't a realtime stream as far as we can tell; just return a 404 */
            QString response = "HTTP/1.1 404 Not Found\r\n";
            response += "Content-Type: text/plain; charset=utf-8\r\n";
            response += "Connection: Closed\r\n";
            response += "\r\n";
            response += "Can't find a real-time stream at that location\r\n";
            trySendData(response.toUtf8());
            doneWithSocket();
            return;
        }
    }

    // now it's a valid stream in progress with the next fragments ready to request

    if (fragments.length() == 0) {
        consecutiveEmptyPlaylists += 1;
    }
    else {
        consecutiveEmptyPlaylists = 0;
    }

    if (consecutiveEmptyPlaylists > 3) {
        qDebug() << "too many empty playlists in a row, treating this stream as over";
        doneWithSocket();
        return;
    }

    // find the position of the last fragment served in the current fragment list
    int lastFragmentProcessedPos = fragments.indexOf(lastFragmentProcessed);

    qDebug() << "last playlist processed at offset" << lastFragmentProcessedPos;

    // process the next fragment we recognize the previous one, or else the first fragment
    int nextFragmenttoProcess = lastFragmentProcessedPos + 1;

    if (nextFragmenttoProcess < fragments.length()) {
        QString fragment = fragments.at(nextFragmenttoProcess);

        lastFragmentProcessed = fragment;

        //qDebug() << "check pos " << nextFragmenttoProcess << " should be " << fragments.indexOf(lastFragmentProcessed);

        requestAndSendFragment(fragment);
        return;
    }

    // there was nothing to do, we'll need to set a timer so we check the playlist again 
    setupNextPlaylistTimer();
}

void PrefetchStream::trySendData(const QByteArray & data) {
    if (alive) {
        qint64 result = socket->write(data);
        if (result == -1) {
            qDebug() << "an error occurred while writing";
            stop();
        }
    }
}

void PrefetchStream::requestAndSendFragment(const QString &url) {
    currentFragmentBytesSoFar = 0;

    QNetworkRequest request;

    request.setUrl(QUrl(url));

    QNetworkReply * reply = operation->get(request);

    connect(reply, &QNetworkReply::readyRead, this, &PrefetchStream::handleFragmentPart);
    connect(reply, &QNetworkReply::finished, this, &PrefetchStream::handleFragmentFinished);
    connect(reply, &QNetworkReply::aboutToClose, this, &PrefetchStream::handleFragmentAboutToClose);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &PrefetchStream::handleFragmentError);
}


void PrefetchStream::handleFragmentPart() {

    //qDebug() << "block at offset" << currentFragmentBytesSoFar;

    QPointer<QNetworkReply> reply = qobject_cast<QNetworkReply *>(sender());

    if (reply->error() != QNetworkReply::NoError) {

        qDebug() << "fragment reply HTTP error" << reply->error();
        stop();
        return;
    }

    QByteArray data = reply->read(readBlockSize);

    int length = data.length();
    //qDebug() << "Cur data is" << length;

    //qDebug() << "alive?" << alive;

    currentFragmentBytesSoFar += length;

    //qDebug() << "socket is open?" << socket->isOpen();

    trySendData(data);
}

void PrefetchStream::handleFragmentFinished() {

    qDebug() << "finished fragment" << currentFragmentBytesSoFar << "bytes";

    QNetworkReply* reply = qobject_cast<QNetworkReply *>(sender());

    if (alive) {
        /* Based on ~ 2s fragments we are shooting for ~ 60ms blocks */
        if (currentFragmentBytesSoFar / 32 > readBlockSize) {
            qDebug() << "read block size too small, adjusting";
            while (currentFragmentBytesSoFar / 32 > readBlockSize) {
                readBlockSize *= 2;
            }
            qDebug() << "new read block size" << readBlockSize;
        }

        // continue the stream by requesting the latest playlist
        requestPlaylist(false);
    }

    reply->deleteLater();
}

void PrefetchStream::handleFragmentAboutToClose() {
    qDebug() << "fragment about to close";
}

void PrefetchStream::handleFragmentError(QNetworkReply::NetworkError code) {
    qDebug() << "fragment error handler code" << code;
}

void PrefetchStream::setupNextPlaylistTimer() {
    qDebug() << "setting timer for next playlist";
    nextPlaylistTimer.start();
}

void PrefetchStream::timeForNextPlaylist() {
    qDebug() << "hit next playlist timer";
    requestPlaylist(false);
}

QStringList PrefetchStream::getPrefetchUrls(const QByteArray & data) {
    QStringList out;

    const QByteArray PREFETCH_PREFIX = "#EXT-X-TWITCH-PREFETCH:";

    for (const QByteArray & line: data.split('\n')) {
        if (line.startsWith(PREFETCH_PREFIX)) {
            QByteArray url = line.mid(PREFETCH_PREFIX.length());
            out.append(url);
        }
    }

    qDebug() << "got" << out.length() << "prefetch urls";

    return out;
}

void PrefetchStream::doneWithSocket() {
    if (alive) {
        qDebug() << "prefetch stream client socket ending normally";

        alive = false;
        socket->waitForBytesWritten();
        connect(socket, &QTcpSocket::disconnected, this, &PrefetchStream::afterDeadPrefetchDisconnect);
        socket->disconnectFromHost();

        emit died();
    }
}


void PrefetchStream::stop() {
    if (alive) {
        qDebug() << "prefetch stream stopped";
        // FIXME for now let's just close the socket unannounced
        alive = false;

        socket->close();
        socket->deleteLater();

        emit died();
    }
}

void PrefetchStream::afterDeadPrefetchDisconnect() {
    qDebug() << "after prefetch socket disconnect, deleting socket, alive " << alive;
    socket->deleteLater();
}



