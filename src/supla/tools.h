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

#ifndef _tools_H_
#define _tools_H_

#include <stddef.h>
#include <stdint.h>
#include "definitions.h"
#include "supla/IEEE754tools.h"

void float2DoublePacked(float number, uint8_t *bar, int byteOrder = LSBFIRST);
float doublePacked2float(uint8_t *bar);

long adjustRange(long input, long inMin, long inMax, long outMin, long outMax);

bool isArrayEmpty(void* array, size_t arraySize);

// Converts inputLength bytes from input to HEX and adds bytes separator
// if required.
// output buffor has to be at least (2 * inputLength + 1) bytes long without
// separator, or: (3 * inputLength) bytes long with separator.
// Trailing '\0' is added.
// Returns amount of non-null chars written.
int generateHexString(const void *input,
    char *output,
    int inputLength,
    char separator = 0);

// Converts hex string value to integer
uint32_t hexStringToInt(const char *str, int len);

// Converts hex string value to integer
uint32_t stringToUInt(const char *str, int len = -1);


// Decode url string from buffer into buffer (inplace)
// Replace '+' with ' '.
// Replace %xy with proper byte.
// If not complete % parameter is found at the end, then it is omitted.
void urlDecodeInplace(char *buffer, int size);

void deviceSoftwareReset();

#endif
