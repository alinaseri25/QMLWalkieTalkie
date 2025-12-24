cmake_minimum_required(VERSION 3.16)

project(QMLWalkieTalkie VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2 multimedia)
find_package(Qt6 REQUIRED COMPONENTS Core)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(QMLWalkieTalkie
    main.cpp
)

qt_add_qml_module(QMLWalkieTalkie
    URI QMLWalkieTalkie
    QML_FILES
        Main.qml
        QML_FILES theme/Theme.qml
        QML_FILES components/WalkieHeader.qml
        QML_FILES pages/SettingsPage.qml
        SOURCES audiobackend.h audiobackend.cpp
)

if(ANDROID)
    set_target_properties(QMLWalkieTalkie PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR
            ${CMAKE_CURRENT_SOURCE_DIR}/android
    )
endif()

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(QMLWalkieTalkie PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.QMLWalkieTalkie
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

include(GNUInstallDirs)
install(TARGETS QMLWalkieTalkie
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
