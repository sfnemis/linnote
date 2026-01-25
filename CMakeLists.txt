cmake_minimum_required(VERSION 3.22)
project(linnote VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Fix for Qt 6.10+ with GCC 15 (GNU_PROPERTY_1_NEEDED_INDIRECT_EXTERN_ACCESS)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--copy-dt-needed-entries")

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets DBus Network Sql)

# Find KDE Frameworks
find_package(KF6WindowSystem QUIET)
find_package(KF6GlobalAccel QUIET)

# Find LayerShellQt for Wayland always-on-top support
find_package(LayerShellQt QUIET)

# Generate version header from template
configure_file(
    "${CMAKE_SOURCE_DIR}/config/version.h.in"
    "${CMAKE_BINARY_DIR}/generated/version.h"
    @ONLY
)
include_directories("${CMAKE_BINARY_DIR}/generated")

# Source files
set(SOURCES
    main.cpp
    core/Note.cpp
    core/NoteManager.cpp
    core/Settings.cpp
    core/SlashCommand.cpp
    core/MathEvaluator.cpp
    core/CurrencyConverter.cpp
    core/UnitConverter.cpp
    core/Timer.cpp
    core/Theme.cpp
    core/OcrHelper.cpp
    core/UrlShortener.cpp
    core/TextAnalyzer.cpp
    core/ExampleNotes.cpp
    core/UpdateChecker.cpp
    ui/MainWindow.cpp
    ui/NoteEditor.cpp
    ui/MarkdownHighlighter.cpp
    ui/CodeHighlighter.cpp
    ui/TrayIcon.cpp
    ui/PageSelector.cpp
    ui/SettingsDialog.cpp
    ui/ModeHelper.cpp
    ui/CommandPopup.cpp
    ui/SearchBar.cpp
    ui/SearchModal.cpp
    ui/PasswordDialog.cpp
    ui/FirstRunDialog.cpp
    ui/ConfettiWidget.cpp

    integration/PortalShortcuts.cpp
    integration/ClipboardManager.cpp
    integration/DBusService.cpp
    storage/Export.cpp
    storage/NoteStorage.cpp
    storage/BackupManager.cpp
    storage/SqliteStorage.cpp
    storage/Crypto.cpp
)

set(HEADERS
    core/Note.h
    core/NoteMode.h
    core/NoteManager.h
    core/Settings.h
    core/SlashCommand.h
    core/MathEvaluator.h
    core/CurrencyConverter.h
    core/UnitConverter.h
    core/Timer.h
    core/Theme.h
    core/ExampleNotes.h
    core/UpdateChecker.h
    ui/MainWindow.h
    ui/NoteEditor.h
    ui/TrayIcon.h
    ui/PageSelector.h
    ui/SettingsDialog.h
    ui/ModeHelper.h
    ui/CommandPopup.h
    ui/FirstRunDialog.h
    integration/PortalShortcuts.h
    integration/ClipboardManager.h
    integration/DBusService.h
    integration/DesktopHelper.h
    storage/Export.h
    storage/NoteStorage.h
    storage/SqliteStorage.h

)

# Qt Resources
set(RESOURCES
    resources/resources.qrc
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${RESOURCES})

# Link Qt6 libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::DBus
    Qt6::Network
    Qt6::Sql
)

# Link KDE Frameworks if available
if(KF6WindowSystem_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE KF6::WindowSystem)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_KWINDOWSYSTEM)
    message(STATUS "KWindowSystem found - X11 support enabled")
endif()

# Link KGlobalAccel for KDE global shortcuts
if(KF6GlobalAccel_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE KF6::GlobalAccel)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_KGLOBALACCEL)
    message(STATUS "KGlobalAccel found - KDE global shortcuts enabled")
endif()

# Link LayerShellQt for Wayland always-on-top
if(LayerShellQt_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE LayerShellQt::Interface)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_LAYERSHELLQT)
    message(STATUS "LayerShellQt found - Wayland always-on-top enabled")
else()
    message(STATUS "LayerShellQt not found - Wayland always-on-top disabled")
endif()

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/integration
    ${CMAKE_CURRENT_SOURCE_DIR}/storage
)

# Install targets
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(FILES linnote.desktop DESTINATION share/applications)

# Install icons for all sizes
foreach(SIZE 16 32 48 64 128 256 512)
    install(FILES resources/icons/app/linnote-${SIZE}.png
            DESTINATION share/icons/hicolor/${SIZE}x${SIZE}/apps
            RENAME linnote.png)
endforeach()

# Testing support
option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
