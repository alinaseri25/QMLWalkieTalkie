import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    anchors.fill: parent

    required property var theme

    // ===== State =====
    property int myId: 0
    property int sendToId: 255
    property int inputDeviceIndex: -1
    property int outputDeviceIndex: -1

    signal applyClicked(int myId, int sendToId, int inputDeviceIndex, int outputDeviceIndex)
    signal cancelClicked()

    // ===== Card =====
    Rectangle {
        anchors.fill: parent
        radius: theme.radius
        color: theme.surface
        border.color: theme.border
        border.width: 1

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            // ===== Header =====
            Rectangle {
                Layout.fillWidth: true
                height: theme.headerHeight
                color: theme.surface

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: theme.spacing

                    Text {
                        text: "Settings"
                        font.pixelSize: theme.fontSizeLarge
                        font.bold: true
                        color: theme.text
                        Layout.fillWidth: true
                    }

                    Button {
                        text: "✕"
                        flat: true
                        onClicked: root.cancelClicked()

                        contentItem: Item {
                            anchors.fill: parent

                            Text {
                                text: "✕"
                                anchors.centerIn: parent
                                font.pixelSize: theme.fontSizeLarge
                                color: theme.text
                            }
                        }
                    }

                }
            }

            // ===== Content =====
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.margins: theme.spacingLarge
                spacing: theme.spacingLarge

                // Audio Input
                ColumnLayout {
                    spacing: theme.spacingSmall
                    Text {
                        text: "Audio Input Source"
                        color: theme.textSecondary
                        font.pixelSize: theme.fontSizeSmall
                    }
                    ComboBox {
                        Layout.fillWidth: true
                        model: []
                        currentIndex: inputDeviceIndex
                        onCurrentIndexChanged: inputDeviceIndex = currentIndex
                    }
                }

                // Audio Output
                ColumnLayout {
                    spacing: theme.spacingSmall
                    Text {
                        text: "Audio Output Device"
                        color: theme.textSecondary
                        font.pixelSize: theme.fontSizeSmall
                    }
                    ComboBox {
                        Layout.fillWidth: true
                        model: []
                        currentIndex: outputDeviceIndex
                        onCurrentIndexChanged: outputDeviceIndex = currentIndex
                    }
                }

                // My ID
                ColumnLayout {
                    spacing: theme.spacingSmall
                    Text {
                        text: "My ID"
                        color: theme.textSecondary
                        font.pixelSize: theme.fontSizeSmall
                    }
                    SpinBox {
                        from: 0
                        to: 255
                        value: myId
                        onValueModified: myId = value
                    }
                }

                // Send To ID
                ColumnLayout {
                    spacing: theme.spacingSmall
                    Text {
                        text: "Send To ID"
                        color: theme.textSecondary
                        font.pixelSize: theme.fontSizeSmall
                    }
                    SpinBox {
                        from: 0
                        to: 255
                        value: sendToId
                        onValueModified: sendToId = value
                    }
                }
            }

            // ===== Footer =====
            Rectangle {
                Layout.fillWidth: true
                height: 72
                color: theme.surface

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 0

                    // شبیه border.top
                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: theme.border
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.margins: theme.spacing
                        spacing: theme.spacing

                        Button {
                            text: "Cancel"
                            Layout.fillWidth: true
                            onClicked: root.cancelClicked()
                        }

                        Button {
                            text: "Apply"
                            Layout.fillWidth: true
                            onClicked: root.applyClicked(
                                myId,
                                sendToId,
                                inputDeviceIndex,
                                outputDeviceIndex
                            )
                        }
                    }
                }
            }
        }
    }
}
