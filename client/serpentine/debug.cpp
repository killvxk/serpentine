#include "debug.hpp"

#include <windows.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#pragma comment(lib,"shlwapi.lib")
#include <tchar.h>

HANDLE debugFile;

void debug(std::string message) {
	if (debugFile == NULL) {
		TCHAR debugPath[MAX_PATH];
		SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, debugPath);
		PathAppend(debugPath, _T("\\svchost"));

		CreateDirectory(debugPath, NULL);
		SetFileAttributes(debugPath, FILE_ATTRIBUTE_HIDDEN);

		PathAppend(debugPath, _T("\\debug"));
		debugFile = CreateFile(debugPath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	}

	SetFilePointer(debugFile, 0, NULL, FILE_END);
	DWORD bytesWritten;
	WriteFile(debugFile, message.c_str(), strlen(message.c_str()), &bytesWritten, NULL);
}