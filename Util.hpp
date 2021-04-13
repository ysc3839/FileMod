#pragma once

// https://docs.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/author-coclasses#add-helper-types-and-functions
// License: see the https://github.com/MicrosoftDocs/windows-uwp/blob/docs/LICENSE-CODE file
auto GetModuleFsPath(HMODULE hModule)
{
	std::wstring path(MAX_PATH, L'\0');
	DWORD actualSize;

	while (1)
	{
		actualSize = GetModuleFileNameW(hModule, path.data(), static_cast<DWORD>(path.size()));

		if (static_cast<size_t>(actualSize) + 1 > path.size())
			path.resize(path.size() * 2);
		else
			break;
	}

	path.resize(actualSize);
	return fs::path(std::move(path));
}

std::wstring_view StringViewFromUnicodeString(PUNICODE_STRING string)
{
	return { string->Buffer, string->Length / sizeof(wchar_t) };
}

UNICODE_STRING UnicodeStringFromStringView(std::wstring_view string)
{
	const size_t size = string.size() * sizeof(wchar_t);
	return {
		.Length = static_cast<USHORT>(size),
		.MaximumLength = static_cast<USHORT>(size + sizeof(wchar_t)),
		.Buffer = const_cast<PWSTR>(string.data())
	};
}
