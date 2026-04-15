#include "bcrypt_hook.h"
#include "onnx_dump.h"

#include <Windows.h>
#include <bcrypt.h>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <string>
#include <mutex>

#include "MinHook.h"

#pragma comment(lib, "bcrypt.lib")

// ---------------------------------------------------------------------------
// Logging
// ---------------------------------------------------------------------------

static FILE   *g_log      = nullptr;
static std::mutex g_log_mutex;
static long long  g_call_seq = 0;   // monotonic call counter

static void log_open() {
    char path[MAX_PATH];
    DWORD pid = GetCurrentProcessId();
    snprintf(path, sizeof(path), "bcrypt_dump_%lu.log", (unsigned long)pid);
    g_log = fopen(path, "a");
    if (g_log) {
        fprintf(g_log, "\n\n=== BCrypt hook session started (PID %lu) ===\n\n", (unsigned long)pid);
        fflush(g_log);
    }
}

static void log_close() {
    if (g_log) {
        fprintf(g_log, "\n=== BCrypt hook session ended ===\n");
        fflush(g_log);
        fclose(g_log);
        g_log = nullptr;
    }
}

// hex-dump up to max_bytes bytes of buf, indented by indent spaces
static void log_hexdump(const void *buf, size_t len, size_t max_bytes, const char *indent) {
    if (!g_log || !buf || len == 0) return;
    size_t show = (len < max_bytes) ? len : max_bytes;
    const unsigned char *p = (const unsigned char *)buf;
    for (size_t i = 0; i < show; i += 16) {
        fprintf(g_log, "%s%04zx  ", indent, i);
        for (size_t j = 0; j < 16; j++) {
            if (i + j < show) fprintf(g_log, "%02x ", p[i + j]);
            else               fprintf(g_log, "   ");
            if (j == 7)        fprintf(g_log, " ");
        }
        fprintf(g_log, " |");
        for (size_t j = 0; j < 16 && i + j < show; j++) {
            unsigned char c = p[i + j];
            fprintf(g_log, "%c", (c >= 0x20 && c < 0x7f) ? c : '.');
        }
        fprintf(g_log, "|\n");
    }
    if (show < len)
        fprintf(g_log, "%s... (%zu more bytes)\n", indent, len - show);
}

static const char *ntstatus_str(NTSTATUS s) {
    switch ((ULONG)s) {
        case 0x00000000: return "STATUS_SUCCESS";
        case 0xC0000001: return "STATUS_UNSUCCESSFUL";
        case 0xC0000005: return "STATUS_ACCESS_VIOLATION";
        case 0xC000000D: return "STATUS_INVALID_PARAMETER";
        case 0xC0000017: return "STATUS_NO_MEMORY";
        case 0x80090005: return "NTE_BAD_DATA";
        case 0x80090006: return "NTE_BAD_FLAGS";
        case 0x8009000B: return "NTE_BAD_KEY";
        case 0x8009000D: return "NTE_BAD_LEN";
        default: {
            static char buf[32];
            snprintf(buf, sizeof(buf), "0x%08lX", (unsigned long)(ULONG)s);
            return buf;
        }
    }
}

// Log a wide-string property/algorithm name safely
static void log_wstr(const wchar_t *ws) {
    if (!g_log) return;
    if (!ws) { fprintf(g_log, "(null)"); return; }
    char tmp[256] = {};
    WideCharToMultiByte(CP_UTF8, 0, ws, -1, tmp, (int)sizeof(tmp) - 1, nullptr, nullptr);
    fprintf(g_log, "%s", tmp);
}

// ---------------------------------------------------------------------------
// Original function pointers (trampolines filled by MinHook)
// ---------------------------------------------------------------------------

typedef NTSTATUS (WINAPI *Orig_BCryptOpenAlgorithmProvider_t)(
    BCRYPT_ALG_HANDLE *, LPCWSTR, LPCWSTR, ULONG);
typedef NTSTATUS (WINAPI *Orig_BCryptCloseAlgorithmProvider_t)(
    BCRYPT_ALG_HANDLE, ULONG);
typedef NTSTATUS (WINAPI *Orig_BCryptGetProperty_t)(
    BCRYPT_HANDLE, LPCWSTR, PUCHAR, ULONG, ULONG *, ULONG);
typedef NTSTATUS (WINAPI *Orig_BCryptSetProperty_t)(
    BCRYPT_HANDLE, LPCWSTR, PUCHAR, ULONG, ULONG);
typedef NTSTATUS (WINAPI *Orig_BCryptGenerateSymmetricKey_t)(
    BCRYPT_ALG_HANDLE, BCRYPT_KEY_HANDLE *, PUCHAR, ULONG, PUCHAR, ULONG, ULONG);
typedef NTSTATUS (WINAPI *Orig_BCryptDestroyKey_t)(
    BCRYPT_KEY_HANDLE);
typedef NTSTATUS (WINAPI *Orig_BCryptEncrypt_t)(
    BCRYPT_KEY_HANDLE, PUCHAR, ULONG, VOID *, PUCHAR, ULONG, PUCHAR, ULONG, ULONG *, ULONG);
typedef NTSTATUS (WINAPI *Orig_BCryptDecrypt_t)(
    BCRYPT_KEY_HANDLE, PUCHAR, ULONG, VOID *, PUCHAR, ULONG, PUCHAR, ULONG, ULONG *, ULONG);
typedef NTSTATUS (WINAPI *Orig_BCryptCreateHash_t)(
    BCRYPT_ALG_HANDLE, BCRYPT_HASH_HANDLE *, PUCHAR, ULONG, PUCHAR, ULONG, ULONG);
typedef NTSTATUS (WINAPI *Orig_BCryptDestroyHash_t)(
    BCRYPT_HASH_HANDLE);
typedef NTSTATUS (WINAPI *Orig_BCryptHashData_t)(
    BCRYPT_HASH_HANDLE, PUCHAR, ULONG, ULONG);
typedef NTSTATUS (WINAPI *Orig_BCryptFinishHash_t)(
    BCRYPT_HASH_HANDLE, PUCHAR, ULONG, ULONG);

static Orig_BCryptOpenAlgorithmProvider_t   orig_BCryptOpenAlgorithmProvider   = nullptr;
static Orig_BCryptCloseAlgorithmProvider_t  orig_BCryptCloseAlgorithmProvider  = nullptr;
static Orig_BCryptGetProperty_t             orig_BCryptGetProperty             = nullptr;
static Orig_BCryptSetProperty_t             orig_BCryptSetProperty             = nullptr;
static Orig_BCryptGenerateSymmetricKey_t    orig_BCryptGenerateSymmetricKey    = nullptr;
static Orig_BCryptDestroyKey_t              orig_BCryptDestroyKey              = nullptr;
static Orig_BCryptEncrypt_t                 orig_BCryptEncrypt                 = nullptr;
static Orig_BCryptDecrypt_t                 orig_BCryptDecrypt                 = nullptr;
static Orig_BCryptCreateHash_t              orig_BCryptCreateHash              = nullptr;
static Orig_BCryptDestroyHash_t             orig_BCryptDestroyHash             = nullptr;
static Orig_BCryptHashData_t                orig_BCryptHashData                = nullptr;
static Orig_BCryptFinishHash_t              orig_BCryptFinishHash              = nullptr;

// max bytes shown in hex dumps
#define DUMP_MAX 256

// ---------------------------------------------------------------------------
// Detour implementations
// ---------------------------------------------------------------------------

static NTSTATUS WINAPI det_BCryptOpenAlgorithmProvider(
    BCRYPT_ALG_HANDLE *phAlgorithm, LPCWSTR pszAlgId,
    LPCWSTR pszImplementation, ULONG dwFlags)
{
    NTSTATUS st = orig_BCryptOpenAlgorithmProvider(phAlgorithm, pszAlgId, pszImplementation, dwFlags);
    std::lock_guard<std::mutex> lk(g_log_mutex);
    if (g_log) {
        long long seq = ++g_call_seq;
        fprintf(g_log, "[%lld] BCryptOpenAlgorithmProvider\n", seq);
        fprintf(g_log, "  pszAlgId=");       log_wstr(pszAlgId);
        fprintf(g_log, "\n  pszImpl=");      log_wstr(pszImplementation);
        fprintf(g_log, "\n  dwFlags=0x%08lX\n", (unsigned long)dwFlags);
        fprintf(g_log, "  -> hAlgorithm=%p  status=%s\n\n",
                phAlgorithm ? *phAlgorithm : nullptr, ntstatus_str(st));
        fflush(g_log);
    }
    return st;
}

static NTSTATUS WINAPI det_BCryptCloseAlgorithmProvider(
    BCRYPT_ALG_HANDLE hAlgorithm, ULONG dwFlags)
{
    NTSTATUS st = orig_BCryptCloseAlgorithmProvider(hAlgorithm, dwFlags);
    std::lock_guard<std::mutex> lk(g_log_mutex);
    if (g_log) {
        long long seq = ++g_call_seq;
        fprintf(g_log, "[%lld] BCryptCloseAlgorithmProvider\n", seq);
        fprintf(g_log, "  hAlgorithm=%p  dwFlags=0x%08lX\n", hAlgorithm, (unsigned long)dwFlags);
        fprintf(g_log, "  -> status=%s\n\n", ntstatus_str(st));
        fflush(g_log);
    }
    return st;
}

static NTSTATUS WINAPI det_BCryptGetProperty(
    BCRYPT_HANDLE hObject, LPCWSTR pszProperty,
    PUCHAR pbOutput, ULONG cbOutput, ULONG *pcbResult, ULONG dwFlags)
{
    NTSTATUS st = orig_BCryptGetProperty(hObject, pszProperty, pbOutput, cbOutput, pcbResult, dwFlags);
    std::lock_guard<std::mutex> lk(g_log_mutex);
    if (g_log) {
        long long seq = ++g_call_seq;
        fprintf(g_log, "[%lld] BCryptGetProperty\n", seq);
        fprintf(g_log, "  hObject=%p  property=", hObject); log_wstr(pszProperty);
        fprintf(g_log, "\n  cbOutput=%lu  dwFlags=0x%08lX\n", (unsigned long)cbOutput, (unsigned long)dwFlags);
        fprintf(g_log, "  -> status=%s  pcbResult=%lu\n", ntstatus_str(st),
                pcbResult ? (unsigned long)*pcbResult : 0UL);
        if (BCRYPT_SUCCESS(st) && pbOutput && pcbResult && *pcbResult > 0)
            log_hexdump(pbOutput, *pcbResult, DUMP_MAX, "    ");
        fprintf(g_log, "\n");
        fflush(g_log);
    }
    return st;
}

static NTSTATUS WINAPI det_BCryptSetProperty(
    BCRYPT_HANDLE hObject, LPCWSTR pszProperty,
    PUCHAR pbInput, ULONG cbInput, ULONG dwFlags)
{
    // snapshot input before calling (buffer may change after)
    unsigned char snap[DUMP_MAX];
    ULONG snapLen = (pbInput && cbInput > 0) ? (cbInput < DUMP_MAX ? cbInput : DUMP_MAX) : 0;
    if (snapLen) memcpy(snap, pbInput, snapLen);

    NTSTATUS st = orig_BCryptSetProperty(hObject, pszProperty, pbInput, cbInput, dwFlags);
    std::lock_guard<std::mutex> lk(g_log_mutex);
    if (g_log) {
        long long seq = ++g_call_seq;
        fprintf(g_log, "[%lld] BCryptSetProperty\n", seq);
        fprintf(g_log, "  hObject=%p  property=", hObject); log_wstr(pszProperty);
        fprintf(g_log, "\n  cbInput=%lu  dwFlags=0x%08lX\n", (unsigned long)cbInput, (unsigned long)dwFlags);
        if (snapLen) log_hexdump(snap, snapLen, DUMP_MAX, "    ");
        fprintf(g_log, "  -> status=%s\n\n", ntstatus_str(st));
        fflush(g_log);
    }
    return st;
}

static NTSTATUS WINAPI det_BCryptGenerateSymmetricKey(
    BCRYPT_ALG_HANDLE hAlgorithm, BCRYPT_KEY_HANDLE *phKey,
    PUCHAR pbKeyObject, ULONG cbKeyObject,
    PUCHAR pbSecret, ULONG cbSecret, ULONG dwFlags)
{
    // snapshot key material before calling
    unsigned char snap[DUMP_MAX];
    ULONG snapLen = (pbSecret && cbSecret > 0) ? (cbSecret < DUMP_MAX ? cbSecret : DUMP_MAX) : 0;
    if (snapLen) memcpy(snap, pbSecret, snapLen);

    NTSTATUS st = orig_BCryptGenerateSymmetricKey(hAlgorithm, phKey, pbKeyObject, cbKeyObject,
                                                  pbSecret, cbSecret, dwFlags);
    std::lock_guard<std::mutex> lk(g_log_mutex);
    if (g_log) {
        long long seq = ++g_call_seq;
        fprintf(g_log, "[%lld] BCryptGenerateSymmetricKey\n", seq);
        fprintf(g_log, "  hAlgorithm=%p  cbKeyObject=%lu  cbSecret=%lu  dwFlags=0x%08lX\n",
                hAlgorithm, (unsigned long)cbKeyObject, (unsigned long)cbSecret, (unsigned long)dwFlags);
        fprintf(g_log, "  KEY MATERIAL (%lu bytes):\n", (unsigned long)cbSecret);
        if (snapLen) log_hexdump(snap, snapLen, DUMP_MAX, "    ");
        fprintf(g_log, "  -> hKey=%p  status=%s\n\n",
                phKey ? *phKey : nullptr, ntstatus_str(st));
        fflush(g_log);
    }
    return st;
}

static NTSTATUS WINAPI det_BCryptDestroyKey(BCRYPT_KEY_HANDLE hKey)
{
    NTSTATUS st = orig_BCryptDestroyKey(hKey);
    std::lock_guard<std::mutex> lk(g_log_mutex);
    if (g_log) {
        long long seq = ++g_call_seq;
        fprintf(g_log, "[%lld] BCryptDestroyKey  hKey=%p\n", seq, hKey);
        fprintf(g_log, "  -> status=%s\n\n", ntstatus_str(st));
        fflush(g_log);
    }
    return st;
}

static NTSTATUS WINAPI det_BCryptEncrypt(
    BCRYPT_KEY_HANDLE hKey, PUCHAR pbInput, ULONG cbInput,
    VOID *pPaddingInfo, PUCHAR pbIV, ULONG cbIV,
    PUCHAR pbOutput, ULONG cbOutput, ULONG *pcbResult, ULONG dwFlags)
{
    // snapshot plaintext and IV before calling (IV is updated in-place by CFB)
    unsigned char snapPT[DUMP_MAX], snapIV[DUMP_MAX];
    ULONG snapPTLen = (pbInput && cbInput > 0) ? (cbInput < DUMP_MAX ? cbInput : DUMP_MAX) : 0;
    ULONG snapIVLen = (pbIV    && cbIV   > 0) ? (cbIV   < DUMP_MAX ? cbIV   : DUMP_MAX) : 0;
    if (snapPTLen) memcpy(snapPT, pbInput, snapPTLen);
    if (snapIVLen) memcpy(snapIV, pbIV,    snapIVLen);

    NTSTATUS st = orig_BCryptEncrypt(hKey, pbInput, cbInput, pPaddingInfo,
                                     pbIV, cbIV, pbOutput, cbOutput, pcbResult, dwFlags);

    // Real encryption pass with path plaintext: rename last saved _decrypt.bin.
    if (BCRYPT_SUCCESS(st) && cbIV > 0 && snapPTLen > 0 && cbInput <= 512)
        OnnxDump_RenameLastIfMatch(snapPT, snapPTLen);

    std::lock_guard<std::mutex> lk(g_log_mutex);
    if (g_log) {
        long long seq = ++g_call_seq;
        fprintf(g_log, "[%lld] BCryptEncrypt\n", seq);
        fprintf(g_log, "  hKey=%p  cbInput=%lu  cbIV=%lu  cbOutput=%lu  dwFlags=0x%08lX\n",
                hKey, (unsigned long)cbInput, (unsigned long)cbIV,
                (unsigned long)cbOutput, (unsigned long)dwFlags);
        fprintf(g_log, "  PLAINTEXT (%lu bytes):\n", (unsigned long)cbInput);
        if (snapPTLen) log_hexdump(snapPT, snapPTLen, DUMP_MAX, "    ");
        fprintf(g_log, "  IV (%lu bytes):\n", (unsigned long)cbIV);
        if (snapIVLen) log_hexdump(snapIV, snapIVLen, DUMP_MAX, "    ");
        fprintf(g_log, "  -> status=%s  pcbResult=%lu\n", ntstatus_str(st),
                pcbResult ? (unsigned long)*pcbResult : 0UL);
        fprintf(g_log, "  CIPHERTEXT (%lu bytes):\n",
                pcbResult ? (unsigned long)*pcbResult : 0UL);
        if (BCRYPT_SUCCESS(st) && pbOutput && pcbResult && *pcbResult > 0)
            log_hexdump(pbOutput, *pcbResult, DUMP_MAX, "    ");
        fprintf(g_log, "\n");
        fflush(g_log);
    }
    return st;
}

static NTSTATUS WINAPI det_BCryptDecrypt(
    BCRYPT_KEY_HANDLE hKey, PUCHAR pbInput, ULONG cbInput,
    VOID *pPaddingInfo, PUCHAR pbIV, ULONG cbIV,
    PUCHAR pbOutput, ULONG cbOutput, ULONG *pcbResult, ULONG dwFlags)
{
    // snapshot ciphertext and IV before calling
    unsigned char snapCT[DUMP_MAX], snapIV[DUMP_MAX];
    ULONG snapCTLen = (pbInput && cbInput > 0) ? (cbInput < DUMP_MAX ? cbInput : DUMP_MAX) : 0;
    ULONG snapIVLen = (pbIV    && cbIV   > 0) ? (cbIV   < DUMP_MAX ? cbIV   : DUMP_MAX) : 0;
    if (snapCTLen) memcpy(snapCT, pbInput, snapCTLen);
    if (snapIVLen) memcpy(snapIV, pbIV,    snapIVLen);

    NTSTATUS st = orig_BCryptDecrypt(hKey, pbInput, cbInput, pPaddingInfo,
                                     pbIV, cbIV, pbOutput, cbOutput, pcbResult, dwFlags);

    // Real decryption pass (cbIV > 0, large block): scan plaintext for .onnx name and save.
    if (BCRYPT_SUCCESS(st) && cbIV > 0 && pbOutput && pcbResult && *pcbResult > 512)
        OnnxDump_TrySave(pbOutput, *pcbResult);

    std::lock_guard<std::mutex> lk(g_log_mutex);
    if (g_log) {
        long long seq = ++g_call_seq;
        fprintf(g_log, "[%lld] BCryptDecrypt\n", seq);
        fprintf(g_log, "  hKey=%p  cbInput=%lu  cbIV=%lu  cbOutput=%lu  dwFlags=0x%08lX\n",
                hKey, (unsigned long)cbInput, (unsigned long)cbIV,
                (unsigned long)cbOutput, (unsigned long)dwFlags);
        fprintf(g_log, "  CIPHERTEXT (%lu bytes):\n", (unsigned long)cbInput);
        if (snapCTLen) log_hexdump(snapCT, snapCTLen, DUMP_MAX, "    ");
        fprintf(g_log, "  IV (%lu bytes):\n", (unsigned long)cbIV);
        if (snapIVLen) log_hexdump(snapIV, snapIVLen, DUMP_MAX, "    ");
        fprintf(g_log, "  -> status=%s  pcbResult=%lu\n", ntstatus_str(st),
                pcbResult ? (unsigned long)*pcbResult : 0UL);
        fprintf(g_log, "  PLAINTEXT (%lu bytes):\n",
                pcbResult ? (unsigned long)*pcbResult : 0UL);
        if (BCRYPT_SUCCESS(st) && pbOutput && pcbResult && *pcbResult > 0)
            log_hexdump(pbOutput, *pcbResult, DUMP_MAX, "    ");
        fprintf(g_log, "\n");
        fflush(g_log);
    }
    return st;
}

static NTSTATUS WINAPI det_BCryptCreateHash(
    BCRYPT_ALG_HANDLE hAlgorithm, BCRYPT_HASH_HANDLE *phHash,
    PUCHAR pbHashObject, ULONG cbHashObject,
    PUCHAR pbSecret, ULONG cbSecret, ULONG dwFlags)
{
    // snapshot HMAC secret before calling
    unsigned char snap[DUMP_MAX];
    ULONG snapLen = (pbSecret && cbSecret > 0) ? (cbSecret < DUMP_MAX ? cbSecret : DUMP_MAX) : 0;
    if (snapLen) memcpy(snap, pbSecret, snapLen);

    NTSTATUS st = orig_BCryptCreateHash(hAlgorithm, phHash, pbHashObject, cbHashObject,
                                        pbSecret, cbSecret, dwFlags);
    std::lock_guard<std::mutex> lk(g_log_mutex);
    if (g_log) {
        long long seq = ++g_call_seq;
        fprintf(g_log, "[%lld] BCryptCreateHash\n", seq);
        fprintf(g_log, "  hAlgorithm=%p  cbHashObject=%lu  cbSecret=%lu  dwFlags=0x%08lX\n",
                hAlgorithm, (unsigned long)cbHashObject, (unsigned long)cbSecret, (unsigned long)dwFlags);
        if (snapLen) {
            fprintf(g_log, "  HMAC SECRET (%lu bytes):\n", (unsigned long)cbSecret);
            log_hexdump(snap, snapLen, DUMP_MAX, "    ");
        }
        fprintf(g_log, "  -> hHash=%p  status=%s\n\n",
                phHash ? *phHash : nullptr, ntstatus_str(st));
        fflush(g_log);
    }
    return st;
}

static NTSTATUS WINAPI det_BCryptDestroyHash(BCRYPT_HASH_HANDLE hHash)
{
    NTSTATUS st = orig_BCryptDestroyHash(hHash);
    std::lock_guard<std::mutex> lk(g_log_mutex);
    if (g_log) {
        long long seq = ++g_call_seq;
        fprintf(g_log, "[%lld] BCryptDestroyHash  hHash=%p\n", seq, hHash);
        fprintf(g_log, "  -> status=%s\n\n", ntstatus_str(st));
        fflush(g_log);
    }
    return st;
}

static NTSTATUS WINAPI det_BCryptHashData(
    BCRYPT_HASH_HANDLE hHash, PUCHAR pbInput, ULONG cbInput, ULONG dwFlags)
{
    // snapshot data before calling
    unsigned char snap[DUMP_MAX];
    ULONG snapLen = (pbInput && cbInput > 0) ? (cbInput < DUMP_MAX ? cbInput : DUMP_MAX) : 0;
    if (snapLen) memcpy(snap, pbInput, snapLen);

    NTSTATUS st = orig_BCryptHashData(hHash, pbInput, cbInput, dwFlags);
    std::lock_guard<std::mutex> lk(g_log_mutex);
    if (g_log) {
        long long seq = ++g_call_seq;
        fprintf(g_log, "[%lld] BCryptHashData\n", seq);
        fprintf(g_log, "  hHash=%p  cbInput=%lu  dwFlags=0x%08lX\n",
                hHash, (unsigned long)cbInput, (unsigned long)dwFlags);
        fprintf(g_log, "  DATA (%lu bytes):\n", (unsigned long)cbInput);
        if (snapLen) log_hexdump(snap, snapLen, DUMP_MAX, "    ");
        fprintf(g_log, "  -> status=%s\n\n", ntstatus_str(st));
        fflush(g_log);
    }
    return st;
}

static NTSTATUS WINAPI det_BCryptFinishHash(
    BCRYPT_HASH_HANDLE hHash, PUCHAR pbOutput, ULONG cbOutput, ULONG dwFlags)
{
    NTSTATUS st = orig_BCryptFinishHash(hHash, pbOutput, cbOutput, dwFlags);
    std::lock_guard<std::mutex> lk(g_log_mutex);
    if (g_log) {
        long long seq = ++g_call_seq;
        fprintf(g_log, "[%lld] BCryptFinishHash\n", seq);
        fprintf(g_log, "  hHash=%p  cbOutput=%lu  dwFlags=0x%08lX\n",
                hHash, (unsigned long)cbOutput, (unsigned long)dwFlags);
        fprintf(g_log, "  -> status=%s\n", ntstatus_str(st));
        fprintf(g_log, "  DIGEST (%lu bytes):\n", (unsigned long)cbOutput);
        if (BCRYPT_SUCCESS(st) && pbOutput && cbOutput > 0)
            log_hexdump(pbOutput, cbOutput, DUMP_MAX, "    ");
        fprintf(g_log, "\n");
        fflush(g_log);
    }
    return st;
}

// ---------------------------------------------------------------------------
// Hook table
// ---------------------------------------------------------------------------

struct HookEntry {
    LPCWSTR      module;
    LPCSTR       func;
    LPVOID       detour;
    LPVOID      *original;
};

static HookEntry g_hooks[] = {
    { L"bcrypt.dll", "BCryptOpenAlgorithmProvider",  (LPVOID)det_BCryptOpenAlgorithmProvider,  (LPVOID *)&orig_BCryptOpenAlgorithmProvider  },
    { L"bcrypt.dll", "BCryptCloseAlgorithmProvider", (LPVOID)det_BCryptCloseAlgorithmProvider, (LPVOID *)&orig_BCryptCloseAlgorithmProvider },
    { L"bcrypt.dll", "BCryptGetProperty",            (LPVOID)det_BCryptGetProperty,            (LPVOID *)&orig_BCryptGetProperty            },
    { L"bcrypt.dll", "BCryptSetProperty",            (LPVOID)det_BCryptSetProperty,            (LPVOID *)&orig_BCryptSetProperty            },
    { L"bcrypt.dll", "BCryptGenerateSymmetricKey",   (LPVOID)det_BCryptGenerateSymmetricKey,   (LPVOID *)&orig_BCryptGenerateSymmetricKey   },
    { L"bcrypt.dll", "BCryptDestroyKey",             (LPVOID)det_BCryptDestroyKey,             (LPVOID *)&orig_BCryptDestroyKey             },
    { L"bcrypt.dll", "BCryptEncrypt",                (LPVOID)det_BCryptEncrypt,                (LPVOID *)&orig_BCryptEncrypt                },
    { L"bcrypt.dll", "BCryptDecrypt",                (LPVOID)det_BCryptDecrypt,                (LPVOID *)&orig_BCryptDecrypt                },
    { L"bcrypt.dll", "BCryptCreateHash",             (LPVOID)det_BCryptCreateHash,             (LPVOID *)&orig_BCryptCreateHash             },
    { L"bcrypt.dll", "BCryptDestroyHash",            (LPVOID)det_BCryptDestroyHash,            (LPVOID *)&orig_BCryptDestroyHash            },
    { L"bcrypt.dll", "BCryptHashData",               (LPVOID)det_BCryptHashData,               (LPVOID *)&orig_BCryptHashData               },
    { L"bcrypt.dll", "BCryptFinishHash",             (LPVOID)det_BCryptFinishHash,             (LPVOID *)&orig_BCryptFinishHash             },
};
static const int g_hook_count = (int)(sizeof(g_hooks) / sizeof(g_hooks[0]));

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

bool BcryptHook_Install() {
    log_open();

    MH_STATUS st = MH_Initialize();
    if (st != MH_OK && st != MH_ERROR_ALREADY_INITIALIZED) {
        if (g_log) fprintf(g_log, "MH_Initialize failed: %s\n", MH_StatusToString(st));
        return false;
    }

    int installed = 0;
    for (int i = 0; i < g_hook_count; i++) {
        MH_STATUS hs = MH_CreateHookApi(
            g_hooks[i].module, g_hooks[i].func,
            g_hooks[i].detour, g_hooks[i].original);
        if (hs == MH_OK) {
            MH_EnableHook(*g_hooks[i].original == nullptr
                          ? MH_ALL_HOOKS
                          : nullptr);
            installed++;
        } else {
            if (g_log)
                fprintf(g_log, "MH_CreateHookApi(%s) failed: %s\n",
                        g_hooks[i].func, MH_StatusToString(hs));
        }
    }

    // Enable all hooks in one shot
    MH_EnableHook(MH_ALL_HOOKS);

    if (g_log) {
        fprintf(g_log, "BCrypt hooks installed: %d / %d\n\n", installed, g_hook_count);
        fflush(g_log);
    }
    return (installed > 0);
}

void BcryptHook_Uninstall() {
    MH_DisableHook(MH_ALL_HOOKS);
    for (int i = 0; i < g_hook_count; i++) {
        if (g_hooks[i].original && *g_hooks[i].original)
            MH_RemoveHook(*g_hooks[i].original);
    }
    MH_Uninitialize();
    log_close();
}
