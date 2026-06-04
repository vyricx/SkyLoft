cmake_minimum_required(VERSION 3.16)
project(SkyLoft VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(SOURCES
    src/main.cpp
    src/core/process/ProcessScanner.cpp
    src/ui/MainWindow.cpp
    src/ui/InjectorWidget.cpp
)

set(HEADERS
    src/core/injector/IInjector.h
    src/core/process/GameProcess.h
    src/core/process/ProcessScanner.h
    src/ui/MainWindow.h
    src/ui/InjectorWidget.h
)

if(WIN32)
    list(APPEND SOURCES src/core/injector/WindowsInjector.cpp)
    list(APPEND HEADERS src/core/injector/WindowsInjector.h)
elseif(UNIX)
    list(APPEND SOURCES src/core/injector/LinuxInjector.cpp)
    list(APPEND HEADERS src/core/injector/LinuxInjector.h)
endif()

qt_add_executable(SkyLoft ${SOURCES} ${HEADERS})

target_include_directories(SkyLoft PRIVATE src)

target_link_libraries(SkyLoft PRIVATE Qt6::Widgets)

if(WIN32)
    target_link_libraries(SkyLoft PRIVATE psapi)
    set_target_properties(SkyLoft PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()
