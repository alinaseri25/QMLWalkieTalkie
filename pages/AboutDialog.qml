import QtQuick
import QtQuick.Layouts
import "../components"

Item {
    id: root
    anchors.fill: parent
    clip: true

    required property var theme

    property string appName: "Application Name"
    property string appVersion: "1.0.0"
    property string appBuildDate: "2024-01-15"
    property string appDescription: ""
    property string companyName: ""
    property string companyWebsite: ""
    property string companyEmail: ""
    property string logoSource: ""
    property bool showCompanySection: companyName !== "" || companyWebsite !== "" || companyEmail !== ""
    property bool showVersionSection: true
    property string companySectionTitle: qsTr("Powered by")
    property string versionSectionTitle: qsTr("Version Information")

    signal closeRequested()

    Rectangle {
        id: dialogFrame
        anchors.fill: parent
        radius: theme.radius.lg
        color: theme.surface
        border.color: theme.border
        border.width: 1
        clip: true

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            // Header
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: theme.headerHeight
                color: theme.surfaceAlt

                Text {
                    anchors.centerIn: parent
                    text: root.appName
                    font.pixelSize: theme.fontSize.md
                    font.bold: true
                    color: theme.textPrimary
                    elide: Text.ElideRight
                }
            }

            // Scrollable content
            Flickable {
                id: flick
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                contentWidth: width
                contentHeight: contentColumn.implicitHeight

                ColumnLayout {
                    id: contentColumn
                    width: flick.width
                    spacing: theme.spacing.lg

                    Item {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        visible: root.logoSource !== ""
                        implicitHeight: 90

                        Image {
                            anchors.centerIn: parent
                            source: root.logoSource
                            height: 90
                            fillMode: Image.PreserveAspectFit
                            smooth: true
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        visible: root.showCompanySection
                        radius: theme.radius.md
                        border.color: theme.border
                        border.width: 1
                        color: theme.surfaceAlt
                        implicitHeight: companyColumn.implicitHeight + theme.spacing.md * 2

                        ColumnLayout {
                            id: companyColumn
                            anchors.fill: parent
                            anchors.margins: theme.spacing.md
                            spacing: theme.spacing.sm

                            Text {
                                text: root.companySectionTitle
                                font.pixelSize: theme.fontSize.sm
                                font.bold: true
                                color: theme.textSecondary
                                Layout.fillWidth: true
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: theme.spacing.sm
                                visible: root.companyName !== ""

                                Text {
                                    text: theme.isRTL ? qsTr("شرکت: ") : qsTr("Company Name:")
                                    font.pixelSize: theme.fontSize.md
                                    color: theme.textPrimary
                                    Layout.preferredWidth: 110
                                }

                                Text {
                                    text: root.companyName
                                    font.pixelSize: theme.fontSize.sm
                                    color: theme.textSecondary
                                    Layout.fillWidth: true
                                    wrapMode: Text.Wrap
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: theme.spacing.sm
                                visible: root.companyWebsite !== ""

                                Text {
                                    text: theme.isRTL ? qsTr("وبسایت: ") : qsTr("Website:")
                                    font.pixelSize: theme.fontSize.md
                                    color: theme.textPrimary
                                    Layout.preferredWidth: 110
                                }

                                Text {
                                    text: root.companyWebsite
                                    font.pixelSize: theme.fontSize.sm
                                    color: theme.textSecondary
                                    Layout.fillWidth: true
                                    wrapMode: Text.Wrap

                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: {
                                            Qt.openUrlExternally(
                                                root.companyWebsite.startsWith("http")
                                                    ? root.companyWebsite
                                                    : "https://" + root.companyWebsite
                                            )
                                        }
                                    }
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: theme.spacing.sm
                                visible: root.companyEmail !== ""

                                Text {
                                    text: theme.isRTL ? qsTr("پست الکترونیک: ") : qsTr("Email:")
                                    font.pixelSize: theme.fontSize.md
                                    color: theme.textPrimary
                                    Layout.preferredWidth: 110
                                }

                                Text {
                                    text: root.companyEmail
                                    font.pixelSize: theme.fontSize.sm
                                    color: theme.textSecondary
                                    Layout.fillWidth: true
                                    wrapMode: Text.Wrap

                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: {
                                            Qt.openUrlExternally("mailto:" + root.companyEmail)
                                        }
                                    }
                                }
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        visible: root.showVersionSection
                        radius: theme.radius.md
                        border.color: theme.border
                        border.width: 1
                        color: theme.surfaceAlt
                        implicitHeight: versionColumn.implicitHeight + theme.spacing.md * 2

                        ColumnLayout {
                            id: versionColumn
                            anchors.fill: parent
                            anchors.margins: theme.spacing.md
                            spacing: theme.spacing.sm

                            Text {
                                text: root.versionSectionTitle
                                font.pixelSize: theme.fontSize.sm
                                font.bold: true
                                color: theme.textSecondary
                                Layout.fillWidth: true
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: theme.spacing.sm

                                Text {
                                    text: theme.isRTL ? qsTr("نسخه: ") : qsTr("Version:")
                                    font.pixelSize: theme.fontSize.md
                                    color: theme.textPrimary
                                    Layout.preferredWidth: 110
                                }

                                Text {
                                    text: root.appVersion
                                    font.pixelSize: theme.fontSize.sm
                                    color: theme.textSecondary
                                    Layout.fillWidth: true
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: theme.spacing.sm

                                Text {
                                    text: theme.isRTL ? qsTr("تاریخ تولید: ") : qsTr("Build Date:")
                                    font.pixelSize: theme.fontSize.md
                                    color: theme.textPrimary
                                    Layout.preferredWidth: 110
                                }

                                Text {
                                    text: root.appBuildDate
                                    font.pixelSize: theme.fontSize.sm
                                    color: theme.textSecondary
                                    Layout.fillWidth: true
                                }
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true

                        Text {
                            visible: root.appDescription !== ""
                            text: root.appDescription
                            font.pixelSize: theme.fontSize.sm
                            color: theme.textSecondary
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                            anchors.centerIn: parent
                        }
                    }

                    Item {
                        Layout.fillHeight: true
                    }
                }
            }

            // Footer
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 72
                color: theme.surface

                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    height: 1
                    color: theme.border
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: theme.spacing.sm
                    spacing: theme.spacing.sm

                    Item { Layout.fillWidth: true }

                    CButton {
                        text: theme.isRTL ? qsTr("بستن") : qsTr("Close")
                        theme: root.theme
                        backgroundColor: root.theme.surfaceAlt
                        onClicked: root.closeRequested()
                    }
                }
            }
        }
    }
}
