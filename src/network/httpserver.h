#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QDataStream>
#include <QMap>
#include <QDebug>
#include <QList>

#include "../model/singletonprovider.h"
#include "prefetchstream.h"

class HttpServer: public QObject
{
    QML_SINGLETON
    Q_OBJECT

    QTcpServer *server = 0;

    bool listenError = false;
    QString m_port;

    static HttpServer *instance;

    explicit HttpServer(QObject *parent = 0);

    QList<PrefetchStream *> prefetchStreams;

    QNetworkAccessManager * networkAccessManager;
public:
    static HttpServer *getInstance();

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
