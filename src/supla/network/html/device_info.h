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

#ifndef _supla_html_device_info_h_
#define _supla_html_device_info_h_

#include <supla/network/html_element.h>

class SuplaDeviceClass;

namespace Supla {

namespace Html {

class DeviceInfo : public HtmlElement {
  public:
    DeviceInfo(SuplaDeviceClass *sdc);
    virtual ~DeviceInfo();
    virtual void send(Supla::WebSender *sender);
//    virtual bool handleResponse() = 0;
  protected:
    SuplaDeviceClass *sdc;
};

};
};

#endif
