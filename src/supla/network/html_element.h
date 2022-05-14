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

#ifndef _supla_html_element_h_
#define _supla_html_element_h_

namespace Supla {

class WebSender;

enum HtmlSection {
  HTML_SECTION_FORM,
  HTML_SECTION_DEVICE_INFO,
  HTML_SECTION_NETWORK,
  HTML_SECTION_PROTOCOL,
  HTML_SECTION_BETA_FORM
};


class HtmlElement {
  public:
    static HtmlElement *begin();
    static HtmlElement *last();

    static const char *selected(bool isSelected);

    HtmlElement *next();

    HtmlElement(HtmlSection section = HTML_SECTION_FORM);
    virtual ~HtmlElement();
    virtual void send(Supla::WebSender *sender) = 0;
    virtual bool handleResponse(const char* key, const char* value);
    HtmlSection section;
  protected:
    static HtmlElement *firstPtr;
    HtmlElement *nextPtr = nullptr;
};

};

#endif