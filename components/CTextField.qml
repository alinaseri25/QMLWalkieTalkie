import QtQuick
import QtQuick.Controls

Rectangle {

    id: root

    property var theme


    /* =========================
       TEXTFIELD API
       ========================= */

    property alias text: input.text
    property alias placeholderText: input.placeholderText
    property alias echoMode: input.echoMode
    property alias validator: input.validator
    property alias inputMethodHints: input.inputMethodHints


    /* =========================
       STATE
       ========================= */

    property bool error: false
    property bool disabled: false


    /* =========================
       SLOT API
       ========================= */

    property Component leading
    property Component trailing


    /* =========================
       SIZE
       ========================= */

    width: parent ? parent.width : implicitWidth
    height: theme.control.heightMedium

    radius: theme.radius.md

    color: theme.surface

    opacity: disabled ? theme.disabledOpacity : 1


    border.width: 1
    border.color:
        error
        ? theme.error
        : (input.activeFocus
           ? theme.accent
           : theme.border)


    /* =========================
       LAYOUT
       ========================= */

    Item {

        id: container

        anchors.fill: parent
    }


    Loader {

        id: leadingLoader

        anchors.left: parent.left
        anchors.leftMargin: theme.spacing.sm
        anchors.verticalCenter: parent.verticalCenter

        sourceComponent: root.leading

        visible: sourceComponent !== undefined && sourceComponent !== null
    }


    Loader {

        id: trailingLoader

        anchors.right: parent.right
        anchors.rightMargin: theme.spacing.sm
        anchors.verticalCenter: parent.verticalCenter

        sourceComponent: root.trailing

        visible: sourceComponent !== undefined && sourceComponent !== null
    }


    TextField {

        id: input

        anchors.top: parent.top
        anchors.bottom: parent.bottom

        anchors.left: parent.left
        anchors.right: parent.right

        anchors.leftMargin:
            leadingLoader.visible
            ? leadingLoader.width + theme.spacing.sm
            : theme.spacing.sm

        anchors.rightMargin:
            trailingLoader.visible
            ? trailingLoader.width + theme.spacing.sm
            : theme.spacing.sm

        background: null

        enabled: !root.disabled

        color: theme.textPrimary
        placeholderTextColor: theme.textSecondary

        verticalAlignment: TextInput.AlignVCenter
    }
}
