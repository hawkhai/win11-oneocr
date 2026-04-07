cmake_minimum_required(VERSION 3.15)
project(win11-oneocr LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ocr ocr.cpp)

# Copy bin/ runtime files (DLLs and model) to output directory
set(BIN_FILES
    "${CMAKE_SOURCE_DIR}/bin/oneocr.dll"
    "${CMAKE_SOURCE_DIR}/bin/oneocr.onemodel"
    "${CMAKE_SOURCE_DIR}/bin/onnxruntime.dll"
)

foreach(BIN_FILE ${BIN_FILES})
    get_filename_component(FILE_NAME ${BIN_FILE} NAME)
    add_custom_command(TARGET ocr POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${BIN_FILE}"
            "$<TARGET_FILE_DIR:ocr>/${FILE_NAME}"
        COMMENT "Copying ${FILE_NAME} to output directory"
    )
endforeach()

# Also copy the test image
add_custom_command(TARGET ocr POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/ocr-book.jpg"
        "$<TARGET_FILE_DIR:ocr>/ocr-book.jpg"
    COMMENT "Copying ocr-book.jpg to output directory"
)
