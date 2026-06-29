cmake_minimum_required(VERSION 3.20)

project(PkmnRedSaveGenerator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

add_executable(
  pkmn-red-save-generator
  "Pkmn Red Save Generator/main.cpp"
)

target_include_directories(
  pkmn-red-save-generator
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party"
)

if(APPLE)
  target_compile_definitions(pkmn-red-save-generator PRIVATE PKMN_SAVEGEN_MACOS=1)
endif()

add_executable(
  pkmn-red-save-generator-tests
  tests/test_main.cpp
  tests/milestone1_tests.cpp
)

target_include_directories(
  pkmn-red-save-generator-tests
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party"
)

if(APPLE)
  target_compile_definitions(pkmn-red-save-generator-tests PRIVATE PKMN_SAVEGEN_MACOS=1)
endif()

add_test(NAME milestone1-tests COMMAND pkmn-red-save-generator-tests)
set_tests_properties(milestone1-tests PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
