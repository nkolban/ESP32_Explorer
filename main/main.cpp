
#include "sdkconfig.h"
#include <esp_log.h>
#include <FATFS_VFS.h>
#include <FileSystem.h>
#include <FreeRTOS.h>
#include <Task.h>
#include <WebServer.h>
#include <WiFiEventHandler.h>
#include <WiFi.h>
#include <stdio.h>
#include <string>
#include "ESP32Explorer.h"

static const char *WIFI_SSID     = "sweetie";
static const char *WIFI_PASSWORD = "l16wint!";

static const char* LOG_TAG = "ESP32_Explorer_MAIN";   // Logging tag

class ESP32_ExplorerTask: public Task {
	void run(void* data) override {
		ESP32_Explorer* pESP32_Explorer = new ESP32_Explorer();
		pESP32_Explorer->start();
	}
}; // ESP32_ExplorerTask


/**
 * @brief Event handler task for the ESP32_Explorer WiFi subsystem.
 */
class ESP32_ExplorerWiFiEventHandler: public WiFiEventHandler {
	// When we get an IP, start the ESP32_Explorer task.
	esp_err_t staGotIp(system_event_sta_got_ip_t event_sta_got_ip) {
		ESP_LOGD(LOG_TAG, "MyWiFiEventHandler(Class): staGotIp");
		ESP32_ExplorerTask *pESP32_ExplorerTask = new ESP32_ExplorerTask();
		pESP32_ExplorerTask->setStackSize(8000);
		pESP32_ExplorerTask->start();
		return ESP_OK;
	}
}; // TargetWiFiEventHandler


/**
 * @brief Wifi management task.
 */
class WiFiTask: public Task {
	void run(void *data) override {

		WiFi *pWifi = new WiFi();  // Can't delete at the end of the task.
		ESP32_ExplorerWiFiEventHandler* eventHandler = new ESP32_ExplorerWiFiEventHandler();
		pWifi->setWifiEventHandler(eventHandler);
		pWifi->setIPInfo("192.168.1.99", "192.168.1.1", "255.255.255.0");
		pWifi->connectAP(WIFI_SSID, WIFI_PASSWORD);
	} // End run
}; // WiFiTask


void task_webserver(void* ignore) {
	WiFiTask* pMyTask = new WiFiTask();
	pMyTask->setStackSize(8000);
	pMyTask->start();
	FreeRTOS::deleteTask();
} // task_webserver

extern "C" {
	int app_main(void);
}

/**
 * @brief Main entry point.
 */
int app_main(void) {
	task_webserver(nullptr);
	return 0;
} // app_main
