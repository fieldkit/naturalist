#ifndef FK_NATURALIST_CORE_MODULE_H_INCLUDED
#define FK_NATURALIST_CORE_MODULE_H_INCLUDED

#include <fk-core.h>

#include "readings.h"

namespace fk {

class NaturalistCoreModule {
private:
    StaticPool<384> appPool{"AppPool"};
    StaticPool<384> modulesPool{"ModulesPool"};
    StaticPool<128> dataPool{"DataPool"};

    // Main services.
    Leds leds;
    Power power{ state };
    Watchdog watchdog{ leds };
    UserButton button{ leds };
    Status status{ state, bus, leds };
    TwoWireBus bus{ Wire };
    FileSystem fileSystem{ bus, dataPool };
    FlashStorage<PersistedState> flashStorage;
    CoreState state{ flashStorage, fileSystem.getData() };
    ModuleCommunications moduleCommunications{ bus, modulesPool };

    // Readings stuff.
    NaturalistReadings readings{ state };

    // Scheduler stuff.
    PeriodicTask periodics[4] {
        fk::PeriodicTask{ 20 * 1000,     CoreFsm::deferred<IgnoredState>() },
        fk::PeriodicTask{ 30 * 1000,     CoreFsm::deferred<IgnoredState>() },
        fk::PeriodicTask{ 60 * 1000 * 5, CoreFsm::deferred<IgnoredState>() },
        #ifdef FK_ENABLE_RADIO
        fk::PeriodicTask{ 60 * 1000,     CoreFsm::deferred<IgnoredState>() },
        #else
        fk::PeriodicTask{ 60 * 1000,     CoreFsm::deferred<IgnoredState>() },
        #endif
    };
    Scheduler scheduler{clock, periodics};

    // Radio stuff.
    #ifdef FK_ENABLE_RADIO
    RadioService radioService;
    SendDataToLoraGateway sendDataToLoraGateway{ radioService, fileSystem, { FileNumber::Data } };
    #endif

    // Wifi stuff.
    HttpTransmissionConfig httpConfig = {
        .streamUrl = WifiApiUrlIngestionStream,
    };
    WifiConnection connection;
    AppServicer appServicer{state, scheduler, fileSystem.getReplies(), connection, moduleCommunications, appPool};
    Wifi wifi{connection};
    Discovery discovery;

    // GPS stuff
    SerialPort gpsSerial{ Hardware::gpsUart };
    GpsService gps{ state, gpsSerial };

    // Service collections.
    MainServices mainServices{
        &leds,
        &watchdog,
        &power,
        &status,
        &state,
        &fileSystem,
        &button,
        &scheduler,
        &moduleCommunications,
        &gps
    };

    WifiServices wifiServices{
        &leds,
        &watchdog,
        &power,
        &status,
        &state,
        &fileSystem,
        &button,
        &scheduler,
        &moduleCommunications,
        &gps,

        &wifi,
        &discovery,
        &httpConfig,
        &wifi.server(),
        &appServicer
    };

public:
    void begin();
    void run();

public:
    CoreState &getState() {
        return state;
    }

};

}

#endif
