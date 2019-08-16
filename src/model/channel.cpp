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

#include "channel.h"

Channel::~Channel(){
    //qDebug() << "Destroyer: Channel";
}

void Channel::updateWith(const Channel &other, bool hasViewerCount)
{
    if (this->getId() == other.getId()) {
        this->setName(other.getName());
        this->setLogourl(other.getLogourl());
        this->setInfo(other.getInfo());
        if (hasViewerCount) {
            this->setViewers(other.getViewers());
        }
        this->setGame(other.getGame());
    }
}

quint32 Channel::getId() const
{
    return id;
}

void Channel::setId(const quint32 &value)
{
    id = value;
}

bool Channel::isFavourite() const
{
    return favourite;
}

void Channel::setFavourite(bool value)
{
    favourite = value;
}

Channel::Channel(){
    alert = true;
    timestamp = 0;
    online = false;
    viewers = 0;
    id = 0;
    favourite = false;
}

Channel::Channel(const quint64 newId) : Channel() {
    this->id = newId;
}

Channel::Channel(const Channel &channel){
    this->id = channel.id;
	this->name = channel.name;
    this->serviceName = channel.serviceName;
	this->info = channel.info;
	this->alert = channel.alert;
	this->timestamp = channel.timestamp;
	this->online = channel.online;
    this->logouri = channel.logouri;
    this->previewuri = channel.previewuri;
    this->game = channel.game;
    this->viewers = channel.viewers;
    this->favourite = channel.favourite;
}

const QString Channel::lastOnline(){
    if (timestamp == 0){
        return "Never";
    }
    else{
        return QDateTime::fromMSecsSinceEpoch(timestamp).toString("MMM dd, hh:mm");
    }
}

const QJsonObject Channel::getJSON() const{
    QVariantMap map;
    map["title"]    = QVariant(name);
    map["uri"]      = QVariant(serviceName);
    map["info"]     = QVariant(info);
    map["logo"]     = QVariant(logouri);
    map["preview"]  = QVariant(previewuri);
    map["alert"]    = QVariant(alert);
    map["lastSeen"] = QVariant(timestamp);
    map["id"]       = QVariant(id);
    return QJsonObject::fromVariantMap(map);
}

void Channel::writeToSettings(QSettings &settings) const
{
    settings.setValue("name", name);
    settings.setValue("serviceName", serviceName);
    settings.setValue("info", info);
    settings.setValue("logouri", logouri);
    settings.setValue("previewuri", previewuri);
    settings.setValue("alert", alert);
    settings.setValue("timestamp", timestamp);
    settings.setValue("id", id);
}

Channel::Channel(const QSettings &settings) : viewers(-1), online(false), favourite(false)
{
    //Deserialization constructor

    name = settings.value("name").toString();
    serviceName = settings.value("serviceName").toString();
    info = settings.value("info").toString();
    logouri = settings.value("logouri").toString();
    previewuri = settings.value("previewuri").toString();
    alert = settings.value("alert").toBool();
    timestamp = settings.value("timestamp").toLongLong();
    id = settings.value("id").toInt();
}

void Channel::setName(const QString &newName){
	name = newName;

}

void Channel::setServiceName(const QString &newUri){
    serviceName = newUri;
}

void Channel::setInfo(const QString &newInfo){
	info = newInfo;
}

void Channel::setAlert(bool newAlert){
    alert = newAlert;
}

void Channel::updateTime(){
    timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
}

qint64 Channel::getTime() const{
	return timestamp;
}

const QString Channel::getName() const{
	return name;
}

const QString Channel::getServiceName() const{
    return serviceName;
}

const QString Channel::getFullUri() const{
    return "http://twitch.tv/" + serviceName;
}

const QString Channel::getInfo() const{
	return info;
}

bool Channel::hasAlert(){
    return alert;
}


void Channel::setOnline(bool b){
	online = b;
    if (online)
        updateTime();
    emit updated();
}

bool Channel::isOnline(){
	return online;
}

void Channel::setLogourl(const QString &uri){
    logouri = uri;
}

const QString Channel::getLogourl() const{
    return !logouri.isEmpty() ? logouri : DEFAULT_LOGO_URL;
}

void Channel::setPreviewurl(const QString &uri){
    previewuri = uri;
}

const QString Channel::getPreviewurl(){
    return previewuri;
}

void Channel::setLastSeen(time_t time){
    this->timestamp = time;
}

bool Channel::greaterThan (Channel* a, Channel* b) {
    if (a->isOnline() == b->isOnline()){ //BOTH ONLINE OR BOTH OFFLINE
        if (a->isOnline()){  //BOTH ONLINE, COMPARISON BY VIEWER COUNT
            return (a->getViewers() >= b->getViewers());
        }
        else{ //BOTH OFFLINE, COMPARISON BY DEFAULT QSTRING METHOD
            return (QString::compare(a->getName(),b->getName()) < 0);
        }
    }
    return a->isOnline();    //OTHER IS ONLINE AND OTHER IS NOT
}

qint32 Channel::getViewers() const
{
    return online ? viewers : -1;
}

void Channel::setViewers(qint32 value)
{
    viewers = value;
}

const QString Channel::getGame() const
{
    return game;
}

void Channel::setGame(const QString &value)
{
    game = value;
}
