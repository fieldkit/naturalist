#ifndef CHECK_CORE_H_INCLUDED
#define CHECK_CORE_H_INCLUDED

#include <fk-core.h>
#include <battery_gauge.h>

namespace fk {

class CheckCore {
private:
    fk::Leds leds_;

public:
    fk::Leds& leds() {
        return leds_;
    }

public:
    void setup();
    bool fuelGauge();
    bool flashMemory();
    bool gps();
    bool sdCard();
    bool radio();
    bool wifi();
    bool rtc();
    bool macEeprom();

public:
    virtual bool check();
    virtual void task();

private:
    const char *id2chip(const unsigned char *id);

};

}

#endif
