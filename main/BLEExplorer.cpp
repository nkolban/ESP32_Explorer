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
#include "Task.h"
#include "Memory.h"

#include "BLEClient.h"
static const char* LOG_TAG = "BLEExplorer";

BLEExplorer::BLEExplorer() {
	// TODO Auto-generated constructor stub

}

BLEExplorer::~BLEExplorer() {
	// TODO Auto-generated destructor stub
}

JsonObject BLEExplorer::enumerateServices(BLERemoteService *p, std::map<std::string, BLERemoteCharacteristic*> *charact, std::string _addr){
	JsonObject obj = JSON::createObject();
	JsonArray arr = JSON::createArray();

	obj.setString("id", p->getUUID().toString());
	obj.setString("text", p->getUUID().toString());
	obj.setString("icon", "service");
	//obj.setString("parent", "#");
	JsonObject state = JSON::createObject();
	state.setBoolean("opened", true);
	obj.setObject("state", state);
//TODO create an array of children characteristics and each characteristic add as object to service object
	for (auto it=charact->begin(); it!=charact->end(); ++it) {
		JsonObject ch = JSON::createObject();
		ch.setString("id", it->second->getUUID().toString());
		ch.setString("text", it->second->getUUID().toString());
		ch.setString("icon", "characteristic");
		arr.addObject(ch);
	}

	obj.setArray("children", arr);
	return obj;
}

JsonArray BLEExplorer::connect(std::string _addr){
	Memory::startTraceAll();
	BLEAddress *pAddress = new BLEAddress(_addr);
	BLEClient*  pClient  = BLEDevice::createClient();

	// Connect to the remove BLE Server.
	pClient->connect(*pAddress);
	std::map<std::string, BLERemoteService*> *pRemoteServices = pClient->getServices();
	if (pRemoteServices == nullptr) {
		ESP_LOGD(LOG_TAG, "Failed to find services");
		return 0;
	}
	JsonArray arr = JSON::createArray();
	for (auto it=pRemoteServices->begin(); it!=pRemoteServices->end(); ++it) {
		std::map<std::string, BLERemoteCharacteristic*> *pRemoteCharacteristics = it->second->getCharacteristics();
		arr.addObject(enumerateServices(it->second, pRemoteCharacteristics, _addr));
	}

	pClient->disconnect();
	free(pAddress);
	free(pClient);
	Memory::stopTrace();
	Memory::dump();
	return arr;
}
/**
 * @brief Perform a BLE Scan and return the results.
 */
JsonArray BLEExplorer::scan() {
	ESP_LOGD(LOG_TAG, ">> scan");

	BLEDevice::init("");
	BLEScan* pBLEScan = BLEDevice::getScan();
	pBLEScan->setActiveScan(true);
	BLEScanResults scanResults = pBLEScan->start(5);
	JsonObject obj = JSON::createObject();
	JsonArray arr = JSON::createArray();
	arr.addString("BLE devices");
	for (int c = 0; c < scanResults.getCount(); ++c) {
		BLEAdvertisedDevice dev =  scanResults.getDevice(c);
		arr.addObject(enumerateDevices(dev));
		//obj.setObject(enumerateDevice(dev));
	}
	//obj.setArray("children", arr);
	obj.setInt("deviceCount", scanResults.getCount());
	ESP_LOGD(LOG_TAG, "<< scan");
	return arr;
} // scan

JsonObject BLEExplorer::enumerateDevices(BLEAdvertisedDevice device){ //TODO expandable complex info about device advertising
	JsonObject obj = JSON::createObject();
	obj.setString("id", device.getAddress().toString());
	obj.setString("parent", "#");
	obj.setString("text", device.getName());
	JsonObject state = JSON::createObject();
	state.setBoolean("opened", true);
	obj.setObject("state", state);
/*	JsonObject obj2 = JSON::createObject();
	obj2.setString("address", device.getAddress().toString());
	obj2.setString("parent", device.getAddress().toString());
	obj2.setString("text", "Address");
	obj2.setBoolean("children", false);*/

	JsonArray arr = JSON::createArray();
//	arr.addObject(obj2);
	obj.setArray("children", arr);

	return obj;
}
