# generate version header
set(VERSION_IN ${CMAKE_CURRENT_SOURCE_DIR}/src/version.in)
set(VERSION_OUT ${CMAKE_CURRENT_SOURCE_DIR}/include/fsm/version.h)
configure_file(${VERSION_IN} ${VERSION_OUT} @ONLY)

file(GLOB_RECURSE this_src ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp version.in)
file(GLOB_RECURSE this_hdr ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

include_directories(BEFORE
  ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(${PROJECT_NAME}_fsmlib OBJECT ${this_src} ${this_hdr})
add_clang_format(${PROJECT_NAME}_fsmlib)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include)

