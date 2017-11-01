/*
 * NVS.cpp
 *
 *  Created on: Mar 27, 2017
 *      Author: kolban
 */

#include "CPPNVS.h"
#include <esp_err.h>
#include <esp_log.h>
#include <stdlib.h>

const char* LOG_TAG = "CPPNVS";

/**
 * @brief Constructor.
 *
 * @param [in] name The namespace to open for access.
 * @param [in] openMode
 */
NVS::NVS(std::string name, nvs_open_mode openMode) {
	m_name = name;
	::nvs_open(name.c_str(), openMode, &m_handle);
} // NVS


NVS::~NVS() {
	::nvs_close(m_handle);
} // ~NVS


/**
 * @brief Commit any work performed in the namespace.
 */
void NVS::commit() {
	::nvs_commit(m_handle);
} // commit


/**
 * @brief Erase ALL the keys in the namespace.
 */
void NVS::erase() {
	::nvs_erase_all(m_handle);
} // erase


/**
 * @brief Erase a specific key in the namespace.
 *
 * @param [in] key The key to erase from the namespace.
 */
void NVS::erase(std::string key) {
	::nvs_erase_key(m_handle, key.c_str());
} // erase


/**
 * @brief Retrieve a string/blob value by key.
 *
 * @param [in] key The key to read from the namespace.
 * @param [out] result The string read from the %NVS storage.
 */
void NVS::get(std::string key, std::string* result, bool isBlob) {
	size_t length;
	if (isBlob) {
		::nvs_get_blob(m_handle, key.c_str(), NULL, &length);
	} else {
		::nvs_get_str(m_handle, key.c_str(), NULL, &length);
	}
	char *data = (char *)malloc(length);
	if (isBlob) {
		::nvs_get_blob(m_handle, key.c_str(), data, &length);
	} else {
		::nvs_get_str(m_handle, key.c_str(), data, &length);
	}
	*result = std::string(data);
	free(data);
} // get


void NVS::get(std::string key, uint32_t& value) {
	::nvs_get_u32(m_handle, key.c_str(), &value);
} // get - uint32_t


void NVS::get(std::string key, uint8_t* result, size_t& length) {
	ESP_LOGD(LOG_TAG, ">> get: key: %s, blob: inputSize: %d", key.c_str(), length);
	esp_err_t rc = ::nvs_get_blob(m_handle, key.c_str(), result, &length);
	if (rc != ESP_OK) {
		ESP_LOGD(LOG_TAG, "nvs_get_blob: %d", rc);
	}
	ESP_LOGD(LOG_TAG, "<< get: outputSize: %d", length);
} // get - blob



/**
 * @brief Set the string/blob value by key.
 *
 * @param [in] key The key to set from the namespace.
 * @param [in] data The value to set for the key.
 */
void NVS::set(std::string key, std::string data, bool isBlob) {
	ESP_LOGD(LOG_TAG, ">> set: key: %s, string: value=%s", key.c_str(), data.c_str());
	if (isBlob) {
		::nvs_set_blob(m_handle, key.c_str(), data.data(), data.length());
	} else {
		::nvs_set_str(m_handle, key.c_str(), data.c_str());
	}
	ESP_LOGD(LOG_TAG, "<< set");
} // set


void NVS::set(std::string key, uint32_t value) {
	ESP_LOGD(LOG_TAG, ">> set: key: %s, u32: value=%d", key.c_str(), value);
	::nvs_set_u32(m_handle, key.c_str(), value);
	ESP_LOGD(LOG_TAG, "<< set");
} // set - uint32_t


void NVS::set(std::string key, uint8_t* data, size_t length) {
	ESP_LOGD(LOG_TAG, ">> set: key: %s, blob: length=%d", key.c_str(), length);
	esp_err_t rc = ::nvs_set_blob(m_handle, key.c_str(), data, length);
	if (rc != ESP_OK) {
		ESP_LOGD(LOG_TAG, "nvs_get_blob: %d", rc);
	}
	ESP_LOGD(LOG_TAG, "<< set");
} // set (BLOB)
