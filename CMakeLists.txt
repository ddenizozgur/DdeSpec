cmake_minimum_required(VERSION 3.5)

project(DdeEd LANGUAGES C)
find_package(OpenGL REQUIRED)

set(SOURCES
    main.c
    Spec.h)

add_executable(DdeEd ${SOURCES})

target_link_libraries(DdeEd PRIVATE OpenGL::GL)

install(TARGETS DdeEd
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
