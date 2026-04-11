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

    ListModel {
        id: chatModel
    }

    background: Rectangle {
        color: appTheme.background
    }

    signal settingapplied(
        int myId,
        int sendToId,
        int inputDeviceIndex,
        int outputDeviceIndex,
        int outputBufferSize
    )
    signal startSend()
    signal stopSend()

    signal sendMessage(string msg)

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
            id: pttButton
            width: 200
            height: 200
            radius: 100
            anchors.top: parent.top
            anchors.topMargin: 20
            anchors.horizontalCenter: parent.horizontalCenter

            color: transmitting
                ? appTheme.accentRed
                : appTheme.accentOrange

            border.color: appTheme.border
            border.width: 1

            Text {
                anchors.centerIn: parent
                text: transmitting ? "TRANSMITTING" : "PUSH TO TALK"
                color: transmitting ? "white" : "black"
                font.pixelSize: appTheme.fontSize.sm
                font.bold: true
            }

            MouseArea {
                anchors.fill: parent

                onPressed: {
                    if (!handsFree)
                    {
                        pttPressed = true
                        startSend()
                    }
                }
                onReleased: {
                    if (!handsFree)
                    {
                        pttPressed = false
                        stopSend()
                    }
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

        // بخش چت
        Column {
            anchors.top: pttButton.bottom
            anchors.topMargin: 20
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: appTheme.spacing.sm
            spacing: 10

            // نمایش پیام‌ها (readonly)
            Rectangle {
                id: rcvSpace
                width: parent.width
                height: parent.height - 70
                color: appTheme.surface
                border.color: appTheme.border
                border.width: 1
                radius: appTheme.radius.md

                ListView {
                    id: chatListView
                    anchors.fill: parent
                    anchors.margins: 8
                    clip: true
                    model: chatModel
                    spacing: appTheme.spacing.sm

                    delegate: Rectangle {
                        width: chatListView.width
                        radius: 8
                        color: appTheme.surface
                        border.color: appTheme.border
                        border.width: 1

                        implicitHeight: messageText.implicitHeight + 16

                        Text {
                            id: messageText
                            anchors.fill: parent
                            anchors.margins: 8

                            text: model.message
                            textFormat: Text.RichText
                            wrapMode: Text.Wrap
                            font.pixelSize: appTheme.fontSize.sm
                            color: appTheme.textPrimary
                        }
                    }
                }
            }

            // ورودی پیام + دکمه ارسال
            Row {
                width: parent.width
                spacing: 10


                Rectangle {
                    width: parent.width - 80
                    height: 60
                    color: appTheme.surface
                    border.color: appTheme.border
                    border.width: 1
                    radius: appTheme.radius.md

                    CTextField{
                        id: messageInput
                        theme: appTheme
                        anchors.fill: parent
                        anchors.margins: 8
                        placeholderText: qsTr("پیام...")
                    }
                }

                CButton {
                    theme: appTheme
                    text: qsTr("ارسال")
                    width: 70
                    height: 60
                    onClicked: {
                        if (messageInput.text.trim().length > 0) {
                            sendMessage(messageInput.text)
                            messageInput.text = ""
                        }
                    }
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
            anchors.margins: appTheme.spacing.sm

            Text {
                text: "Hands‑Free"
                Layout.fillWidth: true
                color: appTheme.textPrimary
            }

            Switch {
                checked: handsFree
                onCheckedChanged: {
                    handsFree = checked
                    pttPressed = false
                    handsFreeLatched = false
                    if(handsFree)
                    {
                        startSend()
                    }
                    else
                    {
                        stopSend()
                    }
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
            radius: appTheme.radius.md
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
                    outputDeviceIndex,
                    outputBufferSize
                )
                settingsDialog.close()
            }
        }
    }

    Toast {
        id: reportToast
        themeManager: appTheme
    }

    Component.onCompleted: {
        settingapplied.connect(audioBackend.onSettingapplied)
        startSend.connect(audioBackend.onStartSend)
        stopSend.connect(audioBackend.onStopSend)
        sendMessage.connect(audioBackend.onSendMessage)
    }

    Connections {
        target: audioBackend

        function onSetWindowsTittle(_Tittle) {
            title = _Tittle
        }

        function onNewVersionFound(_versionCode){
            reportToast.showMessage(false,"New Version founded\r\nVersion : " + _versionCode)
        }

        function onNewTextMessage(_msg){
            chatModel.insert(0,{message: _msg})
        }

        function onDebugMessage(_state,_msg){
            reportToast.showMessage(_state,_msg)
        }
    }
}
