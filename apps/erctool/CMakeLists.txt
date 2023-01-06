set(ERCTOOL erctool.cpp)

source_group(apps\\erctool FILES ${ERCTOOL})

add_executable(erctool ${ERCTOOL})

target_link_libraries(erctool PRIVATE cxxopts::cxxopts erc)

if (OPENBLACK_CLANG_TIDY_CHECKS)
  # FIXME(bwrsandman) MSVC is throwing false errors about exceptions being disabled
  if (NOT MSVC AND CLANG_TIDY)
    set_target_properties(erctool PROPERTIES CXX_CLANG_TIDY ${CLANG_TIDY})
  else ()
    message("Clang-tidy checks requested but unavailable")
  endif ()
endif ()

if (MSVC)
  target_compile_options(erctool PRIVATE /W4 /WX)
else ()
  target_compile_options(
    erctool
    PRIVATE -Wall -Wextra -pedantic -Werror
  )
endif ()

set_property(TARGET erctool PROPERTY FOLDER "tools")
