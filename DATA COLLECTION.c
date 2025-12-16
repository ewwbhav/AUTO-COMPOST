/*
 * SSIP AUTOCOMPOST - FIRMWARE V1 (DATA COLLECTION MODE)
 * - Controls Mixer (Time-based + Moisture-based)
 * - Controls Fan (Temp/Gas-based)
 * - Logs CLEAN CSV DATA for Machine Learning Training
 */

#include <Arduino.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ====== PIN MAPPING (MATCHING YOUR PCB) ======
#define PIN_SOIL_MOISTURE   34
#define PIN_MQ135           35
#define PIN_DHT22           13 // Moved to 13 per your soldering
#define PIN_DS18B20         4  // Swapped with DHT per your soldering

#define PIN_FAN_MOSFET      25

#define PIN_MIXER_L_EN      26
#define PIN_MIXER_R_EN      27
#define PIN_MIXER_L_PWM     32
#define PIN_MIXER_R_PWM     33

#define I2C_SDA             21
#define I2C_SCL             22

// ====== CALIBRATION (UPDATE THESE!) ======
const int dryValue          = 3200; // Value in Air
const int wetValue          = 1400; // Value in Water

// ====== THRESHOLDS ======
const int SOIL_DRY_THRESHOLD    = 30;   // %
const float AMBIENT_MAX_TEMP    = 45.0; // °C
const float COMPOST_TARGET_MAX  = 60.0; // °C
const int MQ135_MAX_PPM         = 1000; // PPM

// ====== TIMERS ======
const unsigned long MIXING_INTERVAL_MS = 43200000; // 12 Hours
const unsigned long MIXING_DURATION_MS = 60000;    // 60 Seconds
const unsigned long LOG_INTERVAL_MS    = 5000;     // Log data every 5 sec

// ====== OBJECTS ======
#define DHTTYPE DHT22
DHT dht(PIN_DHT22, DHTTYPE);
OneWire oneWire(PIN_DS18B20);
DallasTemperature ds18b20(&oneWire);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ====== VARIABLES ======
unsigned long lastMixTime = 0;
unsigned long lastLogTime = 0;

struct SensorData {
    float ambTemp;
    float ambHum;
    float compostTemp;
    int   soilMoisture;
    int   airQualityPPM;
};

// ====== SETUP FUNCTIONS ======
void setupMixer() {
    pinMode(PIN_MIXER_L_EN, OUTPUT); pinMode(PIN_MIXER_R_EN, OUTPUT);
    pinMode(PIN_MIXER_L_PWM, OUTPUT); pinMode(PIN_MIXER_R_PWM, OUTPUT);
    digitalWrite(PIN_MIXER_L_EN, HIGH); digitalWrite(PIN_MIXER_R_EN, HIGH);
    
    ledcSetup(0, 20000, 8); // Channel 0 for Mixer
    ledcAttachPin(PIN_MIXER_L_PWM, 0);
    digitalWrite(PIN_MIXER_R_PWM, LOW);
}

void setupFan() {
    ledcSetup(1, 20000, 8); // Channel 1 for Fan
    ledcAttachPin(PIN_FAN_MOSFET, 1);
}

void setMixerSpeed(int percent) {
    ledcWrite(0, map(constrain(percent, 0, 100), 0, 100, 0, 255));
}

void setFanSpeed(int percent) {
    ledcWrite(1, map(constrain(percent, 0, 100), 0, 100, 0, 255));
}

// ====== SENSOR READING ======
SensorData readSensors() {
    SensorData d;
    d.ambTemp = dht.readTemperature();
    d.ambHum = dht.readHumidity();
    
    ds18b20.requestTemperatures();
    d.compostTemp = ds18b20.getTempCByIndex(0);
    
    int rawMoist = analogRead(PIN_SOIL_MOISTURE);
    d.soilMoisture = map(constrain(rawMoist, wetValue, dryValue), dryValue, wetValue, 0, 100);
    
    int rawGas = analogRead(PIN_MQ135);
    d.airQualityPPM = map(rawGas, 0, 4095, 0, 2000); // Simple map for logging
    
    return d;
}

// ====== MAIN SETUP ======
void setup() {
    Serial.begin(115200);
    Wire.begin(I2C_SDA, I2C_SCL);
    
    dht.begin();
    ds18b20.begin();
    lcd.init(); lcd.backlight();
    
    setupMixer();
    setupFan();
    
    lcd.print("SSIP Data Mode");
    delay(2000);
    lcd.clear();
    
    // CSV Header for Excel
    Serial.println("AmbTemp,AmbHum,SoilMoist,SoilTemp,GasPPM,FanState,MixerState");
}

// ====== MAIN LOOP ======
void loop() {
    unsigned long now = millis();
    
    // Run Logic every 5 seconds
    if (now - lastLogTime >= LOG_INTERVAL_MS) {
        lastLogTime = now;
        SensorData d = readSensors();
        
        // --- CONTROL LOGIC ---
        bool fanOn = false;
        bool mixOn = false;
        
        // Fan Logic
        if (d.compostTemp > COMPOST_TARGET_MAX || d.airQualityPPM > MQ135_MAX_PPM || d.ambTemp > AMBIENT_MAX_TEMP) {
            fanOn = true;
            setFanSpeed(100);
        } else {
            setFanSpeed(0);
        }
        
        // Mixer Logic (Timer + Low Moisture)
        if (now - lastMixTime >= MIXING_INTERVAL_MS || d.soilMoisture < SOIL_DRY_THRESHOLD) {
            // Only mix if we haven't mixed recently (debounce check)
             if (now - lastMixTime >= MIXING_INTERVAL_MS) {
                mixOn = true;
                lcd.setCursor(0,1); lcd.print("Mixing...");
                setMixerSpeed(70);
                delay(MIXING_DURATION_MS); // BLOCKING DELAY FOR SAFETY
                setMixerSpeed(0);
                lastMixTime = now;
                lcd.clear();
             }
        }
        
        // --- DISPLAY ---
        lcd.setCursor(0,0); 
        lcd.print("T:"); lcd.print((int)d.compostTemp); 
        lcd.print(" M:"); lcd.print(d.soilMoisture); lcd.print("%");
        
        // --- DATA LOGGING (CSV FORMAT) ---
        Serial.print(d.ambTemp); Serial.print(",");
        Serial.print(d.ambHum); Serial.print(",");
        Serial.print(d.soilMoisture); Serial.print(",");
        Serial.print(d.compostTemp); Serial.print(",");
        Serial.print(d.airQualityPPM); Serial.print(",");
        Serial.print(fanOn); Serial.print(",");
        Serial.println(mixOn);
    }
}
