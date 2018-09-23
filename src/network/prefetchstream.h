#ifndef PREFETCHSTREAM_H
#define PREFETCHSTREAM_H

#include <QObject>
#include <QTcpSocket>
#include <QDebug>
#include <QTimer>
#include <QUrl>
#include <QNetworkReply>
#include <QQueue>


class QNetworkAccessManager;


class PrefetchStream : public QObject
{
    Q_OBJECT

public:
	explicit PrefetchStream(QTcpSocket * socket, QString streamUrl, QNetworkAccessManager *operation, QObject * parent=0);
    virtual ~PrefetchStream();
    void start();
    void stop();

private:
    QTcpSocket * socket;
    QUrl streamUrl;
    QNetworkAccessManager *operation;
    QString lastFragmentProcessed;
    bool alive;
    int consecutiveEmptyPlaylists;
    int readBlockSize;
    int currentFragmentBytesSoFar;
    qint64 unsentDataSize;

    QTimer nextPlaylistTimer;
    QQueue<QString> prefetchUrlsQueue;

    QStringList getPrefetchUrls(const QByteArray & data);
    void requestPlaylist(bool first);
    void requestAndSendFragment(const QString &url);
    void setupNextPlaylistTimer();
    void doneWithSocket();

    void trySendData(const QByteArray & data);

private slots:
    void handlePlaylistResponse();
    void handleFragmentPart();
    void handleFragmentAboutToClose();
    void handleFragmentError(QNetworkReply::NetworkError code);
    void handleFragmentFinished();
    void timeForNextPlaylist();
    void handleSocketBytesWritten(qint64 bytes);
    void afterDeadPrefetchDisconnect();

signals:
    void died();
};

#endif // PREFETCHSTREAM_H
