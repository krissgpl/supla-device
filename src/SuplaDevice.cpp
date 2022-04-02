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

#include <string.h>

#include "SuplaDevice.h"
#include "supla-common/log.h"
#include "supla-common/srpc.h"
#include "supla/channel.h"
#include "supla/device/last_state_logger.h"
#include "supla/element.h"
#include "supla/io.h"
#include "supla/network/network.h"
#include "supla/storage/storage.h"
#include "supla/timer.h"
#include "supla/time.h"
#include "supla/storage/config.h"
#include "supla/tools.h"

void SuplaDeviceClass::status(int newStatus, const char *msg, bool alwaysLog) {
  bool showLog = false;

  if (currentStatus == STATUS_CONFIG_MODE) {
    // Config mode is final state and the only exit goes through reset
    return;
  }

  if (currentStatus != newStatus &&
      !(newStatus == STATUS_REGISTER_IN_PROGRESS &&
        currentStatus > STATUS_REGISTER_IN_PROGRESS)) {
    if (impl_arduino_status != nullptr) {
      impl_arduino_status(newStatus, msg);
    }
    currentStatus = newStatus;
    showLog = true;
    if (lastStateLogger) {
      lastStateLogger->log(msg);
    }
  }
  if (alwaysLog || showLog)
    supla_log(LOG_INFO, "Current status: [%d] %s", newStatus, msg);
}

char *SuplaDeviceClass::getLastStateLog() {
  if (lastStateLogger) {
    return lastStateLogger->getLog();
  }
  return nullptr;
}

bool SuplaDeviceClass::prepareLastStateLog() {
  if (lastStateLogger) {
    return lastStateLogger->prepareLastStateLog();
  }
  return false;
}

SuplaDeviceClass::SuplaDeviceClass()
    : port(-1),
      connectionFailCounter(0),
      networkIsNotReadyCounter(0),
      currentStatus(STATUS_UNKNOWN),
      impl_arduino_status(nullptr),
      clock(nullptr) {
  srpc = nullptr;
  registered = 0;
  lastIterateTime = 0;
  waitForIterate = 0;
}

SuplaDeviceClass::~SuplaDeviceClass() {
}

void SuplaDeviceClass::setStatusFuncImpl(
    _impl_arduino_status impl_arduino_status) {
  this->impl_arduino_status = impl_arduino_status;
}

bool SuplaDeviceClass::isSrpcInitialized(bool msg) {
  if (srpc != nullptr) {
    if (msg)
      status(STATUS_ALREADY_INITIALIZED, "SuplaDevice is already initialized");

    return true;
  }

  return false;
}

bool SuplaDeviceClass::begin(const char GUID[SUPLA_GUID_SIZE],
                             const char *Server,
                             const char *email,
                             const char authkey[SUPLA_AUTHKEY_SIZE],
                             unsigned char version) {
  setGUID(GUID);
  setServer(Server);
  setEmail(email);
  setAuthKey(authkey);

  return begin(version);
}

bool SuplaDeviceClass::begin(unsigned char version) {
  isSrpcInitialized(true);
  if (initializationDone) {
    return false;
  }

  supla_log(LOG_DEBUG, "Supla - starting initialization");


  Supla::Storage::Init();

  if (Supla::Storage::IsConfigStorageAvailable()) {
    addFlags(SUPLA_DEVICE_FLAG_CALCFG_ENTER_CFG_MODE);
    loadDeviceConfig();
    lastStateLogger = new Supla::Device::LastStateLogger();

    // Load elements configuration
    for (auto element = Supla::Element::begin(); element != nullptr;
        element = element->next()) {
      element->onLoadConfig();
      delay(0);
    }
  }


  // Pefrorm dry run of write state to validate stored state section with
  // current device configuration
  if (Supla::Storage::PrepareState(true)) {
    supla_log(LOG_DEBUG,
        "Validating storage state section with current device configuration");
    for (auto element = Supla::Element::begin(); element != nullptr;
         element = element->next()) {
      element->onSaveState();
      delay(0);
    }
    // If state storage validation was successful, perform read state
    if (Supla::Storage::FinalizeSaveState()) {
      supla_log(LOG_DEBUG,
          "Storage state section validation completed. Loading elements "
          "state...");
      // Iterate all elements and load state
      Supla::Storage::PrepareState();
      for (auto element = Supla::Element::begin(); element != nullptr;
           element = element->next()) {
        element->onLoadState();
        delay(0);
      }
    }
  }

  // Initialize elements
  for (auto element = Supla::Element::begin(); element != nullptr;
       element = element->next()) {
    element->onInit();
    delay(0);
  }

  // Enable timers
  Supla::initTimers();

  // Core elements initialization is done
  initializationDone = true;

  if (Supla::Network::Instance() == nullptr) {
    status(STATUS_MISSING_NETWORK_INTERFACE, "Network Interface not defined!");
    return false;
  }

  if (isArrayEmpty(Supla::Channel::reg_dev.GUID, SUPLA_GUID_SIZE)) {
    status(STATUS_INVALID_GUID, "Missing GUID");
    // when config storage is available, GUID will be generated automatically
    if (!Supla::Storage::IsConfigStorageAvailable()) {
      return false;
    }
  }

  if (isArrayEmpty(Supla::Channel::reg_dev.AuthKey, SUPLA_AUTHKEY_SIZE)) {
    status(STATUS_INVALID_AUTHKEY, "Missing AuthKey");
    // when config storage is available, AuthKey will be generated automatically
    if (!Supla::Storage::IsConfigStorageAvailable()) {
      return false;
    }
  }

  if (Supla::Channel::reg_dev.ServerName[0] == '\0') {
    status(STATUS_UNKNOWN_SERVER_ADDRESS, "Missing server address");
    if (deviceMode != Supla::DEVICE_MODE_CONFIG) {
      return false;
    }
  }

  if (Supla::Channel::reg_dev.Email[0] == '\0') {
    status(STATUS_MISSING_CREDENTIALS, "Missing email address");
    if (deviceMode != Supla::DEVICE_MODE_CONFIG) {
      return false;
    }
  }

  if (strnlen(Supla::Channel::reg_dev.Name, SUPLA_DEVICE_NAME_MAXSIZE) == 0) {
#if defined(ARDUINO_ARCH_ESP8266)
    setName("ESP8266");
#elif defined(ARDUINO_ARCH_ESP32)
    setName("ESP32");
#elif defined(ARDUINO_ARCH_AVR)
    setName("ARDUINO MEGA");
#else
    setName("SuplaDevice");
#endif
  }

  if (strnlen(Supla::Channel::reg_dev.SoftVer, SUPLA_SOFTVER_MAXSIZE) == 0) {
    setSwVersion("User SW, lib 2.4.1");
  }

  supla_log(LOG_DEBUG, "Initializing network layer");
  char hostname[32] = {};
  generateHostname(hostname, 3);
  Supla::Network::SetHostname(hostname);
  if (deviceMode == Supla::DEVICE_MODE_CONFIG) {
    enterConfigMode();
  } else {
    enterNormalMode();
  }

  TsrpcParams srpc_params;
  srpc_params_init(&srpc_params);
  srpc_params.data_read = &Supla::data_read;
  srpc_params.data_write = &Supla::data_write;
  srpc_params.on_remote_call_received = &Supla::message_received;
  srpc_params.user_params = this;

  srpc = srpc_init(&srpc_params);

  // Set Supla protocol interface version
  srpc_set_proto_version(srpc, version);

  supla_log(LOG_DEBUG, "Using Supla protocol version %d", version);

  status(STATUS_INITIALIZED, "SuplaDevice initialized");
  return true;
}

void SuplaDeviceClass::setName(const char *Name) {
  setString(Supla::Channel::reg_dev.Name, Name, SUPLA_DEVICE_NAME_MAXSIZE);
}

void SuplaDeviceClass::setString(char *dst, const char *src, int max_size) {
  if (src == nullptr) {
    dst[0] = 0;
    return;
  }

  int size = strlen(src);

  if (size + 1 > max_size) size = max_size - 1;

  strncpy(dst, src, size);
  dst[size] = 0;
}

void SuplaDeviceClass::onTimer(void) {
  for (auto element = Supla::Element::begin(); element != nullptr;
       element = element->next()) {
    element->onTimer();
  }
}

void SuplaDeviceClass::onFastTimer(void) {
  // Iteration over all impulse counters will count incomming impulses. It is
  // after SuplaDevice initialization (because we have to read stored counter
  // values) and before any other operation like connection to Supla cloud
  // (because we want to count impulses even when we have connection issues.
  for (auto element = Supla::Element::begin(); element != nullptr;
       element = element->next()) {
    element->onFastTimer();
  }
}

void SuplaDeviceClass::iterate(void) {
  if (!initializationDone) {
    return;
  }

  unsigned long _millis = millis();

  if (deviceRestartTimeoutTimestamp != 0 &&
      _millis - deviceRestartTimeoutTimestamp > 5 * 60 * 1000) {
    supla_log(LOG_INFO, "Config mode 5 min timeout. Reset device");
    leaveConfigModeAndRestart();
  }
  if (forceRestartTimeMs &&
      _millis - deviceRestartTimeoutTimestamp > forceRestartTimeMs) {
    supla_log(LOG_INFO, "Leave cfg mode requested. Reset device");
    leaveConfigModeAndRestart();
  }

  iterateAlwaysElements(_millis);

  if (!isSrpcInitialized(false)) {
    return;
  }

  if (waitForIterate != 0 && _millis < waitForIterate) {
    return;
  } else {
    waitForIterate = 0;
  }

  if (deviceMode != Supla::DEVICE_MODE_CONFIG) {
    if (!iterateNetworkSetup(_millis)) {
      return;
    }

    // Establish connection with Supla server
    if (!Supla::Network::Connected()) {
      uptime.setConnectionLostCause(
          SUPLA_LASTCONNECTIONRESETCAUSE_SERVER_CONNECTION_LOST);
      registered = 0;
      int result =
        Supla::Network::Connect(Supla::Channel::reg_dev.ServerName, port);
      if (1 == result) {
        uptime.resetConnectionUptime();
        connectionFailCounter = 0;
        supla_log(LOG_DEBUG, "Connected to Supla Server");

      } else {
        status(STATUS_SERVER_DISCONNECTED, "Not connected to Supla server");
        supla_log(LOG_DEBUG,
            "Connection fail (%d). Server: %s",
            result,
            Supla::Channel::reg_dev.ServerName);
        Supla::Network::Disconnect();
        waitForIterate = _millis + 2000;
        connectionFailCounter++;
        return;
      }
    }

    // Handle communication with Supla server and maintain link
    if (iterateSuplaProtocol(_millis)) {
      if (_millis - lastIterateTime > 0) {
        // Iterate all elements
        for (auto element = Supla::Element::begin(); element != nullptr;
            element = element->next()) {
          if (!element->iterateConnected(srpc)) {
            break;
          }
          delay(0);
        }
        lastIterateTime = _millis;
      }
    }
  } else {
  }
}

void SuplaDeviceClass::onVersionError(TSDC_SuplaVersionError *version_error) {
  status(STATUS_PROTOCOL_VERSION_ERROR, "Protocol version error");
  supla_log(LOG_DEBUG,
      "Protocol version error. Server min: %d; Server version: %d",
      version_error->server_version_min,
      version_error->server_version);

  Supla::Network::Disconnect();

  waitForIterate = millis() + 5000;
}

void SuplaDeviceClass::onRegisterResult(
    TSD_SuplaRegisterDeviceResult *register_device_result) {
  _supla_int_t activity_timeout = 0;

  switch (register_device_result->result_code) {
    // OK scenario
    case SUPLA_RESULTCODE_TRUE:
      activity_timeout = register_device_result->activity_timeout;
      Supla::Network::Instance()->setActivityTimeout(activity_timeout);
      registered = 1;
      supla_log(LOG_DEBUG,
                "Device registered (activity timeout %d s, server version: %d, "
                "server min version: %d)",
                register_device_result->activity_timeout,
                register_device_result->version,
                register_device_result->version_min);
      lastIterateTime = millis();
      status(STATUS_REGISTERED_AND_READY, "Registered and ready");

      if (activity_timeout != ACTIVITY_TIMEOUT) {
        supla_log(
            LOG_DEBUG, "Changing activity timeout to %d", ACTIVITY_TIMEOUT);
        TDCS_SuplaSetActivityTimeout at;
        at.activity_timeout = ACTIVITY_TIMEOUT;
        srpc_dcs_async_set_activity_timeout(srpc, &at);
      }

      for (auto element = Supla::Element::begin(); element != nullptr;
           element = element->next()) {
        element->onRegistered();
        delay(0);
      }

      return;

      // NOK scenarios
    case SUPLA_RESULTCODE_TEMPORARILY_UNAVAILABLE:
      status(STATUS_TEMPORARILY_UNAVAILABLE, "Temporarily unavailable!", true);
      break;

    case SUPLA_RESULTCODE_GUID_ERROR:
      status(STATUS_INVALID_GUID, "Incorrect device GUID!", true);
      break;

    case SUPLA_RESULTCODE_AUTHKEY_ERROR:
      status(STATUS_INVALID_AUTHKEY, "Incorrect AuthKey!", true);
      break;

    case SUPLA_RESULTCODE_BAD_CREDENTIALS:
      status(STATUS_BAD_CREDENTIALS, "Bad credentials - incorrect AuthKey or email", true);
      break;

    case SUPLA_RESULTCODE_REGISTRATION_DISABLED:
      status(STATUS_REGISTRATION_DISABLED, "Registration disabled!", true);
      break;

    case SUPLA_RESULTCODE_DEVICE_LIMITEXCEEDED:
      status(STATUS_DEVICE_LIMIT_EXCEEDED, "Device limit exceeded!", true);
      break;

    case SUPLA_RESULTCODE_NO_LOCATION_AVAILABLE:
      status(STATUS_NO_LOCATION_AVAILABLE, "No location available!", true);
      break;

    case SUPLA_RESULTCODE_DEVICE_DISABLED:
      status(STATUS_DEVICE_IS_DISABLED, "Device is disabled!", true);
      break;

    case SUPLA_RESULTCODE_LOCATION_DISABLED:
      status(STATUS_LOCATION_IS_DISABLED, "Location is disabled!", true);
      break;

    case SUPLA_RESULTCODE_LOCATION_CONFLICT:
      status(STATUS_LOCATION_CONFLICT, "Location conflict!", true);
      break;

    case SUPLA_RESULTCODE_CHANNEL_CONFLICT:
      status(STATUS_CHANNEL_CONFLICT, "Channel conflict!", true);
      break;

    default:
      status(STATUS_UNKNOWN_ERROR, "Unknown registration error", true);
      supla_log(LOG_ERR,
                "Register result code %i",
                register_device_result->result_code);
      break;
  }

  Supla::Network::Disconnect();
  waitForIterate = millis() + 5000;
}

void SuplaDeviceClass::channelSetActivityTimeoutResult(
    TSDC_SuplaSetActivityTimeoutResult *result) {
  Supla::Network::Instance()->setActivityTimeout(result->activity_timeout);
  supla_log(
      LOG_DEBUG, "Activity timeout set to %d s", result->activity_timeout);
}

void SuplaDeviceClass::setServerPort(int value) {
  port = value;
}

void SuplaDeviceClass::setSwVersion(const char *swVersion) {
  setString(Supla::Channel::reg_dev.SoftVer, swVersion, SUPLA_SOFTVER_MAXSIZE);
}

int SuplaDeviceClass::getCurrentStatus() {
  return currentStatus;
}

void SuplaDeviceClass::fillStateData(TDSC_ChannelState &channelState) {
  channelState.Fields |= SUPLA_CHANNELSTATE_FIELD_UPTIME |
                         SUPLA_CHANNELSTATE_FIELD_CONNECTIONUPTIME;

  channelState.Uptime = uptime.getUptime();
  channelState.ConnectionUptime = uptime.getConnectionUptime();
  if (uptime.getLastResetCause() > 0) {
    channelState.Fields |= SUPLA_CHANNELSTATE_FIELD_LASTCONNECTIONRESETCAUSE;
    channelState.LastConnectionResetCause = uptime.getLastResetCause();
  }
}

void SuplaDeviceClass::setGUID(const char GUID[SUPLA_GUID_SIZE]) {
  memcpy(Supla::Channel::reg_dev.GUID, GUID, SUPLA_GUID_SIZE);
}

void SuplaDeviceClass::setAuthKey(const char authkey[SUPLA_AUTHKEY_SIZE]) {
  memcpy(Supla::Channel::reg_dev.AuthKey, authkey, SUPLA_AUTHKEY_SIZE);
}

void SuplaDeviceClass::setEmail(const char *email) {
  setString(Supla::Channel::reg_dev.Email, email, SUPLA_EMAIL_MAXSIZE);
}

void SuplaDeviceClass::setServer(const char *server) {
  setString(
      Supla::Channel::reg_dev.ServerName, server, SUPLA_SERVER_NAME_MAXSIZE);
}

void SuplaDeviceClass::onGetUserLocaltimeResult(
    TSDC_UserLocalTimeResult *result) {
  if (clock) {
    clock->parseLocaltimeFromServer(result);
  }
}

void SuplaDeviceClass::addClock(Supla::Clock *_clock) {
  supla_log(LOG_DEBUG, "Clock class added");
  clock = _clock;
}

Supla::Clock *SuplaDeviceClass::getClock() {
  return clock;
}

void SuplaDeviceClass::loadDeviceConfig() {
  auto cfg = Supla::Storage::ConfigInstance();

  bool configIncomplete = false;
  char buf[256] = {};

  // Device generic config
  if (cfg->getDeviceName(buf)) {
    setName(buf);
  }

  if (cfg->getGUID(buf)) {
    setGUID(buf);
  }

  deviceMode = cfg->getDeviceMode();
  if (deviceMode == Supla::DEVICE_MODE_NOT_SET) {
    deviceMode = Supla::DEVICE_MODE_NORMAL;
  }

  // Supla protocol specific config
  if (cfg->isSuplaCommProtocolEnabled()) {
    if (cfg->getSuplaServer(buf)) {
      setServer(buf);
    } else {
      supla_log(LOG_DEBUG, "Config incomplete: missing server");
      configIncomplete = true;
    }
    setServerPort(cfg->getSuplaServerPort());

    if (cfg->getEmail(buf)) {
      setEmail(buf);
    } else {
      supla_log(LOG_DEBUG, "Config incomplete: missing email");
      configIncomplete = true;
    }

    if (cfg->getAuthKey(buf)) {
      setAuthKey(buf);
    }

  }

  // MQTT protocol specific config
  if (cfg->isMqttCommProtocolEnabled()) {
    // TODO add MQTT config
    supla_log(LOG_ERR, "MQTT support not implemented yet");
  }

  // WiFi specific config
  auto net = Supla::Network::Instance();

  if (net != nullptr) {
    if (cfg->getWiFiSSID(buf)) {
      net->setSsid(buf);
    } else {
      supla_log(LOG_DEBUG, "Config incomplete: missing WiFi SSID");
      configIncomplete = true;
    }

    if (cfg->getWiFiPassword(buf)) {
      net->setPassword(buf);
    } else {
      supla_log(LOG_DEBUG, "Config incomplete: missing WiFi password");
      configIncomplete = true;
    }
  }

  if (configIncomplete) {
    supla_log(LOG_DEBUG, "Config incomplete: deviceMode = CONFIG");
    deviceMode = Supla::DEVICE_MODE_CONFIG;
  }
}

void SuplaDeviceClass::iterateAlwaysElements(unsigned long _millis) {
  uptime.iterate(_millis);

  // Iterate all elements
  for (auto element = Supla::Element::begin(); element != nullptr;
       element = element->next()) {
    element->iterateAlways();
    delay(0);
  }

  // Iterate all elements and saves state
  if (Supla::Storage::SaveStateAllowed(_millis)) {
    saveStateToStorage();
  }

}


bool SuplaDeviceClass::iterateNetworkSetup(unsigned long _millis) {
  // Restart network after >1 min of failed connection attempts
  if (connectionFailCounter > 30) {
    connectionFailCounter = 0;
    supla_log(LOG_DEBUG,
              "Connection fail counter overflow. Trying to setup network "
              "interface again");
    Supla::Network::Setup();
    return false;
  }

  if (!Supla::Network::IsReady()) {
    uptime.setConnectionLostCause(
        SUPLA_LASTCONNECTIONRESETCAUSE_WIFI_CONNECTION_LOST);
    waitForIterate = _millis + 100;
    status(STATUS_NETWORK_DISCONNECTED, "No connection to network");
    networkIsNotReadyCounter++;
    if (networkIsNotReadyCounter > 20) {
      networkIsNotReadyCounter = 0;
      connectionFailCounter++;
    }
    return false;
  }

  networkIsNotReadyCounter = 0;
  Supla::Network::Iterate();
  return true;
}

bool SuplaDeviceClass::iterateSuplaProtocol(unsigned int _millis) {
  if (srpc_iterate(srpc) == SUPLA_RESULT_FALSE) {
    status(STATUS_ITERATE_FAIL, "Iterate fail");
    Supla::Network::Disconnect();

    waitForIterate = _millis + 5000;
    return false;
  }

  if (registered == 0) {
    // Perform registration if we are not yet registered
    registered = -1;
    lastIterateTime = _millis;
    status(STATUS_REGISTER_IN_PROGRESS, "Register in progress");
    if (!srpc_ds_async_registerdevice_e(srpc, &Supla::Channel::reg_dev)) {
      supla_log(LOG_DEBUG, "Fatal SRPC failure!");
    }
    return false;
  } else if (registered == -1) {
    // Handle registration timeout (in case of no reply received)
    if (_millis - lastIterateTime > 10*1000) {
      supla_log(LOG_DEBUG, "No reply to registration message. Resetting connection.");
      status(STATUS_SERVER_DISCONNECTED, "Not connected to Supla server");
      Supla::Network::Disconnect();

      waitForIterate = _millis + 2000;
      connectionFailCounter++;
    }
    return false;
  } else if (registered == 1) {
    // Device is registered and everything is correct

    if (Supla::Network::Ping(srpc) == false) {
      uptime.setConnectionLostCause(
          SUPLA_LASTCONNECTIONRESETCAUSE_ACTIVITY_TIMEOUT);
      supla_log(LOG_DEBUG, "TIMEOUT - lost connection with server");
      status(STATUS_SERVER_DISCONNECTED, "Not connected to Supla server");
      Supla::Network::Disconnect();
    }

    return true;
  }
  return false;
}

void SuplaDeviceClass::enterConfigMode() {
  status(STATUS_CONFIG_MODE, "Config mode", true);
  if (deviceMode == Supla::DEVICE_MODE_CONFIG) {
    // if we enter cfg mode with deviceMode already set to cfgmode, then
    // configuration is incomplete, so there is no timeout to leave config
    // mode
    deviceRestartTimeoutTimestamp = 0;
  } else {
    deviceRestartTimeoutTimestamp = millis();
  }

  deviceMode = Supla::DEVICE_MODE_CONFIG;
  Supla::Network::Disconnect();
  Supla::Network::SetConfigMode();
  Supla::Network::Setup();
  // TODO start local http server
}

void SuplaDeviceClass::leaveConfigModeAndRestart() {
  supla_log(LOG_INFO, "Leaving config mode");
  deviceMode = Supla::DEVICE_MODE_NORMAL;
  saveStateToStorage();
  if (Supla::Storage::ConfigInstance()) {
    Supla::Storage::ConfigInstance()->commit();
  }

  // TODO stop supla timers

  Supla::Network::Uninit();
  supla_log(LOG_INFO, "Resetting in 0.5s...");
  delay(500);
  supla_log(LOG_INFO, "See you soon!");
  deviceSoftwareReset();
}

void SuplaDeviceClass::enterNormalMode() {
  supla_log(LOG_INFO, "Enter normal mode");
  // TODO stop local http server
  deviceMode = Supla::DEVICE_MODE_NORMAL;
  Supla::Network::SetNormalMode();
  Supla::Network::Setup();
}

void SuplaDeviceClass::setManufacurerId(_supla_int16_t id) {
  Supla::Channel::reg_dev.ManufacturerID = id;
}

void SuplaDeviceClass::setProductId(_supla_int16_t id) {
  Supla::Channel::reg_dev.ProductID = id;
}

void SuplaDeviceClass::addFlags(_supla_int_t newFlags) {
  Supla::Channel::reg_dev.Flags |= newFlags;
}

void SuplaDeviceClass::removeFlags(_supla_int_t flags) {
  Supla::Channel::reg_dev.Flags &= ~flags;
}

int SuplaDeviceClass::handleCalcfgFromServer(TSD_DeviceCalCfgRequest *request) {
  if (request) {
    switch (request->Command) {
      case SUPLA_CALCFG_CMD_ENTER_CFG_MODE: {
        supla_log(LOG_INFO, "CALCFG ENTER CFGMODE received");
        enterConfigMode();
        return SUPLA_CALCFG_RESULT_DONE;
      }
      default:
        break;
    }
  }
  return SUPLA_CALCFG_RESULT_NOT_SUPPORTED;
}

void SuplaDeviceClass::saveStateToStorage() {
  Supla::Storage::PrepareState();
  for (auto element = Supla::Element::begin(); element != nullptr;
      element = element->next()) {
    element->onSaveState();
    delay(0);
  }
  Supla::Storage::FinalizeSaveState();
}

int SuplaDeviceClass::generateHostname(char *buf, int size) {
  char name[32] = {};
  if (size < 0) {
    size = 0;
  }
  if (size > 6) {
    size = 6;
  }

  char *srcName = Supla::Channel::reg_dev.Name;

  int nameLength = strlen(Supla::Channel::reg_dev.Name);

  if (nameLength > 18) {
    nameLength = 18;
  }

  if (nameLength == 0) {
    setName("SuplaDevice");
    nameLength = strlen(Supla::Channel::reg_dev.Name);
  }

  int destIdx = 0;
  for (int i = 0; i < nameLength; i++) {
    if (srcName[i] < 32) {
      continue;
    } else if (srcName[i] < 48) {
      name[destIdx++] = '-';
    } else if (srcName[i] < 58) {
      name[destIdx++] = srcName[i]; // copy numbers
    } else if (srcName[i] < 65) {
      name[destIdx++] = '-';
    } else if (srcName[i] < 91) {
      name[destIdx++] = srcName[i]; // copy capital chars
    } else if (srcName[i] < 97) {
      name[destIdx++] = '-';
    } else if (srcName[i] < 123) {
      name[destIdx++] = srcName[i] - 32; // capitalize small chars
    }
  }

  uint8_t mac[6] = {};
  name[destIdx++] = '-';

  if (Supla::Network::GetMacAddr(mac)) {
    destIdx += generateHexString(mac + (6 - size), &(name[destIdx]), (6 - size));
  }

  name[destIdx++] = 0;
  strncpy(buf, name, 32);

  return destIdx;
}

void SuplaDeviceClass::disableCfgModeTimeout() {
  if (!forceRestartTimeMs) {
    deviceRestartTimeoutTimestamp = 0;
  }
}

void SuplaDeviceClass::scheduleLeaveConfigMode(int timeout) {
  if (timeout <= 0) {
    forceRestartTimeMs = 1;
  } else {
    forceRestartTimeMs = timeout;
  }
  deviceRestartTimeoutTimestamp = millis();
}

SuplaDeviceClass SuplaDevice;
