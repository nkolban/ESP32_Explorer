/*
 * WiFi.h
 *
 *  Created on: Feb 25, 2017
 *      Author: kolban
 */

#ifndef MAIN_WIFI_H_
#define MAIN_WIFI_H_
#include "sdkconfig.h"

#include <string>
#include <vector>
#include <mdns.h>
#include <esp_err.h>
#include "FreeRTOS.h"
#include "WiFiEventHandler.h"

/**
 * @brief Manage mDNS server.
 */
class MDNS {
public:
    MDNS();
    ~MDNS();
    void serviceAdd(const std::string& service, const std::string& proto, uint16_t port);
    void serviceInstanceSet(const std::string& service, const std::string& proto, const std::string& instance);
    void servicePortSet(const std::string& service, const std::string& proto, uint16_t port);
    void serviceRemove(const std::string& service, const std::string& proto);
    void setHostname(const std::string& hostname);
    void setInstance(const std::string& instance);
    // If we the above functions with a basic char*, a copy would be created into an std::string,
    // making the whole thing require twice as much processing power and speed
    void serviceAdd(const char* service, const char* proto, uint16_t port);
    void serviceInstanceSet(const char* service, const char* proto, const char* instance);
    void servicePortSet(const char* service, const char* proto, uint16_t port);
    void serviceRemove(const char* service, const char* proto);
    void setHostname(const char* hostname);
    void setInstance(const char* instance);
private:
    mdns_server_t *m_mdns_server = nullptr;
};

class WiFiAPRecord {
public:
    friend class WiFi;

    /**
     * @brief Get the auth mode.
     * @return The auth mode.
     */
    wifi_auth_mode_t getAuthMode() {
        return m_authMode;
    }

    /**
     * @brief Get the RSSI.
     * @return the RSSI.
     */
    int8_t getRSSI() {
        return m_rssi;
    }

    /**
     * @brief Get the SSID.
     * @return the SSID.
     */
    std::string getSSID() {
        return m_ssid;
    }

    std::string toString();

private:
    uint8_t          m_bssid[6];
    int8_t           m_rssi;
    std::string      m_ssid;
    wifi_auth_mode_t m_authMode;
};

/**
 * @brief %WiFi driver.
 *
 * Encapsulate control of %WiFi functions.
 *
 * Here is an example fragment that illustrates connecting to an access point.
 * @code{.cpp}
 * #include <WiFi.h>
 * #include <WiFiEventHandler.h>
 *
 * class TargetWiFiEventHandler: public WiFiEventHandler {
 *    esp_err_t staGotIp(system_event_sta_got_ip_t event_sta_got_ip) {
 *       ESP_LOGD(tag, "MyWiFiEventHandler(Class): staGotIp");
 *       // Do something ...
 *       return ESP_OK;
 *    }
 * };
 *
 * WiFi wifi;
 *
 * TargetWiFiEventHandler *eventHandler = new TargetWiFiEventHandler();
 * wifi.setWifiEventHandler(eventHandler);
 * wifi.connectAP("myssid", "mypassword");
 * @endcode
 */
class WiFi {
private:
		static esp_err_t    eventHandler(void* ctx, system_event_t* event);
		void                init();
    uint32_t            ip;
    uint32_t            gw;
    uint32_t            netmask;
    WiFiEventHandler*   m_pWifiEventHandler;
    uint8_t             m_dnsCount=0;
    bool                m_eventLoopStarted;
    bool                m_initCalled;
  	FreeRTOS::Semaphore m_gotIpEvt = FreeRTOS::Semaphore("GotIpEvt");

public:
    WiFi();
    ~WiFi();
    void                      addDNSServer(const std::string& ip);
    void                      addDNSServer(const char* ip);
    void                      addDNSServer(ip_addr_t ip);
    void                      setDNSServer(int numdns, const std::string& ip);
    void                      setDNSServer(int numdns, const char* ip);
    void                      setDNSServer(int numdns, ip_addr_t ip);
    struct in_addr            getHostByName(const std::string& hostName);
    struct in_addr            getHostByName(const char* hostName);
    void                      connectAP(const std::string& ssid, const std::string& password, bool waitForConnection=true);
    void                      dump();
    static std::string        getApMac();
    static tcpip_adapter_ip_info_t getApIpInfo();
    static std::string        getApSSID();
    static std::string        getMode();
    static tcpip_adapter_ip_info_t getStaIpInfo();
    static std::string        getStaMac();
    static std::string        getStaSSID();
    std::vector<WiFiAPRecord> scan();
    void                      startAP(const std::string& ssid, const std::string& passwd);
    void                      setIPInfo(const std::string& ip, const std::string& gw, const std::string& netmask);
    void                      setIPInfo(const char* ip, const char* gw, const char* netmask);
    void                      setIPInfo(uint32_t ip, uint32_t gw, uint32_t netmask);
    void                      setWifiEventHandler(WiFiEventHandler *wifiEventHandler);
};

#endif /* MAIN_WIFI_H_ */
