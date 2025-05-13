cmake_minimum_required(VERSION 3.5)

project(NDT VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(util) # Add util directory for header searches
include_directories(widget) # Add widget directory for header searches
include_directories(component) # Add component directory
include_directories(page) # Add page directory

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

set(PROJECT_SOURCES
        main.cpp
        application.cpp
        application.h
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        thememanager.cpp
        thememanager.h
        util/util.h
        config.h
        widget/NcButton.h
        widget/NcButton.cpp
        widget/statusbarwidget.h
        widget/statusbarwidget.cpp
        widget/statusbarwidget.ui
        widget/menuwidget.h
        widget/menuwidget.cpp
        widget/menuwidget.ui
        widget/basescreen.h
        widget/basescreen.cpp
        widget/baselist.h
        widget/baselist.cpp
        widget/concentriccircleswidget.h
        widget/concentriccircleswidget.cpp
        component/component.h
        component/component.cpp
        component/navigationcomponent.h
        component/navigationcomponent.cpp
        component/componentmanager.h
        component/componentmanager.cpp
        component/inputcomponent.h
        component/inputcomponent.cpp
        page/homepage.h
        page/homepage.cpp
        page/homepage.ui
        page/settingscreen.h
        page/settingscreen.cpp
        res/resource.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(NDT
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET NDT APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(NDT SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(NDT
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(NDT PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(NDT PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(NDT)
endif()

install(TARGETS NDT RUNTIME DESTINATION /usr/bin)
