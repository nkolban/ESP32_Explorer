/*
 * BLEExplorer.h
 *
 *  Created on: Sep 27, 2017
 *      Author: kolban
 */

#ifndef MAIN_BLEEXPLORER_H_
#define MAIN_BLEEXPLORER_H_
#include <freertos/FreeRTOS.h>
class BLEExplorer {
public:
	BLEExplorer();
	virtual ~BLEExplorer();
	uint8_t scan();
};

#endif /* MAIN_BLEEXPLORER_H_ */
