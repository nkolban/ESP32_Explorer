
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
//#include "bt.h"
//#include <BLEDevice.h>

static const char *WIFI_SSID     = "Orange-8F54";
static const char *WIFI_PASSWORD = "33413006";
extern "C"{
#include "esp_heap_trace.h"
}
#define NUM_RECORDS 100
static heap_trace_record_t trace_record[NUM_RECORDS]; // This buffer must be in internal RAM

extern "C" {
	int app_main(void);
}

//static const char* LOG_TAG = "ESP32_Explorer_MAIN";   // Logging tag


class ESP32_ExplorerTask: public Task {
	void run(void* data) override {
		ESP32_Explorer* pESP32_Explorer = new ESP32_Explorer();
		pESP32_Explorer->start();
	}
}; // ESP32_ExplorerTask


/**
 * @brief Wifi management task.
 */
class WiFiTask: public Task {
	void run(void *data) override {

		WiFi *pWifi = new WiFi();  // Can't delete at the end of the task.
		pWifi->setIPInfo("192.168.1.99", "192.168.1.1", "255.255.255.0");
		pWifi->connectAP(WIFI_SSID, WIFI_PASSWORD);
		ESP32_ExplorerTask *pESP32_ExplorerTask = new ESP32_ExplorerTask();
		pESP32_ExplorerTask->setStackSize(6000);
		pESP32_ExplorerTask->start();
	} // End run
}; // WiFiTask


void task_webserver(void* ignore) {
	WiFiTask* pMyTask = new WiFiTask();
	pMyTask->setStackSize(6000);
	pMyTask->start();
	FreeRTOS::deleteTask();
} // task_webserver



/**
 * @brief Main entry point.
 */
int app_main(void) {
/*	BLEDevice::init("");
   	ESP_LOGD("MAIN", "%d", xPortGetFreeHeapSize());
	esp_bt_controller_mem_release(ESP_BT_MODE_BLE);
   	ESP_LOGD("MAIN", "%d", xPortGetFreeHeapSize());*/
    ESP_ERROR_CHECK( heap_trace_init_standalone(trace_record, NUM_RECORDS) );

	task_webserver(nullptr);
	return 0;
} // app_main
