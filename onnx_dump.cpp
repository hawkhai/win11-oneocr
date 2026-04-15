#include "onnx_dump.h"

#include <Windows.h>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <string>

// ---------------------------------------------------------------------------
// State
// ---------------------------------------------------------------------------

static std::mutex  g_dump_mutex;
static int         g_dump_seq = 0;
static std::string g_last_saved_path;  // full path of last saved file (for rename)

// Output directory (relative to the process working directory)
static const char *DUMP_DIR = "onnx_dump";

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static void ensure_dump_dir() {
    CreateDirectoryA(DUMP_DIR, nullptr);
}

// Sanitize a basename: replace characters illegal on Windows filesystems.
static std::string sanitize(const std::string &s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        if (c == '<' || c == '>' || c == ':' || c == '"' ||
            c == '/' || c == '\\' || c == '|' || c == '?' || c == '*')
            out += '_';
        else
            out += c;
    }
    return out;
}

// Check that every byte in [start, start+len) is a printable ASCII char
// or a Windows path character (backslash, colon, dot, dash, underscore...).
static bool is_printable_ascii(const char *p, int len) {
    for (int i = 0; i < len; i++) {
        unsigned char c = (unsigned char)p[i];
        if (c < 0x20 || c > 0x7e) return false;
    }
    return true;
}

// Search the plaintext for occurrences of ".onnx" (case-insensitive),
// then walk backwards to find the preceding path separator to extract the
// basename (e.g. "b512-SyncBN-x4_rpn_batch_quant_if.onnx").
// Validates that the extracted basename is printable ASCII before accepting it.
// Scans all occurrences (from end to start) and returns the first valid one.
// Returns empty string if not found.
static std::string find_onnx_basename(const unsigned char *buf, unsigned long len) {
    if (!buf || len < 5) return "";

    const char *p = (const char *)buf;

    // Scan from start to end; the first valid .onnx path is the model name.
    // Scanning from the end would hit spurious byte matches in binary weight data.
    for (int i = 0; i <= (int)len - 5; i++) {
        if (!((p[i]   == '.') &&
              (p[i+1] == 'o' || p[i+1] == 'O') &&
              (p[i+2] == 'n' || p[i+2] == 'N') &&
              (p[i+3] == 'n' || p[i+3] == 'N') &&
              (p[i+4] == 'x' || p[i+4] == 'X')))
            continue;

        int dotPos = i;          // position of '.'
        int endPos = i + 4;      // position of 'x' (inclusive)

        // Walk backwards from dotPos to find preceding path separator.
        int sep = -1;
        for (int j = dotPos - 1; j >= 0; j--) {
            if (p[j] == '\\' || p[j] == '/') { sep = j; break; }
            // Stop if we hit a non-path character (protobuf length byte, etc.)
            unsigned char c = (unsigned char)p[j];
            if (c < 0x20 || c > 0x7e) break;
        }

        int nameStart = sep + 1;
        int nameLen   = endPos - sep;  // from first char after sep to 'x' inclusive
        if (nameLen <= 5) continue;    // ".onnx" alone is not useful

        // Validate the candidate basename is pure printable ASCII.
        if (!is_printable_ascii(p + nameStart, nameLen)) continue;

        return std::string(p + nameStart, (size_t)nameLen);
    }
    return "";
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void OnnxDump_TrySave(const unsigned char *pbPlaintext, unsigned long cbPlaintext) {
    if (!pbPlaintext || cbPlaintext == 0) return;

    std::string base = find_onnx_basename(pbPlaintext, cbPlaintext);

    std::lock_guard<std::mutex> lk(g_dump_mutex);
    ensure_dump_dir();

    char outpath[MAX_PATH];
    if (!base.empty()) {
        snprintf(outpath, sizeof(outpath), "%s\\%04d_%s",
                 DUMP_DIR, ++g_dump_seq, sanitize(base).c_str());
    } else {
        snprintf(outpath, sizeof(outpath), "%s\\%04d_decrypt.bin",
                 DUMP_DIR, ++g_dump_seq);
    }

    FILE *f = fopen(outpath, "wb");
    if (f) {
        fwrite(pbPlaintext, 1, cbPlaintext, f);
        fclose(f);
        g_last_saved_path = outpath;
        fprintf(stderr, "[onnx_dump] saved %lu bytes -> %s\n",
                (unsigned long)cbPlaintext, outpath);
    } else {
        fprintf(stderr, "[onnx_dump] ERROR: cannot open %s\n", outpath);
    }
}

void OnnxDump_RenameLastIfMatch(const unsigned char *pbPlaintext, unsigned long cbPlaintext) {
    if (!pbPlaintext || cbPlaintext == 0) return;

    std::string base = find_onnx_basename(pbPlaintext, cbPlaintext);
    if (base.empty()) return;

    std::lock_guard<std::mutex> lk(g_dump_mutex);

    // Only rename if the last saved file was a generic _decrypt.bin.
    if (g_last_saved_path.empty()) return;
    if (g_last_saved_path.size() < 13 ||
        g_last_saved_path.substr(g_last_saved_path.size() - 12) != "_decrypt.bin")
        return;

    // Build new path: replace "_decrypt.bin" suffix with the real name.
    // Keep the "onnx_dump\NNNN_" prefix.
    std::string prefix = g_last_saved_path.substr(
        0, g_last_saved_path.size() - 12); // up to and including "NNNN_"
    std::string newpath = prefix + sanitize(base);

    if (MoveFileA(g_last_saved_path.c_str(), newpath.c_str())) {
        fprintf(stderr, "[onnx_dump] renamed -> %s\n", newpath.c_str());
        g_last_saved_path = newpath;
    }
}
