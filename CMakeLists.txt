cmake_minimum_required(VERSION 3.5)

project(Dimploma VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the QT version
find_package(Qt6 COMPONENTS Core QUIET)
if (NOT Qt6_FOUND)
    set(QT_VERSION 5 CACHE STRING "Qt version for QGeoView")
else()
    set(QT_VERSION 6 CACHE STRING "Qt version for QGeoView")
endif()

set(QCUSTOMPLOT_DIR "${CMAKE_SOURCE_DIR}/libs/qcustomplot")
find_package(Qt${QT_VERSION} REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Network
    PrintSupport
    Charts
    SerialPort
    Sql
)

file(GLOB SRC
     "*.h"
     "*.cpp"
     "*.ui"
     "*.qrc"
     "*.qss"
     "command/*.h"
     "command/*.cpp"
     "libs/qcustomplot/*.h"
     "libs/qcustomplot/*.cpp"
     inscommandprocessor.h
     inscommandprocessor.cp
     "togglebutton.cpp"
     "togglebutton.h"
     "togglebutton.ui"
     "objectnavigator.h"
     "timescaleselector.h"
     "timescaleselector.ui"
     "settingswidget.cpp"
     "settingswidget.h"
     "settingswidget.ui"
     "rangeslider.cpp"
     "rangeslider.h"
     "DynamicPlotBuffer.cpp"
     "DynamicPlotBuffer.h"
     "DataTableWidget.cpp"
     "DataTableWidget.h"
)

set(PROJECT_SOURCES
    ${SRC}
)

if(${QT_VERSION} GREATER_EQUAL 6)
    qt_add_executable(Dimploma
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Dimploma APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(Dimploma SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(Dimploma
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_compile_options(${PROJECT_NAME} PRIVATE -Wa,-mbig-obj)

target_link_libraries(Dimploma PRIVATE
    Qt${QT_VERSION}::Core
    Qt${QT_VERSION}::Network
    Qt${QT_VERSION}::Gui
    Qt${QT_VERSION}::Widgets
    Qt${QT_VERSION}::Charts
    Qt${QT_VERSION}::SerialPort
    Qt${QT_VERSION}::Sql
    PRIVATE Qt${QT_VERSION}::PrintSupport
)

target_include_directories(Dimploma PRIVATE ${QCUSTOMPLOT_DIR})

set_target_properties(Dimploma PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS Dimploma
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION EQUAL 6)
    qt_finalize_executable(Dimploma)
endif()
