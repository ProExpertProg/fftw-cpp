# This script is used to find the FFTW3 library.
#
# Output variables:
# - FFTW3_FOUND, whether the library was found
# - FFTW3_INCLUDE_DIRS, where to find fftw3.h
# - FFTW3_LIBRARIES_DIR, where to find the library
# - FFTW3_LIBRARIES, the library (and dependencies) to link against
# - FFTW3::fftw3, an imported target for the library.
#   This target also contains the proper include directories and dependencies needed
#
# Hints:
# - FFTW3_INCLUDE_DIR, where to find fftw3.h
# - FFTW3_LIBRARY_DIR, where to find the library
# - FFTW3_LIBRARY, the library to link against
# Any number of hints can be passed. The remaining ones are inferred from the ones passed,
# assuming the library is installed in a `lib` directory next to the `include` directory,
# which contains the header file.

# If library dir is not passed, it is inferred from the library path or the include dir
if (FFTW3_INCLUDE_DIR OR FFTW3_LIBRARY_DIR OR FFTW3_LIBRARY)
    if (FFTW3_LIBRARY AND NOT FFTW3_LIBRARY_DIR)
        # try to find based on lib path
        get_filename_component(FFTW3_LIBRARY_DIR ${FFTW3_LIBRARY} DIRECTORY)
    endif ()

    if (NOT FFTW3_INCLUDE_DIR)
        find_path(FFTW3_INCLUDE_DIR fftw3.h PATH_SUFFIXES fftw3 HINTS ${FFTW3_LIBRARY_DIR}/../include NO_DEFAULT_PATH)
    endif ()

    if (NOT FFTW3_LIBRARY)
        if (NOT FFTW3_LIBRARY_DIR)
            # try to find based on include path
            set(FFTW3_LIBRARY_DIR ${FFTW3_INCLUDE_DIR}/../lib)
        endif()
        # try to find based on library_dir
        find_library(FFTW3_LIBRARY NAMES fftw3 HINTS ${FFTW3_LIBRARY_DIR} NO_DEFAULT_PATH)
    endif ()
else ()
    # No hints passed, look in default locations
    find_path(FFTW3_INCLUDE_DIR fftw3.h PATH_SUFFIXES fftw3)
    find_library(FFTW3_LIBRARY NAMES fftw3)
    if (FFTW3_LIBRARY)
        # try to find based on lib path
        get_filename_component(FFTW3_LIBRARY_DIR ${FFTW3_LIBRARY} DIRECTORY)
    endif ()
endif ()

# handles REQUIRED, QUIET, output etc.
find_package_handle_standard_args(FFTW3 DEFAULT_MSG FFTW3_LIBRARY FFTW3_INCLUDE_DIR)

if (FFTW3_FOUND)
    # set the other output variables
    set(FFTW3_LIBRARIES_DIR ${FFTW3_LIBRARY_DIR})
    set(FFTW3_INCLUDE_DIRS ${FFTW3_INCLUDE_DIR})
    set(FFTW3_LIBRARIES ${FFTW3_LIBRARY})

    # Create imported target FFTW3::fftw3
    add_library(FFTW3::fftw3 STATIC IMPORTED)
    set_target_properties(FFTW3::fftw3 PROPERTIES IMPORTED_LOCATION ${FFTW3_LIBRARY})
    target_include_directories(FFTW3::fftw3 INTERFACE ${FFTW3_INCLUDE_DIRS})

    if (UNIX)
        # Needs `m` on UNIX
        set(FFTW3_LIBRARIES ${FFTW3_LIBRARIES} m)
        target_link_libraries(FFTW3::fftw3 INTERFACE m)
    endif ()
endif ()

