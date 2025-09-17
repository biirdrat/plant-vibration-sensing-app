/*
  Board Versions
    Arduino AVR - 1.8.6
    Arduino ESP32 Boards - 2.0.13
    esp32 - 2.011
  
  Libraries used
  - ESPAsyncWebServer by ESP32ASync 3.7.10
  - AsyncTCP by ESP32Async 3.4.7
  - ArduinoJson 7.4.2
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
const uint32_t SPI_CLOCK_SPEED = 16000000;
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


  while(!initializeADS7175Spi())
  {
    printToSerial("SPI Failed to initialized, retrying...\n");
    delay(1000);
  }

  configureADS7175();

  // Set onboard LED high
  digitalWrite(LED_PIN, HIGH);

  printToSerial("\nMain Program Running!\n");
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

bool initializeADS7175Spi()
{
  bool initializedSuccessfully = false;

  // Start SPI
  spiMaster.begin();
  SPISettings settings(SPI_CLOCK_SPEED, MSBFIRST, SPI_MODE3);
  spiMaster.beginTransaction(settings);

  // Reset ADS7175 SPI
  digitalWrite(SS, HIGH);
  delay(1000);

  // Read value in ID Register
  uint32_t idValue = readSpiRegister(ID_REGISTER);
  
  // Check if ID Value is Valid
  if(idValue == 0)
  {
    spiMaster.endTransaction();
    printToSerial("SPI communication with ADC failed to initialize. A value was not read.\n");
  }
  else if((idValue & 0xFF0) != 0xCD0)
  {
    spiMaster.endTransaction();
    printToSerial("SPI communication with ADC failed to initialize. Invalid ID Register Value Read: 0x%08X\n", idValue);
  }
  else
  {
    printToSerial("SPI communication with ADC initialized successfully. ID Register Value Read: 0x%08X\n", idValue);
    initializedSuccessfully = true;
  }

  return initializedSuccessfully;
}

void configureADS7175()
{

  // Enable channel 1 AIN0 is positive input and AIN1 is negative input
  writeSpiRegister(CHANNEL0_REGISTER, 2, 0x8001);

  // Disable channel 2
  writeSpiRegister(CHANNEL1_REGISTER, 2, 0x0000);

  // Disable channel 3
  writeSpiRegister(CHANNEL2_REGISTER, 2, 0x0000);

  // Disable channel 4
  writeSpiRegister(CHANNEL3_REGISTER, 2, 0x0000);

  // Configure Setup 0
  writeSpiRegister(SETUP_CONFIG0_REGISTER, 2, 0x0F00);

  // Configure Filter 0
  writeSpiRegister(FILTER_CONFIG0_REGISTER, 2, 0x0504);

  // Configure Offset 0
  writeSpiRegister(OFFSET0_REGISTER, 3, 0x000000);

  // Configure Gain 0
  writeSpiRegister(GAIN0_REGISTER, 3, 0x000000);

  // Configure ADC Mode
  writeSpiRegister(ADC_MODE_REGISTER, 2, 0x000000);

  // Configure Interface Mode
  writeSpiRegister(INTERFACE_MODE_REGISTER, 2, 0x000000);
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

      // Shift left so MSB comes first
      valueRead = (valueRead << 8) | byteRead;
    }
  }

  return valueRead;
}

uint32_t readSpiRegister(uint8_t registerNum)
{
  // Enable Chip Select
  digitalWrite(SS, LOW);

  uint32_t valueRead = 0;

  // Specify read request and which register by writing to communications register
  byte writeByte = 0b01000000 | (registerNum & 0b00111111);
  spiMaster.transfer(writeByte);

  // Read number of bytes based on register
  switch(registerNum)
  {
    case ID_REGISTER:
    {
      valueRead = spiRead(2);
      break;
    }

    case SETUP_CONFIG0_REGISTER:
    {
      valueRead = spiRead(2);
      break;
    }
    
    case FILTER_CONFIG0_REGISTER:
    {
      valueRead = spiRead(2);
      break;
    }
  }

  // Disable Chip Select
  digitalWrite(SS, HIGH);

  return valueRead;
}

void writeSpiRegister(uint8_t registerNum, uint8_t numBytes, uint32_t writeData)
{
  // Enable Chip Select
  digitalWrite(SS, LOW);

  // Specify write request and which register by writing to communications register
  byte writeByte = 0b00000000 | (registerNum & 0b00111111);
  spiMaster.transfer(writeByte);

  if(numBytes > 0 && numBytes <= 4)
  {
    for(int byteIdx = (numBytes-1); byteIdx >= 0; byteIdx--)
    {
      
      uint8_t dataByte = (uint8_t)((writeData >> (byteIdx * 8)) & 0xFF);
      spiMaster.transfer(dataByte);
    }
  }

  // Disable Chip Select
  digitalWrite(SS, HIGH);
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