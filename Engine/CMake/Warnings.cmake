add_library(SnowyArkWarnings INTERFACE)

if(MSVC)
    target_compile_options(SnowyArkWarnings INTERFACE /W4 /permissive-)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang|AppleClang)$")
    target_compile_options(SnowyArkWarnings INTERFACE -Wall -Wextra -Wpedantic)
endif()
