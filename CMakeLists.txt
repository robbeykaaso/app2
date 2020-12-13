cmake_minimum_required(VERSION 3.5)

set(USECGAL OFF)
set(USEDNN OFF)
set(USECMRC OFF)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#if (CMAKE_BUILD_TYPE STREQUAL Debug)
#set(${CMAKE_BINARY_DIR} install)
if (NOT DEFINED MS)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../build-app1-Desktop_Qt_5_13_2_MSVC2017_64bit-${CMAKE_BUILD_TYPE}/plugin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/install/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../build-app1-Desktop_Qt_5_13_2_MSVC2017_64bit-${CMAKE_BUILD_TYPE}/plugin)
else()
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${MS})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/install/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${MS})
endif()

project(REA_APP2)

#message(":${CMAKE_BUILD_TYPE}")

#add_definitions(-DEXPORTDLL)

find_package(Qt5 COMPONENTS Widgets Core Quick REQUIRED)
set(REA_DIR ${CMAKE_CURRENT_LIST_DIR}/../rea/cmake)
#message("xxx: ${CMAKE_CURRENT_LIST_DIR}")
find_package(REA)
#https://github.com/CGAL/cgal/wiki/How-to-use-CGAL-with-CMake-or-your-own-build-system

set(my_Sources
    "main.cpp"
    "qml.qrc"
    )

set(my_Libs
    Qt5::Core Qt5::Quick Qt5::Widgets
    ${REA_LIBS}
    )

if (${USECGAL} STREQUAL "ON")
    find_package(CGAL REQUIRED COMPONENTS Core)
    add_definitions(-DUSECGAL)
    set(my_Sources
        ${my_Sources}
        "shape/shape.cpp"
        "shape/editNode0.cpp")
    set(my_Libs
        ${my_Libs}
        CGAL::CGAL CGAL::CGAL_Core)
endif()

if (${USEDNN} STREQUAL "ON")
    add_compile_options(-bigobj)
    find_package(OpenCV REQUIRED)
    set(my_Sources
        ${my_Sources}
        "dnn/training_server.cpp")
    set(my_Libs
        ${my_Libs}
        ${OpenCV_LIBS})
    include_directories(${OpenCV_INCLUDE_DIRS})
    include_directories("D:/thirdParty/tinydnn/tiny-dnn")
endif()

if (${USECMRC} STREQUAL "ON")
    include(CMakeRC.cmake)
    cmrc_add_resource_library(dna
                              NAMESPACE rc
                              cmrc/src.txt
                              )
    set(my_Sources
        ${my_Sources}
        "cmrc/test.cpp")
    set(my_Libs
        ${my_Libs}
        dna)
endif()

# 根据库文件代码生成动态库
add_library(REA_APP2 SHARED
    ${my_Sources}
    )

# 创建动态库别名

#add_library(hello::library ALIAS REA_APP2)

target_link_libraries(REA_APP2 PRIVATE
    ${my_Libs})
# 包含指定头文件所在的目录
target_include_directories(REA_APP2 PUBLIC
    ${PROJECT_SOURCE_DIR}/include)
#target_include_directories(REA_APP2 PUBLIC  ${PROJECT_SOURCE_DIR}/imageboardplugins)

# 创建可执行程序

#add_executable(hello_binary
#src/main.cpp
#)

# 链接动态库文件
#target_link_libraries( hello_binary PRIVATE hello::library)
include_directories(${REA_INCLUDE_DIRS})
