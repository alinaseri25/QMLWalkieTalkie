import QtQuick

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
        font.pixelSize: theme.fontSizeLarge
        font.bold: true
        color: theme.text
    }

    Rectangle {
        width: 42
        height: 42
        radius: theme.radius
        color: theme.hoverColor

        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.rightMargin: theme.spacing

        Text {
            anchors.centerIn: parent
            text: "⚙️"
            font.pixelSize: 20
            color: theme.text
        }

        MouseArea {
            anchors.fill: parent
            onClicked: root.settingsClicked()
        }
    }
}
