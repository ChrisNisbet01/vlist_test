set(CURRENT_EXE_NAME vlist_plain)

find_library(UBOX ubox)

set(SOURCE_FILES
        plain.c)
add_executable(${CURRENT_EXE_NAME} ${SOURCE_FILES})
target_link_libraries(${CURRENT_EXE_NAME} ubox)

set(CURRENT_EXE_NAME vlist_no_delete)

set(SOURCE_FILES
        no_delete.c)
add_executable(${CURRENT_EXE_NAME} ${SOURCE_FILES})
target_link_libraries(${CURRENT_EXE_NAME} ubox)

set(CURRENT_EXE_NAME vlist_keep_old)

set(SOURCE_FILES
        keep_old.c)
add_executable(${CURRENT_EXE_NAME} ${SOURCE_FILES})
target_link_libraries(${CURRENT_EXE_NAME} ubox)
