#ifndef CHECK_CORE_H_INCLUDED
#define CHECK_CORE_H_INCLUDED

#include <fk-core.h>
#include <battery_gauge.h>

namespace fk {

class CheckCore {
private:
    Leds leds_;
    BatteryGauge gauge_;
    uint32_t checked_{ 0 };
    uint32_t toggled_{ 0 };
    float previous_{ 0.0f };
    bool enabled_{ true };
    bool success_{ false };

public:
    Leds& leds() {
        return leds_;
    }

    bool success() {
        return success_;
    }

    bool enabled() {
        return enabled_;
    }

    virtual bool toggle_peripherals() {
        return true;
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
    virtual void sample();

private:
    const char *id2chip(const unsigned char *id);

};

}

#endif
