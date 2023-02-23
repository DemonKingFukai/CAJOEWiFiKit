#include <Heltec.h>
#include <SSD1306.h>

#define Version "V1.3.1"
#define CONS

SSD1306  display(0x3c, 5, 4);

const int inputPin = 26;

int counts = 0;  // Tube events
int counts2 = 0;
int cpm = 0;      // CPM
unsigned long lastCountTime;  // Time measurement
unsigned long startCountTime; // Time measurement

void IRAM_ATTR ISR_impulse()
{ // Captures count of events from Geiger counter board
  counts++;
  counts2++;
}

void displayInit()
{
  Heltec.display->init();
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

void setup()
{
#ifdef CONS
  Serial.begin(115200);
#endif
  displayInit();
  attachInterrupt(inputPin, ISR_impulse, FALLING);
  lastCountTime = millis();
  startCountTime = millis();
  displayString("Geiger", 64, 0);
  displayString("Counter", 64, 20);
  delay(1000);
}

void loop()
{
  if ((millis() - lastCountTime) > 1000)
  {
    cpm = counts2 * 60 / ((millis() - startCountTime) / 1000);
    counts2 = 0;
    lastCountTime = millis();
    displayInt(cpm, 64, 50); // Display CPM value at (64, 50) position
  }
}
