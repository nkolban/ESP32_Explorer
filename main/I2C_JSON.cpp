/*
 * I2C.cpp
 *
 *  Created on: 18.09.2017
 *      Author: darek
 */


#include <stdlib.h>
#include <JSON.h>
extern "C" {
	#include <soc/i2c_struct.h>
}

JsonObject I2C_JSON() {
	JsonObject obj = JSON::createObject();
	JsonObject tmpObj = JSON::createObject();

	tmpObj.setString("test", "text");

	obj.setObject("conf", tmpObj);
	return obj;
}
