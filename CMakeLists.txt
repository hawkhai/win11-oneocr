cmake_minimum_required(VERSION 3.15)
project(win11-oneocr LANGUAGES C CXX)
if(CMAKE_VERSION VERSION_GREATER_EQUAL 4 AND NOT DEFINED CMAKE_POLICY_VERSION_MINIMUM)
    # Pinned OpenCV dependencies still declare policies older than CMake 4.
    set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(EMSCRIPTEN)
    # Keep real exceptions at the C ABI and preserve the native FP contract.
    add_compile_options(-fexceptions -ffp-contract=off -msimd128)
    add_link_options(-fexceptions -msimd128)
endif()

option(ONEOCR_BUILD_PORTABLE "Build the native ONNX source runtime" ON)
option(ONEOCR_BUILD_LEGACY "Build the Windows-only proprietary DLL adapter" OFF)
include(CTest)
if(ONEOCR_BUILD_PORTABLE)
    include(cmake/opencv-source.cmake)
    include(cmake/icu-source.cmake)
    add_subdirectory(native)
    install(FILES THIRD_PARTY_NOTICES.md docs/portable.md docs/error-codes.md docs/wasm.md DESTINATION share/oneocr)
    install(DIRECTORY licenses/ DESTINATION share/oneocr/licenses)
endif()
if(ONEOCR_BUILD_LEGACY)
if(NOT WIN32)
    message(FATAL_ERROR "The legacy OneOCR DLL backend requires Windows")
endif()
add_executable(ocr_legacy ocr.cpp)

# oneocr_wrapper DLL
add_library(oneocr_wrapper SHARED oneocr_wrapper.cpp)

# oneocr_test: header-only usage, no .lib linking needed
add_executable(oneocr_test oneocr_test.cpp)
add_dependencies(oneocr_test oneocr_wrapper)

# Copy bin/ runtime files (DLLs and model) to output directory
set(BIN_FILES
    "${CMAKE_SOURCE_DIR}/bin/oneocr.dll"
    "${CMAKE_SOURCE_DIR}/bin/oneocr.onemodel"
    "${CMAKE_SOURCE_DIR}/bin/onnxruntime.dll"
)

foreach(TARGET_NAME ocr_legacy oneocr_wrapper oneocr_test)
    foreach(BIN_FILE ${BIN_FILES})
        get_filename_component(FILE_NAME ${BIN_FILE} NAME)
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${BIN_FILE}"
                "$<TARGET_FILE_DIR:${TARGET_NAME}>/${FILE_NAME}"
            COMMENT "Copying ${FILE_NAME} to ${TARGET_NAME} output directory"
        )
    endforeach()
endforeach()

# Copy test image for ocr.exe and oneocr_test.exe
foreach(TARGET_NAME ocr_legacy oneocr_test)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/ocr-book.jpg"
            "$<TARGET_FILE_DIR:${TARGET_NAME}>/ocr-book.jpg"
        COMMENT "Copying ocr-book.jpg to ${TARGET_NAME} output directory"
    )
endforeach()

# Copy oneocr_wrapper.dll to oneocr_test output directory
add_custom_command(TARGET oneocr_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:oneocr_wrapper>"
        "$<TARGET_FILE_DIR:oneocr_test>/oneocr_wrapper.dll"
    COMMENT "Copying oneocr_wrapper.dll to oneocr_test output directory"
)
endif()
