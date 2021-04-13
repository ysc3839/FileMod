#pragma once

// Added "NTSYSCALLAPI" (actually "__declspec(dllimport)") to original NtCreateFile.
// By doing this, we can get real address of NtCreateFile directly, without using GetProcAddress.
// Without "__declspec(dllimport)", the NtCreateFile points to a stub like "jmp qword ptr [__imp_NtCreateFile]".
extern "C" NTSYSCALLAPI decltype(NtCreateFile) IMP_NtCreateFile;
extern "C" NTSYSCALLAPI decltype(NtOpenFile) IMP_NtOpenFile;

// Below is from C++/WinRT
#ifdef _M_IX86
#define IMP_LINK(function, count) __pragma(comment(linker, "/alternatename:__imp__IMP_" #function "@" #count "=__imp__" #function "@" #count))
#else
#define IMP_LINK(function, count) __pragma(comment(linker, "/alternatename:__imp_IMP_" #function "=__imp_" #function))
#endif

IMP_LINK(NtCreateFile, 44)
IMP_LINK(NtOpenFile, 24)

#undef IMP_LINK
