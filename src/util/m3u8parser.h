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

#ifndef M3U8PARSER_H
#define M3U8PARSER_H

#include <QVariantMap>
#include <QMap>
#include <QByteArray>
#include <QDateTime>

namespace m3u8 {

    static QVariantMap getTimeInfo(const QByteArray &data) {
        QVariantMap out;

        out.insert("metadataEpochTimeMS", QDateTime::currentMSecsSinceEpoch());

        const QString INFO_PREFIX = "#EXT-X-TWITCH-INFO:";
        for (auto line : QString(data).split("\n")) {
            if (line.startsWith(INFO_PREFIX)) {
                for (QString entry : line.mid(INFO_PREFIX.length()).split(",")) {
                    int eqPos = entry.indexOf('=');
                    if (eqPos != -1) {
                        QString key = entry.left(eqPos);
                        QString value = entry.mid(eqPos + 1);
                        if (value.startsWith('"') && value.endsWith('"')) {
                            value = value.mid(1, value.length() - 2);
                        }
                        if (key == "STREAM-TIME") {
                            out.insert("streamTimeS", value.toDouble());
                        }
                        else if (key == "BROADCAST-ID") {
                            out.insert("broadcastId", value);
                        }
                    }
                }
                break;
            }
        }
        return out;
    }

    static QVariantMap getUrls(const QByteArray &data)
    {
        QVariantMap streams;

        QString streamName;

        qDebug() << "m3u8:\n" << data;

        foreach(QString str, QString(data).split("\n")){

            if (str.contains("VIDEO=")){
                str.remove(0, str.indexOf("VIDEO=") + 6);
                str.replace("\"","");
                streamName = str;
            }
            else if (!streamName.isEmpty() && str.startsWith("http://")){

                if (streamName == "chunked")
                    streamName = "source";

                streams.insert(streamName, str);

//                qDebug() << lastKey << ", " << str;

//                if (lastKey == "mobile")
//                    streams[0] = str;

//                else if (lastKey == "low")
//                    streams[1] = str;

//                else if (lastKey == "medium")
//                    streams[2] = str;

//                else if (lastKey == "high")
//                    streams[3] = str;

//                else if (lastKey == "chunked")
//                    streams[4] = str;

                streamName.clear();
            }
        }

        return streams;
    }
}

#endif // M3U8PARSER_H
