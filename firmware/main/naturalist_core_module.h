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
    FlashStorage<PersistedState> flashStorage{ watchdog };
    CoreState state{ flashStorage, fileSystem.getData() };
    ModuleCommunications moduleCommunications{ bus, modulesPool };

    // Readings stuff.
    NaturalistReadings readings{ state };

    // Scheduler stuff.
    PeriodicTask noop{ 0, { CoreFsm::deferred<WifiStartup>() } };
    lwcron::Task *tasks[1] {
        &noop
    };
    lwcron::Scheduler scheduler{tasks};

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
    AppServicer appServicer{state, fileSystem.getReplies(), connection, moduleCommunications, appPool};
    Wifi wifi{connection};
    Discovery discovery;

    // GPS stuff
    SerialPort gpsSerial{ Hardware::gpsUart };
    GpsService gps{ state, gpsSerial };

    LiveDataManager liveData;

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
        &appServicer,
        &liveData
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
