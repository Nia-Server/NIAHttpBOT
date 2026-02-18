cmake_minimum_required(VERSION 3.21)

set(PROJECT NIAHttpBOT)
set(CMAKE_CXX_STANDARD 20)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

project(${PROJECT})


cmake_host_system_information(RESULT CPU_CORES_FIGURE QUERY NUMBER_OF_LOGICAL_CORES)

if(NOT CMAKE_CXX_COMPILER_LOADED)
	message(FATAL_ERROR, "You should use C++ compiler instead of C")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	if(CMAKE_COMPILER_IS_GNUCXX)
		set(LINK_LIBRARIES_FLAG ws2_32 crypt32 cryptui)
		set(AVX2_FLAG -mavx2)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast") # -march=native
		message("linked ws2_32, crypt32, cryptui for httplib.h")
	else()
		set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
		set(AVX2_FLAG /arch:AVX2)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ")
		add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP${CPU_CORES_FIGURE}>)
	endif()
	if(MSVC)
		add_compile_options("/utf-8")
	endif()
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set(AVX2_FLAG -mavx2)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")
endif()

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("${AVX2_FLAG}" SUPPORTS_AVX2)
if(SUPPORTS_AVX2)
	message("-> enable AVX2 (\"${AVX2_FLAG}\")")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${AVX2_FLAG}")
endif()

include(FetchContent)
set(LEVELDB_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(LEVELDB_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
set(LEVELDB_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
	leveldb
	GIT_REPOSITORY https://github.com/google/leveldb.git
	GIT_TAG 1.23
)
FetchContent_MakeAvailable(leveldb)

if(EXISTS "${leveldb_SOURCE_DIR}/util/env_posix.cc")
	file(READ "${leveldb_SOURCE_DIR}/util/env_posix.cc" LEVELDB_ENV_POSIX_CC)
	string(REPLACE "std::memory_order::memory_order_relaxed" "std::memory_order_relaxed" LEVELDB_ENV_POSIX_CC "${LEVELDB_ENV_POSIX_CC}")
	file(WRITE "${leveldb_SOURCE_DIR}/util/env_posix.cc" "${LEVELDB_ENV_POSIX_CC}")
endif()

set_property(TARGET leveldb PROPERTY CXX_STANDARD 17)
set_property(TARGET leveldb PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET leveldb PROPERTY CXX_EXTENSIONS OFF)

if(MSVC)
	set_property(TARGET leveldb PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

file(GLOB_RECURSE SOURCE CONFIGURE_DEPENDS
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
include_directories(
	./header
	./src
	./src/app
	./src/common
	./src/api/bds
	./src/api/db
	./src/api/file
	./src/api/game
	./src/bot/qq
	./src/ui
)
add_definitions(-D__PROJECT__="${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(${PROJECT} ${SOURCE})
target_link_libraries(${PROJECT} PRIVATE leveldb)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	target_link_libraries(${PROJECT} PRIVATE
		"${CMAKE_SOURCE_DIR}/lib/libcrypto.lib"
		"${CMAKE_SOURCE_DIR}/lib/libssl.lib")

	add_custom_command(TARGET ${PROJECT} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
				"${CMAKE_SOURCE_DIR}/lib/libssl-3-x64.dll"
				"$<TARGET_FILE_DIR:${PROJECT}>/libssl-3-x64.dll"
		COMMAND ${CMAKE_COMMAND} -E copy
				"${CMAKE_SOURCE_DIR}/lib/libcrypto-3-x64.dll"
				"$<TARGET_FILE_DIR:${PROJECT}>/libcrypto-3-x64.dll"
		COMMENT "Copying dll to executable directory")
endif()


