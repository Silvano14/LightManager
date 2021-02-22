#define BLYNK_PRINT Serial
char SSID[] = "";
char PSWD[] = "";
char AUTH_BLYNK[] = "";

#include <SPI.h>
//#include <WiFiUdp.h>
#include <RTCZero.h>
// Simple library that include Blynk and WiFiNINA -> https://github.com/blynkkk/blynk-library/blob/master/src/BlynkSimpleWiFiNINA.h
#include <BlynkSimpleWiFiNINA.h>

int sunset = 17;
#define PIR_PIN 7
#define LIGHT_PIN 6

RTCZero rtc;

int status = WL_IDLE_STATUS;

int lightState = LOW;

// Greenwich Mean Time
const int GMT = 1;
boolean isEnablePIR = 1;
int counterGetTime = 0;
boolean notifying = false;
int oneTime = 0;

void setup() {
    Serial.begin(115200);
    Serial.println("Starting...");
    Blynk.begin(AUTH_BLYNK, SSID, PSWD);

    unsigned long epoch;

    int numberOfTries = 0, maxTries = 5;

// IF YOU WANT CONNECT WITHOUT BLYNK, COMMENT BLYNK PARTS AND UNCOMMENT THIS

//  // attempt to connect to WiFi network:
//  while ( status != WL_CONNECTED) {
//
//    Serial.print("Attempting to connect to SSID: ");
//
//    Serial.println(SSID);
//
//    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
//    status = WiFi.begin(SSID, PSWD);
//
    Serial.println("Waiting for connection...");
    delay(10000);
//
//  }

    String fv = WiFi.firmwareVersion();
    if (fv < "1.0.0") {
        Serial.println("Please upgrade the firmware");
    }

    rtc.begin();

    do {
        epoch = WiFi.getTime();
        numberOfTries++;
    } while ((epoch == 0) && (numberOfTries < maxTries));

    if (numberOfTries == maxTries) {

        Serial.print("NTP unreachable!!");

        while (1);

    } else {

        Serial.print("Epoch received: ");

        Serial.println(epoch);

        rtc.setEpoch(epoch);

        Serial.println();
    }

    // Pins
    pinMode(PIR_PIN, INPUT);
    pinMode(LIGHT_PIN, OUTPUT);
}

void loop() {
    Blynk.run();

    if (digitalRead(PIR_PIN) == HIGH && isSunset(sunset) && isEnablePIR) {
        Serial.println("I see you!");
        lightState = HIGH;
    }

    if (!notifying) {
        oneTime = 0;
    }
    digitalWrite(LIGHT_PIN, lightState);
    keepOnForNMinutes(1);
    delay(200);
}

boolean isSunset(int timeSunset) {
    return ((rtc.getHours() + GMT) >= timeSunset);
}

int sum = 0;
int hoursState = 0;
int minutesState = 0;
int secondsState = 0;

void keepOnForNMinutes(int minutes) {

    if (lightState == HIGH) {

        if (counterGetTime == 0) {
            hoursState = rtc.getHours() + GMT;
            minutesState = rtc.getMinutes();
            secondsState = rtc.getSeconds();
            counterGetTime++;
            Serial.println(
                    "Time get it! - " + String(hoursState) + ":" + String(minutesState) + ":" + String(secondsState));
            Serial.println("Keeping on...");
            sum = minutesState + minutes;
        }

        if (minutes <= 0) {
            Serial.println("You put a wrong value: " + String(minutes));
        }

        if (sum >= 60) {
            sum -= 60;
        }

        if (notifying && oneTime == 0) {
            Blynk.notify("La luce rimarrà accesa fino alle: " + String(hoursState) + ":" + String(sum) + ":" +
                         String(secondsState));
            oneTime += 1;
        }

        Serial.println(notifying);
        Serial.println(oneTime);


        if (rtc.getMinutes() == sum && rtc.getSeconds() == secondsState) {
            counterGetTime = 0;
            lightState = LOW;
        }
    }
}

void setManualLight(int value) {
    lightState = value;
    counterGetTime = 0;
    hoursState = 0;
    minutesState = 0;
    secondsState = 0;
    notifying = false;
}

BLYNK_WRITE(V1){
        isEnablePIR = param.asInt();
        notifying = false;
}

BLYNK_WRITE(V3){
        setManualLight(param.asInt());
}

BLYNK_WRITE(V4){
        notifying = param.asInt();
}
