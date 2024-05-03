# build the unit test executable.
set(CURRENT_EXE_NAME vlist_ut)

find_library(UBOX ubox)

set(SOURCE_FILES
        main.cpp
        ut.cpp)
add_executable(${CURRENT_EXE_NAME} ${SOURCE_FILES})
add_dependencies(${CURRENT_EXE_NAME} cpputest)
target_link_libraries(${CURRENT_EXE_NAME} CppUTest CppUTestExt ubox)
