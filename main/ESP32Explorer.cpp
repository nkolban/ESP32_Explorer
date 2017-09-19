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
#include <HttpServer.h>
#include <TFTP.h>
#include <JSON.h>
#include <WiFi.h>
#include <stdio.h>
#include <driver/i2c.h>

#include <GeneralUtils.h>

#include <esp_wifi.h>

static const char* LOG_TAG = "ESP32Explorer";

extern JsonObject I2S_JSON();
extern JsonObject GPIO_JSON();
extern JsonObject WIFI_JSON();
extern JsonObject SYSTEM_JSON();
extern JsonObject FILESYSTEM_GET_JSON_DIRECTORY(std::string path, bool isRecursive);
extern JsonObject FILESYSTEM_GET_JSON_CONTENT(std::string path);
extern JsonObject I2C_JSON();
extern JsonObject I2C_SCAN_JSON();

static void handleTest(HttpRequest *pRequest, HttpResponse *pResponse) {
	ESP_LOGD(LOG_TAG, "handleTest called");
	ESP_LOGD(LOG_TAG, "Path: %s" ,pRequest->getPath().c_str());
	ESP_LOGD(LOG_TAG, "Method: %s", pRequest->getMethod().c_str());
	ESP_LOGD(LOG_TAG, "Query:");
	std::map<std::string, std::string> queryMap = pRequest->getQuery();
	std::map<std::string, std::string>::iterator itr;
	for (itr=queryMap.begin(); itr != queryMap.end(); itr++) {
		ESP_LOGD(LOG_TAG, "name: %s, value: %s", itr->first.c_str(), itr->second.c_str());
	}
	ESP_LOGD(LOG_TAG, "Body: %s", pRequest->getBody().c_str());
	pResponse->sendData("hello!");
} // handleTest


static void handle_REST_SYSTEM(HttpRequest *pRequest, HttpResponse *pResponse) {
	ESP_LOGD(LOG_TAG, "handle_REST_SYSTEM");
	pResponse->addHeader("access-control-allow-origin", "*");
	pResponse->addHeader("Content-Type", "application/json");
	JsonObject obj = SYSTEM_JSON();
	pResponse->sendData(obj.toString());
	JSON::deleteObject(obj);
} // handle_REST_GPIO


static void handle_REST_I2S(HttpRequest *pRequest, HttpResponse *pResponse) {
	ESP_LOGD(LOG_TAG, "handle_REST_I2S");
	pResponse->addHeader("access-control-allow-origin", "*");
	pResponse->addHeader("Content-Type", "application/json");
	JsonObject obj = I2S_JSON();
	pResponse->sendData(obj.toString());
	JSON::deleteObject(obj);
} // handle_REST_I2S


static void handle_REST_GPIO(HttpRequest *pRequest, HttpResponse *pResponse) {
	ESP_LOGD(LOG_TAG, "handle_REST_GPIO");
	pResponse->addHeader("access-control-allow-origin", "*");
	pResponse->addHeader("Content-Type", "application/json");
	JsonObject obj = GPIO_JSON();
	pResponse->sendData(obj.toString());
	JSON::deleteObject(obj);
} // handle_REST_GPIO


static void handle_REST_GPIO_SET(HttpRequest *pRequest, HttpResponse *pResponse) {
	ESP_LOGD(LOG_TAG, "handle_REST_GPIO_SET");
	pResponse->addHeader("access-control-allow-origin", "*");
	std::vector<std::string> parts = pRequest->pathSplit();
	std::stringstream stream(parts[4]);
	int gpio;
	stream >> gpio;
	gpio_set_level((gpio_num_t)gpio, 1);
	JsonObject obj = GPIO_JSON();
	pResponse->sendData(obj.toString());
	JSON::deleteObject(obj);
} // handle_REST_GPIO_SET


static void handle_REST_GPIO_CLEAR(HttpRequest *pRequest, HttpResponse *pResponse) {
	ESP_LOGD(LOG_TAG, "handle_REST_GPIO_CLEAR");
	pResponse->addHeader("access-control-allow-origin", "*");
	std::vector<std::string> parts = pRequest->pathSplit();
	std::stringstream stream(parts[4]);
	int gpio;
	stream >> gpio;
	gpio_set_level((gpio_num_t)gpio, 0);
	JsonObject obj = GPIO_JSON();
	pResponse->sendData(obj.toString());
	JSON::deleteObject(obj);
} // handle_REST_GPIO_CLEAR


static void handle_REST_GPIO_DIRECTION_INPUT(HttpRequest *pRequest, HttpResponse *pResponse) {
	ESP_LOGD(LOG_TAG, "handle_REST_GPIO_DIRECTION_INPUT");
	pResponse->addHeader("access-control-allow-origin", "*");
	std::vector<std::string> parts = pRequest->pathSplit();
	std::stringstream stream(parts[5]);
	int gpio;
	stream >> gpio;
	gpio_set_direction((gpio_num_t)gpio, GPIO_MODE_INPUT);
	JsonObject obj = GPIO_JSON();
	pResponse->sendData(obj.toString());
	JSON::deleteObject(obj);
} // handle_REST_GPIO_DIRECTION_INPUT


static void handle_REST_GPIO_DIRECTION_OUTPUT(HttpRequest *pRequest, HttpResponse *pResponse) {
	ESP_LOGD(LOG_TAG, "handle_REST_GPIO_DIRECTION_OUTPUT");
	pResponse->addHeader("access-control-allow-origin", "*");
	std::vector<std::string> parts = pRequest->pathSplit();
	std::stringstream stream(parts[5]);
	int gpio;
	stream >> gpio;
	gpio_set_direction((gpio_num_t)gpio, GPIO_MODE_OUTPUT);
	JsonObject obj = GPIO_JSON();
	pResponse->sendData(obj.toString());
	JSON::deleteObject(obj);
} // handle_REST_GPIO_DIRECTION_OUTPUT


static void handle_REST_WiFi(HttpRequest* pRequest, HttpResponse* pResponse) {
	ESP_LOGD(LOG_TAG, "handle_REST_WIFI");
	pResponse->addHeader("access-control-allow-origin", "*");
	pResponse->addHeader("Content-Type", "application/json");
	JsonObject obj = WIFI_JSON();
	pResponse->sendData(obj.toString());
	JSON::deleteObject(obj);
} // handle_REST_WiFi


static void handle_REST_LOG_SET(HttpRequest* pRequest, HttpResponse* pResponse) {
	ESP_LOGD(LOG_TAG, "handle_REST_LOG_SET");
	std::vector<std::string> parts = pRequest->pathSplit();
	std::stringstream stream(parts[4]);
	int logLevel;
	stream >> logLevel;
	::esp_log_level_set("*", (esp_log_level_t)logLevel);
	pResponse->addHeader("access-control-allow-origin", "*");
	pResponse->addHeader("Content-Type", "application/json");
	JsonObject obj = SYSTEM_JSON();
	pResponse->sendData(obj.toString());
	JSON::deleteObject(obj);
} // handle_REST_LOG_SET


static void handle_REST_FILE_GET(HttpRequest *pRequest, HttpResponse *pResponse) {
	ESP_LOGD(LOG_TAG, "handle_REST_FILE_GET");
	pResponse->addHeader("access-control-allow-origin", "*");
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
		ESP_LOGD(LOG_TAG, "Getting content of file, length: %d", file.length());
		obj = FILESYSTEM_GET_JSON_CONTENT(path.c_str());
	}

	pResponse->sendData(obj.toString());
	JSON::deleteObject(obj);
} // handle_REST_FILE_GET


static void handle_REST_FILE_DELETE(HttpRequest *pRequest, HttpResponse *pResponse) {
	ESP_LOGD(LOG_TAG, "handle_REST_FILE_DELETE");
	pResponse->addHeader("access-control-allow-origin", "*");
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


static void handle_REST_FILE_POST(HttpRequest* pRequest, HttpResponse* pResponse) {
	ESP_LOGD(LOG_TAG, "handle_REST_FILE_POST");
	pResponse->addHeader("access-control-allow-origin", "*");
	std::vector<std::string> parts = pRequest->pathSplit();
	std::string path = "";
	for (int i=3; i<parts.size(); i++) {
		path += "/" + parts[i];
	}
	std::map<std::string, std::string> queryParams = pRequest->getQuery();
	std::string directory = queryParams["directory"];
	if (directory == "true") {
		ESP_LOGD(LOG_TAG, "Create a directory: %s", path.c_str());
	}
	//JsonObject obj = FILESYSTEM_GET_JSON(path.c_str());
	int rc = FileSystem::mkdir(path);
	JsonObject obj = JSON::createObject();
	obj.setInt("rc", rc);
	pResponse->sendData(obj.toString());
	JSON::deleteObject(obj);
} // handle_REST_FILE_POST

/*
class MyMultiPart : public WebServer::HTTPMultiPart {
public:
	void begin(std::string varName,	std::string fileName) {
		ESP_LOGD(LOG_TAG, "MyMultiPart begin(): varName=%s, fileName=%s",
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
		ESP_LOGD(LOG_TAG, "MyMultiPart end()");
		if (m_currentVar == "myfile") {
			std::string fileName = m_path + "/" + m_fileName;
			ESP_LOGD(LOG_TAG, "Write to file: %s ... data: %s", fileName.c_str(), m_fileData.c_str());

			//std::ofstream myfile;
			//myfile.open(fileName, std::ios::out | std::ios::binary | std::ios::trunc);
			//myfile << m_fileData;
			//myfile.close();

			FILE *ffile = fopen(fileName.c_str(), "w");
			fwrite(m_fileData.data(), m_fileData.length(), 1, ffile);
			fclose(ffile);
		}
	} // end

	void data(std::string data) {
		ESP_LOGD(LOG_TAG, "MyMultiPart data(): length=%d", data.length());
		if (m_currentVar == "path") {
			m_path += data;
		}
		else if (m_currentVar == "myfile") {
			m_fileData += data;
		}
	} // data

	void multipartEnd() {
		ESP_LOGD(LOG_TAG, "MyMultiPart multipartEnd()");
	} // multipartEnd

	void multipartStart() {
		ESP_LOGD(LOG_TAG, "MyMultiPart multipartStart()");
	} // multipartStart

private:
	std::string m_fileName;
	std::string m_path;
	std::string m_currentVar;
	std::string m_fileData;
};

class MyMultiPartFactory : public WebServer::HTTPMultiPartFactory {
	WebServer::HTTPMultiPart* newInstance() {
		return new MyMultiPart();
	}
};
*/
/*
class MyWebSocketHandler : public WebServer::WebSocketHandler {
	void onMessage(std::string message) {
		ESP_LOGD(LOG_TAG, "MyWebSocketHandler: Data length: %s", message.c_str());
		JsonObject obj = JSON::parseObject(message);
		std::string command = obj.getString("command");
		ESP_LOGD(LOG_TAG, "Command: %s", command.c_str());
		if (command == "getfile") {
			std::string path = obj.getString("path");
			ESP_LOGD(LOG_TAG, "   path: %s", path.c_str());
			File f(path);
			std::string fileContent = f.getContent(0, 1000);
			ESP_LOGD(LOG_TAG, "Length of file content is %d", fileContent.length());
			// send response
			sendData(fileContent);
		}
	} // onMessage
}; // MyWebSocketHandler

class MyWebSocketHandlerFactory : public WebServer::WebSocketHandlerFactory {
	WebServer::WebSocketHandler* newInstance() {
		return new MyWebSocketHandler();
	}
};
*/

static void handle_REST_I2C_INIT(HttpRequest *pRequest, HttpResponse *pResponse) {
	ESP_LOGD(LOG_TAG, ">> init_I2C");
	pResponse->addHeader("access-control-allow-origin", "*");
	pResponse->addHeader("Content-Type", "application/json");
	#define PIN_SDA 22
	#define PIN_CLK 21

	i2c_config_t conf;
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = (gpio_num_t)PIN_SDA;
	conf.scl_io_num = (gpio_num_t)PIN_CLK;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = 400000;
	::i2c_param_config(I2C_NUM_0, &conf);
	::i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
	ESP_LOGI(LOG_TAG, "<< init_i2c");

	JsonObject obj = I2C_JSON();
	pResponse->sendData(obj.toString());
	JSON::deleteObject(obj);
} // handle_REST_I2C

static void handle_REST_I2C_SCAN(HttpRequest *pRequest, HttpResponse *pResponse) {
	ESP_LOGD(LOG_TAG, ">> init_I2C");
	pResponse->addHeader("access-control-allow-origin", "*");
	pResponse->addHeader("Content-Type", "application/json");
	JsonObject obj = I2C_SCAN_JSON();
	pResponse->sendData(obj.toString());
	JSON::deleteObject(obj);
} // handle_REST_I2C


class WebServerTask : public Task {
   void run(void *data) {
  	 /*
  	  * Create a WebServer and register handlers for REST requests.
  	  */
	  	 HttpServer* pWebServer = new HttpServer();
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
	  	 //pWebServer->addPathHandler("GET",    "^\\/ESP32\\/I2C$",                      handle_REST_I2C);
	  	 pWebServer->addPathHandler("POST",    "^\\/ESP32\\/I2C\\/INIT$",              handle_REST_I2C_INIT);
	  	 pWebServer->addPathHandler("GET",    "^\\/ESP32\\/I2C\\/SCAN$",               handle_REST_I2C_SCAN);
	   	 //pWebServer->setMultiPartFactory(new MyMultiPartFactory());
  	 //pWebServer->setWebSocketHandlerFactory(new MyWebSocketHandlerFactory());
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
	FATFS_VFS* fs = new FATFS_VFS("/spiflash", "storage");
	fs->mount();
	//FileSystem::mkdir("/spiflash/mydir2");
	//FileSystem::mkdir("/spiflash/mydir2/fred");
	//FileSystem::dumpDirectory("/spiflash/");

	WebServerTask* webServerTask = new WebServerTask();
	webServerTask->setStackSize(40000);
	webServerTask->start();

	TFTPTask* pTFTPTask = new TFTPTask();
	pTFTPTask->setStackSize(8000);
	//pTFTPTask->start();
	ESP32CPP::GPIO::setOutput(GPIO_NUM_25);
	ESP32CPP::GPIO::setOutput(GPIO_NUM_26);
	ESP32CPP::GPIO::high(GPIO_NUM_25);
	ESP32CPP::GPIO::low(GPIO_NUM_26);
} // start
