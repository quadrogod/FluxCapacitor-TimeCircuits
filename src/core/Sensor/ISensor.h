//
// Created by Alexey Molchanov on 12.11.2025.
//

#ifndef FLUXCAPACITOR_TIMECIRCUITS_ISENSOR_H
#define FLUXCAPACITOR_TIMECIRCUITS_ISENSOR_H

class ISensor {
public:
    virtual ~ISensor() {}

    virtual float getValue() = 0;
    virtual float getProgress() = 0;
    virtual bool shouldTrigger() = 0;
    virtual bool isActive() = 0;
    virtual void update() = 0;
};

#endif //FLUXCAPACITOR_TIMECIRCUITS_ISENSOR_H