#pragma once
#include <cstddef>

// Called from det_BCryptDecrypt when the real decryption fires (cbIV > 0,
// pbOutput != nullptr, *pcbResult > threshold).
// Saves the plaintext to onnx_dump/<seq>_<name>.  If a valid .onnx basename
// is found in the data it is used; otherwise a generic _decrypt.bin name is
// assigned and may be renamed later by OnnxDump_RenameLastIfMatch.
void OnnxDump_TrySave(const unsigned char *pbPlaintext, unsigned long cbPlaintext);

// Called from det_BCryptEncrypt when the real encryption fires (cbIV > 0,
// small buffer <= 512 bytes).  Extracts the .onnx basename from the path
// plaintext and renames the most recent generically-named _decrypt.bin
// in onnx_dump/ to use this basename instead.
void OnnxDump_RenameLastIfMatch(const unsigned char *pbPlaintext, unsigned long cbPlaintext);
