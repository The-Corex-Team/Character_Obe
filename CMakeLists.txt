cmake_minimum_required(VERSION 3.20)
project(CharacterObe VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR})

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/ExportDialog.cpp
    src/FindDialog.cpp
)

set(HEADERS
    src/MainWindow.h
    src/ExportDialog.h
    src/FindDialog.h
)

set(UI_FILES
    characterobe.ui
)

add_executable(CharacterObe WIN32
    ${SOURCES}
    ${HEADERS}
    ${UI_FILES}
)

target_link_libraries(CharacterObe PRIVATE Qt6::Widgets)

if(WIN32)
    add_custom_command(TARGET CharacterObe POST_BUILD
        COMMAND "${Qt6_DIR}/../../../bin/windeployqt6.exe"
                --no-translations
                "$<TARGET_FILE:CharacterObe>"
        COMMENT "Running windeployqt6..."
    )
endif()
