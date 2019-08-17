#!/bin/bash
set -e -x
QT_PREFIX=/usr

if [ "$(lsb_release -is)" == "Ubuntu" ]; then
	if [ "$(lsb_release -rs)" == "16.04" ]; then
		QT_PREFIX=/opt/qt512
	fi
fi
${QT_PREFIX}/bin/qmake orion.pro
make -j4
