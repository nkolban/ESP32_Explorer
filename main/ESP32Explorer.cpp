/*
 * ESP32Explorer.cpp
 *
 *  Created on: May 22, 2017
 *      Author: kolban
 */
#include <string>
#include <fstream>
#include <iostream>
#include <GPIO.h>
#include "ESP32Explorer.h"
#include "sdkconfig.h"
#include <esp_log.h>
#include <FATFS_VFS.h>
#include <FileSystem.h>
#include <FreeRTOS.h>
#include <Task.h>
#include <WebServer.h>
#include <TFTP.h>
#include <JSON.h>
#include <WiFi.h>
#include <stdio.h>

#include <GeneralUtils.h>

#include <esp_wifi.h>

static char tag[] = "ESP32Explorer";

extern JsonObject I2S_JSON();
extern JsonObject GPIO_JSON();
extern JsonObject WIFI_JSON();
extern JsonObject SYSTEM_JSON();
extern JsonObject FILESYSTEM_GET_JSON_DIRECTORY(std::string path, bool isRecursive);
extern JsonObject FILESYSTEM_GET_JSON_CONTENT(std::string path);

static void handleTest(WebServer::HTTPRequest *pRequest, WebServer::HTTPResponse *pResponse) {
	ESP_LOGD(tag, "handleTest called");
	ESP_LOGD(tag, "Path: %s" ,pRequest->getPath().c_str());
	ESP_LOGD(tag, "Method: %s", pRequest->getMethod().c_str());
	ESP_LOGD(tag, "Query:");
	std::map<std::string, std::string> queryMap = pRequest->getQuery();
	std::map<std::string, std::string>::iterator itr;
	for (itr=queryMap.begin(); itr != queryMap.end(); itr++) {
		ESP_LOGD(tag, "name: %s, value: %s", itr->first.c_str(), itr->second.c_str());
	}
	ESP_LOGD(tag, "Body: %s", pRequest->getBody().c_str());
	pResponse->sendData("hello!");
} // handleTest


static void handle_REST_SYSTEM(WebServer::HTTPRequest *pRequest, WebServer::HTTPResponse *pResponse) {
	ESP_LOGD(tag, "handle_REST_SYSTEM");
	pResponse->addHeader("Content-Type", "application/json");
	JsonObject obj = SYSTEM_JSON();
	pResponse->sendData(obj.toString());
	JSON::deleteObject(obj);
} // handle_REST_GPIO


static void handle_REST_I2S(WebServer::HTTPRequest *pRequest, WebServer::HTTPResponse *pResponse) {
	ESP_LOGD(tag, "handle_REST_I2S");
	pResponse->addHeader("Content-Type", "application/json");
	JsonObject obj = I2S_JSON();
	pResponse->sendData(obj.toString());
	JSON::deleteObject(obj);
} // handle_REST_I2S


static void handle_REST_GPIO(WebServer::HTTPRequest *pRequest, WebServer::HTTPResponse *pResponse) {
	ESP_LOGD(tag, "handle_REST_GPIO");
	pResponse->addHeader("Content-Type", "application/json");
	JsonObject obj = GPIO_JSON();
	pResponse->sendData(obj.toString());
	JSON::deleteObject(obj);
} // handle_REST_GPIO


static void handle_REST_GPIO_SET(WebServer::HTTPRequest *pRequest, WebServer::HTTPResponse *pResponse) {
	ESP_LOGD(tag, "handle_REST_GPIO_SET");
	std::vector<std::string> parts = pRequest->pathSplit();
	std::stringstream stream(parts[4]);
	int gpio;
	stream >> gpio;
	gpio_set_level((gpio_num_t)gpio, 1);
	JsonObject obj = GPIO_JSON();
	pResponse->sendData(obj.toString());
	JSON::deleteObject(obj);
} // handle_REST_GPIO_SET


static void handle_REST_GPIO_CLEAR(WebServer::HTTPRequest *pRequest, WebServer::HTTPResponse *pResponse) {
	ESP_LOGD(tag, "handle_REST_GPIO_CLEAR");
	std::vector<std::string> parts = pRequest->pathSplit();
	std::stringstream stream(parts[4]);
	int gpio;
	stream >> gpio;
	gpio_set_level((gpio_num_t)gpio, 0);
	JsonObject obj = GPIO_JSON();
	pResponse->sendData(obj.toString());
	JSON::deleteObject(obj);
} // handle_REST_GPIO_CLEAR


static void handle_REST_GPIO_DIRECTION_INPUT(WebServer::HTTPRequest *pRequest, WebServer::HTTPResponse *pResponse) {
	ESP_LOGD(tag, "handle_REST_GPIO_DIRECTION_INPUT");
	std::vector<std::string> parts = pRequest->pathSplit();
	std::stringstream stream(parts[5]);
	int gpio;
	stream >> gpio;
	gpio_set_direction((gpio_num_t)gpio, GPIO_MODE_INPUT);
	JsonObject obj = GPIO_JSON();
	pResponse->sendData(obj.toString());
	JSON::deleteObject(obj);
} // handle_REST_GPIO_DIRECTION_INPUT


static void handle_REST_GPIO_DIRECTION_OUTPUT(WebServer::HTTPRequest *pRequest, WebServer::HTTPResponse *pResponse) {
	ESP_LOGD(tag, "handle_REST_GPIO_DIRECTION_OUTPUT");
	std::vector<std::string> parts = pRequest->pathSplit();
	std::stringstream stream(parts[5]);
	int gpio;
	stream >> gpio;
	gpio_set_direction((gpio_num_t)gpio, GPIO_MODE_OUTPUT);
	JsonObject obj = GPIO_JSON();
	pResponse->sendData(obj.toString());
	JSON::deleteObject(obj);
} // handle_REST_GPIO_DIRECTION_OUTPUT


static void handle_REST_WiFi(WebServer::HTTPRequest *pRequest, WebServer::HTTPResponse *pResponse) {
	ESP_LOGD(tag, "handle_REST_WIFI");
	pResponse->addHeader("Content-Type", "application/json");
	JsonObject obj = WIFI_JSON();
	pResponse->sendData(obj.toString());
	JSON::deleteObject(obj);
} // handle_REST_WiFi


static void handle_REST_LOG_SET(WebServer::HTTPRequest *pRequest, WebServer::HTTPResponse *pResponse) {
	ESP_LOGD(tag, "handle_REST_LOG_SET");
	std::vector<std::string> parts = pRequest->pathSplit();
	std::stringstream stream(parts[4]);
	int logLevel;
	stream >> logLevel;
	::esp_log_level_set("*", (esp_log_level_t)logLevel);
	pResponse->addHeader("Content-Type", "application/json");
	JsonObject obj = SYSTEM_JSON();
	pResponse->sendData(obj.toString());
	JSON::deleteObject(obj);
} // handle_REST_LOG_SET


static void handle_REST_FILE_GET(WebServer::HTTPRequest *pRequest, WebServer::HTTPResponse *pResponse) {
	ESP_LOGD(tag, "handle_REST_FILE_GET");
	std::vector<std::string> parts = pRequest->pathSplit();
	std::string path = "";
	for (int i=3; i<parts.size(); i++) {
		path += "/" + parts[i];
	}
	File file(path);
	JsonObject obj = JSON::createObject();
	if (file.isDirectory()) {
		obj = FILESYSTEM_GET_JSON_DIRECTORY(path.c_str(), true);
	} else {
		ESP_LOGD(tag, "Getting content of file, length: %d", file.length());
		obj = FILESYSTEM_GET_JSON_CONTENT(path.c_str());
	}

	pResponse->sendData(obj.toString());
	JSON::deleteObject(obj);
} // handle_REST_FILE_GET


static void handle_REST_FILE_DELETE(WebServer::HTTPRequest *pRequest, WebServer::HTTPResponse *pResponse) {
	ESP_LOGD(tag, "handle_REST_FILE_DELETE");
	std::vector<std::string> parts = pRequest->pathSplit();
	std::string path = "";
	for (int i=3; i<parts.size(); i++) {
		path += "/" + parts[i];
	}
	int rc = FileSystem::remove(path);
	JsonObject obj = JSON::createObject();
	obj.setInt("rc", rc);
	pResponse->sendData(obj.toString());
	JSON::deleteObject(obj);
} // handle_REST_FILE_GET


static void handle_REST_FILE_POST(WebServer::HTTPRequest *pRequest, WebServer::HTTPResponse *pResponse) {
	ESP_LOGD(tag, "handle_REST_FILE_POST");
	std::vector<std::string> parts = pRequest->pathSplit();
	std::string path = "";
	for (int i=3; i<parts.size(); i++) {
		path += "/" + parts[i];
	}
	std::map<std::string, std::string> queryParams = pRequest->getQuery();
	std::string directory = queryParams["directory"];
	if (directory == "true") {
		ESP_LOGD(tag, "Create a directory: %s", path.c_str());
	}
	//JsonObject obj = FILESYSTEM_GET_JSON(path.c_str());
	int rc = FileSystem::mkdir(path);
	JsonObject obj = JSON::createObject();
	obj.setInt("rc", rc);
	pResponse->sendData(obj.toString());
	JSON::deleteObject(obj);
} // handle_REST_FILE_POST

class MyMultiPart : public WebServer::HTTPMultiPart {
public:
	void begin(std::string varName,	std::string fileName) {
		ESP_LOGD(tag, "MyMultiPart begin(): varName=%s, fileName=%s",
			varName.c_str(), fileName.c_str());
		m_currentVar = varName;
		if (varName == "path") {
			m_path = "";
		} else if (varName == "myfile") {
			m_fileData = "";
			m_fileName = fileName;
		}
	} // begin

	void end() {
		ESP_LOGD(tag, "MyMultiPart end()");
		if (m_currentVar == "myfile") {
			std::string fileName = m_path + "/" + m_fileName;
			ESP_LOGD(tag, "Write to file: %s ... data: %s", fileName.c_str(), m_fileData.c_str());
			/*
			std::ofstream myfile;
			myfile.open(fileName, std::ios::out | std::ios::binary | std::ios::trunc);
			myfile << m_fileData;
			myfile.close();
			*/
			FILE *ffile = fopen(fileName.c_str(), "w");
			fwrite(m_fileData.data(), m_fileData.length(), 1, ffile);
			fclose(ffile);
		}
	} // end

	void data(std::string data) {
		ESP_LOGD(tag, "MyMultiPart data(): length=%d", data.length());
		if (m_currentVar == "path") {
			m_path += data;
		}
		else if (m_currentVar == "myfile") {
			m_fileData += data;
		}
	} // data

	void multipartEnd() {
		ESP_LOGD(tag, "MyMultiPart multipartEnd()");
	} // multipartEnd

	void multipartStart() {
		ESP_LOGD(tag, "MyMultiPart multipartStart()");
	} // multipartStart

private:
	std::string m_fileName;
	std::string m_path;
	std::string m_currentVar;
	std::string m_fileData;
};

class MyMultiPartFactory : public WebServer::HTTPMultiPartFactory {
	WebServer::HTTPMultiPart *newInstance() {
		return new MyMultiPart();
	}
};

class MyWebSocketHandler : public WebServer::WebSocketHandler {
	void onMessage(std::string message) {
		ESP_LOGD(tag, "MyWebSocketHandler: Data length: %s", message.c_str());
		JsonObject obj = JSON::parseObject(message);
		std::string command = obj.getString("command");
		ESP_LOGD(tag, "Command: %s", command.c_str());
		if (command == "getfile") {
			std::string path = obj.getString("path");
			ESP_LOGD(tag, "   path: %s", path.c_str());
			File f(path);
			std::string fileContent = f.getContent(0, 1000);
			ESP_LOGD(tag, "Length of file content is %d", fileContent.length());
			// send response
			sendData(fileContent);
		}
	}
};

class MyWebSocketHandlerFactory : public WebServer::WebSocketHandlerFactory {
	WebServer::WebSocketHandler *newInstance() {
		return new MyWebSocketHandler();
	}
};

class WebServerTask : public Task {
   void run(void *data) {
  	 /*
  	  * Create a WebServer and register handlers for REST requests.
  	  */
  	 WebServer *pWebServer = new WebServer();
  	 pWebServer->setRootPath("/spiflash");
  	 pWebServer->addPathHandler("GET",    "\\/hello\\/.*",                         handleTest);
  	 pWebServer->addPathHandler("GET",    "^\\/ESP32\\/WIFI$",                     handle_REST_WiFi);
  	 pWebServer->addPathHandler("GET",    "^\\/ESP32\\/I2S$",                      handle_REST_I2S);
  	 pWebServer->addPathHandler("GET",    "^\\/ESP32\\/GPIO$",                     handle_REST_GPIO);
  	 pWebServer->addPathHandler("POST",   "^\\/ESP32\\/GPIO\\/SET",                handle_REST_GPIO_SET);
  	 pWebServer->addPathHandler("POST",   "^\\/ESP32\\/GPIO\\/CLEAR",              handle_REST_GPIO_CLEAR);
  	 pWebServer->addPathHandler("POST",   "^\\/ESP32\\/GPIO\\/DIRECTION\\/INPUT",  handle_REST_GPIO_DIRECTION_INPUT);
  	 pWebServer->addPathHandler("POST",   "^\\/ESP32\\/GPIO\\/DIRECTION\\/OUTPUT", handle_REST_GPIO_DIRECTION_OUTPUT);
  	 pWebServer->addPathHandler("POST",   "^\\/ESP32\\/LOG\\/SET",                 handle_REST_LOG_SET);
  	 pWebServer->addPathHandler("GET",    "^\\/ESP32\\/FILE",                      handle_REST_FILE_GET);
  	 pWebServer->addPathHandler("POST",   "^\\/ESP32\\/FILE",                      handle_REST_FILE_POST);
  	 pWebServer->addPathHandler("DELETE", "^\\/ESP32\\/FILE",                      handle_REST_FILE_DELETE);
  	 pWebServer->addPathHandler("GET",    "^\\/ESP32\\/SYSTEM$",                   handle_REST_SYSTEM);
  	 pWebServer->setMultiPartFactory(new MyMultiPartFactory());
  	 pWebServer->setWebSocketHandlerFactory(new MyWebSocketHandlerFactory());
  	 pWebServer->start(80); // Start the WebServer listening on port 80.
   }
};

class TFTPTask : public Task {
   void run(void *data) {
  	 TFTP tftp;
  	 tftp.setBaseDir("/spiflash");
  	 tftp.start();
   }
};

ESP32_Explorer::ESP32_Explorer() {
}

ESP32_Explorer::~ESP32_Explorer() {
}

void ESP32_Explorer::start() {
	FATFS_VFS *fs = new FATFS_VFS("/spiflash", "storage");
	fs->mount();
	//FileSystem::mkdir("/spiflash/mydir2");
	//FileSystem::mkdir("/spiflash/mydir2/fred");
	//FileSystem::dumpDirectory("/spiflash/");

	WebServerTask *webServerTask = new WebServerTask();
	webServerTask->setStackSize(40000);
	webServerTask->start();

	TFTPTask *pTFTPTask = new TFTPTask();
	pTFTPTask->setStackSize(8000);
	pTFTPTask->start();
	ESP32CPP::GPIO::setOutput(GPIO_NUM_25);
	ESP32CPP::GPIO::setOutput(GPIO_NUM_26);
	ESP32CPP::GPIO::high(GPIO_NUM_25);
	ESP32CPP::GPIO::low(GPIO_NUM_26);
} // start
