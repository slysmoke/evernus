set(BREAKPAD_ROOT_HINTS ${BREAKPAD_ROOT_DIR} ENV BREAKPAD_ROOT_DIR)

if(WIN32)
    set(BREAKPAD_HANDLER_HEADER "client/windows/handler/exception_handler.h")

    find_library(
        BREAKPAD_COMMON_LIBRARY_DEBUG
        NAMES "common.lib"
        HINTS ${BREAKPAD_ROOT_HINTS}
        PATH_SUFFIXES client/windows/Debug/lib
    )
    find_library(
        BREAKPAD_COMMON_LIBRARY_RELEASE
        NAMES "common.lib"
        HINTS ${BREAKPAD_ROOT_HINTS}
        PATH_SUFFIXES client/windows/Release/lib
    )

    mark_as_advanced(BREAKPAD_COMMON_LIBRARY_DEBUG BREAKPAD_COMMON_LIBRARY_RELEASE)

    find_library(
        BREAKPAD_GEN_CLIENT_LIBRARY_DEBUG
        NAMES "crash_generation_client.lib"
        HINTS ${BREAKPAD_ROOT_HINTS}
        PATH_SUFFIXES client/windows/Debug/lib
    )
    find_library(
        BREAKPAD_GEN_CLIENT_LIBRARY_RELEASE
        NAMES "crash_generation_client.lib"
        HINTS ${BREAKPAD_ROOT_HINTS}
        PATH_SUFFIXES client/windows/Release/lib
    )

    mark_as_advanced(BREAKPAD_GEN_CLIENT_LIBRARY_DEBUG BREAKPAD_GEN_CLIENT_LIBRARY_RELEASE)

    find_library(
        BREAKPAD_HANDLER_LIBRARY_DEBUG
        NAMES "exception_handler.lib"
        HINTS ${BREAKPAD_ROOT_HINTS}
        PATH_SUFFIXES client/windows/Debug/lib
    )
    find_library(
        BREAKPAD_HANDLER_LIBRARY_RELEASE
        NAMES "exception_handler.lib"
        HINTS ${BREAKPAD_ROOT_HINTS}
        PATH_SUFFIXES client/windows/Release/lib
    )

    mark_as_advanced(BREAKPAD_HANDLER_LIBRARY_DEBUG BREAKPAD_HANDLER_LIBRARY_RELEASE)
elseif(APPLE)
    set(BREAKPAD_HANDLER_HEADER "client/mac/handler/exception_handler.h")
    find_library(
        BREAKPAD_HANDLER_LIBRARY
        NAMES Breakpad
        HINTS ${BREAKPAD_ROOT_HINTS}
        PATH_SUFFIXES client/mac/build/Release/
    )

    # This is because cmake somehow does not recognize *.framework as framework
    # and we need a proper library name. Also it cannot find Breakpad lib inside
    # the framework, even given exact path.
    set(BREAKPAD_HANDLER_LIBRARY "${BREAKPAD_HANDLER_LIBRARY}/Breakpad")

    mark_as_advanced(BREAKPAD_HANDLER_LIBRARY)
else()
    set(BREAKPAD_HANDLER_HEADER "client/linux/handler/exception_handler.h")

    find_library(
        BREAKPAD_HANDLER_LIBRARY
        NAMES libbreakpad_client.a
        HINTS ${BREAKPAD_ROOT_HINTS}
        PATH_SUFFIXES lib
    )

    mark_as_advanced(BREAKPAD_HANDLER_LIBRARY)
endif()

find_path(
    BREAKPAD_HANDLER_INCLUDE_DIR
    NAMES "${BREAKPAD_HANDLER_HEADER}"
    HINTS ${BREAKPAD_ROOT_HINTS}
    PATH_SUFFIXES breakpad
)

mark_as_advanced(BREAKPAD_HANDLER_INCLUDE_DIR)

if(APPLE)
    add_library(Breakpad::Client SHARED IMPORTED)
else()
    add_library(Breakpad::Client STATIC IMPORTED)
endif()

set_target_properties(
    Breakpad::Client
    PROPERTIES
    IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
    INTERFACE_INCLUDE_DIRECTORIES "${BREAKPAD_HANDLER_INCLUDE_DIR}"
)

if(WIN32)
    set_target_properties(
        Breakpad::Client
        PROPERTIES
        IMPORTED_CONFIGURATIONS "DEBUG;RELEASE"
        IMPORTED_LOCATION_DEBUG "${BREAKPAD_HANDLER_LIBRARY_DEBUG}"
        IMPORTED_LOCATION_RELEASE "${BREAKPAD_HANDLER_LIBRARY_RELEASE}"
        IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG "${BREAKPAD_GEN_CLIENT_LIBRARY_DEBUG};${BREAKPAD_COMMON_LIBRARY_DEBUG}"
        IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "${BREAKPAD_GEN_CLIENT_LIBRARY_RELEASE};${BREAKPAD_COMMON_LIBRARY_RELEASE}"
    )
else()
    set_target_properties(
        Breakpad::Client
        PROPERTIES
        FRAMEWORK TRUE
        IMPORTED_LOCATION "${BREAKPAD_HANDLER_LIBRARY}"
    )
endif()
