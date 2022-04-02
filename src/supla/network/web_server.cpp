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

#include "web_server.h"
#include <supla-common/log.h>
#include "supla/network/html_generator.h"
#include "supla/network/network.h"
#include <supla/storage/storage.h>
#include <SuplaDevice.h>
#include <string.h>
#include <supla/tools.h>
#include <supla/network/html_element.h>

const unsigned char Supla::favico[] = {
  0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x10, 0x10, 0x00, 0x00, 0x01, 0x00,
  0x20, 0x00, 0x68, 0x04, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x28, 0x00,
  0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x04, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x50, 0xD3, 0x00, 0x5C, 0x50, 0xD2, 0x00, 0xAF, 0x50, 0xD2,
  0x00, 0xE3, 0x50, 0xD2, 0x00, 0xFD, 0x51, 0xD4, 0x00, 0xFF, 0x51, 0xD5,
  0x00, 0xFF, 0x50, 0xD3, 0x00, 0xFD, 0x50, 0xD2, 0x00, 0xE3, 0x50, 0xD2,
  0x00, 0xAF, 0x50, 0xD3, 0x00, 0x5C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x51, 0xD1,
  0x00, 0x16, 0x50, 0xD2, 0x00, 0xBD, 0x50, 0xD2, 0x00, 0xFF, 0x50, 0xD2,
  0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFF, 0x4F, 0xD0, 0x00, 0xFF, 0x3E, 0xA6,
  0x00, 0xFF, 0x38, 0x98, 0x00, 0xFF, 0x4B, 0xC5, 0x00, 0xFF, 0x50, 0xD3,
  0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFF, 0x50, 0xD2,
  0x00, 0xBD, 0x51, 0xD1, 0x00, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x50, 0xD2, 0x00, 0xBD, 0x50, 0xD2, 0x00, 0xFF, 0x50, 0xD2,
  0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFF, 0x51, 0xD4, 0x00, 0xFF, 0x40, 0xAD,
  0x00, 0xFF, 0x30, 0x85, 0x00, 0xFF, 0x3A, 0x9D, 0x00, 0xFF, 0x32, 0x8A,
  0x00, 0xFF, 0x50, 0xD3, 0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFF, 0x50, 0xD2,
  0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFF, 0x50, 0xD2, 0x00, 0xBD, 0x00, 0x00,
  0x00, 0x00, 0x50, 0xD3, 0x00, 0x5C, 0x50, 0xD2, 0x00, 0xFF, 0x50, 0xD2,
  0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFF, 0x51, 0xD4,
  0x00, 0xFF, 0x41, 0xAD, 0x00, 0xFF, 0x2E, 0x83, 0x00, 0xFF, 0x37, 0x98,
  0x00, 0xFF, 0x31, 0x86, 0x00, 0xFF, 0x51, 0xD4, 0x00, 0xFF, 0x50, 0xD2,
  0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFF, 0x50, 0xD2,
  0x00, 0xFF, 0x50, 0xD3, 0x00, 0x5C, 0x50, 0xD2, 0x00, 0xAF, 0x50, 0xD2,
  0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFF, 0x50, 0xD2,
  0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFF, 0x4D, 0xCA, 0x00, 0xFF, 0x29, 0x76,
  0x00, 0xFF, 0x33, 0x8D, 0x00, 0xFF, 0x31, 0x87, 0x00, 0xFF, 0x4C, 0xC7,
  0x00, 0xFF, 0x51, 0xD3, 0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFF, 0x50, 0xD2,
  0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFF, 0x50, 0xD2, 0x00, 0xAF, 0x50, 0xD2,
  0x00, 0xE3, 0x50, 0xD2, 0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFF, 0x50, 0xD2,
  0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFF, 0x51, 0xD4, 0x00, 0xFF, 0x43, 0xB4,
  0x00, 0xFF, 0x32, 0x8A, 0x00, 0xFF, 0x52, 0xD7, 0x00, 0xFF, 0x41, 0xAF,
  0x00, 0xFF, 0x2E, 0x81, 0x00, 0xFF, 0x4C, 0xC8, 0x00, 0xFF, 0x4F, 0xD0,
  0x00, 0xFF, 0x51, 0xD4, 0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFF, 0x50, 0xD2,
  0x00, 0xE3, 0x50, 0xD2, 0x00, 0xFD, 0x50, 0xD2, 0x00, 0xFF, 0x50, 0xD2,
  0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFF, 0x51, 0xD5,
  0x00, 0xFF, 0x32, 0x8A, 0x00, 0xFF, 0x38, 0x9A, 0x00, 0xFF, 0x51, 0xD5,
  0x00, 0xFF, 0x51, 0xD4, 0x00, 0xFF, 0x37, 0x97, 0x00, 0xFF, 0x1B, 0x55,
  0x00, 0xFF, 0x22, 0x65, 0x00, 0xFF, 0x38, 0x98, 0x00, 0xFF, 0x50, 0xD2,
  0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFD, 0x50, 0xD2, 0x00, 0xFF, 0x50, 0xD2,
  0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFF, 0x50, 0xD3,
  0x00, 0xFF, 0x4E, 0xCE, 0x00, 0xFF, 0x21, 0x62, 0x00, 0xFF, 0x44, 0xB6,
  0x00, 0xFF, 0x51, 0xD4, 0x00, 0xFF, 0x51, 0xD5, 0x00, 0xFF, 0x3E, 0xA6,
  0x00, 0xFF, 0x22, 0x65, 0x00, 0xFF, 0x43, 0xB3, 0x00, 0xFF, 0x1E, 0x5A,
  0x00, 0xFF, 0x49, 0xC0, 0x00, 0xFF, 0x50, 0xD3, 0x00, 0xFF, 0x50, 0xD2,
  0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFF, 0x51, 0xD4,
  0x00, 0xFF, 0x4E, 0xCE, 0x00, 0xFF, 0x3D, 0xA3, 0x00, 0xFF, 0x1A, 0x53,
  0x00, 0xFF, 0x4E, 0xCD, 0x00, 0xFF, 0x51, 0xD4, 0x00, 0xFF, 0x50, 0xD3,
  0x00, 0xFF, 0x35, 0x92, 0x00, 0xFF, 0x19, 0x4F, 0x00, 0xFF, 0x31, 0x86,
  0x00, 0xFF, 0x20, 0x61, 0x00, 0xFF, 0x4C, 0xC7, 0x00, 0xFF, 0x50, 0xD3,
  0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFD, 0x50, 0xD2, 0x00, 0xFF, 0x50, 0xD3,
  0x00, 0xFF, 0x3C, 0xA1, 0x00, 0xFF, 0x17, 0x4A, 0x00, 0xFF, 0x0B, 0x2E,
  0x00, 0xFF, 0x0A, 0x2A, 0x00, 0xFF, 0x3D, 0xA4, 0x00, 0xFF, 0x44, 0xB6,
  0x00, 0xFF, 0x2C, 0x7D, 0x00, 0xFF, 0x32, 0x8A, 0x00, 0xFF, 0x3C, 0xA0,
  0x00, 0xFF, 0x32, 0x8A, 0x00, 0xFF, 0x45, 0xB9, 0x00, 0xFF, 0x50, 0xD3,
  0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFD, 0x50, 0xD2, 0x00, 0xE3, 0x51, 0xD4,
  0x00, 0xFF, 0x44, 0xB7, 0x00, 0xFF, 0x0E, 0x36, 0x00, 0xFF, 0x30, 0x85,
  0x00, 0xFF, 0x45, 0xB8, 0x00, 0xFF, 0x30, 0x84, 0x00, 0xFF, 0x09, 0x28,
  0x00, 0xFF, 0x1F, 0x5E, 0x00, 0xFF, 0x3F, 0xA8, 0x00, 0xFF, 0x50, 0xD2,
  0x00, 0xFF, 0x51, 0xD5, 0x00, 0xFF, 0x51, 0xD5, 0x00, 0xFF, 0x51, 0xD3,
  0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFF, 0x50, 0xD2, 0x00, 0xE3, 0x50, 0xD2,
  0x00, 0xAF, 0x51, 0xD5, 0x00, 0xFF, 0x33, 0x8C, 0x00, 0xFF, 0x19, 0x4F,
  0x00, 0xFF, 0x50, 0xD3, 0x00, 0xFF, 0x51, 0xD4, 0x00, 0xFF, 0x50, 0xD1,
  0x00, 0xFF, 0x15, 0x47, 0x00, 0xFF, 0x34, 0x8F, 0x00, 0xFF, 0x52, 0xD7,
  0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFF, 0x50, 0xD2,
  0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFF, 0x50, 0xD2,
  0x00, 0xAF, 0x50, 0xD0, 0x00, 0x5C, 0x51, 0xD5, 0x00, 0xFF, 0x38, 0x98,
  0x00, 0xFF, 0x13, 0x40, 0x00, 0xFF, 0x4C, 0xC7, 0x00, 0xFF, 0x52, 0xD7,
  0x00, 0xFF, 0x4B, 0xC5, 0x00, 0xFF, 0x11, 0x3D, 0x00, 0xFF, 0x3A, 0x9E,
  0x00, 0xFF, 0x51, 0xD5, 0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFF, 0x50, 0xD2,
  0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFF, 0x50, 0xD2,
  0x00, 0xFF, 0x50, 0xD3, 0x00, 0x5C, 0x00, 0x00, 0x00, 0x00, 0x50, 0xD2,
  0x00, 0xBD, 0x4C, 0xC8, 0x00, 0xFF, 0x1B, 0x53, 0x00, 0xFF, 0x18, 0x4C,
  0x00, 0xFF, 0x2B, 0x7A, 0x00, 0xFF, 0x17, 0x49, 0x00, 0xFF, 0x1C, 0x58,
  0x00, 0xFF, 0x4D, 0xCB, 0x00, 0xFF, 0x50, 0xD3, 0x00, 0xFF, 0x50, 0xD2,
  0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFF, 0x50, 0xD2,
  0x00, 0xFF, 0x50, 0xD2, 0x00, 0xBD, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x55, 0xDB, 0x00, 0x15, 0x50, 0xD2, 0x00, 0xBD, 0x4B, 0xC5,
  0x00, 0xFF, 0x31, 0x86, 0x00, 0xFF, 0x23, 0x68, 0x00, 0xFF, 0x32, 0x8A,
  0x00, 0xFF, 0x4C, 0xC7, 0x00, 0xFF, 0x50, 0xD3, 0x00, 0xFF, 0x50, 0xD2,
  0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFF, 0x50, 0xD2,
  0x00, 0xFF, 0x50, 0xD2, 0x00, 0xBD, 0x51, 0xD1, 0x00, 0x16, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x50, 0xD3, 0x00, 0x5C, 0x52, 0xD6, 0x00, 0xAF, 0x51, 0xD5,
  0x00, 0xE3, 0x51, 0xD6, 0x00, 0xFD, 0x50, 0xD3, 0x00, 0xFF, 0x50, 0xD2,
  0x00, 0xFF, 0x50, 0xD2, 0x00, 0xFD, 0x50, 0xD2, 0x00, 0xE3, 0x50, 0xD2,
  0x00, 0xAF, 0x50, 0xD3, 0x00, 0x5C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

Supla::WebServer::WebServer(Supla::HtmlGenerator *generator) {
  if (generator) {
    htmlGenerator = generator;
  } else {
    destroyGenerator = true;
    htmlGenerator = new HtmlGenerator;
  }
}

Supla::WebServer::~WebServer() {
  if (destroyGenerator) {
    delete htmlGenerator;
  }
}

void Supla::WebServer::setSuplaDeviceClass(SuplaDeviceClass *ptr) {
  sdc = ptr;
}

void Supla::WebServer::notifyClientConnected() {
  if (sdc) {
    sdc->disableCfgModeTimeout();
  }
}

void Supla::WebServer::parsePost(const char* postContent,
    int size,
    bool lastChunk) {
  if (postContent == nullptr || size <= 0) {
    resetParser();
    return;
  }

  const char* startPtr = postContent;
  const char* endPtr = &postContent[size + 1];

  for (auto ptr = startPtr; ptr != endPtr; ptr++) {
    if (!keyFound && ((*ptr) == '=' || (ptr + 1) == endPtr)) {
      ptrdiff_t keyLength = (ptr - startPtr);
      if (keyLength > HTML_KEY_LENGTH - 1 - partialSize) {
        keyLength = HTML_KEY_LENGTH - 1 - partialSize;
      }
      memcpy(key + partialSize, startPtr, keyLength);
      startPtr = ptr + 1;
      if (*ptr == '=') {
        keyFound = true;
        partialSize = 0;
      } else {
        partialSize += keyLength;
      }
    } else if (keyFound && ((*ptr) == '&' || (ptr + 1) == endPtr)) {
      ptrdiff_t valLength = ptr - startPtr;
      if (valLength > HTML_VAL_LENGTH - 1 - partialSize) {
        valLength = HTML_VAL_LENGTH - 1 - partialSize;
      }
      memcpy(value + partialSize, startPtr, valLength);
      startPtr = ptr + 1;
      if ((*ptr) == '&' || lastChunk) {
        keyFound = false;
        // keys are alfanumeric only so we don't decode them. Values
        // are provided by user, so those have to be decoded
        urlDecodeInplace(value, HTML_VAL_LENGTH);
        // handle key value here
        supla_log(LOG_DEBUG, "SERVER: key %s, value %s", key, value);
        for (auto htmlElement = Supla::HtmlElement::begin(); htmlElement;
            htmlElement = htmlElement->next()) {
          if (htmlElement->handleResponse(key, value)) {
            break;
          }
        }
        if (strcmp(key, "rbt") == 0) {
          int reboot = stringToUInt(value);
          supla_log(LOG_DEBUG, "rbt found %d", reboot);
          if (reboot == 2) {
            sdc->scheduleLeaveConfigMode(2500);
          } else if (reboot) {
            sdc->scheduleLeaveConfigMode();
          }
        }
        partialSize = 0;
        memset(key, 0, HTML_KEY_LENGTH);
        memset(value, 0, HTML_VAL_LENGTH);
      } else {
        partialSize += valLength;
      }
    }
  }


  if (lastChunk) {
    if (Supla::Storage::ConfigInstance()) {
      Supla::Storage::ConfigInstance()->commit();
    }
    resetParser();
  }
}

void Supla::WebServer::resetParser() {
  partialSize = 0;
  keyFound = false;
  memset(key, 0 , HTML_KEY_LENGTH);
  memset(value, 0 , HTML_VAL_LENGTH);

}

