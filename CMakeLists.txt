# CMake 프로그램의 최소 버전
cmake_minimum_required(VERSION 3.11.0)

# 프로젝트 정보
project(
  claujson_cmake_test
  VERSION 0.0.1
  DESCRIPTION "claujson_cmake_test"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17) # C++17
set(CMAKE_BUILD_TYPE "Release")

set(PROGRAM_NAME claujson_cmake_test)

set(ROOT .)

set(LIB_DIR ${ROOT})

set(SOURCE_DIR ${ROOT})

set(INCLUDE_DIR ${ROOT})

# build setting.
add_executable(${PROGRAM_NAME} ${SOURCE_DIR}/main.cpp 
 ${SOURCE_DIR}/vm.cpp ${SOURCE_DIR}/vm_lint.cpp ${SOURCE_DIR}/vm_run.cpp)

include(FetchContent)
FetchContent_Declare(
  claujson
  GIT_REPOSITORY "https://github.com/claujson/claujson"
  GIT_TAG "main"
  )
FetchContent_MakeAvailable(claujson)

include(FetchContent)
FetchContent_Declare(
  Mimalloc
  GIT_REPOSITORY "https://github.com/microsoft/mimalloc"
  GIT_TAG "v2.1.7"
  )
set(MI_BUILD_TESTS OFF)
set(MI_BUILD_EXAMPLES OFF)
FetchContent_MakeAvailable(Mimalloc)


# header..
target_include_directories(${PROGRAM_NAME} PRIVATE ${INCLUDE_DIR})

if (UNIX) 
	target_compile_options(${PROGRAM_NAME} PRIVATE -m64 -std=c++20  -march=native -pthread  -Wno-narrowing  -O2)
elseif (MSVC)
	target_compile_definitions(${PROGRAM_NAME} PRIVATE
        "$<$<CONFIG:Release>:"
            "NDEBUG"
        ">"
        "_CONSOLE;"
        "UNICODE;"
        "_UNICODE"
    )
	
	if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_compile_options(${PROGRAM_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /std:c++20;
                /Oi;
                /GL
            >
            /permissive-;
            /sdl;
            /W3;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )
    endif()
endif()


if (UNIX)
	target_link_libraries(${PROGRAM_NAME} PRIVATE claujson mimalloc pthread)	
  
elseif (MSVC)
	target_link_libraries(${PROGRAM_NAME} PRIVATE claujson mimalloc-static)
	
endif()
