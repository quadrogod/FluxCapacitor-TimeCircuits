//
// Created by Alexey Molchanov on 12.11.2025.
//

#ifndef FLUXCAPACITOR_TIMECIRCUITS_TCDATETIME_H
#define FLUXCAPACITOR_TIMECIRCUITS_TCDATETIME_H

#include <Arduino.h>

struct TCDateTime {
    int m, d, y, h, min;
    bool valid;

    TCDateTime() { m=d=y=h=min=0; valid=false; }

    String toText() const {
        char buf[20];
        snprintf(buf, sizeof(buf), "%02d.%02d.%04d %02d:%02d", d, m, y, h, min);
        return String(buf);
    }
};

#endif //FLUXCAPACITOR_TIMECIRCUITS_TCDATETIME_H