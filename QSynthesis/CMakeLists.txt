cmake_minimum_required(VERSION 3.17)

project(QSynthesis VERSION 0.1.86 LANGUAGES CXX)
find_package(Qt5 COMPONENTS Core Concurrent Gui Multimedia Network Widgets Xml Svg REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set Directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

# Determining Current System
if(APPLE AND NOT IOS)
    message(STATUS "[INFO] Current System is Macintosh")
    set(ASSETS_FILE "Resources/platforms/mac/qsynth2.icns")
    set(STATIC_LIB_PATH "Libraries/macosx/clang64")
elseif(UNIX AND NOT ANDROID)
    message(STATUS "[INFO] Current System is Linux")
    set(STATIC_LIB_PATH "Libraries/linux/x86_64")
elseif(WIN32)
    message(STATUS "[INFO] Current System is Windows")
    set(ASSETS_FILE "Resources/platforms/win/res.rc")

    #菜鸡，不会加 32 bit 编译支持，放弃……
    if(MINGW)
        message(STATUS "[INFO] Current Compiler is MinGW")
        #set(STATIC_LIB_PATH "Libraries/windows/mingw81")
        set(STATIC_LIB_PATH "Libraries/windows/mingw81_64")
    elseif(MSVC)
        message(STATUS "[INFO] Current Compiler is M$VC")
        #set(STATIC_LIB_PATH "Libraries/windows/msvc2019_64")
        set(STATIC_LIB_PATH "Libraries/windows/mingw81_64")
    else()
        message(FATAL_ERROR "Unsupported Compiler !!!")
    endif()
else()
    message(FATAL_ERROR "Unsupported System !!!")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(EFSW_STATIC_LIB "efsw-static-debug")
else()
    set(EFSW_STATIC_LIB "efsw-static-release")
endif()

file(GLOB_RECURSE QS_SHIT_VER_SOURCE
    Application/*
    Backend/*
    Frontend/*
    Global/*
    Main/*
    MainWindow/*
)
add_executable(QSynthesis ${QS_SHIT_VER_SOURCE})
target_link_libraries(QSynthesis PRIVATE
    Qt::Core
    Qt::Concurrent
    Qt::Gui
    Qt::Multimedia
    Qt::Network
    Qt::Widgets
    Qt::Xml
    Qt::Svg
    ${EFSW_STATIC_LIB}
)
target_link_options(QSynthesis PRIVATE -L${CMAKE_SOURCE_DIR}/${STATIC_LIB_PATH} -l ${EFSW_STATIC_LIB})
target_include_directories(QSynthesis
    PRIVATE Application
    PRIVATE Backend
    PRIVATE Backend/Base
    PRIVATE Backend/Common
    PRIVATE Backend/Structures
    PRIVATE Backend/Utils
    PRIVATE Backend/Documents/DirInfos
    PRIVATE Backend/Documents/Files
    PRIVATE Backend/Documents/Import
    PRIVATE Frontend
    PRIVATE Frontend/Base
    PRIVATE Frontend/Base/Widgets
    PRIVATE Frontend/Base/Storage
    PRIVATE Frontend/Common
    PRIVATE Frontend/Singletons
    PRIVATE Frontend/Tabs
    PRIVATE Global
    PRIVATE Global/QS
    PRIVATE Global/UTAU
    PRIVATE Main
    PRIVATE MainWindow
)
target_sources(QSynthesis
    PRIVATE "${CMAKE_SOURCE_DIR}/${ASSETS_FILE}"
)
