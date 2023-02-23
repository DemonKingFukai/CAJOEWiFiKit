#include <Heltec.h>
#include <SSD1306.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Chart.h>

#define Version "V1.3.1"
#define CONS

SSD1306 display(0x3c, 5, 4);
WebServer server(80);
Chart chart;
DNSServer dnsServer;

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

void handleRoot()
{
  String html = "<html><head><script src='https://cdn.jsdelivr.net/npm/chart.js'></script></head><body>";
  html += "<h1>Geiger Counter</h1>";
  html += "<h2>Current Radiation Level: " + String(cpm) + " CPM</h2>";
  html += "<canvas id='chart'></canvas>";
  html += "<script>var chartData = {labels: [" + chart.getLabels() + "], datasets: [{label: 'Radiation Level', data: [";
  for (int i = 0; i < chart.getMaxValues(); i++) {
    html += "{x: " + String(millis() - ((chart.getMaxValues() - i - 1) * 1000)) + ", y: " + String(chart.getValue(i)) + "}";
    if (i != chart.getMaxValues() - 1)
      {
  html += "}, ";
} else {
  html += "}";
}
}
html += "]}]};";
html += "var ctx = document.getElementById('chart').getContext('2d');";
html += "var chartInstance = new Chart(ctx, {type: 'line', data: chartData});</script>";
html += "<p><a href='/reset'>Reset</a></p>";
html += "</body></html>";
server.send(200, "text/html", html);
}

void handleReset()
{
software_Reset();
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

// Create WiFi hotspot
WiFi.softAP("Geiger Counter");
delay(100);

// Initialize chart
chart.setMaxValues(60);

// DNS Server
dnsServer.start(53, "*", WiFi.softAPIP());

// Web Server
server.on("/", handleRoot);
server.on("/reset", handleReset);
server.begin();
}

void loop()
{
dnsServer.processNextRequest();
server.handleClient();
if ((millis() - lastCountTime) > 1000)
{
cpm = counts2 * 60;
// Update chart
chart.addValue(cpm);
  // Reset counts
counts2 = 0;

// Update display
displayInt(cpm, 64, 45);
lastCountTime = millis();
}
}
  
