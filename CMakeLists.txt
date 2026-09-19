cmake_minimum_required(VERSION 3.16)

project(linuxCrossProgrammingTest LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_executable(linuxCrossProgrammingTest
  main.cpp
  pjsipconfigmanager.h
  pjsipconfigmanager.cpp
)
target_link_libraries(linuxCrossProgrammingTest PRIVATE Qt${QT_VERSION_MAJOR}::Core)

set_target_properties(linuxCrossProgrammingTest PROPERTIES
    OUTPUT_NAME "CPTest"
    QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
)

include(GNUInstallDirs)
install(TARGETS linuxCrossProgrammingTest
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
