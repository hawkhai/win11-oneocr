cmake_minimum_required(VERSION 3.15)
project(win11-oneocr LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MinHook paths (relative to project root)
set(MINHOOK_DIR "${CMAKE_SOURCE_DIR}/../../note/minhook")
set(MINHOOK_INCLUDE_DIR "${MINHOOK_DIR}/include")
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(MINHOOK_LIB "${MINHOOK_DIR}/bin/MinHook.x64.lib")
    set(MINHOOK_DLL "${MINHOOK_DIR}/bin/MinHook.x64.dll")
else()
    set(MINHOOK_LIB "${MINHOOK_DIR}/bin/MinHook.x86.lib")
    set(MINHOOK_DLL "${MINHOOK_DIR}/bin/MinHook.x86.dll")
endif()

add_executable(ocr ocr.cpp)

# oneocr_wrapper DLL
add_library(oneocr_wrapper SHARED oneocr_wrapper.cpp)

# oneocr_test: header-only usage + MinHook BCrypt tracing
add_executable(oneocr_test oneocr_test.cpp bcrypt_hook.cpp onnx_dump.cpp)
target_include_directories(oneocr_test PRIVATE ${MINHOOK_INCLUDE_DIR})
target_link_libraries(oneocr_test PRIVATE ${MINHOOK_LIB} bcrypt)
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

# Copy MinHook runtime DLL alongside oneocr_test
add_custom_command(TARGET oneocr_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${MINHOOK_DLL}"
        "$<TARGET_FILE_DIR:oneocr_test>/$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,MinHook.x64.dll,MinHook.x86.dll>"
    COMMENT "Copying MinHook DLL to oneocr_test output directory"
)

# x64 Release: copy all final artifacts to ${CMAKE_SOURCE_DIR}/bin
set(DIST_DIR "${CMAKE_SOURCE_DIR}/bin")
set(COPY_SCRIPT "${CMAKE_SOURCE_DIR}/cmake/copy_release_artifacts.cmake")

# oneocr_wrapper.dll
add_custom_command(TARGET oneocr_wrapper POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -D CONFIG=$<CONFIG>
        -D ARCH=${CMAKE_SIZEOF_VOID_P}
        -D DIST_DIR=${DIST_DIR}
        "-D FILES=$<TARGET_FILE:oneocr_wrapper>"
        -D CMAKE_COMMAND=${CMAKE_COMMAND}
        -P "${COPY_SCRIPT}"
    COMMENT "x64 Release: copying oneocr_wrapper.dll to bin/"
)

# oneocr_test.exe + MinHook DLL
add_custom_command(TARGET oneocr_test POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -D CONFIG=$<CONFIG>
        -D ARCH=${CMAKE_SIZEOF_VOID_P}
        -D DIST_DIR=${DIST_DIR}
        "-D FILES=$<TARGET_FILE:oneocr_test>;${MINHOOK_DLL}"
        -D CMAKE_COMMAND=${CMAKE_COMMAND}
        -P "${COPY_SCRIPT}"
    COMMENT "x64 Release: copying oneocr_test.exe + MinHook DLL to bin/"
)
