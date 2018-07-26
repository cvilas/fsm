find_package(Threads QUIET)
set(EXTRA_LIBS ${EXTRA_LIBS} ${CMAKE_THREAD_LIBS_INIT})

include_directories(BEFORE
  ${PROJECT_SOURCE_DIR}/fsm/include)
link_directories()

set(examples
    motor_control_example
    )

foreach(example_name ${examples})
  add_executable(${PROJECT_NAME}_${example_name} ${example_name}.cpp)
  target_link_libraries(${PROJECT_NAME}_${example_name} ${PROJECT_LIBRARY_TARGET} ${EXTRA_LIBS})
  add_dependencies(${PROJECT_NAME}_${example_name} ${PROJECT_LIBRARY_TARGET})
  add_clang_format(${PROJECT_NAME}_${example_name})
  install(TARGETS ${PROJECT_NAME}_${example_name}
          EXPORT ${PROJECT_NAME}-targets
          RUNTIME DESTINATION bin
          COMPONENT examples)
endforeach()
