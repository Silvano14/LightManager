#include <SPI.h>
#include <RTCZero.h>
// Simple library that include Blynk and WiFiNINA -> https://github.com/blynkkk/blynk-library/blob/master/src/BlynkSimpleWiFiNINA.h
#include <BlynkSimpleWiFiNINA.h>
#include "credentials.h"

#define BLYNK_PRINT Serial
#define PIR_PIN 7
#define LIGHT_PIN 6

RTCZero rtc;

// Greenwich Mean Time
const int GMT = 1;

int status = WL_IDLE_STATUS;
int lightState = LOW;

int sunset = 17;

boolean isEnablePIR = true;
int counterGetTime = 0;
boolean notifying = false;
boolean hasNotified = false;

BlynkTimer timer;

void myTimerEvent()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V1, isEnablePIR);
  Blynk.virtualWrite(V3, lightState);
  Blynk.virtualWrite(V4, notifying);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");
  Blynk.begin(AUTH_BLYNK, SSID, PSWD);

  unsigned long epoch;

  int numberOfTries = 0, maxTries = 5;

  Serial.println("Waiting for connection...");
  delay(10000);

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

  // Setup a function to be called every second
  timer.setInterval(1000L, myTimerEvent);
}

void loop() {
  Blynk.run();

  if (digitalRead(PIR_PIN) == HIGH && isSunset(sunset) && isEnablePIR) {
    Serial.println("I see you!");
    lightState = HIGH;
  }

  if (notifying) {
    hasNotified = false;
  }

  digitalWrite(LIGHT_PIN, lightState);
  keepOnForNMinutes(1);

// Initiates BlynkTimer
   timer.run(); 

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

    if (notifying && !hasNotified) {
      Blynk.notify("The light go on until: " + String(hoursState) + ":" + String(sum) + ":" + String(secondsState));
      hasNotified = true;
      // Just one time
      notifying = false;
    }

    if (rtc.getMinutes() == sum && rtc.getSeconds() == secondsState) {
      counterGetTime = 0;
      lightState = LOW;
    }
  }
}

void setManualLight(int value) {
  lightState = value;

  // Cleaning status
  counterGetTime = 0;
  hoursState = 0;
  minutesState = 0;
  secondsState = 0;
  notifying = false;
}

// Virtual pin 1 to active Passive InfraRed
BLYNK_WRITE(V1){
    isEnablePIR = param.asInt();
    notifying = false;
}

// Virtual Pin 3 to turn on the light for N minutes
BLYNK_WRITE(V3){
    setManualLight(param.asInt());
}

// Virtual Pin 4 if you want receive a notify
BLYNK_WRITE(V4){
    notifying = param.asInt() >= 1 ? true : false;
}
