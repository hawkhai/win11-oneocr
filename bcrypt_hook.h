#pragma once

// Initialize MinHook and install all BCrypt hooks.
// Call this once after oneocr.dll has been loaded into the process.
// Log output is written to bcrypt_dump_<pid>.log in the current directory.
// Returns true on success.
bool BcryptHook_Install();

// Disable and remove all BCrypt hooks, uninitialize MinHook.
// Call this before unloading oneocr.dll.
void BcryptHook_Uninstall();
