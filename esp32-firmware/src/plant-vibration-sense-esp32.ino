/*
  Board Versions
    Arduino AVR - 1.8.6
    Arduino ESP32 Boards - 2.0.13
    esp32 - 2.011
  
  Libraries used
  - ESPAsyncWebServer by ESP32ASync 3.7.10
  - AsyncTCP 1.1.4
*/

#include <WiFi.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <SPI.h>

const char* ssid = "ESP32 Vibration Sense";
const char* password = "12345678";
const IPAddress localIP(192, 168, 4, 1);
const IPAddress gateway(192, 168, 4, 1);
const IPAddress subnet(255, 255, 255, 0);
const uint32_t SPI_CLOCK_SPEED = 10000000;
const uint16_t NUM_DATA_VALUES = 200;
const uint16_t PRINT_BUFFER_SIZE = 2000;
const uint8_t LED_PIN = 2;

StaticJsonDocument<2000> dataJson;

AsyncWebServer espWebServer(80);

SPIClass spiMaster(VSPI);

String linkedClientIP;

bool currentlyLinked = false;

// Mutex for critical section
portMUX_TYPE dataMutex = portMUX_INITIALIZER_UNLOCKED;

uint16_t dataBuffer[NUM_DATA_VALUES];

char printBuffer[PRINT_BUFFER_SIZE];

void setup() 
{
  Serial.begin(921600);
  
  // Delay for Serial to Initailize
  delay(1000);
  
  // Indent new line for printing
  Serial.println("");

  initializeGpioPins();

  initializeDataJson();

  initializeWifiAp();

  initializeAsyncWebServer();

  // Set onboard LED high
  digitalWrite(LED_PIN, HIGH);

  printToSerial("Main Program Running!\n");
}

void loop() 
{

}

void initializeGpioPins()
{
  pinMode(LED_PIN, OUTPUT);
}

void initializeDataJson()
{
  JsonArray sensorArray = dataJson.createNestedArray("sensordata");

  for (int dataIdx = 0; dataIdx < NUM_DATA_VALUES+1; dataIdx++) 
  {
    sensorArray.add(0);
  }

  // Serialize to JSON string
  String output;
  serializeJson(dataJson, output);
  printToSerial("%s\n", output.c_str());


  size_t jsonLength = measureJson(dataJson);
  Serial.printf("JSON serialized length: %u\n", jsonLength);
}

void initializeWifiAp()
{
  // Set network configurations
  WiFi.softAPConfig(localIP, gateway, subnet);

  // Set Wi-Fi mode to Access Point
  WiFi.softAP(ssid, password);

  // Print the IP address of the ESP32
  IPAddress IP = WiFi.softAPIP();
  printToSerial("Wifi Access Point Started. ESP32 IP Address: %s\n", IP.toString().c_str());
}

void initializeAsyncWebServer()
{
  espWebServer.on("/link", HTTP_GET, handleLinkRequest);
  espWebServer.on("/data", HTTP_GET, handleDataRequest);
  espWebServer.begin();
}

void initializeSpiMaster()
{
  spiMaster.begin(SCK, MISO, MOSI, SS);
  SPISettings settings(10000000, MSBFIRST, SPI_MODE3);
  spiMaster.beginTransaction(settings);
}

void handleLinkRequest(AsyncWebServerRequest *request)
{
  String requestClientIP = request->client()->remoteIP().toString();

  printToSerial("Link requested from IP Address: %s\n", requestClientIP.c_str());

  if(!currentlyLinked)
  {
    currentlyLinked = true;
    linkedClientIP = requestClientIP;
    printToSerial("Client IP: %s is linked to server!\n", linkedClientIP.c_str());
  }
  else
  {
    printToSerial("Link request denied, a client has already been linked.\n");
  }

  String response = "Link Successful";
  request->send(200, "text/plain", response);
}

void handleDataRequest(AsyncWebServerRequest *request)
{
  portENTER_CRITICAL(&dataMutex);

  portEXIT_CRITICAL(&dataMutex);

  String response = "Data!";
  request->send(200, "text/plain", response);
}

void printToSerial(const char *fmt, ...) 
{
  va_list args;
  va_start(args, fmt);
  vsnprintf(printBuffer, PRINT_BUFFER_SIZE, fmt, args);
  va_end(args);

  Serial.print(printBuffer);
}
