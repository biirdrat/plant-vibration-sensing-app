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

// Spi ADC Registers
const uint8_t STATUS_REGISTER = 0b000000;
const uint8_t ADC_MODE_REGISTER = 0b000001;
const uint8_t INTERFACE_MODE_REGISTER = 0b000010;
const uint8_t REGISTER_CHECKSUM_REGISTER = 0b000011;
const uint8_t DATA_REGISTER = 0b000100;
const uint8_t GPIO_CONFIGURATION_REGISTER = 0b000110;
const uint8_t ID_REGISTER = 0b00000111;
const uint8_t CHANNEL0_REGISTER = 0b010000;
const uint8_t CHANNEL1_REGISTER = 0b010001;
const uint8_t CHANNEL2_REGISTER = 0b010010;
const uint8_t CHANNEL3_REGISTER = 0b010011;
const uint8_t SETUP_CONFIG0_REGISTER = 0b100000;
const uint8_t SETUP_CONFIG1_REGISTER = 0b100001;
const uint8_t SETUP_CONFIG2_REGISTER = 0b100010;
const uint8_t SETUP_CONFIG3_REGISTER = 0b100011;
const uint8_t FILTER_CONFIG0_REGISTER = 0b101000;
const uint8_t FILTER_CONFIG1_REGISTER = 0b101001;
const uint8_t FILTER_CONFIG2_REGISTER = 0b101010;
const uint8_t FILTER_CONFIG3_REGISTER = 0b101011;
const uint8_t OFFSET0_REGISTER = 0b110000;
const uint8_t OFFSET1_REGISTER = 0b110001;
const uint8_t OFFSET2_REGISTER = 0b110010;
const uint8_t OFFSET3_REGISTER = 0b110011;
const uint8_t GAIN0_REGISTER = 0b111000;
const uint8_t GAIN1_REGISTER = 0b111001;
const uint8_t GAIN2_REGISTER = 0b111010;
const uint8_t GAIN3_REGISTER = 0b111011;

const char* ssid = "ESP32 Vibration Sense";
const char* password = "12345678";
const IPAddress localIP(192, 168, 4, 1);
const IPAddress gateway(192, 168, 4, 1);
const IPAddress subnet(255, 255, 255, 0);
const uint32_t SPI_CLOCK_SPEED = 1000000;
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

  // initializeDataJson();

  // initializeWifiAp();

  // initializeAsyncWebServer();

  initializeSpiMaster();

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
  pinMode(SS, OUTPUT);
  digitalWrite(SS, HIGH);
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
  spiMaster.begin();
  SPISettings settings(SPI_CLOCK_SPEED, MSBFIRST, SPI_MODE3);
  spiMaster.beginTransaction(settings);
  digitalWrite(SS, LOW);
  requestSpiReadRegister(ID_REGISTER);
  spiRead(2);

}

void spiWriteByte(byte dataByte)
{
  spiMaster.transfer(dataByte);
}

uint32_t spiRead(int numBytes)
{
  uint32_t valueRead = 0;

  if (numBytes > 0 && numBytes <= 4)
  {
    for (int byteIdx = 0; byteIdx < numBytes; byteIdx++)
    {
      // Read one byte from SPI
      uint8_t byteRead = spiMaster.transfer(0x00);
      Serial.println(byteRead);

      // Shift left so MSB comes first
      valueRead = (valueRead << 8) | byteRead;
    }
  }

  printToSerial("Value read: 0x%08lX\n", valueRead);
  return valueRead;
}

uint32_t readSpiRegister(uint8_t registerNum)
{

}

void requestSpiReadRegister(byte reg)
{
  byte writeByte = 0b01000000 | (reg & 0b00111111);
  Serial.println(writeByte);
  spiMaster.transfer(writeByte);
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
