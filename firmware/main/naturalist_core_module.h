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

    Supervisor<5> background{ true };
    Supervisor<5> servicing{ true };

    TwoWireBus bus{ Wire };
    FileSystem fileSystem{ bus, dataPool };
    Watchdog watchdog{ leds };
    Power power{ state };
    SerialFlashChip serialFlash;
    FlashStorage storage{ serialFlash };
    CoreState state{ storage, fileSystem.getData() };
    Leds leds;
    NaturalistReadings readings{ state };
    SerialPort gpsPort{ Serial2 };
    ReadGps readGps{ state, gpsPort };
    NoopTask noop;

    HttpTransmissionConfig transmissionConfig = {
        .streamUrl = API_INGESTION_STREAM,
    };
    TransmitAllFilesTask transmitAllFilesTask{background, fileSystem, state, wifi, transmissionConfig};

    PeriodicTask periodics[4] {
        fk::PeriodicTask{ 20 * 1000, readGps },
        fk::PeriodicTask{ 30 * 1000, readings },
        fk::PeriodicTask{ 60 * 1000, transmitAllFilesTask },
        #ifdef FK_ENABLE_RADIO
        fk::PeriodicTask{ 60 * 1000, sendDataToLoraGateway  },
        #else
        fk::PeriodicTask{ 60 * 1000, noop },
        #endif
    };
    Scheduler scheduler{state, clock, background, periodics};
    LiveData liveData{readings, state};
    WifiConnection connection;
    ModuleCommunications moduleCommunications{ bus, background, modulesPool };
    AppServicer appServicer{bus, liveData, state, scheduler, fileSystem.getReplies(), connection, moduleCommunications, appPool};
    Wifi wifi{state, connection, appServicer, servicing};
    Discovery discovery{ bus, wifi };

    #ifdef FK_ENABLE_RADIO
    RadioService radioService;
    SendDataToLoraGateway sendDataToLoraGateway{ radioService, fileSystem, 0 };
    #endif

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
