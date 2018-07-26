cmake_minimum_required(VERSION 3.6)
project(fsm CXX)
enable_testing(true)

#----------------------------------------------------------------------------------------------------------------------
# project version
#----------------------------------------------------------------------------------------------------------------------
string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%d %H:%M:%S UTC" UTC)
set (VERSION_MAJOR 0)
set (VERSION_MINOR 5)
set (VERSION_PATCH 0)
set (VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
message("Build version will be ${VERSION} (${BUILD_TIMESTAMP})")

#----------------------------------------------------------------------------------------------------------------------
# Build system configuration files
#----------------------------------------------------------------------------------------------------------------------
include(${PROJECT_SOURCE_DIR}/cmake/colours.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/add_clang_format.cmake)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)

#----------------------------------------------------------------------------------------------------------------------
# Build options
#----------------------------------------------------------------------------------------------------------------------

# turn on c++14 compile flags. Don't use non-standard extensions to iso cpp
# NOTE: external projects may not apply the same flags
set(CMAKE_CXX_STANDARD 14) # Turn on C++14 compile flags
set(CMAKE_CXX_EXTENSIONS OFF) # Turn off non-standard extensions to ISo C++
set(CMAKE_POSITION_INDEPENDENT_CODE ON) # Required for utilities::demangle
option(BUILD_SHARED_LIBS "Create shared libraries" ON)
option(BUILD_TESTS "Build test harness" ON)

#------------------------------------------------------------------------------
# External dependencies
#------------------------------------------------------------------------------

#find_package(fastcdr REQUIRED)
#include_directories(${fastcdr_INCLUDE_DIR} ${fastrtps_INCLUDE_DIR})
#set(EXTRA_LIBS fastrtps fastcdr)

#------------------------------------------------------------------------------
# Configure linter (for development)
#------------------------------------------------------------------------------

find_package(ClangTools QUIET)
if(CLANG_TIDY_FOUND)
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_BIN};-checks=*, -fuchsia*, -google*")
else(CLANG_TIDY_FOUND)
    message(STATUS "${Yellow}clang-tidy not found. Static analysis won't be done.${ColourReset}")
endif(CLANG_TIDY_FOUND)
if(NOT CLANG_FORMAT_FOUND)
    message(STATUS "${Yellow}clang-format not found. Style checks won't be done.${ColourReset}")
endif(NOT CLANG_FORMAT_FOUND)

#------------------------------------------------------------------------------
# Documentation target
#------------------------------------------------------------------------------

set(DOC_PATH ${PROJECT_SOURCE_DIR}/docs/generated)
file(MAKE_DIRECTORY ${DOC_PATH})

find_package(Doxygen QUIET)
if (DOXYGEN_FOUND)
    # set output path
    message(STATUS "${Green}Docs location (run 'make docs'): ${DOC_PATH}${ColourReset}")

    # set input and output files. @DOC_PATH@ is referenced in doxyfile.in
    set(DOXYGEN_IN ${PROJECT_SOURCE_DIR}/docs/doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_BINARY_DIR}/doxyfile)
    file(GLOB_RECURSE DOC_FILES
        ${PROJECT_SOURCE_DIR}/.travis.yml
        ${PROJECT_SOURCE_DIR}/todo.md
        ${PROJECT_SOURCE_DIR}/README.md
        ${PROJECT_SOURCE_DIR}/LICENSE.md)

    # request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    # note the option ALL which allows to build the docs together with the application
    add_custom_target( docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        SOURCES ${DOC_FILES}
        COMMENT "Generating API documentation"
        VERBATIM )

    install(DIRECTORY ${DOC_PATH} DESTINATION share/${PROJECT_NAME})

else (DOXYGEN_FOUND)
  message(STATUS "${Yellow}Doxygen not found. No documentation will be built${ColourReset}")
endif (DOXYGEN_FOUND)

#------------------------------------------------------------------------------
# targets
#------------------------------------------------------------------------------

set(PROJECT_LIBRARY_TARGET ${PROJECT_NAME})

add_subdirectory(fsm)
add_subdirectory(tests)
add_subdirectory(examples)

#------------------------------------------------------------------------------
# compile objects into a library
#------------------------------------------------------------------------------

add_library(${PROJECT_LIBRARY_TARGET}
    $<TARGET_OBJECTS:${PROJECT_NAME}_fsmlib>)
set_target_properties(${PROJECT_LIBRARY_TARGET} PROPERTIES VERSION ${VERSION} SOVERSION ${VERSION_MAJOR} )
if (DOXYGEN_FOUND)
add_dependencies(${PROJECT_LIBRARY_TARGET} docs)
endif()

#------------------------------------------------------------------------------
# installation
#------------------------------------------------------------------------------

configure_file(${PROJECT_SOURCE_DIR}/cmake/config.cmake.in
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}-config.cmake" @ONLY)

configure_file(${PROJECT_SOURCE_DIR}/cmake/config-version.cmake.in
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}-config-version.cmake" @ONLY)

install(TARGETS ${PROJECT_LIBRARY_TARGET}
        EXPORT ${PROJECT_NAME}-targets
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        COMPONENT libraries)

install(EXPORT ${PROJECT_NAME}-targets DESTINATION lib/${PROJECT_NAME}/cmake)

install(FILES
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}-config.cmake"
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}-config-version.cmake"
  DESTINATION lib/${PROJECT_NAME}/cmake)
