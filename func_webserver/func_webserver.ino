#include <ESP8266WiFi.h>
#include <Servo.h>

const int SERVO_PIN = 4;  // D2
const int FEED_BTN_PIN = 10; // D1; not working (due to pin conflict???)
const int RESET_BTN_PIN = 2; // D4

bool feed_btn_flag = false;
bool reset_btn_flag = false;

Servo servo;

const char* ssid = "Minghao's X1";
const char* password = "walterisaacso";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String _header;

// Auxiliar variables to store the current output state
String output5State = "off";
String output4State = "off";

// Assign output variables to GPIO pins
const int output5 = 5;
const int output4 = 4;

int state = 2;

void setup() {  
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(output5, OUTPUT);
  pinMode(output4, OUTPUT);
  // Set outputs to LOW
  digitalWrite(output5, LOW);
  digitalWrite(output4, LOW);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();

  servo.attach(SERVO_PIN);
  delay(500);
//  servo.write(0);
//  delay(2000);
  servo.write(90);

  pinMode(FEED_BTN_PIN, INPUT);
  pinMode(RESET_BTN_PIN, INPUT);
  
  Serial.println("Setup done");
}

void loop(){
  feed_btn_pressed();
  reset_btn_pressed();
  
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        _header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (_header.indexOf("GET /1") >= 0 && state == 2) {
              state = 1;
              servo.write(0);
              delay(5000);
              servo.write(90);
              Serial.println("******Feed once******");
            } else if (_header.indexOf("GET /0") >= 0 && state == 1) {
              state = 0;
              servo.write(180);
              delay(5000);
              servo.write(90);
              Serial.println("******Feed twice******");
            } else if (_header.indexOf("GET /u") >= 0) {
              Serial.println("******Refill needed******");
            } else if (_header.indexOf("GET /2") >= 0 && state != 2) {
              Serial.println("******Refilled!******");
              state = 2;
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>Pets Full Web Server</h1>");
            
            // Display current state  
            client.println("<p>Num of Feed Left: " + String(state) + "</p>");
            // If the output5State is off, it displays the ON button
            switch (state) {
              case 2:
                client.println("<p><a href=\"/1\"><button class=\"button\">Feed my baby now!</button></a></p>");     
                break;
              case 1:
                client.println("<p><a href=\"/0\"><button class=\"button\">Feed my baby now!</button></a></p>");
                break;
              case 0:
                client.println("<p><a href=\"/u\"><button class=\"button button2\">Refill it!</button></a></p>");
                break;
            }

            client.println("<p><a href=\"/2\"><button class=\"button\">I just refilled it!</button></a></p>");
               
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    _header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }

  delay(50);
}

bool feed_btn_pressed() {
  if (digitalRead(FEED_BTN_PIN) == LOW) {
    if (!feed_btn_flag) {
      Serial.println("Feed btn pressed");
      if (state >= -1) {
        state --;
      }

      switch (state) {
        case 1:
          servo.write(0);
          delay(5000);
          servo.write(90);
          break;
        case 0:
          servo.write(180);
          delay(5000);
          servo.write(90);
          break;
      }
    }
    feed_btn_flag = true;
  }
  else {
    feed_btn_flag = false;
  }
}

bool reset_btn_pressed() {
  if (digitalRead(RESET_BTN_PIN) == LOW) {
    if (!reset_btn_flag) {
      Serial.println("I just refilled it!");
      state = 2;
    }
    reset_btn_flag = true;
  }
  else {
    reset_btn_flag = false;
  }
}
