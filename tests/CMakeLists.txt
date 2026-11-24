cmake_minimum_required(VERSION 3.22)

# Find Qt6 Test and required components
find_package(Qt6 REQUIRED COMPONENTS Test Core Sql Network Widgets Gui)
find_package(OpenSSL REQUIRED COMPONENTS Crypto)

# Set automation options
set(CMAKE_AUTOMOC ON)

# Include parent directories for headers
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/core)
include_directories(${CMAKE_SOURCE_DIR}/ui)
include_directories(${CMAKE_SOURCE_DIR}/integration)
include_directories(${CMAKE_SOURCE_DIR}/storage)

# ============ Core Tests ============

# Test for MathEvaluator
add_executable(test_matheval
    core/test_matheval.cpp
    ${CMAKE_SOURCE_DIR}/core/MathEvaluator.cpp
)
target_link_libraries(test_matheval PRIVATE Qt6::Test Qt6::Core)
add_test(NAME MathEvaluatorTests COMMAND test_matheval)

# Test for UnitConverter
add_executable(test_unitconv
    core/test_unitconv.cpp
    ${CMAKE_SOURCE_DIR}/core/UnitConverter.cpp
)
target_link_libraries(test_unitconv PRIVATE Qt6::Test Qt6::Core)
add_test(NAME UnitConverterTests COMMAND test_unitconv)

# Test for TextAnalyzer
add_executable(test_textanalyzer
    core/test_textanalyzer.cpp
    ${CMAKE_SOURCE_DIR}/core/TextAnalyzer.cpp
)
target_link_libraries(test_textanalyzer PRIVATE Qt6::Test Qt6::Core)
add_test(NAME TextAnalyzerTests COMMAND test_textanalyzer)

# Test for CurrencyConverter
add_executable(test_currency
    core/test_currency.cpp
    ${CMAKE_SOURCE_DIR}/core/CurrencyConverter.cpp
    ${CMAKE_SOURCE_DIR}/core/Settings.cpp
    ${CMAKE_SOURCE_DIR}/storage/SqliteStorage.cpp
    ${CMAKE_SOURCE_DIR}/core/Note.cpp
)
target_link_libraries(test_currency PRIVATE Qt6::Test Qt6::Core Qt6::Network Qt6::Sql Qt6::Gui)
add_test(NAME CurrencyConverterTests COMMAND test_currency)

# Test for Settings
add_executable(test_settings
    core/test_settings.cpp
    ${CMAKE_SOURCE_DIR}/core/Settings.cpp
    ${CMAKE_SOURCE_DIR}/storage/SqliteStorage.cpp
    ${CMAKE_SOURCE_DIR}/core/Note.cpp
)
target_link_libraries(test_settings PRIVATE Qt6::Test Qt6::Core Qt6::Sql Qt6::Widgets)
add_test(NAME SettingsTests COMMAND test_settings)

# ============ Storage Tests ============

# Test for Crypto
add_executable(test_crypto
    storage/test_crypto.cpp
    ${CMAKE_SOURCE_DIR}/storage/Crypto.cpp
)
target_link_libraries(test_crypto PRIVATE Qt6::Test Qt6::Core OpenSSL::Crypto)
add_test(NAME CryptoTests COMMAND test_crypto)

# Test for SqliteStorage
add_executable(test_sqlite
    storage/test_sqlite.cpp
    ${CMAKE_SOURCE_DIR}/storage/SqliteStorage.cpp
    ${CMAKE_SOURCE_DIR}/core/Note.cpp
)
target_link_libraries(test_sqlite PRIVATE Qt6::Test Qt6::Core Qt6::Sql)
add_test(NAME SqliteStorageTests COMMAND test_sqlite)

# ============ Headless ============

# test_currency links Qt6::Gui and test_settings links Qt6::Widgets, so
# QTEST_MAIN gives them a QGuiApplication and a QApplication, which need a
# window system. Over SSH, in a container or in CI there is none, and both
# abort with SIGABRT before printing a single line, which reads like a
# broken test rather than a missing display. The offscreen plugin costs
# nothing and is set for every test so a new one cannot inherit the trap.
set_tests_properties(
    MathEvaluatorTests
    UnitConverterTests
    TextAnalyzerTests
    CurrencyConverterTests
    SettingsTests
    CryptoTests
    SqliteStorageTests
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)
