import QtQuick
import QtQuick.Controls

Rectangle {

    id: root

    property var theme

    property alias text: label.text

    property color backgroundColor: theme.accent
    property color textColor: theme.textOnAccent

    signal clicked

    implicitWidth: 100
    implicitHeight: theme.control.heightLarge

    radius: theme.radius.md

    color: mouseArea.pressed
           ? Qt.darker(backgroundColor, 1.2)
           : backgroundColor

    opacity: enabled ? 1.0 : theme.disabledOpacity


    Text {

        id: label

        anchors.centerIn: parent

        color: textColor

        font.pixelSize: 15
        font.bold: true
    }


    MouseArea {

        id: mouseArea

        anchors.fill: parent

        onClicked: root.clicked()
    }
}
