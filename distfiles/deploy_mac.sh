#!/bin/bash
set -e -x

QTVER=5.9

# macdeploy
~/Qt/$QTVER/clang_64/bin/macdeployqt orion.app -qmldir=~/git/orion/src/qml

# qml libs 
mkdir orion.app/Contents/Resources/qml
#cp -r ~/Qt/$QTVER/clang_64/qml/Enginio orion.app/Contents/Resources/qml
cp -r ~/Qt/$QTVER/clang_64/qml/Qt orion.app/Contents/Resources/qml
cp -r ~/Qt/$QTVER/clang_64/qml/QtQml orion.app/Contents/Resources/qml
cp -r ~/Qt/$QTVER/clang_64/qml/QtQuick orion.app/Contents/Resources/qml
cp -r ~/Qt/$QTVER/clang_64/qml/QtQuick.2 orion.app/Contents/Resources/qml
#cp -r ~/Qt/$QTVER/clang_64/qml/Communi orion.app/Contents/Resources/qml

#sh fixlibs.sh orion.app

