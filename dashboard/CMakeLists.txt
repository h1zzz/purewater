# MIT License Copyright (c) 2021, h1zzz

cmake_minimum_required(VERSION 3.16.0)

project(purewater VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(TARGET ${PROJECT_NAME})

find_package(Qt6 COMPONENTS Widgets REQUIRED)
find_package(Qt6 COMPONENTS Network REQUIRED)

set(SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.ui
)

set(LIBS
    Qt6::Widgets
    Qt6::Network
)

add_executable(${TARGET} ${SOURCES})
target_link_libraries(${TARGET} ${LIBS})
