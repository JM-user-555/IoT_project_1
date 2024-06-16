#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

#define Led1 2  // GPIO 2 is the built-in LED on most ESP32 boards
#define SERVO_PIN 18 // GPIO 18 is used for servo control on many ESP32 boards
#define LDR_PIN 34 // LDR connected to GPIO 34 (ADC1_CH6)
#define EEPROM_SIZE 512
#define MAX_LDR_VALUES 10 // Store last 10 LDR values

const char* ssid = "";// add ur ssid
const char* password = "";//add password

Servo servo; // Create a servo object

WebServer server(80); // Object of WebServer(HTTP port, 80 is default)

int ldrValues[MAX_LDR_VALUES] = {0}; // Array to store last 10 LDR values
int ldrIndex = 0; // Index to keep track of the current LDR value

void setup() {
  pinMode(Led1, OUTPUT);
  digitalWrite(Led1, LOW); // Turn off LED initially

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  ConnectToWifi(ssid, password); // Function to connect to WiFi

  servo.setPeriodHertz(50); // Set the servo control frequency to 50Hz
  servo.attach(SERVO_PIN); // Attach the servo to GPIO 18

  EEPROM.begin(EEPROM_SIZE); // Initialize EEPROM

  server.on("/", HTTP_GET, handleRoot);
  server.on("/led/on", HTTP_GET, handleLedOn);
  server.on("/led/off", HTTP_GET, handleLedOff);
  server.on("/servo/set", HTTP_GET, handleSetServoAngle); // New handler for setting servo angle
  server.on("/ldr", HTTP_GET, handleLDR); // Handler to display LDR value
  server.on("/ldr/button", HTTP_GET, handleLDRButton); // Handler for refreshing LDR value
  server.on("/json", HTTP_GET, handleJson); // Handler to display JSON data
  server.begin();
  Serial.println("HTTP server started");

  // Load stored LDR values from EEPROM
  loadLDRValues();
}

void loop() {
  server.handleClient();
}

void handleRoot() {
  int ldrValue = analogRead(LDR_PIN); // Read LDR sensor value
  String htmlPage = "<html><head><style>"
                    "body { font-family: Arial, sans-serif; background-color: blue; display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; }"
                    ".content-box { background-color: white; padding: 20px; border-radius: 10px; box-shadow: 0px 0px 10px rgba(0,0,0,0.1); text-align: center; }"
                    "h1 { color: #007bff; }"
                    "button { background-color: white; color: black; padding: 10px 20px; border: none; border-radius: 10%; cursor: pointer; }"
                    "button:hover { background-color: turquoise; }"
                    "input[type=text] { padding: 10px; border-radius: 10%; margin-top: 10px; }"
                    ".set-angle-btn { background-color: white; color: black; border-radius: 10%; padding: 10px 20px; border: none; cursor: pointer; margin-top: 10px; }"
                    ".set-angle-btn:hover { background-color: turquoise; }"
                    "form { display: flex; flex-direction: column; align-items: center; margin-top: 10px; }"
                    "a { text-decoration: none; }"
                    "</style>"
                    "<script>"
                    "function refreshLDR() {"
                    "  fetch('/ldr')"
                    "    .then(response => response.text())"
                    "    .then(data => {"
                    "      document.getElementById('ldrValue').innerText = data;"
                    "    });"
                    "}"
                    "setInterval(refreshLDR, 5000);"
                    "</script>"
                    "</head><body>"
                    "<div class=\"content-box\">"
                    "<h1>IOT FINAL PROJECT</h1><br><br>"
                    "<a href=\"/led/on\"><button>Turn LED ON</button></a>&nbsp;&nbsp;"
                    "<a href=\"/led/off\"><button>Turn LED OFF</button></a><br><br>"
                    "<form action=\"/servo/set\">"
                    "Set Servo Angle: <input type=\"text\" name=\"angle\"><input type=\"submit\" value=\"Set Angle\" class=\"set-angle-btn\">"
                    "</form>"
                    "<br><br>LDR Sensor Value: <span id=\"ldrValue\">" + String(ldrValue) + "</span> "
                    "<br><br><a href=\"/json\"><button>View LDR JSON Data</button></a>"
                     "<br><br><h2 style='color:blue'>Done by Ghadi and Jad</h2>"
                    "</div>"
                    "</body></html>";
  server.send(200, "text/html", htmlPage);
}


void handleLedOn() {
  digitalWrite(Led1, HIGH); // Turn on the LED
  server.send(200, "text/plain", "LED is ON");
}

void handleLedOff() {
  digitalWrite(Led1, LOW); // Turn off the LED
  server.send(200, "text/plain", "LED is OFF");
}

void handleSetServoAngle() {
  String angleParam = server.arg("angle");
  if (angleParam != "") {
    int angle = angleParam.toInt();
    if (angle >= 0 && angle <= 180) { // Check if angle is within valid range
      servo.write(angle); // Set servo angle
      server.send(200, "text/plain", "Servo angle set to: " + String(angle));
    } else {
      server.send(400, "text/plain", "Invalid angle value. Angle must be between 0 and 180.");
    }
  } else {
    server.send(400, "text/plain", "Missing angle parameter");
  }
}

void handleLDR() {
  int ldrValue = analogRead(LDR_PIN); // Read LDR sensor value
  saveLDRValue(ldrValue); // Save LDR value to EEPROM
  server.send(200, "text/plain", String(ldrValue));
}

void handleLDRButton() {
  server.send(200, "text/plain", String(analogRead(LDR_PIN))); // Send LDR value as response
}

void handleJson() {
  String jsonString = readLDRValues();
  server.send(200, "application/json", jsonString);
}

void saveLDRValue(int ldrValue) {
  // Update the circular buffer
  ldrValues[ldrIndex] = ldrValue;
  ldrIndex = (ldrIndex + 1) % MAX_LDR_VALUES;

  // Create a JSON document
  StaticJsonDocument<200> doc;
  JsonArray array = doc.to<JsonArray>();
  for (int i = 0; i < MAX_LDR_VALUES; i++) {
    array.add(ldrValues[i]);
  }

  // Serialize JSON document to a string
  String jsonString;
  serializeJson(doc, jsonString);

  // Write JSON string to EEPROM
  for (int i = 0; i < jsonString.length(); i++) {
    EEPROM.write(i, jsonString[i]);
  }
  EEPROM.write(jsonString.length(), '\0'); // Null-terminate the string
  EEPROM.commit();
}

String readLDRValues() {
  // Read JSON string from EEPROM
  String jsonString;
  char ch;
  int i = 0;
  do {
    ch = char(EEPROM.read(i));
    jsonString += ch;
    i++;
  } while (ch != '\0' && i < EEPROM_SIZE);

  return jsonString;
}

void loadLDRValues() {
  String jsonString = readLDRValues();
  if (jsonString.length() > 0) {
    // Deserialize JSON string
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, jsonString);
    if (!error) {
      // Copy values to the array
      JsonArray array = doc.as<JsonArray>();
      for (int i = 0; i < array.size() && i < MAX_LDR_VALUES; i++) {
        ldrValues[i] = array[i];
      }
    }
  }
}

void ConnectToWifi(const char* ssid, const char* password) {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


