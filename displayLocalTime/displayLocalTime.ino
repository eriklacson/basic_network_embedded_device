// Simple Network Embedded Device
// Extracts date and time from timezonedb.com
// and displays on LCD screen


#include <ArduinoJson.h> //JSON Library
#include <Ethernet.h> //Ethernet Library
#include <SPI.h> //SPI Library
#include <LiquidCrystal.h> // Liquid Crystal Library

// initialize the LCD library with RS pin on 8, E pin on 9, and pin 5,4,3,2 as output
LiquidCrystal lcd(9, 8, 5, 4, 3, 2);

// initialize http client
EthernetClient client;

const char* server = "api.timezonedb.com";  // API server address
// API endpoint
const char* resource = "/v2/get-time-zone?key=G09GM1DXFAH7&format=json&by=zone&zone=Asia/Manila"; 
const unsigned long BAUD_RATE = 9600;      // serial connection speed
const unsigned long HTTP_TIMEOUT = 10000;  // max respone time from server
const size_t MAX_CONTENT_SIZE = 512;       // max size of the HTTP response

// data type to extract from endpoint
struct LocalTime {
  char country[32];
  char time[32];
};

//Initialize program
void setup() {
  
  //initialize serial library
  initSerial();

  //initialize Ethernet library
  initEthernet();
   
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  
}

void loop() {
  //reconnect if Ethernet is disconnected
  networkReconnect();
    
  if (connect(server)) {
    if (sendRequest(server,resource) && skipResponseHeaders()) {
      LocalTime localTime;
      if (readReponseContent(&localTime)) {
        printUserData(&localTime);
        printLCD(&localTime);
      }
    }
  }

  //disconnect from server
  disconnect();

  //wait before next reconnection
  wait();
}

// Initialize Serial port
void initSerial() {
  Serial.begin(BAUD_RATE);
  while (!Serial) {
    ;  // wait for serial port to initialize
  }
  Serial.println("Serial ready");
}

// Initialize Ethernet library
void initEthernet() {
  byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
  if (!Ethernet.begin(mac)) {
    Serial.println("Failed to configure Ethernet");
    return;
  }
  Serial.println("Ethernet ready");
  printIPAddress();
  
  delay(1000);
}

// Print local IP address to serial
void printIPAddress()
{
  Serial.print("IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
  }

  Serial.println();
}


//reconnect if ethernet is disconnected
void networkReconnect() {

  switch (Ethernet.maintain())
  {
    case 1:
      //renewed fail
      Serial.println("Error: renewed fail");
      break;

    case 2:
      //renewed success
      Serial.println("Renewed success");

      //print your local IP address:
      printIPAddress();
      break;

    case 3:
      //rebind fail
      Serial.println("Error: rebind fail");
      break;

    case 4:
      //rebind success
      Serial.println("Rebind success");

      //print your local IP address:
      printIPAddress();
      break;

    default:
      //nothing happened
      break;

  }
}

// Open connection to the HTTP server
bool connect(const char* hostName) {
  Serial.print("Connect to ");
  Serial.println(hostName);

  bool ok = client.connect(hostName, 80);

  Serial.println(ok ? "Connected" : "Connection Failed!");
  return ok;
}

// Send the HTTP GET request to the server
bool sendRequest(const char* host, const char* resource) {
  Serial.print("GET ");
  Serial.println(resource);

  client.print("GET ");
  client.print(resource);
  client.println(" HTTP/1.0");
  client.print("Host: ");
  client.println(host);
  client.println("Connection: close");
  client.println();

  return true;
}

// Strip HTTP headers
bool skipResponseHeaders() {
  // HTTP headers end with an empty line
  char endOfHeaders[] = "\r\n\r\n";

  client.setTimeout(HTTP_TIMEOUT);
  bool ok = client.find(endOfHeaders);

  if (!ok) {
    Serial.println("No response or invalid response!");
  }

  return ok;
}

// Parse the JSON from the input string and extract the interesting values
// Here is the JSON we need to parse
bool readReponseContent(struct LocalTime* localTime) {
  // Compute optimal size of the JSON buffer according to what we need to parse.
  // See https://bblanchon.github.io/ArduinoJson/assistant/
  const size_t BUFFER_SIZE = JSON_OBJECT_SIZE(13) + 215;

  // Allocate a temporary memory pool
  DynamicJsonBuffer jsonBuffer(BUFFER_SIZE);

  JsonObject& root = jsonBuffer.parseObject(client);

  if (!root.success()) {
    Serial.println("JSON parsing failed!");
    return false;
  }

  Serial.println("insert copy variables here");
  // Here were copy the strings we're interested in
  strcpy(localTime->country, root["countryName"]);
  strcpy(localTime->time, root["formatted"]);
  
  return true;
}

// Print the data extracted from the JSON
void printUserData(const struct LocalTime* localTime) {
  Serial.println("insert print routine here");
  Serial.print("Country = ");
  Serial.println(localTime->country);
  Serial.print("Time = ");
  Serial.println(localTime->time);
}

// Output to LCD
void printLCD(const struct LocalTime* localTime) {
  lcd.print(localTime->country);
  lcd.setCursor(0, 1);
  lcd.print(localTime->time);
}


// Close the connection with the HTTP server
void disconnect() {
  Serial.println("Disconnect");
  client.stop();
}

// Pause 
void wait() {
  Serial.println("Wait 30 seconds");
  delay(30000);
}
