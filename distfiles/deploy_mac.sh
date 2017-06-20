#!/bin/bash
set -e -x

QTVER=5.9.0

QTDIR=/usr/local/Cellar/qt/$QTVER

# macdeploy
$QTDIR/bin/macdeployqt orion.app -qmldir=./src/qml

# qml libs 
mkdir orion.app/Contents/Resources/qml
#cp -r $QTDIR/qml/Enginio orion.app/Contents/Resources/qml
cp -r $QTDIR/qml/Qt orion.app/Contents/Resources/qml
cp -r $QTDIR/qml/QtQml orion.app/Contents/Resources/qml
cp -r $QTDIR/qml/QtQuick orion.app/Contents/Resources/qml
cp -r $QTDIR/qml/QtQuick.2 orion.app/Contents/Resources/qml
#cp -r $QTDIR/qml/Communi orion.app/Contents/Resources/qml

#sh fixlibs.sh orion.app

