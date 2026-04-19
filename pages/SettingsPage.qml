import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../components"


Item {
    id: root
    anchors.fill: parent

    required property var theme

    required property var backend

    // ===== State =====
    property int myId: 1
    property int sendToId: 255
    property int inputDeviceIndex: 0
    property int outputDeviceIndex: 0
    property int outputBufferSize: 1024

    signal applyClicked(int myId, int sendToId, int inputDeviceIndex, int outputDeviceIndex, int outputBufferSize)
    signal cancelClicked()

    // ===== Card =====
    Rectangle {
        anchors.fill: parent
        radius: theme.radius.md
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
                    anchors.margins: theme.spacing.sm

                    Text {
                        text: "Settings"
                        font.pixelSize: theme.fontSize.md
                        font.bold: true
                        color: theme.textPrimary
                        Layout.fillWidth: true
                    }
                }
            }

            // ===== Content =====
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.margins: theme.spacing.md
                spacing: theme.spacing.md

                ColumnLayout {
                    spacing: theme.spacing.xs
                    Text {
                        text: "UUID : "
                        color: theme.textSecondary
                        font.pixelSize: theme.fontSize.xs
                    }

                    Text {
                        id: txtUUID
                        text: "---"
                        color: theme.textSecondary
                        font.pixelSize: theme.fontSize.xs
                    }
                }

                // Audio Input
                ColumnLayout {
                    spacing: theme.spacing.xs
                    Text {
                        text: "Audio Input Source"
                        color: theme.textSecondary
                        font.pixelSize: theme.fontSize.xs
                    }
                    ComboBox {
                        Layout.fillWidth: true
                        model: audioBackend.inputDevicesModel

                        // اگر قبلاً چیزی انتخاب نشده
                        Component.onCompleted: ensureSelection()
                        onCountChanged: ensureSelection()

                        function ensureSelection() {
                            if (count > 0 && currentIndex === -1) {
                                currentIndex = 0
                                inputDeviceIndex = 0
                            }
                        }

                        currentIndex: inputDeviceIndex
                        onCurrentIndexChanged: inputDeviceIndex = currentIndex
                        textRole: "display"
                    }
                }

                // Audio Output
                ColumnLayout {
                    spacing: theme.spacing.xs
                    Text {
                        text: "Audio Output Device"
                        color: theme.textSecondary
                        font.pixelSize: theme.fontSize.xs
                    }
                    ComboBox {
                        Layout.fillWidth: true
                        model: audioBackend.outputDevicesModel

                        // اگر قبلاً چیزی انتخاب نشده
                        Component.onCompleted: ensureSelection()
                        onCountChanged: ensureSelection()

                        function ensureSelection() {
                            if (count > 0 && currentIndex === -1) {
                                currentIndex = 0
                                inputDeviceIndex = 0
                            }
                        }

                        currentIndex: outputDeviceIndex
                        onCurrentIndexChanged: outputDeviceIndex = currentIndex
                        textRole: "display"
                    }
                }

                // My ID
                ColumnLayout {
                    spacing: theme.spacing.xs
                    Text {
                        text: "My ID"
                        color: theme.textSecondary
                        font.pixelSize: theme.fontSize.xs
                    }
                    SpinBox {
                        from: 0
                        to: 255
                        value: myId
                        editable: true
                        onValueModified: myId = value
                    }
                }

                // Send To ID
                ColumnLayout {
                    spacing: theme.spacing.xs
                    Text {
                        text: "Send To ID"
                        color: theme.textSecondary
                        font.pixelSize: theme.fontSize.xs
                    }
                    SpinBox {
                        from: 0
                        to: 255
                        value: sendToId
                        editable: true
                        onValueModified: sendToId = value
                    }
                }

                // Buffer Size
                ColumnLayout {
                    spacing: theme.spacing.xs
                    Text {
                        text: "Audio Output Buffer"
                        color: theme.textSecondary
                        font.pixelSize: theme.fontSize.xs
                    }
                    ComboBox {
                        Layout.fillWidth: true
                        model: [
                            { text: "1KB", value: 1024 },
                            { text: "2KB", value: 2048 },
                            { text: "3KB", value: 3072 }
                        ]

                        Component.onCompleted: {
                            if (currentIndex === -1)
                                currentIndex = 0
                            outputBufferSize = model[currentIndex].value
                        }

                        onCurrentIndexChanged: {
                            if (currentIndex >= 0)
                                outputBufferSize = model[currentIndex].value
                        }
                        textRole: "text"
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
                        Layout.margins: theme.spacing.sm
                        spacing: theme.spacing.sm

                        CButton {
                            text: "Cancel"
                            theme: root.theme
                            Layout.fillWidth: true
                            onClicked: root.cancelClicked()
                        }

                        CButton {
                            text: "Apply"
                            theme: root.theme
                            Layout.fillWidth: true
                            onClicked: root.applyClicked(
                                myId,
                                sendToId,
                                inputDeviceIndex,
                                outputDeviceIndex,
                                outputBufferSize
                            )
                        }
                    }
                }
            }
        }
    }

    Connections{
        target: backend

        function onSetUUID(UUID){
            txtUUID.text = UUID
        }
    }
}
