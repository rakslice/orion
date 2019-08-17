#!/bin/bash
set -e -x

case "$(lsb_release -is)" in
	Ubuntu|Debian)
		sudo apt-get install g++ make libmpv-dev qt5-default qtdeclarative5-dev libqt5svg5-dev qtquickcontrols2-5-dev qml-module-qtquick-controls qml-module-qtquick-controls2 qml-module-qt-labs-settings
		;;

esac

