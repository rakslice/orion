#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QDataStream>
#include <QMap>
#include <QDebug>
#include <QList>

#include "prefetchstream.h"
class HttpServer: public QObject
{
    Q_OBJECT

    QTcpServer *server = 0;

    bool listenError = false;
    QString m_port;


    QList<PrefetchStream *> prefetchStreams;

    QNetworkAccessManager * networkAccessManager;
public:
    explicit HttpServer(QObject *parent = 0);
    Q_INVOKABLE QString port();

    bool isOk() const;

    void setNetworkAccessManager(QNetworkAccessManager * networkAccessManager);

public slots:
    // starts server
    void start();

    // destroys server
    void stop();

    void onConnect();

    void onRead();

private slots:
    void prefetchStreamDied();
        
signals:
    void codeReceived(QString code);
    void error();
};

#endif // HTTPSERVER_H
