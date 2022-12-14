cmake_minimum_required (VERSION 3.11)

if (CMAKE_BUILD_TYPE MATCHES "^[Rr]elease")

option(BUILD_DOC "Build documentation" ON)

# check if Doxygen is installed
find_package(Doxygen)
if (DOXYGEN_FOUND)
    # set input and output files
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message("Doxygen build started")

    # note the option ALL which allows to build the docs together with the application
    add_custom_target( doc_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )
else (DOXYGEN_FOUND)
  message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)

endif()

enable_language(CXX)
set(CMAKE_CXX_FLAGS "-g -Wall -Wno-restrict -std=c++20")

project (Compiler)

include_directories(.)
include_directories("./include")
include_directories("./src")

Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v2.x
)

FetchContent_MakeAvailable(Catch2)

set (Compiler_VERSION_MAJOR 0)
set (Compiler_VERSION_MINOR 1)

# Discover source and header files
file(GLOB_RECURSE Compiler_SOURCES "src/*.cpp")
file(GLOB_RECURSE Compiler_HEADERS "include/*.h")
file(GLOB_RECURSE Compiler_TESTS "test/*.cpp")

set(Compiler_INCLUDE_DIRS "")
foreach(_headerFile ${Compiler_HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list(APPEND Compiler_INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES Compiler_INCLUDE_DIRS)

# Link compiler into shared library.
add_library(CompileLib SHARED ${Compiler_SOURCES})
target_include_directories(CompileLib PRIVATE ${Compiler_INCLUDE_DIRS})
install(TARGETS CompileLib
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

set(MAIN "src/main.cpp")

# Define the Compiler run configuration
add_executable(Compiler.out ${MAIN})
set_target_properties(Compiler.out PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(Compiler.out PRIVATE ${Compiler_INCLUDE_DIRS})
target_link_libraries(Compiler.out CompileLib)

# Define the Test run configuration
add_executable(Test.out ${Compiler_TESTS})
target_compile_definitions(Test.out PRIVATE TEST_BUILD)
set_target_properties(Test.out PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(Test.out PRIVATE ${Compiler_INCLUDE_DIRS})
target_link_libraries(Test.out CompileLib Catch2::Catch2)