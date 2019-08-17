#!/bin/bash
set -e -x

case "$(lsb_release -is)" in
	Ubuntu|Debian)
		if [ "$(lsb_release -rs)" == "16.04" ]; then
			#sudo add-apt-repository -y ppa:beineri/opt-qt-5.11.1-xenial
			sudo add-apt-repository -y ppa:beineri/opt-qt-5.12.3-xenial
			sudo apt update
			#sudo apt-get install -y g++ make libmpv-dev qt511-meta-minimal qt511declarative qt511svg qt511quickcontrols qt511quickcontrols2 libgl1-mesa-dev
			sudo apt-get install -y g++ make libmpv-dev qt512-meta-minimal qt512declarative qt512svg qt512quickcontrols qt512quickcontrols2 libgl1-mesa-dev
		else
			sudo apt-get install -y g++ make libmpv-dev qt5-default qtdeclarative5-dev libqt5svg5-dev qtquickcontrols2-5-dev qml-module-qtquick-controls qml-module-qtquick-controls2 qml-module-qt-labs-settings
		fi
		;;

esac

