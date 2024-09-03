#pragma once

#include <string>
#include <boost/asio.hpp>

class Networking {
private:
	boost::asio::ip::tcp::socket* serverSocket;
	boost::asio::io_service *ios;
	Networking();
public:
	static Networking& getInstance();
	static void runPinger(boost::asio::io_service* ios, boost::asio::ip::tcp::socket* serverSocket);
	static void runReceiver(boost::asio::ip::tcp::socket* serverSocket);
	static void sendToServer(std::string message);
};

