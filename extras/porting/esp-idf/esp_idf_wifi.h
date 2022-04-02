/*
   Copyright (C) AC SOFTWARE SP. Z O.O.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
   */

#ifndef esp_idf_wifi_h__
#define esp_idf_wifi_h__

// FreeRTOS includes
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/task.h>
// ESP-IDF includes
#include <esp_tls.h>
#include <esp_wifi.h>

// supla-device includes
#include <supla/network/netif_wifi.h>


namespace Supla {
  class EspIdfWifi : public Supla::Wifi {
    public:
      EspIdfWifi(const char *wifiSsid = nullptr,
          const char *wifiPassword = nullptr,
          unsigned char *ip = nullptr);
      virtual ~EspIdfWifi();

      int read(void *buf, int count) override;
      int write(void *buf, int count) override;
      int connect(const char *server, int port = -1) override;
      bool connected() override;
      bool isReady() override;
      void disconnect() override;
      void setup() override;
      void uninit() override;
      bool getMacAddr(uint8_t *out) override;

      void enableSSL(bool value);
      void setTimeout(int timeoutMs);
      void fillStateData(TDSC_ChannelState &channelState) override;

      void setIpReady(bool ready);
      void setIpv4Addr(unsigned _supla_int_t);
      void setWifiConnected(bool state);

      bool isInConfigMode();

    protected:
      bool isSecured = true;
      bool initDone = false;
      bool isWifiConnected = false;
      bool isIpReady = false;
      bool isServerConnected = false;
      EventGroupHandle_t wifiEventGroup;
      esp_tls_t *client = nullptr;
      int timeoutMs = 10000;
      unsigned _supla_int_t ipv4 = 0;
#ifdef SUPLA_DEVICE_ESP32
      esp_netif_t* staNetIf = nullptr;
      esp_netif_t* apNetIf = nullptr;
#endif
  };

};  // namespace Supla

#endif /*esp_idf_wifi_h__*/
