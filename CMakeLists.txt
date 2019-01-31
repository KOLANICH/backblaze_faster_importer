CMAKE_MINIMUM_REQUIRED(VERSION 3.14)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

include(CheckSymbolExists)
include(CheckIncludeFile)
include(CheckIncludeFileCXX)
include(FetchContent)

include(ExternalProject)
include(Hardening)
include(VersionFromGit)
include(CheckStdFileSystem)

include(DownloadSQLite)
include(DownloadSQLiteCPP)
include(DownloadCLI11)
include(GenerateSchemaHeaders)

find_package(Python COMPONENTS Interpreter)

PROJECT(FasterImport)

#find_package(SQLite3)

set(CMAKE_USE_RELATIVE_PATHS TRUE)

set(PACKAGE_NAME "FasterImport" CACHE STRING "Program name")
set(PACKAGE_VERSION ${VERSION})
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
#getVersionFromGit(FasterImport "v0.1")

set(Source_dir "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(CMake_Misc_Dir "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

#making resource file
#set(license_file "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
#file(READ ${license_file} LICENSE_TEXT)
#string(REPLACE "\"" "\"\"" LICENSE_TEXT ${LICENSE_TEXT})
#string(REPLACE "\r\n" "\n" LICENSE_TEXT ${LICENSE_TEXT})
#string(REPLACE "\n" "\\r\\n" LICENSE_TEXT ${LICENSE_TEXT})


checkStdFileSystem(CPP_DIR_ITERATION_WORKS)
check_symbol_exists("opendir" "dirent.h" DIRENT_PRESENT)

# config.h
set(CONFIG_FILENAME "config.h")
set(CONFIG_FILE "${Source_dir}/generated/${CONFIG_FILENAME}")
mark_as_advanced(CONFIG_FILE)
set(CONFIG_FILE_PROTO "${CMake_Misc_Dir}/${CONFIG_FILENAME}.in")

message(STATUS "generating config: ${CONFIG_FILE_PROTO} -> ${CONFIG_FILE}")
configure_file("${CONFIG_FILE_PROTO}" "${CONFIG_FILE}")

downloadCLI11IfNeeded(cli11)
downloadSQLiteIfNeeded(sqlite3)
downloadSQLiteCPPIfNeeded(SQLITE_CPP)

file(GLOB_RECURSE SRCFILES "${Source_dir}/*.c" "${Source_dir}/*.cpp" "${sqlite3_AMALGAMATION_SOURCE_DIR}/sqlite3.c")
file(GLOB_RECURSE resource_files "${Source_dir}/*.rc" "${Source_dir}/*.ico")
if(MSVC_IDE)
	file(GLOB_RECURSE HDRFILES "${Source_dir}/*.h")
endif()

message(STATUS "${SRCFILES} ${HDRFILES} ${resource_files}")
add_executable(FasterImport "${SRCFILES}" "${HDRFILES}" "${resource_files}")
target_include_directories(FasterImport PUBLIC "${CLI11_INCLUDE_DIR}" "${SQLITE_CPP_INCLUDE_DIR}" "${sqlite3_AMALGAMATION_SOURCE_DIR}")

generateSchemaHeaders(FasterImport)
add_dependencies(FasterImport cli11)

target_link_libraries(FasterImport PUBLIC "ssp")

target_compile_definitions(FasterImport PUBLIC "-D_UNICODE=1" "-DUNICODE=1")

set_property(TARGET FasterImport PROPERTY CXX_STANDARD 20)
set_property(TARGET FasterImport PROPERTY PREFIX "")
harden(FasterImport)

if(MSVC_IDE)
	source_group("res" FILES ${resource_files})
endif()