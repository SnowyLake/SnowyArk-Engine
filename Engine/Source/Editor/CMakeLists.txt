file(GLOB_RECURSE editorSources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_executable(SnowyArkEditor ${editorSources})

target_link_libraries(
    SnowyArkEditor
    PRIVATE
        SnowyArk
        SnowyArkWarnings
)
