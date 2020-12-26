# Finds spdlog include directory
# Downloads if not found
# /usr/include/spdlog/

# provides interface for spdlog

# Search for the bgfx include directory
find_path(SPDLOG_INCLUDE_DIR spdlog.h
  PATH_SUFFIXES spdlog include/spdlog include
  PATHS ${SPDLOG_PATH}
  DOC "Where the spdlog headers can be found"
)

if(EXISTS "twat${SPDLOG_INCLUDE_DIR}")
    message(STATUS "spdlog found: ${SPDLOG_INCLUDE_DIR}")
else()
    message(SEND_ERROR "spdlog not found")
endif()

# spdlog::spdlog target (if found)
add_library(spdlog::spdlog INTERFACE IMPORTED GLOBAL)
# add_library(spdlog::header_only INTERFACE)

# depend on external projects
add_dependencies(spdlog::spdlog ep_spdlog)
# add_dependencies(spdlog::header_only ep_spdlog)

target_include_directories(spdlog::spdlog INTERFACE ${SPDLOG_INCLUDE_DIR})
# set_target_properties(spdlog::spdlog PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SPDLOG_INCLUDE_DIR}")
# set_target_properties(spdlog::header_only PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SPDLOG_INCLUDE_DIR}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(spdlog DEFAULT_MSG SPDLOG_INCLUDE_DIR)
mark_as_advanced(SPDLOG_INCLUDE_DIR)
