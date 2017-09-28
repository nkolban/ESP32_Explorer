/*
 * BLEExplorer.cpp
 *
 *  Created on: Sep 27, 2017
 *      Author: kolban
 */

#include "BLEExplorer.h"
#include "sdkconfig.h"
#include <esp_log.h>
#include <BLEDevice.h>
static const char* LOG_TAG = "BLEExplorer";

BLEExplorer::BLEExplorer() {
	// TODO Auto-generated constructor stub

}

BLEExplorer::~BLEExplorer() {
	// TODO Auto-generated destructor stub
}

/**
 * @brief Perform a BLE Scan and return the results.
 */
void BLEExplorer::scan() {
	/*
	ESP_LOGD(LOG_TAG, ">> scan");
	BLEDevice::init("");
	BLEScan* pBLEScan = BLEDevice::getScan();
	//pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
	pBLEScan->setActiveScan(true);
	BLEScanResults scanResults = pBLEScan->start(30);
	ESP_LOGD(LOG_TAG, "<< scan");
	*/
} // scan
