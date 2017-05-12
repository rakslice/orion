/*
 * Copyright © 2015-2016 Antti Lamminsalo
 *
 * This file is part of Orion.
 *
 * Orion is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with Orion.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QUrl>
#include "../model/channel.h"
#include "../model/game.h"
#include "../model/vod.h"
#include "../network/replaychat.h"

/**
 * @brief The JsonParser class
 * Handles parsing of json documents to business logic-objects
 */

class JsonParser
{
public:
    static QList<Channel*> parseStreams(const QByteArray&);
    static QList<Game*> parseGames(const QByteArray&);
    static QList<Channel*> parseChannels(const QByteArray&);
    static QList<Channel*> parseFavourites(const QByteArray&);
    static QList<Channel*> parseFeatured(const QByteArray&);
    static QList<Vod *> parseVods(const QByteArray&);
    static Game* parseGame(const QJsonObject&);
    static Channel* parseStream(const QByteArray&);
    static Channel* parseStreamJson(const QJsonObject&, const bool expectChannel);
    static Channel* parseChannel(const QByteArray&);
    static Channel* parseChannelJson(const QJsonObject&);
    static Vod* parseVod(const QJsonObject&);
    static QString parseChannelStreamExtractionInfo(const QByteArray&);
    static QString parseVodExtractionInfo(const QByteArray&);
    static QPair<QString, quint64> parseUser(const QByteArray&);
    static int parseTotal(const QByteArray&);
    static QMap<int, QMap<int, QString>> parseEmoteSets(const QByteArray&);
    static QMap<QString, QMap<QString, QString>> parseChannelBadgeUrls(const QByteArray &data);
    static QMap<QString, QMap<QString, QMap<QString, QString>>> parseBadgeUrlsBetaFormat(const QByteArray &data);
    static QList<ReplayChatMessage> parseVodChatPiece(const QByteArray &data);
    static QMap<QString, QList<QString>> parseChatterList(const QByteArray &data);
    static void parseBitsData(const QByteArray &data, QMap<QString, QMap<QString, QString>> & outUrls, QMap<QString, QMap<QString, QString>> & outColors);
    static void setHiDpi(bool setting);
private:
    static bool hiDpi;
};

#endif // JSONPARSER_H
