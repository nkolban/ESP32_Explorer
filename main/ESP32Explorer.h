/*
 * ESP32Explorer.h
 *
 *  Created on: May 22, 2017
 *      Author: kolban
 */

#ifndef MAIN_ESP32EXPLORER_H_
#define MAIN_ESP32EXPLORER_H_
#include "BLEExplorer.h_SAVE"

class ESP32_Explorer {
public:
	ESP32_Explorer();
	virtual ~ESP32_Explorer();
	void start();
private:
	BLEExplorer *m_pBLEExplorer;
};

#endif /* MAIN_ESP32EXPLORER_H_ */
