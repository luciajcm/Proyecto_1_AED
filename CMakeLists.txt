cmake_minimum_required(VERSION 3.16)
project(SpreadsheetApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Automoc para Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Buscar Qt5 o Qt6 (prefiere Qt6)
find_package(Qt6 QUIET COMPONENTS Core Widgets)
if(Qt6_FOUND)
    message(STATUS "Usando Qt6")
    set(QT_VERSION_MAJOR 6)
    set(QT_LIBS Qt6::Core Qt6::Widgets)
else()
    find_package(Qt5 REQUIRED COMPONENTS Core Widgets)
    message(STATUS "Usando Qt5")
    set(QT_VERSION_MAJOR 5)
    set(QT_LIBS Qt5::Core Qt5::Widgets)
endif()

set(SOURCES
    src/main.cpp
    src/SparseMatrix.cpp
    src/FormulaParser.cpp
    src/MainWindow.cpp
)

set(HEADERS
    src/Cell.h
    src/SparseMatrix.h
    src/FormulaParser.h
    src/MainWindow.h
)

add_executable(spreadsheet ${SOURCES} ${HEADERS})
target_include_directories(spreadsheet PRIVATE src)
target_link_libraries(spreadsheet PRIVATE ${QT_LIBS})

# Warnings
if(MSVC)
    target_compile_options(spreadsheet PRIVATE /W4)
else()
    target_compile_options(spreadsheet PRIVATE -Wall -Wextra -Wpedantic)
endif()
