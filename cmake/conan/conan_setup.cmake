# cmake/FindOrAddThirdParty.cmake

# Usage:
#   find_or_add_third_party(<LIB_NAME> <PKG_NAME> [<INCLUDE_HEADER>])
# Example:
#   find_or_add_third_party(spdlog spdlog spdlog/spdlog.h)
#   find_or_add_third_party(fmt fmt fmt/format.h)
#
# Sets:
#   <LIB_NAME>_TARGET (e.g., SPDLOG_TARGET)
#
# Option:
#   USE_CONAN_<LIB_NAME_UPPER>_PKG (default OFF)

function(find_or_add_third_party LIB_NAME PKG_NAME INCLUDE_HEADER)
    string(TOUPPER "${LIB_NAME}" LIB_NAME_UPPER)
    set(USE_CONAN_OPTION "USE_CONAN_${LIB_NAME_UPPER}_PKG")
    option(${USE_CONAN_OPTION} "Use ${PKG_NAME} from Conan or system package" OFF)

    set(TARGET_VAR "${LIB_NAME_UPPER}_TARGET")

    if(NOT DEFINED "${LIB_NAME_UPPER}_FOUND_GLOBAL")
        set("${LIB_NAME_UPPER}_FOUND_GLOBAL" FALSE CACHE INTERNAL "")

        if(${USE_CONAN_OPTION})
            find_package(${PKG_NAME} REQUIRED)
            set(${TARGET_VAR} ${PKG_NAME}::${PKG_NAME} PARENT_SCOPE)
            set("${LIB_NAME_UPPER}_FOUND_GLOBAL" TRUE CACHE INTERNAL "")
            message(STATUS "[third_party] Using ${PKG_NAME} from Conan or system package.")
        else()
            set(THIRD_PARTY_DIR "${CMAKE_SOURCE_DIR}/third_party/${PKG_NAME}")
            if(EXISTS "${THIRD_PARTY_DIR}/include/${INCLUDE_HEADER}")
                add_subdirectory(${THIRD_PARTY_DIR} EXCLUDE_FROM_ALL)
                set(${TARGET_VAR} ${PKG_NAME} PARENT_SCOPE)
                set("${LIB_NAME_UPPER}_FOUND_GLOBAL" TRUE CACHE INTERNAL "")
                message(STATUS "[third_party] Using ${PKG_NAME} from third_party directory.")
            else()
                message(FATAL_ERROR "[third_party] ${PKG_NAME} not found in third_party! Set -D${USE_CONAN_OPTION}=ON to use Conan/system package.")
            endif()
        endif()
    else()
        # Already found, just set the target variable
        if(${USE_CONAN_OPTION})
            set(${TARGET_VAR} ${PKG_NAME}::${PKG_NAME} PARENT_SCOPE)
        else()
            set(${TARGET_VAR} ${PKG_NAME} PARENT_SCOPE)
        endif()
    endif()
endfunction()
