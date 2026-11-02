#ifndef ONEOCR_PORTABLE_H
#define ONEOCR_PORTABLE_H
#include <stddef.h>
#include <stdint.h>
#if defined(_WIN32) && defined(ONEOCR_SHARED)
# ifdef ONEOCR_BUILDING
#  define ONEOCR_API __declspec(dllexport)
# else
#  define ONEOCR_API __declspec(dllimport)
# endif
#else
# define ONEOCR_API
#endif
#ifdef __cplusplus
extern "C" {
#endif
typedef struct oneocr_context oneocr_context;
typedef struct oneocr_options {
    int32_t max_lines; /* 0 uses 1000; valid values 1..1000. */
    int32_t resize_width; /* Both zero uses 1152x768. */
    int32_t resize_height;
} oneocr_options;
enum oneocr_status {
    ONEOCR_OK=0, ONEOCR_INVALID_ARGUMENT=-12, ONEOCR_IMAGE_ERROR=-13,
    ONEOCR_MODEL_ERROR=-30, ONEOCR_RUNTIME_ERROR=-31, ONEOCR_ALLOCATION_ERROR=-19
};
/* All paths are UTF-8. Each context serializes its own inference calls.
 * Error/JSON strings returned by these functions must use oneocr_free().
 * Set output pointers to NULL on entry; failures also leave JSON NULL.
 * A context must not be destroyed while another thread is using it. */
ONEOCR_API int oneocr_create(const char* model_directory, oneocr_context** context, char** error);
ONEOCR_API int oneocr_recognize_file(oneocr_context*, const char* utf8_path, const oneocr_options*, char** json, char** error);
ONEOCR_API int oneocr_recognize_bgra(oneocr_context*, const uint8_t* data, size_t data_size, int32_t width, int32_t height, int64_t stride, const oneocr_options*, char** json, char** error);
ONEOCR_API void oneocr_destroy(oneocr_context*);
ONEOCR_API void oneocr_free(void*);
/* Borrowed static UTF-8 JSON. Do not pass this pointer to oneocr_free(). */
ONEOCR_API const char* oneocr_capabilities(void);
#ifdef __cplusplus
}
#endif
#endif
