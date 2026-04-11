import QtQuick
import QtQuick.Controls
import QtQuick.Effects

Rectangle {

    id: root

    property var theme

    default property alias content: contentItem.data

    radius: theme.radius.lg

    color: theme.surface

    border.color: theme.border
    border.width: 1


    layer.enabled: true

    layer.effect: MultiEffect {

        shadowEnabled: true
        shadowBlur: 0.5
        shadowVerticalOffset: 2
        shadowColor: "#40000000"
    }


    Item {

        id: contentItem

        anchors.fill: parent
        anchors.margins: theme.spacing.md
    }
}
