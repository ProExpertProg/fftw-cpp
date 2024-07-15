# Use as hints
if (FFTW3_INCLUDE_DIR OR FFTW3_LIBRARY)
    if (NOT FFTW3_INCLUDE_DIR)
        # try to find based on lib path
        get_filename_component(FFTW3_LIBRARY_DIR ${FFTW3_LIBRARY} DIRECTORY)
        find_path(FFTW3_INCLUDE_DIR fftw3.h PATH_SUFFIXES fftw3 HINTS ${FFTW3_LIBRARY_DIR}/../include)
    elseif (NOT FFTW3_LIBRARY)
        # try to find based on include path
        find_library(FFTW3_LIBRARY NAMES fftw3 HINTS ${FFTW3_INCLUDE_DIR}/../lib)
    endif ()
else ()
    find_path(FFTW3_INCLUDE_DIR fftw3.h PATH_SUFFIXES fftw3)
    find_library(FFTW3_LIBRARY NAMES fftw3)
endif ()

find_package_handle_standard_args(FFTW3 DEFAULT_MSG FFTW3_LIBRARY FFTW3_INCLUDE_DIR)

if (FFTW3_FOUND)
    # set the other output variables
    get_filename_component(FFTW3_LIBRARIES_DIR ${FFTW3_LIBRARY} DIRECTORY)
    set(FFTW3_INCLUDE_DIRS ${FFTW3_INCLUDE_DIR})
    set(FFTW3_LIBRARIES ${FFTW3_LIBRARY})

    if (UNIX)
        # Needs `m` on UNIX
        set(FFTW3_LIBRARIES ${FFTW3_LIBRARIES} m)
    endif ()

    # Create imported target FFTW3::fftw3
    add_library(FFTW3::fftw3 STATIC IMPORTED)
    target_link_libraries(FFTW3::fftw3 INTERFACE ${FFTW3_LIBRARIES})
    target_include_directories(FFTW3::fftw3 INTERFACE ${FFTW3_INCLUDE_DIRS})
endif ()

