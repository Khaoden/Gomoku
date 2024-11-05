set(CMAKE_PREFIX_PATH "D:/softInstall/QT/6.7.2/mingw_64/bin")
cmake_minimum_required(VERSION 3.16)
project(gobang)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

add_executable(gobang
        main.cpp
        GoBang.cpp
        GoBang.h)

target_link_libraries(gobang
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets)