cmake_minimum_required(VERSION 3.15)
project(win11-oneocr LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ocr ocr.cpp)

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

foreach(TARGET_NAME ocr oneocr_wrapper oneocr_test)
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
foreach(TARGET_NAME ocr oneocr_test)
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
