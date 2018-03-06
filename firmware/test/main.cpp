#include <Wire.h>
#include <SPI.h>
#include <wiring_private.h>
#include <cstdarg>
#include <FuelGauge.h>
#include <WiFi101.h>
#include <SD.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <Adafruit_TSL2561_U.h>
#include <Adafruit_MPL3115A2.h>
#include <Adafruit_TSL2591.h>
#include <Adafruit_SHT31.h>
#include <SerialFlash.h>
#include <RTClib.h>

#include "debug.h"

const uint8_t PIN_SD_CS = 12;
const uint8_t PIN_WINC_CS = 7;
const uint8_t PIN_WINC_IRQ = 16;
const uint8_t PIN_WINC_RST = 15;
const uint8_t PIN_WINC_EN = 38;
const uint8_t PIN_WINC_WAKE = 8;

Uart Serial2(&sercom1, 11, 10, SERCOM_RX_PAD_0, UART_TX_PAD_2);

void SERCOM1_Handler()
{
    Serial2.IrqHandler();
}

void platformSerial2Begin(int32_t baud) {
    Serial2.begin(baud);

    // Order is very important here. This has to happen after the call to begin.
    pinPeripheral(10, PIO_SERCOM);
    pinPeripheral(11, PIO_SERCOM);
}

class ModuleHardware {
public:
    static constexpr uint8_t PIN_FLASH_CS = 5;

public:
    TwoWire bno055Wire{ &sercom2, 4, 3 };
    Adafruit_SHT31 sht31Sensor;
    Adafruit_MPL3115A2 mpl3115a2Sensor;
    Adafruit_TSL2591 tsl2591Sensor{ 2591 };
    Adafruit_BNO055 bnoSensor{ 55, BNO055_ADDRESS_A, &bno055Wire };
    SerialFlashChip serialFlash;

public:
    void setup() {
        SPI.begin();
        Wire.begin();

        bno055Wire.begin();

        pinPeripheral(4, PIO_SERCOM_ALT);
        pinPeripheral(3, PIO_SERCOM_ALT);

        pinMode(A3, OUTPUT);
        pinMode(A4, OUTPUT);
        pinMode(A5, OUTPUT);
    }

    void leds(bool on) {
        digitalWrite(A3, on ? HIGH : LOW);
        digitalWrite(A4, on ? HIGH : LOW);
        digitalWrite(A5, on ? HIGH : LOW);
    }

};

class Sensors {
private:
    ModuleHardware *hw;

public:
    Sensors(ModuleHardware &hw) : hw(&hw) {
    }

public:
    void takeReading() {
        auto started = millis();

        auto shtTemperature = hw->sht31Sensor.readTemperature();
        auto shtHumidity = hw->sht31Sensor.readHumidity();

        auto pressurePascals = hw->mpl3115a2Sensor.getPressure();
        auto altitudeMeters = hw->mpl3115a2Sensor.getAltitude();
        auto mplTempCelsius = hw->mpl3115a2Sensor.getTemperature();
        auto pressureInchesMercury = pressurePascals / 3377.0;

        auto fullLuminosity = hw->tsl2591Sensor.getFullLuminosity();
        auto ir = fullLuminosity >> 16;
        auto full = fullLuminosity & 0xFFFF;
        auto lux = hw->tsl2591Sensor.calculateLux(full, ir);

        uint8_t system = 0, gyro = 0, accel = 0, mag = 0;
        hw->bnoSensor.getCalibration(&system, &gyro, &accel, &mag);

        sensors_event_t event;
        hw->bnoSensor.getEvent(&event);

        debugfln("sensors: %fC %f%%, %fC %fpa %f\"/Hg %fm", shtTemperature, shtHumidity, mplTempCelsius, pressurePascals, pressureInchesMercury, altitudeMeters);
        debugfln("sensors: ir(%d) full(%d) visible(%d) lux(%d)", ir, full, full - ir, lux);
        debugfln("sensors: cal(%d, %d, %d, %d) xyz(%f, %f, %f)", system, gyro, accel, mag, event.orientation.x, event.orientation.y, event.orientation.z);
    }
};

class MacEeprom {
private:
    uint8_t address;

public:
    MacEeprom() : address(0x50) {
    }

public:
    bool read128bMac(uint8_t *id) {
        Wire.beginTransmission(address);
        Wire.write(0xf8);
        Wire.endTransmission();
        Wire.requestFrom(address, 8);

        uint8_t index = 0;
        while (Wire.available()){
            id[index++] = Wire.read();
        }

        return true;
    }
};

class Check {
private:
    ModuleHardware *hw;

public:
    Check(ModuleHardware &hw) : hw(&hw) {
    }

public:
    bool sht31() {
        if (!hw->sht31Sensor.begin()) {
            debugfln("test: SHT31 FAILED");
            return false;
        }

        debugfln("test: SHT31 PASSED");
        return true;
    }

    bool mpl3115a2() {
        if (!hw->mpl3115a2Sensor.begin()) {
            debugfln("test: MPL3115A2 FAILED");
            return false;
        }

        debugfln("test: MPL3115A2 PASSED");
        return true;
    }

    bool tsl2591() {
        if (!hw->tsl2591Sensor.begin()) {
            debugfln("test: TSL25911FN FAILED");
            return false;
        }

        debugfln("test: TSL25911FN PASSED");
        return true;
    }

    bool bno055() {
        debugfln("test: BNO055 Checking...");

        if (!hw->bnoSensor.begin()) {
            debugfln("test: BNO055 FAILED");
            return false;
        }

        hw->bnoSensor.setExtCrystalUse(true);

        debugfln("test: BNO055 PASSED");
        return true;
    }

    bool flashMemory() {
        debugfln("test: Checking flash memory...");

        if (!hw->serialFlash.begin(ModuleHardware::PIN_FLASH_CS)) {
            debugfln("test: Flash memory FAILED");
            return false;
        }

        uint8_t buffer[256];
        hw->serialFlash.readID(buffer);
        if (buffer[0] == 0) {
            debugfln("test: Flash memory FAILED");
            return false;
        }

        uint32_t chipSize = hw->serialFlash.capacity(buffer);
        if (chipSize == 0) {
            debugfln("test: Flash memory FAILED");
            return false;
        }

        debugfln("test: Flash memory PASSED");
        return true;
    }

    bool sdCard() {
        debugfln("test: Checking SD...");

        if (!SD.begin(PIN_SD_CS)) {
            debugfln("test: SD FAILED");
            return false;
        }

        digitalWrite(PIN_SD_CS, HIGH);
        debugfln("test: SD PASSED");

        return true;
    }

    bool gps() {
        debugfln("test: Checking gps...");

        platformSerial2Begin(9600);

        uint32_t charactersRead = 0;
        uint32_t start = millis();
        while (millis() - start < 5 * 1000 && charactersRead < 100)  {
            while (Serial2.available()) {
                Serial.print((char)Serial2.read());
                charactersRead++;
            }
        }

        debugfln("");

        if (charactersRead < 100) {
            debugfln("test: GPS FAILED");
            return false;
        }

        debugfln("test: GPS PASSED");
        return true;
    }


    bool wifi() {
        debugfln("test: Checking wifi...");

        delay(500);

        digitalWrite(PIN_WINC_RST, HIGH);

        WiFi.setPins(PIN_WINC_CS, PIN_WINC_IRQ, PIN_WINC_RST);

        digitalWrite(PIN_WINC_EN, LOW);
        delay(50);

        digitalWrite(PIN_WINC_EN, HIGH);

        delay(50);

        if (WiFi.status() == WL_NO_SHIELD) {
            debugfln("test: Wifi FAILED");
            return false;
        }

        debugfln("test: Wifi firmware version: ");
        auto fv = WiFi.firmwareVersion();
        debugfln("test: Wifi version: %s", fv);
        debugfln("test: Wifi PASSED");

        return true;
    }

    bool check() {
        auto failures = false;
        if (!flashMemory()) {
            failures = true;
        }
        if (!macEeprom()) {
            failures = true;
        }
        if (!rtc()) {
            failures = true;
        }
        if (!fuelGauge()) {
            failures = true;
        }
        if (!mpl3115a2()) {
            failures = true;
        }
        if (!tsl2591()) {
            failures = true;
        }
        if (!sht31()) {
            failures = true;
        }

        if (!bno055()) {
            failures = true;
        }

        if (!gps()) {
            failures = true;
        }
        if (!sdCard()) {
            failures = true;
        }
        if (!wifi()) {
            failures = true;
        }

        hw->leds(true);

        return !failures;
    }

    bool fuelGauge() {
        FuelGauge gauge;

        gauge.powerOn();

        if (gauge.version() != 3) {
            debugfln("test: Gauge FAILED");
            return true;
        }

        debugfln("test: Gauge PASSED");
        return true;
    }

    bool rtc() {
        RTC_PCF8523 rtc;
        if (!rtc.begin()) {
            debugfln("test: RTC FAILED");
            return false;
        }
        debugfln("test: RTC PASSED");
        return true;
    }

    bool macEeprom() {
        MacEeprom macEeprom;
        uint8_t id[8] = { 0 };

        auto success = macEeprom.read128bMac(id);
        if (!success) {
            debugfln("test: 128bMAC FAILED");
            return false;
        }

        debugfln("test: 128bMAC: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x", id[0], id[1], id[2], id[3], id[4], id[5], id[6], id[7]);

        debugfln("test: 128bMAC PASSED");

        return success;
    }


    void failed() {
        while (true) {
            hw->leds(false);
            delay(100);
            hw->leds(true);
            delay(100);
        }
    }

};

void setup() {
    Serial.begin(115200);

    while (!Serial) {
        delay(100);
    }

    debugfln("test: Setup");

    ModuleHardware hw;
    hw.setup();

    debugfln("test: Begin");

    Check check(hw);
    if (!check.check()) {
        check.failed();
    }

    debugfln("test: Done");

    Sensors sensors(hw);

    while (true) {
        sensors.takeReading();
    }

    delay(100);
}

void loop() {

}
