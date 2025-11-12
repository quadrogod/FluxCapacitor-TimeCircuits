//
// Created by Alexey Molchanov on 12.11.2025.
//

#ifndef FLUXCAPACITOR_TIMECIRCUITS_IRTCPROVIDER_H
#define FLUXCAPACITOR_TIMECIRCUITS_IRTCPROVIDER_H

#include "Globals.h"

class IRTCProvider {
public:
    virtual ~IRTCProvider() {}

    virtual bool init() = 0;
    virtual bool isAvailable() = 0;
    virtual TCDateTime getCurrentTime() = 0;
    virtual void setTime(const TCDateTime& dt) = 0;
    virtual bool hasMinuteTick() = 0;
    virtual bool isSupported(const TCDateTime& dt) = 0;
};

#endif //FLUXCAPACITOR_TIMECIRCUITS_IRTCPROVIDER_H