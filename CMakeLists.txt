cmake_minimum_required(VERSION 3.21)

project(FireflyPlayer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Svg
    Multimedia
    MultimediaWidgets
)

set(SOURCES
    src/main.cpp
    src/core/FileSignatureDetector.cpp
    src/core/PlayerController.cpp
    src/ui/MainWindow.cpp
    src/ui/HeaderBar.cpp
    src/ui/MediaViewer.cpp
    src/ui/VideoPlayer.cpp
    src/ui/ImageViewer.cpp
    src/ui/PlaybackControls.cpp
    src/ui/ProgressBar.cpp
    src/ui/VolumeSlider.cpp
    src/ui/SpeedSelector.cpp
    src/ui/SettingsPanel.cpp
    src/ui/FileAssocDialog.cpp
    src/managers/ThemeManager.cpp
    src/managers/DPIAdapter.cpp
    src/managers/FileAssociation.cpp
    src/managers/PluginManager.cpp
    src/managers/IconManager.cpp
    src/utils/FormatUtils.cpp
)

set(HEADERS
    src/core/FileSignatureDetector.h
    src/core/PlayerController.h
    src/ui/MainWindow.h
    src/ui/HeaderBar.h
    src/ui/MediaViewer.h
    src/ui/VideoPlayer.h
    src/ui/ImageViewer.h
    src/ui/PlaybackControls.h
    src/ui/ProgressBar.h
    src/ui/VolumeSlider.h
    src/ui/SpeedSelector.h
    src/ui/SettingsPanel.h
    src/ui/FileAssocDialog.h
    src/managers/ThemeManager.h
    src/managers/DPIAdapter.h
    src/managers/FileAssociation.h
    src/managers/PluginManager.h
    src/managers/IconManager.h
    src/utils/FormatUtils.h
)

set(RESOURCES
    resources/resources.qrc
)

set(WIN_RESOURCES
    resources/app.rc
)

if(WIN32)
    qt_add_executable(FireflyPlayer
        ${SOURCES}
        ${HEADERS}
        ${RESOURCES}
        ${WIN_RESOURCES}
    )
else()
    qt_add_executable(FireflyPlayer
        ${SOURCES}
        ${HEADERS}
        ${RESOURCES}
    )
endif()

target_include_directories(FireflyPlayer PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(FireflyPlayer PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Svg
    Qt6::Multimedia
    Qt6::MultimediaWidgets
)

if(WIN32)
    set_target_properties(FireflyPlayer PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

if(APPLE)
    set_target_properties(FireflyPlayer PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "FireflyPlayer"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    )
endif()
