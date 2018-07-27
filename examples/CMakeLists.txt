find_package(Threads QUIET)
set(EXTRA_LIBS ${EXTRA_LIBS} ${CMAKE_THREAD_LIBS_INIT})

include_directories(BEFORE
  ${PROJECT_SOURCE_DIR}/fsm/include)
link_directories()

set(examples
    motor_control_example
    )

file(GLOB_RECURSE this_src ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE this_hdr ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_executable(${PROJECT_NAME}_motor_control_example ${this_src} ${this_hdr})
target_link_libraries(${PROJECT_NAME}_motor_control_example ${PROJECT_LIBRARY_TARGET} ${EXTRA_LIBS})
add_dependencies(${PROJECT_NAME}_motor_control_example ${PROJECT_LIBRARY_TARGET})
add_clang_format(${PROJECT_NAME}_motor_control_example)
install(TARGETS ${PROJECT_NAME}_motor_control_example
      EXPORT ${PROJECT_NAME}-targets
      RUNTIME DESTINATION bin
      COMPONENT examples)
