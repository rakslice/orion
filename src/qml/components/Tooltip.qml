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
import QtQuick.Window 2.0
import "../styles.js" as Styles

Window {
    id: root
    flags: Qt.SplashScreen | Qt.NoFocus | Qt.X11BypassWindowManagerHint | Qt.BypassWindowManagerHint

    property real dpHeight: 320
    property real dpWidth: 512
    height: dp(dpHeight)
    width: dp(dpWidth)

    property string text
    property string img
    property variant lastScreenName: null

    Rectangle {
        id: rootRect
        anchors.fill: parent
        color: "#000000"

        SpinnerIcon {
            id:_spinner
            anchors.fill: parent
            iconSize: 60
        }

        Image {
            id: img
            source: root.img
            anchors.fill: parent
            onProgressChanged: {
                if (progress >= 1.0)
                    _spinner.visible = false
            }

            onSourceChanged: {
                _spinner.visible = true
            }
        }

        //Container for text
        Rectangle {
            id: header
            anchors.fill: text
            color: Styles.shadeColor
            opacity: 0.7
            height: text.height
        }

        Text {
            id: text
            color: Styles.textColor
            text: root.text
            font.pixelSize: Styles.titleFont.smaller
            anchors{
                bottom: parent.bottom
                left: parent.left
                right: parent.right
            }
            wrapMode: Text.WordWrap
            //renderType: Text.NativeRendering
        }
    }

    function display(mX, mY, parentWindow){
        //console.log("tooltip display(", mX, ",", mY, ")");
        if (root.visible || g_contextMenuVisible){
            return;
        }

        var screen;
        if (parentWindow.hasOwnProperty("screen")) {
            screen = parentWindow.screen;
        } else {
            screen = Screen;
        }

        var newRootX = mX + 20;
        var newRootY = mY;
        var newRootWidth = root.dpWidth;
        var newRootHeight = root.dpHeight;

        //console.log("before adjustment tooltip window will be at ", newRootWidth, "x", newRootHeight, "+", newRootX, "+", newRootY);

        var windowRightEdge = newRootX + root.width;
        var offScreenRight = windowRightEdge > Screen.desktopAvailableWidth;

        if (screen.hasOwnProperty("virtualX")) {
            var screenRightEdge = screen.virtualX + screen.width;
            //console.log("screenRightEdge", screenRightEdge);
            if (windowRightEdge > screenRightEdge) {
                offScreenRight = true;
            }
        }

        if (offScreenRight) {
            //console.log("Tooltip would be off the right, adjusting");
            // move to the left of the cursor
            newRootX -= root.width + 40;
        }

        var windowBottomEdge = newRootY + root.height;
        var offScreenBottom = windowBottomEdge > Screen.desktopAvailableHeight;

        if (screen.hasOwnProperty("virtualY")) {
            var screenBottomEdge = screen.virtualY + screen.height;
            //console.log("screenBottomEdge", screenBottomEdge);
            if (windowBottomEdge > screenBottomEdge) {
                offScreenBottom = true;
            }
        }

        if (offScreenBottom) {
            //console.log("Tooltip would be off the bottom, adjusting");
            // move above the line of the cursor
            newRootY -= root.height;
        }

        if (offScreenBottom || offScreenRight)
            //console.log("after adjustment tooltip window will be at ", newRootWidth, "x", newRootHeight, "+", newRootX, "+", newRootY);

        // ISSUE
        // It's tricky to actually get the tooltip window to move to a different screen -- its size seems to get be calculated
        // incorrectly based on geometry of the last screen it was on, and its screen will be updated based on where the
        // center of the tooltip is according to the bad geometry. We may need to game this to get it to converge to the screen we want it on.
        // Furthermore, it gets recalculated for each individual property we set even though the window isn't visible yet.
        // WORKAROUND
        // - Position it, and then reposition it again after a pause
        // - Make it not cross a screen boundary

        //console.log("Screen name " + screen.name);
        var screenChanged = (lastScreenName !== null) && (lastScreenName !== screen.name);
        lastScreenName = screen.name;

        if (screenChanged) {
            // While we move the window, make it 1x1 px so it doesn't cross a screen boundary
            //console.log("before minimal width");
            root.width = 1;
            //console.log("before minimal height");
            root.height = 1;
            // make it transparent
            //root.opacity = 0;
        } else {
            root.width = newRootWidth;
            root.height = newRootHeight;
        }

        // now move it
        //console.log("setting Tooltip window coordinates; before x");
        root.x = newRootX;
        //console.log("before y");
        root.y = newRootY;

        //console.log("before Tooltip window show()");
        root.show();
        //console.log("after Tooltip window show()");

        if (screenChanged) {
            secondSetX = newRootX;
            secondSetY = newRootY;
            secondSetWidth = newRootWidth;
            secondSetHeight = newRootHeight;
            secondSetTimer.start();
        }
    }

    property int secondSetX
    property int secondSetY
    property int secondSetWidth
    property int secondSetHeight

    Timer {
        id: secondSetTimer
        interval: 20
        repeat: false
        onTriggered: {
            //console.log("second set");
            root.x = secondSetX;
            root.y = secondSetY;
            root.width = secondSetWidth;
            root.height = secondSetHeight;
        }
    }
}
