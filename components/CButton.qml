import QtQuick
import QtQuick.Controls

Rectangle {

    id: root

    property var theme

    property alias text: label.text

    signal clicked

    implicitWidth: 100
    implicitHeight: theme.control.heightLarge

    radius: theme.radius.md

    color: mouseArea.pressed
           ? Qt.darker(theme.accent, 1.2)
           : theme.accent

    opacity: enabled ? 1.0 : theme.disabledOpacity


    Text {

        id: label

        anchors.centerIn: parent

        color: theme.textOnAccent

        font.pixelSize: 15
        font.bold: true
    }


    MouseArea {

        id: mouseArea

        anchors.fill: parent

        onClicked: root.clicked()
    }
}
