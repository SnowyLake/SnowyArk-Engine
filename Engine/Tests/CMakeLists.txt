file(GLOB_RECURSE testSources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_executable(SnowyArkTests ${testSources})

target_link_libraries(
    SnowyArkTests
    PRIVATE
        SnowyArk
        SnowyArkWarnings
)

add_test(NAME SnowyArkTests COMMAND SnowyArkTests)
