#include <Heltec.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "IFTTTWebhook.h"
#include <ThingSpeak.h>
#include <SSD1306.h>
#include <esp_task_wdt.h>

#define Version "V1.3.1"
#define CONS
#define WIFI
#define IFTT

#ifdef CONS
#define PRINT_DEBUG_MESSAGES
#endif

#ifdef WIFI
WiFiClass *wifi = &Heltec.Wifi;
#ifdef IFTT
#define EVENT_NAME "Radioactivity" // Name of your event name, set when you are creating the applet
#endif
#endif

SSD1306  display(0x3c, 5, 4);

const int inputPin = 26;

int counts = 0;  // Tube events
int counts2 = 0;
int cpm = 0;                                             // CPM
unsigned long lastCountTime;                            // Time measurement
unsigned long lastEntryThingspeak;
unsigned long startCountTime;                            // Time measurement
unsigned long startEntryThingspeak;

#ifdef WIFI
unsigned long myChannelNumber = 123456; // replace with your channel number
const char *myWriteAPIKey = "XYZ"; // replace with your channel write API Key
#endif

void IRAM_ATTR ISR_impulse()
{ // Captures count of events from Geiger counter board
  counts++;
  counts2++;
}

void displayInit()
{
  Heltec.display->init();
  Heltec.display->flipScreenVertically();
  Heltec.display->setFont(ArialMT_Plain_24);
}

void displayInt(int dispInt, int x, int y)
{
  Heltec.display->setColor(WHITE);
  Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
  Heltec.display->drawString(x, y, String(dispInt));
  Heltec.display->setFont(ArialMT_Plain_24);
  Heltec.display->display();
}

void displayString(String dispString, int x, int y)
{
  Heltec.display->setColor(WHITE);
  Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
  Heltec.display->drawString(x, y, dispString);
  Heltec.display->setFont(ArialMT_Plain_24);
  Heltec.display->display();
}

/****reset***/
void software_Reset() // Restarts program from beginning but does not reset the peripherals and registers
{
#ifdef CONS
  Serial.println("resetting by software");
#endif
  displayString("Myreset", 64, 15);
  delay(1000);
  esp_restart();
}

void IFTTT(int postValue)
{
#ifdef WIFI
#ifdef IFTT
  IFTTTWebhook webhook(IFTTT_KEY, EVENT_NAME);
  if (!webhook.trigger(String(postValue).c_str()))
  {
#ifdef CONS
    Serial.println("IFTTT Trigger Failed"); // If webhook fails to trigger, print message
#endif
}
#endif
#endif
}

void thingspeakUpdate(int fieldValue)
{
#ifdef WIFI
if (millis() - lastEntryThingspeak > 15000) // Update every 15 seconds
{
ThingSpeak.begin(wifi);
ThingSpeak.setField(1, fieldValue);
ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
lastEntryThingspeak = millis();
ThingSpeak.begin(wifi);
#ifdef PRINT_DEBUG_MESSAGES
Serial.println("Update Thingspeak");
#endif
}
#endif
}

void setup()
{
Serial.begin(115200);
pinMode(inputPin, INPUT);
attachInterrupt(digitalPinToInterrupt(inputPin), ISR_impulse, RISING);
displayInit();
displayString("Starting...", 64, 15);
WiFi.mode(WIFI_STA);
Heltec.begin(true /DisplayEnable Enable/, false /LoRa Disable/, true /Serial Enable/, true /PABOOST Enable/, 915E6 /long BAND/);
Heltec.display->clear();
displayString("Ready", 64, 15);
#ifdef PRINT_DEBUG_MESSAGES
Serial.println("Booting");
#endif
delay(1000);
startEntryThingspeak = lastEntryThingspeak = millis();
}

void loop()
{
cpm = counts - counts2; // count per minute
if (millis() - lastCountTime > 60000)
{ // Update every minute
displayString(" ", 64, 15);
displayInt(cpm, 64, 15);
lastCountTime = millis();
counts2 = counts;
thingspeakUpdate(cpm); // Update ThingSpeak
IFTTT(cpm); // Trigger IFTTT
}

if (millis() - startCountTime > 1800000) // If the device runs for more than 30 minutes
{
software_Reset(); // reset by software
}
}
