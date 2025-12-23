// import QtQuick

// QtObject {
//     /* ===== Layout / Direction ===== */
//     readonly property bool isRTL: true   // برای WalkieHeader

//     /* ===== Modes ===== */
//     property bool isDarkMode: true

//     /* ===== Base Colors ===== */
//     readonly property color background: "#121212"
//     readonly property color surface: "#1E1E1E"
//     readonly property color text: "#FFFFFF"
//     readonly property color textSecondary: "#9E9E9E"
//     readonly property color border: "#2C2C2C"

//     /* ===== Accent Colors ===== */
//     readonly property color accentYellow: "#FBC02D"
//     readonly property color accentRed: "#D32F2F"
//     readonly property color accentOrange: "#FF9800"   // ✅ استفاده در Main.qml
//     readonly property color hoverColor: "#2A2A2A"     // ✅ WalkieHeader

//     /* ===== Spacing ===== */
//     readonly property int spacing: 16
//     readonly property int spacingLarge: 24
//     readonly property int spacingSmall: 8

//     /* ===== Radius & Sizes ===== */
//     readonly property int radius: 12
//     readonly property int headerHeight: 56

//     /* ===== Fonts (Canonical) ===== */
//     readonly property int fontSmall: 12
//     readonly property int fontMedium: 14
//     readonly property int fontLarge: 18

//     /* ===== Fonts (Aliases — VERY IMPORTANT) ===== */
//     // چون در QML های مختلف از این نام‌ها استفاده شده
//     readonly property int fontSizeSmall: fontSmall
//     readonly property int fontSizeMedium: fontMedium
//     readonly property int fontSizeLarge: fontLarge
// }
import QtQuick

QtObject {
    // ===== Mode =====
    property bool isDarkMode: true
    property bool isRTL: false

    // ===== Colors =====
    readonly property color background: "#121212"
    readonly property color surface: "#1E1E1E"
    readonly property color surfaceAlt: "#242424"

    readonly property color text: "#FFFFFF"
    readonly property color textSecondary: "#9E9E9E"

    readonly property color accent: "#E0E0E0"
    readonly property color accentOrange: "#FF9800"
    readonly property color accentRed: "#D32F2F"

    readonly property color border: "#2C2C2C"
    readonly property color hoverColor: "#2A2A2A"

    // ===== Spacing =====
    readonly property int spacingSmall: 1
    readonly property int spacing: 8
    readonly property int spacingLarge: 12

    // ===== Sizes =====
    readonly property int radius: 12
    readonly property int headerHeight: 48

    // ===== Fonts =====
    readonly property int fontSizeSmall: 10
    readonly property int fontSizeMedium: 12
    readonly property int fontSizeLarge: 14
}
