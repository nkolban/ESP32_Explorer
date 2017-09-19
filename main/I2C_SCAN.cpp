/*
 * I2C_SCAN.cpp
 *
 *  Created on: 18.09.2017
 *      Author: darek
 */


#include <stdlib.h>
#include <JSON.h>
#include <driver/i2c.h>

JsonObject I2C_SCAN_JSON() {
	JsonObject obj = JSON::createObject();
	JsonObject tmpObj = JSON::createObject();
	JsonArray tmpArr = JSON::createArray();
	//tmpArr = JSON::createArray();
	int i;
	esp_err_t espRc;
	printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
	printf("00:         ");
	for (i=3; i< 0x78; i++) {
		i2c_cmd_handle_t cmd = i2c_cmd_link_create();
		::i2c_master_start(cmd);
		::i2c_master_write_byte(cmd, (i << 1) | I2C_MASTER_WRITE, 1 /* expect ack */);
		::i2c_master_stop(cmd);

		espRc = ::i2c_master_cmd_begin(I2C_NUM_0, cmd, 100/portTICK_PERIOD_MS);
		char nr[3];
		if (i%16 == 0) {
			sprintf(nr, "address %.2x", i-16);
			obj.setObject(nr, tmpObj);
			tmpObj = JSON::createObject();
			printf("\n%.2x:", i);
		}
		sprintf(nr, "address %.2x", i);
		if (espRc == 0) {
			tmpObj.setBoolean(nr, true);
			tmpArr.addBoolean(true);
			printf(" %.2x", i);
		} else {
			tmpObj.setBoolean(nr, false);
			tmpArr.addBoolean(false);
			printf(" --");
		}
		//ESP_LOGD(tag, "i=%d, rc=%d (0x%x)", i, espRc, espRc);
		::i2c_cmd_link_delete(cmd);
	}
	printf("\n");
	obj.setObject("70", tmpObj);
	obj.setArray("present", tmpArr);
	//obj.setObject("I2C", tmpObj);
	return obj;
}


