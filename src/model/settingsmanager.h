#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QSettings>
#include "singletonprovider.h"

class SettingsManager : public QObject
{
    QML_SINGLETON
    Q_OBJECT

    Q_PROPERTY(bool alert READ alert WRITE setAlert NOTIFY alertChanged)
    Q_PROPERTY(bool closeToTray READ closeToTray WRITE setCloseToTray NOTIFY closeToTrayChanged)
    Q_PROPERTY(int alertPosition READ alertPosition WRITE setAlertPosition NOTIFY alertPositionChanged)
    Q_PROPERTY(int volumeLevel READ volumeLevel WRITE setVolumeLevel NOTIFY volumeLevelChanged)
    Q_PROPERTY(bool minimizeOnStartup READ minimizeOnStartup WRITE setMinimizeOnStartup NOTIFY minimizeOnStartupChanged)
    Q_PROPERTY(int chatEdge READ chatEdge WRITE setChatEdge NOTIFY chatEdgeChanged)
    Q_PROPERTY(bool offlineNotifications READ offlineNotifications WRITE setOfflineNotifications NOTIFY offlineNotificationsChanged)
    Q_PROPERTY(double textScaleFactor READ textScaleFactor WRITE setTextScaleFactor NOTIFY textScaleFactorChanged)
    Q_PROPERTY(QString quality READ quality WRITE setQuality NOTIFY qualityChanged)
    Q_PROPERTY(QString accessToken READ accessToken WRITE setAccessToken NOTIFY accessTokenChanged)
    Q_PROPERTY(bool hasAccessToken READ hasAccessToken NOTIFY accessTokenChanged)
    Q_PROPERTY(bool lightTheme READ lightTheme WRITE setLightTheme NOTIFY lightThemeChanged)
    Q_PROPERTY(QString font READ font WRITE setFont NOTIFY fontChanged)
    Q_PROPERTY(bool versionCheckEnabled READ versionCheckEnabled)
    Q_PROPERTY(bool keepOnTop READ keepOnTop WRITE setKeepOnTop NOTIFY keepOnTopChanged)
    Q_PROPERTY(bool lowLatencyStreams READ lowLatencyStreams WRITE setLowLatencyStreams NOTIFY lowLatencyStreamsChanged)

    bool mAlert;
    bool mCloseToTray;
    int mAlertPosition;
    int mVolumeLevel;
    bool mMinimizeOnStartup;
    bool mSwapChat;
    bool mOfflineNotifications;
    double mTextScaleFactor;
    QString mQuality;
    QString mAccessToken;
    int mChatEdge;
    bool mLightTheme;
    QString mFont;

    bool mHiDpi;
    bool mKeepOnTop;

    bool mLowLatencyStreams;

    explicit SettingsManager(QObject *parent = nullptr);
    static SettingsManager *instance;
public:
    static SettingsManager *getInstance();

    bool alert() const;
    void setAlert(bool alert);

    bool closeToTray() const;
    void setCloseToTray(bool closeToTray);

    int alertPosition() const;
    void setAlertPosition(int alertPosition);

    int volumeLevel() const;
    void setVolumeLevel(int volumeLevel);

    bool minimizeOnStartup() const;
    void setMinimizeOnStartup(bool minimizeOnStartup);

    int chatEdge() const;
    void setChatEdge(int chatEdge);

    bool offlineNotifications() const;
    void setOfflineNotifications(bool offlineNotifications);

    double textScaleFactor() const;
    void setTextScaleFactor(double textScaleFactor);

    QString quality() const;
    void setQuality(const QString &quality);

    QString accessToken() const;

    void setHiDpi(bool dpi);

    bool lightTheme() const;
    void setLightTheme(bool lightTheme);

    QString font() const;
    void setFont(const QString &font);

    bool versionCheckEnabled();

    bool keepOnTop() const;
    void setKeepOnTop(bool keepOnTop);

    bool lowLatencyStreams();
    void setLowLatencyStreams(bool value);

signals:
    void alertChanged();
    void closeToTrayChanged();
    void alertPositionChanged();
    void volumeLevelChanged();
    void minimizeOnStartupChanged();
    void chatEdgeChanged();
    void offlineNotificationsChanged();
    void textScaleFactorChanged();
    void qualityChanged();
    void lightThemeChanged();
    void accessTokenChanged(QString accessToken);
    void fontChanged();
    void keepOnTopChanged();
    void lowLatencyStreamsChanged();

public slots:
    void setAccessToken(const QString accessToken);
    bool hasAccessToken() const;
    void load();

    bool hiDpi() const;
    QString appName() const;
    QString appVersion() const;
    QString appPlayerBackend() const;
    bool isNewerVersion(QString version) const;

private:
    QSettings *settings;
};

#endif // SETTINGSMANAGER_H
