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
#include <sstream>
#include "BLE2902.h"
#include "BLEClient.h"
static const char* LOG_TAG = "BLEExplorer";
static bool isRunning = false;
static BLEServer *pServer;
static BLEServiceMap *mServices;
static BLECharacteristicMap *mCharacteristics;

BLEExplorer::BLEExplorer() {
	// TODO Auto-generated constructor stub
}

BLEExplorer::~BLEExplorer() {
	// TODO Auto-generated destructor stub
}
/**
 * @brief Perform a BLE Scan and return the results.
 */
JsonArray BLEExplorer::scan() {
	ESP_LOGD(LOG_TAG, ">> scan");
	if(!isRunning){
		BLEDevice::init("");
		isRunning = true;
	}
	BLEScan* pBLEScan = BLEDevice::getScan();
	pBLEScan->setActiveScan(true);
	BLEScanResults scanResults = pBLEScan->start(1);
	JsonObject obj = JSON::createObject();
	JsonArray arr = JSON::createArray();
	arr.addString("BLE devices");
	for (int c = 0; c < scanResults.getCount(); ++c) {
		BLEAdvertisedDevice dev =  scanResults.getDevice(c);
		arr.addObject(enumerateDevices(dev));
	}
	obj.setInt("deviceCount", scanResults.getCount());
	ESP_LOGD(LOG_TAG, "<< scan");
	return arr;
} // scan
/*
 * @brief Connect to BLE server to get and enumerate services and characteristics
 */
JsonArray BLEExplorer::connect(std::string _addr){
	BLEAddress *pAddress = new BLEAddress(_addr);
	BLEClient*  pClient  = BLEDevice::createClient();

	// Connect to the remove BLE Server.
	pClient->connect(*pAddress);
	std::map<std::string, BLERemoteService*> *pRemoteServices = pClient->getServices();
	if (pRemoteServices == nullptr) {
		ESP_LOGD(LOG_TAG, "Failed to find services");
		return JSON::createArray();
	}
//	Memory::startTraceAll();
	JsonArray arr = JSON::createArray();
	for (auto it=pRemoteServices->begin(); it!=pRemoteServices->end(); ++it) {
		JsonObject obj = JSON::createObject();
		char tmp[33];
		itoa(it->second->getHandle(), tmp, 16);

		obj.setString("id", tmp);  // todo service's handle
		obj.setString("text", BLEUtils::gattServiceToString(it->second->getUUID().getNative()->uuid.uuid32) + " Service: " + it->second->getUUID().toString());
		obj.setString("icon", "service");
		JsonObject state = JSON::createObject();
		state.setBoolean("opened", true);
		obj.setObject("state", state);
		obj.setString("parent", _addr);
		std::map<std::string, BLERemoteCharacteristic*> *pChars = it->second->getCharacteristics();
		obj.setArray("children", enumerateCharacteristics(pChars));
		arr.addObject(obj);
	}

	pClient->disconnect();
/*
	free(pClient);
	free(pAddress);   //FIXME using it here causing multi heap crash
	free(pRemoteServices);*/
	return arr;
}
/*
 * @brief Enumerate devices
 */
JsonObject BLEExplorer::enumerateDevices(BLEAdvertisedDevice device){
	JsonArray arr = JSON::createArray();
	JsonObject obj = JSON::createObject();
	obj.setString("id", device.getAddress().toString());
	obj.setString("parent", "#");
	obj.setString("text", device.getName() == ""? "N/A":device.getName());
	JsonObject state = JSON::createObject();
	state.setBoolean("opened", true);
	obj.setObject("state", state);
	JsonObject obj2 = JSON::createObject();
	//obj2.setString("id", device.getAddress().toString());
	//obj2.setString("parent", device.getAddress().toString());
	obj2.setString("text", "Address - " + device.getAddress().toString());
	//obj2.setBoolean("children", false);
	arr.addObject(obj2);
	if(device.haveManufacturerData()){
		obj2 = JSON::createObject();
		obj2.setString("text", "Manufacturer - " + atoi(device.getManufacturerData().c_str()));
		arr.addObject(obj2);
	}
	std::stringstream ss;
	if(device.haveAppearance()){
		ss << "Appearance - " << device.getAppearance();
		obj2 = JSON::createObject();
		obj2.setString("text", ss.str());
		arr.addObject(obj2);
	}
	if(device.haveRSSI()){
		ss.str("");
		ss << "RSSI - " <<  (int)device.getRSSI();
		obj2 = JSON::createObject();
		obj2.setString("text", ss.str());
		arr.addObject(obj2);
	}
	if(device.haveTXPower()){
		ss.str("");
		ss << "TX power - " << (int)device.getTXPower();
		obj2 = JSON::createObject();
		obj2.setString("text", ss.str());
		arr.addObject(obj2);
	}
	if(device.haveServiceUUID()){
		ss.str("");
		ss << "Advertised services - " << device.getServiceUUID().toString();
		obj2 = JSON::createObject();
		obj2.setString("text", ss.str());
		arr.addObject(obj2);
	}


	obj.setArray("children", arr);

	return obj;
}
/*
 * @brief Enumerate characteristics from given service
 */
JsonArray BLEExplorer::enumerateCharacteristics(std::map<std::string, BLERemoteCharacteristic*> *characteristicMap){

	//std::map<uint16_t, BLERemoteCharacteristic*> *characteristicMap = pService;
	//pService->getCharacteristics(characteristicMap);
	JsonArray arr = JSON::createArray();
	char tmp[33];

	for (auto it=characteristicMap->begin(); it!=characteristicMap->end(); it++) {  // TODO add descriptors enumerator, add short uuid if characteristic !UNKNOWN
		JsonObject ch = JSON::createObject();
		JsonObject val = JSON::createObject();
		JsonArray ar = JSON::createArray();
		itoa(it->second->getHandle(), tmp, 16);
		ch.setString("id", tmp);  // characteristic's handle

		std::string str = it->second->canRead()?"R":"";
		str += it->second->canWrite()?"\\W":"";
		str += it->second->canBroadcast()?"\\B":"";
		str += it->second->canIndicate()?"\\I":"";
		str += it->second->canNotify()?"\\N]":"]";
		std::stringstream ss1;
		ss1 << BLEUtils::gattCharacteristicUUIDToString(it->second->getUUID().getNative()->uuid.uuid16) << \
				" Characteristic: " << it->second->getUUID().toString() << "[" << \
				str;

		ch.setString("text", ss1.str());

		ch.setString("icon", "characteristic");
		JsonObject state = JSON::createObject();
		state.setBoolean("opened", true);
		ch.setObject("state", state);
		std::map<uint16_t, BLERemoteCharacteristic*> pChars;
		std::map<std::string, BLERemoteDescriptor*> *desc = it->second->getDescriptors();
		ar = enumerateDescriptors(desc);
		std::string f = it->second->readValue();
		ESP_LOGI(LOG_TAG, "Value: %s, %d, %#4x", f.c_str(), it->second->readUInt32(), it->second->readUInt32());
		std::stringstream ss;
		ss << "Value: " << f;
		val.setString("text", ss.str());
		ar.addObject(val);
		ch.setArray("children", ar);
		arr.addObject(ch);
	}

	return arr;
}

JsonArray BLEExplorer::enumerateDescriptors(std::map<std::string, BLERemoteDescriptor*>* descriptors){
	JsonArray arr = JSON::createArray();
	for(auto it=descriptors->begin();it!=descriptors->end();it++){
		JsonObject obj = JSON::createObject();
		obj.setString("icon", "descriptor");
		/*JsonObject val = JSON::createObject();
		std::stringstream ss;
		ss << "Value: " << it->second->readValue();
		val.setString("text", ss.str());
		obj.setObject("children", val);*/
		obj.setString("text", BLEUtils::gattDescriptorUUIDToString(it->second->getUUID().getNative()->uuid.uuid16) + " Descriptor: " + it->second->getUUID().toString());
		arr.addObject(obj);
	}

	return arr;
}

void BLEExplorer::createServer(std::string name){
	BLEDevice::init(name);
	pServer = BLEDevice::createServer();
	mCharacteristics = new BLECharacteristicMap();
	mServices = new BLEServiceMap();
}

void BLEExplorer::startService(uint16_t handle){
	BLEService *pservice = mServices->getByHandle(handle);
	pservice->start();
}

JsonObject BLEExplorer::addService(BLEUUID _uuid){
	JsonObject obj = JSON::createObject();

	char ptr[33];
	BLEService *pservice = pServer->createService(_uuid);
	if(pservice != nullptr){
		mServices->setByHandle(pservice->getHandle(), pservice);
//		pservice->start();
		obj.setString("icon", "service");
		itoa(pservice->getHandle(), ptr, 16);
		obj.setString("handle", ptr);
		obj.setString("parent", "#");
		obj.setString("text", BLEUtils::gattServiceToString(pservice->getUUID().getNative()->uuid.uuid16) + " Service: " + pservice->getUUID().toString());
	}
	return obj;
}

JsonObject BLEExplorer::addCharacteristic(BLEUUID uuid, uint16_t service){
	char ptr[33];
	BLEService *pservice = mServices->getByHandle(service);
	ESP_LOGE(LOG_TAG, "ADD: %d", pservice->getHandle());
	BLECharacteristic *charact = pservice->createCharacteristic(BLEUUID(uuid), {BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE});
	charact->setValue("Private name");
	//mCharacteristics->setByHandle(charact->getHandle(), charact);
	JsonObject obj = JSON::createObject();
	obj.setString("icon", "characteristic");
	itoa(charact->getHandle(), ptr, 16);
	obj.setString("handle", ptr);
	itoa(service, ptr, 16);
	obj.setString("parent", ptr);
	obj.setString("text", BLEUtils::gattCharacteristicUUIDToString(charact->getUUID().getNative()->uuid.uuid16) + " Characteristic: " + charact->getUUID().toString());
	return obj;
}

JsonObject BLEExplorer::addDescriptor(BLEUUID uuid, uint16_t charact){
	char ptr[33];
	BLECharacteristic *pcharact = mCharacteristics->getByHandle(charact);
	assert(charact == pcharact->getHandle());
	BLEDescriptor *descr = new BLE2902();
	pcharact->addDescriptor(descr);
	JsonObject obj = JSON::createObject();
	obj.setString("icon", "descriptor");
	itoa(descr->getHandle(), ptr, 16);
	obj.setString("handle", ptr);
	itoa(pcharact->getHandle(), ptr, 16);
	obj.setString("parent", ptr);
	obj.setString("text", BLEUtils::gattDescriptorUUIDToString(uuid.getNative()->uuid.uuid32) + " Descriptor: " + uuid.toString());
	return obj;
}

void BLEExplorer::startAdvertising(){
	pServer->getAdvertising()->start();
}

void BLEExplorer::stopAdvertising(){
	pServer->getAdvertising()->stop();
}

