cmake_minimum_required(VERSION 3.16)
project(MorseLab VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Исходники в UTF-8 (кириллица в строках и char32_t-литералах)
if(MSVC)
    add_compile_options(/utf-8)
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    morse.h
    morse.cpp
    distortion.h
    distortion.cpp
    pipeline.h
    pipeline.cpp
)

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(MorseLab MANUAL_FINALIZATION ${PROJECT_SOURCES})
else()
    add_executable(MorseLab ${PROJECT_SOURCES})
endif()

target_include_directories(MorseLab PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(MorseLab PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_finalize_executable(MorseLab)
endif()
