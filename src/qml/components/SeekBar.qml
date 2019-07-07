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

import QtQuick 2.5

import "../styles.js" as Styles
import "../util.js" as Util

Item {
    property int duration: 0
    property int position: 0
    property alias containsMouse: mouseArea.containsMouse

    id: root

    signal userChangedPosition(int position)

    function setPosition(position, duration){
        root.duration = duration
        root.position = position

        var fraction = position / duration
        fillBar.width = Math.min(seekBar.width, Math.floor(fraction * seekBar.width))

        time.updateTime()
    }

    function seekForwardStep() {
        // TODO: check if there are any conditions where we should be ignoring key input right now
        console.log("forward seek step");
        if (position < duration - 30) {
            userChangedPosition(position + 30);
        }
    }

    function seekBackStep() {
        // TODO: check if there are any conditions where we should be ignoring key input right now
        console.log("backward seek stop");
        if (position > 30) {
            userChangedPosition(position - 30);
        } else {
            userChangedPosition(0);
        }
    }

    onDurationChanged: {
        if (isVod)
            time.duration = Util.getTime(duration)
    }

    onPositionChanged: {
        if (isVod)
            time.position = Util.getTime(position)
    }

    Rectangle {
        id: seekBar
        color: Styles.seekBar

        anchors {
            left: parent.left
            right: parent.right
            leftMargin: dp(10)
            rightMargin: dp(10)

            verticalCenter: parent.verticalCenter
        }

        height: dp(6)

        Rectangle {
            color: "white"
            id: fillBar

            width: 0
            anchors {
                left: parent.left
                top: parent.top
                bottom: parent.bottom
            }
        }
    }

    MouseArea {
        id: mouseArea
        visible: isVod
        enabled: visible
        anchors {
            top: parent.top
            bottom: parent.bottom
            left: seekBar.left
            right: seekBar.right
            topMargin: dp(10)
            bottomMargin: dp(10)
        }

        cursorShape: Qt.PointingHandCursor

        hoverEnabled: true

        propagateComposedEvents: false
        onClicked: {
            var _pos = Math.round((mouseX / seekBar.width) * duration)
            console.log("User changed pos: ", _pos, duration)
            userChangedPosition(_pos)
        }

        onPositionChanged: {
            var _pos = Math.round((mouseX / seekBar.width) * duration)
            hoverlabel.text = Util.getTime(_pos)
        }

        Rectangle {
            id: hoverlabel
            property alias text: _label.text

            anchors.bottom: parent.top
            height: dp(40)
            width: dp(100)
            visible: parent.containsMouse
            color: Styles.bg

            x: mouseArea.mouseX - width / 2

            radius: dp(7)

            border {
                color: Styles.border
                width: dp(1)
            }

            Text {
                id: _label
                anchors.fill: parent
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                color: Styles.textColor
                font.pixelSize: Styles.titleFont.regular
            }
        }
    }

    Item {

        property string duration
        property string position

        id: time
        anchors {
            top: parent.top
            topMargin: dp(6)
            horizontalCenter: parent.horizontalCenter
        }

        height: _time.contentHeight
        width: _time.contentWidth

        function updateTime() {
            _time.text = position + "/" + duration
        }

        Text {
            id: _time
            text: "hh:mm:ss / hh:mm:ss"
            color: Styles.iconColor
            font.bold: true
            font.pixelSize: Styles.titleFont.extrasmall
            wrapMode: Text.WordWrap
            //renderType: Text.NativeRendering
        }
    }
}
