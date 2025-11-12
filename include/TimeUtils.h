//
// Created by Alexey Molchanov on 12.11.2025.
//

#ifndef FLUXCAPACITOR_TIMECIRCUITS_TIMEUTILS_H
#define FLUXCAPACITOR_TIMECIRCUITS_TIMEUTILS_H

#include "TCDateTime.h"

bool isLeapYear(int y);
bool isDateValid(int M, int D, int Y, int h, int m);
TCDateTime parseDateTime(const String& s);
void convertTo12Hour(int h24, int& h12out, bool& pm);

#endif //FLUXCAPACITOR_TIMECIRCUITS_TIMEUTILS_H