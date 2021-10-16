add_library(automata)

target_sources(automata
  PRIVATE
    automata.cpp
  PUBLIC
    automata.h
  )

target_include_directories(automata
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
  )

if(BUILD_TEST)
  # we use this to get code coverage
  # flags are only valid with the GNU compiler and on Linux
  if(CMAKE_CXX_COMPILER_ID MATCHES GNU AND CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    target_compile_options(automata
      PUBLIC
        "--coverage"
      )
    target_link_options(automata
      INTERFACE
        "--coverage"
      )
  endif()
endif()
