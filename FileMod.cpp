#include "pch.h"

namespace fs = std::filesystem;

#include "ntdlldef.h"
#include "Util.hpp"

constexpr std::wstring_view PREFIX = LR"(\??\)";
constexpr std::wstring_view DATA_NAME = LR"(data\data)"; // dataXXX.cpk
constexpr std::wstring_view DATA_SUFFIX = L".cpk";

std::wstring match;
std::wstring replace;

decltype(NtCreateFile)* origNtCreateFile = IMP_NtCreateFile;
//decltype(NtOpenFile)* origNtOpenFile = IMP_NtOpenFile;

NTSTATUS NTAPI MyNtCreateFile(PHANDLE FileHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, PIO_STATUS_BLOCK IoStatusBlock, PLARGE_INTEGER AllocationSize, ULONG FileAttributes, ULONG ShareAccess, ULONG CreateDisposition, ULONG CreateOptions, PVOID EaBuffer, ULONG EaLength)
{
	if (ObjectAttributes && ObjectAttributes->Length == sizeof(OBJECT_ATTRIBUTES) && !ObjectAttributes->RootDirectory)
	{
		auto objName = StringViewFromUnicodeString(ObjectAttributes->ObjectName);
		if (objName.starts_with(match))
		{
			objName.remove_prefix(match.size()); // objName is "XXX.cpk"

			if (objName.size() == DATA_SUFFIX.size() + 3 && objName.ends_with(DATA_SUFFIX))
			{
				objName.remove_suffix(DATA_SUFFIX.size()); // objName is "XXX"
				if (objName == L"100" || objName == L"201" || objName == L"202" || objName == L"203" || objName == L"204" || objName == L"205")
				{
					std::wstring newObjName;
					newObjName.reserve(replace.size() + 3 + DATA_SUFFIX.size());
					newObjName.assign(replace);
					newObjName.append(objName);
					newObjName.append(DATA_SUFFIX);

					auto ustr = UnicodeStringFromStringView(newObjName);
					auto oa = *ObjectAttributes;
					oa.ObjectName = &ustr;

					return origNtCreateFile(FileHandle, DesiredAccess, &oa, IoStatusBlock, AllocationSize, FileAttributes, ShareAccess, CreateDisposition, CreateOptions, EaBuffer, EaLength);
				}
			}
		}
	}
	return origNtCreateFile(FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, AllocationSize, FileAttributes, ShareAccess, CreateDisposition, CreateOptions, EaBuffer, EaLength);
}

/*NTSTATUS NTAPI MyNtOpenFile(PHANDLE FileHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, PIO_STATUS_BLOCK IoStatusBlock, ULONG ShareAccess, ULONG OpenOptions)
{
	if (ObjectAttributes && ObjectAttributes->Length == sizeof(OBJECT_ATTRIBUTES) && !ObjectAttributes->RootDirectory)
	{

	}
	return origNtOpenFile(FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, ShareAccess, OpenOptions);
}*/

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD ul_reason_for_call,
	[[maybe_unused]] LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);

		fs::path exePath = GetModuleFsPath(nullptr).remove_filename(); // 
		if (exePath.native().starts_with(PREFIX))
		{
			match = exePath.native();
		}
		else
		{
			match = PREFIX;
			match += exePath.native();
		}
		match += DATA_NAME;

		fs::path selfPath = GetModuleFsPath(hModule).remove_filename();
		if (selfPath.native().starts_with(PREFIX))
		{
			replace = selfPath.native();
		}
		else
		{
			replace = PREFIX;
			replace += selfPath.native();
		}
		replace += DATA_NAME;

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(reinterpret_cast<PVOID*>(&origNtCreateFile), MyNtCreateFile);
		//DetourAttach(reinterpret_cast<PVOID*>(&origNtOpenFile), MyNtOpenFile);
		auto error = DetourTransactionCommit();
		if (error != NO_ERROR)
		{
			wchar_t msg[512];
			swprintf_s(msg, L"DetourTransactionCommit error: %d", error);

			MessageBoxW(0, msg, L"Error", MB_ICONERROR);
			return TRUE;
		}
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetach(reinterpret_cast<PVOID*>(&origNtCreateFile), MyNtCreateFile);
		//DetourDetach(reinterpret_cast<PVOID*>(&origNtOpenFile), MyNtOpenFile);
		DetourTransactionCommit();
	}
	return TRUE;
}
