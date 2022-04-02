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

#include <gtest/gtest.h>
#include <supla/tools.h>

TEST(ToolsTests, isArrayEmptyTests) {
  char buf[10] = {};
  char buf2[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
  char buf3[10] = {40, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  char buf4[1000] = {2};
  char buf5[1000] = {};

  EXPECT_TRUE(isArrayEmpty(buf, 10));
  buf[4] = 3;
  EXPECT_FALSE(isArrayEmpty(buf, 10));

  EXPECT_FALSE(isArrayEmpty(buf2, 10));
  EXPECT_TRUE(isArrayEmpty(buf2, 9));

  EXPECT_FALSE(isArrayEmpty(buf3, 10));
  EXPECT_FALSE(isArrayEmpty(buf3, 9));

  EXPECT_FALSE(isArrayEmpty(buf4, 1000));
  EXPECT_TRUE(isArrayEmpty(buf5, 1000));
}

TEST(ToolsTests, adjustRangeTests) {
  EXPECT_EQ(adjustRange(10, 0, 10, 50, 100), 100);
  EXPECT_EQ(adjustRange(5, 0, 10, 50, 100), 75);
  EXPECT_EQ(adjustRange(0, 0, 10, 50, 100), 50);

  EXPECT_EQ(adjustRange(0, 0, 10, -50, -100), -50);
  EXPECT_EQ(adjustRange(0, 0, 10, -100, -50), -100);

  EXPECT_EQ(adjustRange(-5, 0, 10, 50, 100), 25);
}

TEST(ToolsTests, generateHexStringTests) {
  char buf[200];
  EXPECT_EQ(generateHexString("SUPLA", buf, 5), 10);
  EXPECT_STREQ(buf, "5355504C41");
  EXPECT_EQ(generateHexString("SUPLA", buf, 5, ':'), 14);
  EXPECT_STREQ(buf, "53:55:50:4C:41");
  EXPECT_EQ(generateHexString("", buf, 0, ':'), 0);
  EXPECT_STREQ(buf, "");
  EXPECT_EQ(generateHexString("", buf, 1, ':'), 2);
  EXPECT_STREQ(buf, "00");
}

TEST(ToolsTests, hexStringToIntTests) {
  EXPECT_EQ(hexStringToInt("10", 2), 16);
  EXPECT_EQ(hexStringToInt("A0", 2), 160);
  EXPECT_EQ(hexStringToInt("a5", 2), 165);
  EXPECT_EQ(hexStringToInt("5", 1), 5);
  EXPECT_EQ(hexStringToInt("05", 2), 5);
  EXPECT_EQ(hexStringToInt("0a", 2), 10);
  EXPECT_EQ(hexStringToInt("0A", 2), 10);
  EXPECT_EQ(hexStringToInt("FF", 2), 255);
  EXPECT_EQ(hexStringToInt("fF", 2), 255);
  EXPECT_EQ(hexStringToInt("ff", 2), 255);
  EXPECT_EQ(hexStringToInt("0ff", 3), 255);
  EXPECT_EQ(hexStringToInt("1ff", 3), 511);
  EXPECT_EQ(hexStringToInt("45FAc21", 7), 73378849);

  // uint32_t max
  EXPECT_EQ(hexStringToInt("FFFFFFFF", 8), 4294967295);
  // out of uint32_t limit call
  hexStringToInt("FFFFFFFF2", 9);
}

TEST(ToolsTest, urlDecodeInplaceTests) {
  {
    char buf[] = "ala+ma+supla";
    urlDecodeInplace(buf, sizeof(buf) - 1);
    EXPECT_STREQ(buf, "ala ma supla");
  }

  {
    char buf[] = "ala+ma%20supla";
    urlDecodeInplace(buf, sizeof(buf) - 1);
    EXPECT_STREQ(buf, "ala ma supla");
  }

  {
    char buf[] = "%61la ma+supla%1";
    urlDecodeInplace(buf, sizeof(buf) - 1);
    EXPECT_STREQ(buf, "ala ma supla");
  }

  {
    char buf[] = "ala+ma+supla";
    urlDecodeInplace(buf, sizeof(buf) - 1);
    EXPECT_STREQ(buf, "ala ma supla");
  }

  {
    char buf[] = "ala+ma+supla";
    urlDecodeInplace(buf, sizeof(buf) - 1);
    EXPECT_STREQ(buf, "ala ma supla");
  }

}
