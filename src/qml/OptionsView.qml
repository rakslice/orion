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
import "components"
import "styles.js" as Styles

Item{
    anchors.fill: parent
    id: root

    ViewHeader {
        id: header
        text: "Settings"
    }

    Column {
        width: dp(360)

        spacing: 1

        anchors {
            top: header.bottom
            horizontalCenter: parent.horizontalCenter
        }

        OptionCheckbox {
            id: lowLatencyStreamsOption
            anchors {
                left: parent.left
                right: parent.right
            }
            checked: g_cman.lowLatencyStreams
            onClicked: {
                g_cman.lowLatencyStreams = !g_cman.lowLatencyStreams
            }
            text: "Use Low Latency Streams"
        }

        OptionCheckbox {
            id: alertOption
            anchors {
                left: parent.left
                right: parent.right
            }
            checked: g_cman.isAlert()
            onClicked: {
                g_cman.setAlert(checked)
            }
            text: "Enable notifications"
        }


        OptionCheckbox {
            id: notificationsOption
            enabled: alertOption.checked
            anchors {
                left: parent.left
                right: parent.right
            }
            checked: g_cman.offlineNotifications
            onClicked: {
                g_cman.offlineNotifications = !g_cman.offlineNotifications
            }
            text: "Show offline notifications"
        }

        OptionCornerSelect {
            id: alertPosition
            selection: g_cman.getAlertPosition()

            onSelectionChanged: {
                g_cman.setAlertPosition(selection)
            }

            anchors {
                left: parent.left
                right: parent.right
            }
            text: "Notification position"
        }

        OptionCheckbox {
            id: minStartupOption
            anchors {
                left: parent.left
                right: parent.right
            }
            checked: g_cman.isMinimizeOnStartup()
            onClicked: {
                g_cman.setMinimizeOnStartup(checked)
            }
            text: "Start minimized"
        }

        OptionCheckbox {
            id: closeToTrayOption
            anchors {
                left: parent.left
                right: parent.right
            }
            checked: g_cman.isCloseToTray()
            onClicked: {
                g_cman.setCloseToTray(checked)
            }
            text: "Close to tray"
        }
	
        OptionCheckbox {
            id: chatSwapOption
            anchors {
                left: parent.left
                right: parent.right
            }
            checked: g_cman.swapChat
            onClicked: {
                g_cman.swapChat = !g_cman.swapChat
            }
            text: "Swap Chat Side"
        }

        OptionEntry {
            anchors {
                left: parent.left
                right: parent.right
            }
            text: "Chat Text Scale Factor"


            ComboBox {
                id: textScaleFactor
                //currentText: g_cman.getTextScaleFactor()

                width: dp(90)
                height: dp(40)

                anchors {
                    verticalCenter: parent.verticalCenter
                    right: parent.right
                }

                Component.onCompleted: {
                    var entries = ["0.5", "0.75", "1.0", "1.25", "1.5", "1.75", "2", "2.5", "3"];
                    textScaleFactor.entries = entries;

                    var val = g_cman.textScaleFactor;
                    for (var i = 0; i < entries.length; i++) {
                        var cur = entries[i];
                        if (parseFloat(cur) == val) {
                            textScaleFactor.selectItem(cur);
                            break;
                        }
                    }

                }

                compareFunction: function(a, b) {
                    return parseFloat(a) - parseFloat(b);
                }

                onItemChanged: {
                    g_cman.textScaleFactor = parseFloat(item)
                }
            }

        }

        OptionEntry {
            id: loginOption
            text: "Twitch account"
            anchors {
                left: parent.left
                right: parent.right
            }
            BasicButton {
                id: connectButton

                property bool loggedIn: g_cman.isAccessTokenAvailable()
                text: loggedIn ? "Log out" : "Log in"
                anchors {
                    verticalCenter: parent.verticalCenter
                    right: parent.right
                }
                onClicked: {
                    if (!loggedIn) {
                        httpServer.start();
                        var url = "https://api.twitch.tv/kraken/oauth2/authorize?response_type=token&client_id=" + netman.getClientId()
                                + "&redirect_uri=http://localhost:8979"
                                + "&scope=user_read%20user_subscriptions%20user_follows_edit%20chat_login%20user_blocks_read%20user_blocks_edit"
                                + "&force_verify=true";
                        Qt.openUrlExternally(url);


                    }
                    else {
                        g_cman.setAccessToken("")
                        g_cman.checkFavourites()
                        loginOption.text = "Twitch account"
                    }
                }

                Connections {
                    target: g_cman
                    onAccessTokenUpdated: {
                        connectButton.loggedIn = g_cman.isAccessTokenAvailable()
                    }
                    onUserNameUpdated: {
                        loginOption.text = "Logged in as " + name
                    }
                }
            }
        }
    }

    Text {
        anchors {
            bottom: parent.bottom
            right: parent.right
            margins: dp(10)
        }

        font.pixelSize: dp(14)
        color: Styles.white
        text: app_version
    }
}
