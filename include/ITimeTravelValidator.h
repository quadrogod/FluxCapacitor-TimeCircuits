//
// Created by Alexey Molchanov on 13.11.2025.
//

#ifndef FLUXCAPACITOR_TIMECIRCUITS_ITIMETRAVELVALIDATOR_H
#define FLUXCAPACITOR_TIMECIRCUITS_ITIMETRAVELVALIDATOR_H

class ITimeTravelValidator {
public:
    virtual ~ITimeTravelValidator() = default;

    // Проверяет, можно ли выполнить полное путешествие во времени
    virtual bool canPerformFullTimeTravel() const = 0;

    virtual void performTimeTravel() = 0;
};

#endif //FLUXCAPACITOR_TIMECIRCUITS_ITIMETRAVELVALIDATOR_H