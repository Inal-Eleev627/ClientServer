cmake_minimum_required(VERSION 3.12)

set(TARGET Client)
project(${TARGET} VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Quick REQUIRED)

set(PROJECT_SOURCES
    ./main.cpp
    ./mainwindow.cpp
    ./mainwindow.h
    ./mainwindow.ui
    ./clientdialog.cpp
    ./clientdialog.h
    ./clientdialog.ui
    ./clientstatus.cpp
    ./clientstatus.h
    ./clientstatus.ui
    ./tcpclient.cpp
    ./tcpclient.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${TARGET}
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET ${TARGET} APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(${TARGET} SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(${TARGET}
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(${TARGET} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Quick
)

set_target_properties(${TARGET} PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${TARGET})
endif()

include_directories(${TARGET} ${CMAKE_SOURCE_DIR}/../include)

find_package(Boost 1.68 COMPONENTS date_time system REQUIRED)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_directories(${TARGET} PUBLIC ${Boost_LIBRARIES})
endif()

if (CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 20)
    set_property(TARGET ${TARGET} PROPERTY CMAKE_CXX_STANDARD_REQUIRED 20)
endif()
