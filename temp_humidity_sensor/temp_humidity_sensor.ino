/*
Basic network embedded device webserver that sends the output of a
DHT11 Temperature and Humidity Sensor on HTTP request. 
*/

#include <SimpleDHT.h>
#include <SPI.h>
#include <Ethernet.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02
};

//DHT11 Input Pin
int pinDHT11 = 9;

//Initialize DHT11 Sensor
SimpleDHT11 dht11;

//initialize variable for DHT11 sensor
byte temperature = 0;
byte humidity = 0;
byte data[40] = {0};


// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetServer server(80);

void setup() {
  
  // Open serial connection:
  Serial.begin(9600);
   
  // Establish Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    // if can't connect print error msg and loop foever:
    Serial.println("Failed to configure Ethernet using DHCP");
    for (;;)
    ;
  }
  // print your local IP address:
  printIPAddress();
     
}

void loop() {
  //reconnect when network is disconnected
  networkReconnect();

  //read DHT11 Data
  if (dht11.read(pinDHT11, &temperature, &humidity, data)) {
    Serial.println("Read DHT11 failed");
    return;
    }
  else {

      Serial.println("");
      Serial.print("Sample OK: ");
      Serial.print((int)temperature); 
      Serial.print(" *C, ");
      Serial.print((int)humidity); 
      Serial.println(" %");    
    }

  // listen for incoming clients
  EthernetClient client = server.available();  
  if (client) {

      // notify serial each server request
      Serial.println("client connection");

      // length of the incoming text line
      int lineLength = 0; 
    
      while (client.connected()) {
        if (client.available()) {
          
          char thisChar = client.read();
          // if you get a linefeed and the request line is blank,
          // then the request is over:
          if (thisChar == '\n' && lineLength < 1) {
          // send a standard http response header
          makeResponse(client);
          break;
          }
        
          //if you get a newline or carriage return,
          // you're at the end of a line:
          if (thisChar == '\n' || thisChar == '\r') {
            lineLength = 0;
          }
        else {
          // for any other character, increment the line length:
          lineLength++;
          }
          Serial.write(thisChar);
        }
      }
    // close the connection:
    client.stop();

  }

  // DHT11 sampling rate is 1HZ.
  delay(1000);
}

void networkReconnect() {

    //reconnect if ethernet is disconnected
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

// function for printing local IP address to serial
void printIPAddress()
{
  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
  }

  Serial.println();
}

// function for server response
void makeResponse(EthernetClient thisClient) {
  
  thisClient.print("HTTP/1.1 200 OK\n");
  thisClient.print("Content-Type: text/html\n\n");
  thisClient.println("<html><head></head><body>\n");
  thisClient.print("<h1>Hello! I'm an Arduino Web Server!</h1><br>");
  thisClient.print("<b>Current Room Temperature:</b> ");
  thisClient.print((int)temperature);
  thisClient.print(" *C<br>");
  thisClient.print("<b>Current Room Humidity:</b> ");
  thisClient.print((int)humidity);
  thisClient.print(" %<br>"); 
  thisClient.println("</body></html>\n");
  }

