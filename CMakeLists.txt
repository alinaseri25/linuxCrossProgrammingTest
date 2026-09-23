cmake_minimum_required(VERSION 3.16)

# ✅ خواندن VERSION از فایل
file(READ "${CMAKE_SOURCE_DIR}/version.txt" VERSION_STRING)
string(STRIP "${VERSION_STRING}" VERSION_STRING)

# ✅ تجزیه version به اجزای MAJOR.MINOR.PATCH
string(REPLACE "." ";" VERSION_LIST ${VERSION_STRING})
list(GET VERSION_LIST 0 VERSION_MAJOR)
list(GET VERSION_LIST 1 VERSION_MINOR)
list(GET VERSION_LIST 2 VERSION_PATCH)

# ✅ محاسبه versionCode (مثلاً: 1.3.0 → 10300)
math(EXPR VERSION_CODE "${VERSION_MAJOR} * 10000 + ${VERSION_MINOR} * 100 + ${VERSION_PATCH}")

project(linuxCrossProgrammingTest VERSION ${VERSION_STRING} LANGUAGES CXX)

configure_file(version.h.in version.h @ONLY)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_executable(linuxCrossProgrammingTest
  main.cpp
  ../../QtLibraries/pjsipConfigManager/pjsipconfigmanager.h
  ../../QtLibraries/pjsipConfigManager/pjsipconfigmanager.cpp
)

qt6_add_resources(linuxCrossProgrammingTest "resFiles"
    PREFIX "/"
    FILES
        version.txt
)

target_include_directories(linuxCrossProgrammingTest PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
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
