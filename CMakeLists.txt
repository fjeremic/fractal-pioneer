cmake_minimum_required(VERSION 3.5)

project(FractalPioneer LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

qt5_add_resources(QRC_SOURCES
    FractalPioneer.qrc
)

qt5_wrap_ui(QUI_SOURCES
    FractalPioneer.ui
)

add_executable(FractalPioneer
    ${QRC_SOURCES}
    ${QUI_SOURCES}

    main.cpp

    ColorPushButton.cpp
    ColorPushButton.h

    FractalPioneer.cpp
    FractalPioneer.h

    FractalWidget.cpp
    FractalWidget.h
)

target_link_libraries(FractalPioneer PRIVATE Qt5::Widgets)
