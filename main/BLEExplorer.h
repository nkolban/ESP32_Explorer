/*
 * BLEExplorer.h
 *
 *  Created on: Sep 27, 2017
 *      Author: kolban
 */

#ifndef MAIN_BLEEXPLORER_H_
#define MAIN_BLEEXPLORER_H_
#include <freertos/FreeRTOS.h>
#include "JSON.h"
#include "BLEAdvertisedDevice.h"

class BLEExplorer {
public:
	BLEExplorer();
	virtual ~BLEExplorer();
	JsonArray scan();
	JsonArray connect(std::string addr);
	JsonObject enumerateDevices(BLEAdvertisedDevice device);
	JsonObject enumerateServices(BLERemoteService *p, std::map<std::string, BLERemoteCharacteristic*> *charact, std::string _addr);
};

#endif /* MAIN_BLEEXPLORER_H_ */
