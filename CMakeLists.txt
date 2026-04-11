cmake_minimum_required(VERSION 3.16)

# ✅ خواندن VERSION از فایل
file(READ "${CMAKE_SOURCE_DIR}/version.txt" VERSION_STRING)
string(STRIP "${VERSION_STRING}" VERSION_STRING)

# ✅ تجزیه version به اجزای MAJOR.MINOR.PATCH
string(REPLACE "." ";" VERSION_LIST ${VERSION_STRING})
list(GET VERSION_LIST 0 VERSION_MAJOR)
list(GET VERSION_LIST 1 VERSION_MINOR)
list(GET VERSION_LIST 2 VERSION_PATCH)

# ✅ محاسبه versionCode (مثلاً: 1.3.0 → 10300)
math(EXPR VERSION_CODE "${VERSION_MAJOR} * 10000 + ${VERSION_MINOR} * 100 + ${VERSION_PATCH}")

project(QMLWalkieTalkie VERSION ${VERSION_STRING} LANGUAGES CXX)

configure_file(version.h.in version.h @ONLY)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick QuickControls2 multimedia)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(QMLWalkieTalkie
    main.cpp
)

qt_add_qml_module(QMLWalkieTalkie
    URI QMLWalkieTalkie
    QML_FILES
        Main.qml
        theme/Theme.qml
        components/WalkieHeader.qml
        pages/SettingsPage.qml
        components/CButton.qml
        components/CCard.qml
        components/CTextField.qml
        components/Toast.qml
    SOURCES
        audiobackend.h audiobackend.cpp
        packetStructs.h
)

qt6_add_resources(QMLWalkieTalkie "qmlchartsgallery"
    PREFIX "/"
    FILES
        version.txt
)

if(ANDROID)
    set_target_properties(QMLWalkieTalkie PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR
            ${CMAKE_CURRENT_SOURCE_DIR}/android
            QT_ANDROID_VERSION_NAME "${VERSION_STRING}"
            QT_ANDROID_VERSION_CODE "${VERSION_CODE}"
            QT_ANDROID_MIN_SDK_VERSION 24
            QT_ANDROID_TARGET_SDK_VERSION 35
            QT_ANDROID_COMPILE_SDK_VERSION 35
    )
endif()

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(QMLWalkieTalkie PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER org.verya.QMLWalkieTalkie
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(QMLWalkieTalkie
    PRIVATE Qt6::Quick
    Qt6::QuickControls2
    Qt6::Multimedia
)
target_link_libraries(QMLWalkieTalkie PRIVATE Qt6::Core)

target_include_directories(QMLWalkieTalkie PRIVATE ${CMAKE_BINARY_DIR})

include(GNUInstallDirs)
install(TARGETS QMLWalkieTalkie
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
