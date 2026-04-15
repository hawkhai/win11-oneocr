# copy_release_artifacts.cmake
# Called by CMake POST_BUILD via: cmake -P ... -D CONFIG=... -D ARCH=... -D ...
# Only copies when CONFIG==Release and ARCH==64.

if(NOT CONFIG STREQUAL "Release")
    return()
endif()
if(NOT ARCH EQUAL 8)
    return()
endif()

file(MAKE_DIRECTORY "${DIST_DIR}")

foreach(SRC ${FILES})
    get_filename_component(FNAME "${SRC}" NAME)
    message(STATUS "dist -> ${DIST_DIR}/${FNAME}")
    execute_process(COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${SRC}" "${DIST_DIR}/${FNAME}")
endforeach()
