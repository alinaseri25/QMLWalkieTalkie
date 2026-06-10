import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../theme"

Rectangle {
    id: root
    required property var theme
    signal settingsClicked()

    height: theme.headerHeight
    width: parent.width
    color: theme.surface
    border.color: theme.border
    border.width: 1

    Text {
        anchors.centerIn: parent
        text: theme.isRTL ? "واکی‌تاکی" : "Walkie Talkie"
        font.pixelSize: theme.fontSize.md
        font.bold: true
        color: theme.textPrimary
    }

    /* ========= LANGUAGE BUTTON (سمت مخالف) ========= */
    Rectangle {
        id: langPanel
        width: 42
        height: 42
        radius: width / 2
        color: theme.surfaceAlt

        // 👇 مهم — سمت چپ تعریف می‌کنیم
        // Mirroring خودش جای آن را عوض می‌کند
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: theme.spacing.sm

        Text {
            anchors.centerIn: parent
            text: theme.isRTL ? "🇮🇷" : "🇬🇧"
            font.pixelSize: 18
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                theme.isRTL = !theme.isRTL
            }
        }
    }

    Rectangle {
        width: 42
        height: 42
        radius: 42/2
        anchors.right: settigPanel.left
        anchors.verticalCenter: parent.verticalCenter
        anchors.rightMargin: theme.spacing.sm
        color: root.theme.surfaceAlt

        CButton {
            id: showAbout
            theme: root.theme

            text: "i"
            implicitWidth: 42
            implicitHeight: 42
            radius: implicitHeight / 2

            backgroundColor: root.theme.surfaceAlt
            textColor: root.theme.textOnAccent

            Layout.alignment: Qt.AlignVCenter

            onClicked: aboutDialog.open()
        }
    }

    Rectangle {
        id: settigPanel
        width: 42
        height: 42
        radius: theme.radius.md
        color: theme.hoverColor

        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.rightMargin: theme.spacing.sm

        Text {
            anchors.centerIn: parent
            text: "⚙️"
            font.pixelSize: 20
            color: theme.textPrimary
        }

        MouseArea {
            anchors.fill: parent
            onClicked: root.settingsClicked()
        }
    }
}
