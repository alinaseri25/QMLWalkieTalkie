import QtQuick
import QtQuick.Controls

Rectangle {
    id: root

    property var theme
    property int currentIndex: 0
    signal tabChanged(int index)

    implicitHeight: 56
    implicitWidth: 300
    color: theme.surface

    Rectangle {
        anchors.top: parent.top
        width: parent.width
        height: 1
        color: theme.border
    }

    Row {
        anchors.fill: parent

        Repeater {
            model: [
                { icon: "💬", label: "Chats" },
                { icon: "👥", label: "Contacts" },
                { icon: "⚙️", label: "Settings" }
            ]

            delegate: Item {
                width: root.width / 3
                height: root.height

                Column {
                    anchors.centerIn: parent
                    spacing: 2

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: modelData.icon
                        font.pixelSize: 20
                    }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: modelData.label
                        font.pixelSize: 11
                        color: index === root.currentIndex ? theme.accent : theme.textSecondary
                    }
                }

                Rectangle {
                    anchors.top: parent.top
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: 32
                    height: 2
                    radius: 1
                    color: theme.accent
                    visible: index === root.currentIndex
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        root.currentIndex = index
                        root.tabChanged(index)
                    }
                }
            }
        }
    }
}
