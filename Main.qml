import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "components"
import "theme"

ApplicationWindow {
    id: window
    width: 360
    height: 640
    visible: true
    title: "Walkie Talkie"

    Theme { id: appTheme }

    background: Rectangle {
        color: appTheme.background
    }

    signal settingapplied(
        int myId,
        int sendToId,
        int inputDeviceIndex,
        int outputDeviceIndex
    )

    /* ========= STATE ========= */
    property bool handsFree: false
    property bool pttPressed: false
    property bool handsFreeLatched: false

    readonly property bool transmitting:
        handsFree ? handsFreeLatched : pttPressed

    /* ========= HEADER ========= */
    WalkieHeader {
        id: header
        theme: appTheme
        width: parent.width
        anchors.top: parent.top

        onSettingsClicked: settingsDialog.open()
    }

    /* ========= MAIN ========= */
    Item {
        anchors.top: header.bottom
        anchors.bottom: footer.top
        anchors.left: parent.left
        anchors.right: parent.right

        Rectangle {
            width: 200
            height: 200
            radius: 100
            anchors.centerIn: parent

            color: transmitting
                ? appTheme.accentRed
                : appTheme.accentOrange

            border.color: appTheme.border
            border.width: 1

            Text {
                anchors.centerIn: parent
                text: transmitting ? "TRANSMITTING" : "PUSH TO TALK"
                color: transmitting ? "white" : "black"
                font.pixelSize: appTheme.fontSizeMedium
                font.bold: true
            }

            MouseArea {
                anchors.fill: parent

                onPressed: {
                    if (!handsFree)
                        pttPressed = true
                }
                onReleased: {
                    if (!handsFree)
                        pttPressed = false
                }
                onCanceled: {
                    if (!handsFree)
                        pttPressed = false
                }
                onClicked: {
                    if (handsFree)
                        handsFreeLatched = !handsFreeLatched
                }
            }
        }
    }

    /* ========= FOOTER ========= */
    Rectangle {
        id: footer
        height: 72
        width: parent.width
        anchors.bottom: parent.bottom
        color: appTheme.surface
        border.color: appTheme.border
        border.width: 1

        RowLayout {
            anchors.fill: parent
            anchors.margins: appTheme.spacing

            Text {
                text: "Hands‑Free"
                Layout.fillWidth: true
                color: appTheme.text
            }

            Switch {
                checked: handsFree
                onCheckedChanged: {
                    handsFree = checked
                    pttPressed = false
                    handsFreeLatched = false
                }
            }
        }
    }

    /* ========= SETTINGS DIALOG ========= */
    Dialog {
        id: settingsDialog
        modal: true

        // ✅ دقیقاً وسط صفحه
        anchors.centerIn: Overlay.overlay

        // ✅ اندازه منطقی دیالوگ
        width: 300
        height: 400

        // ✅ حذف padding پیش‌فرض Dialog
        padding: 0

        // ✅ background واقعی Dialog
        background: Rectangle {
            color: appTheme.surface
            radius: appTheme.radius
            border.color: appTheme.border
            border.width: 1
        }

        // ✅ content دقیقاً هم‌اندازه Dialog
        contentItem: SettingsPage {
            anchors.fill: parent
            theme: appTheme

            onCancelClicked: {
                settingsDialog.close()
            }
            onApplyClicked: {
                settingapplied(
                    myId,
                    sendToId,
                    inputDeviceIndex,
                    outputDeviceIndex
                )
                settingsDialog.close()
            }
        }
    }

    Component.onCompleted: {
        settingapplied.connect(audioBackend.onSettingapplied)
    }

    Connections {
        target: audioBackend
        function onSetWindowsTittle(_Tittle) {
            title = _Tittle
        }
    }
}
