#include <WiFi.h>
#include <WiFiServer.h>
#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"

const char* ssid = "Pavel - iPhone";
const char* password = "chalkaww"; 
String ip;

ArduinoLEDMatrix matrix;
const char text[] = "    172.20.10.4";
 
const int ledPin = 2; 
const int buzzerPin = 5;
const int magneticSwitchPin = 8;

String correctPassword = "1234";
String enteredPassword = "";
bool passwordEntered = false;
unsigned long alarmTimerStart = 0;
bool timerRunning = false;
unsigned long disarmTimerStart = 0;
bool disarmTimerRunning = false;
bool waitingForDoorClose = false;

bool alarmState = false;

WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  matrix.begin();

  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(magneticSwitchPin, INPUT_PULLUP);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to Wi-Fi...");
  }

  //ip = WiFi.localIP().toString();

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  server.begin();

  matrix.beginDraw();
  matrix.textFont(Font_4x6);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.textScrollSpeed(150);
  matrix.println(text);
  matrix.endText(SCROLL_LEFT);
  matrix.endDraw();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Lost connection to Wi-Fi. Reconnecting...");
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Reconnecting to Wi-Fi...");
    }

    Serial.println("Reconnected to Wi-Fi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }
  
  WiFiClient client = server.available();
  if (client) {
    String request = client.readStringUntil('\r');
    Serial.println(request);
    client.flush();

    int passwordIndex = request.indexOf("password=");
    if (passwordIndex != -1) {
      int endOfPassword = request.indexOf(' ', passwordIndex);
      if (endOfPassword == -1) {
        endOfPassword = request.length();
      }
      enteredPassword = request.substring(passwordIndex + 9, endOfPassword);
      enteredPassword.trim();

      if (enteredPassword == correctPassword) {
        passwordEntered = true;
        timerRunning = false;
        alarmState = false;
        disarmTimerRunning = true;
        disarmTimerStart = millis();
        waitingForDoorClose = true;
        Serial.println("Correct password entered. Alarm disarmed. Waiting for door to close or timer to expire.");
      } else {
        passwordEntered = false;
        Serial.println("Incorrect password entered.");
      }
    }

    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println("Cache-Control: no-cache, no-store, must-revalidate");
    client.println("Pragma: no-cache");
    client.println("Expires: 0");
    client.println();
    client.println("<!DOCTYPE HTML>");
    client.println("<html lang=\"en\">");
    client.println("<head>");
    client.println("<meta charset=\"UTF-8\">");
    client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">");
    client.println("<title>Alarm Control</title>");
    client.println("<style>");
    client.println("body {");
    client.println("  font-family: Arial, sans-serif;");
    client.println("  background-color: #f4f4f9;");
    client.println("  margin: 0;");
    client.println("  padding: 0;");
    client.println("  display: flex;");
    client.println("  justify-content: center;");
    client.println("  align-items: center;");
    client.println("  height: 100vh;");
    client.println("}");
    client.println(".container {");
    client.println("  background: #fff;");
    client.println("  box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);");
    client.println("  border-radius: 8px;");
    client.println("  padding: 20px;");
    client.println("  text-align: center;");
    client.println("  max-width: 400px;");
    client.println("  width: 100%;");
    client.println("}");
    client.println("h1 {");
    client.println("  font-size: 2em;");
    client.println("  margin-bottom: 20px;");
    client.println("  color: #333;");
    client.println("}");
    client.println("form {");
    client.println("  display: flex;");
    client.println("  flex-direction: column;");
    client.println("}");
    client.println("label {");
    client.println("  font-size: 1.2em;");
    client.println("  margin-bottom: 8px;");
    client.println("  color: #555;");
    client.println("}");
    client.println("input[type='text'] {");
    client.println("  padding: 10px;");
    client.println("  font-size: 1em;");
    client.println("  border: 1px solid #ccc;");
    client.println("  border-radius: 4px;");
    client.println("  margin-bottom: 20px;");
    client.println("}");
    client.println("input[type='submit'] {");
    client.println("  background: #007bff;");
    client.println("  color: #fff;");
    client.println("  border: none;");
    client.println("  padding: 10px;");
    client.println("  font-size: 1em;");
    client.println("  border-radius: 4px;");
    client.println("  cursor: pointer;");
    client.println("  transition: background 0.3s;");
    client.println("}");
    client.println("input[type='submit']:hover {");
    client.println("  background: #0056b3;");
    client.println("}");
    client.println("</style>");
    client.println("</head>");
    client.println("<body>");
    client.println("<div class=\"container\">");
    client.println("<h1>Alarm Controller</h1>");
    client.println("<form action=\"\" method=\"GET\">");
    client.println("<label for=\"password\">Enter Password:</label>");
    client.println("<input type=\"text\" id=\"password\" name=\"password\" placeholder=\"Enter password\">");
    client.println("<input type=\"submit\" value=\"Submit\">");
    client.println("</form>");
    client.println("</div>");
    client.println("</body>");
    client.println("</html>");

    client.stop();

    enteredPassword = "";
  }

  if (digitalRead(magneticSwitchPin) == HIGH) {
    if (!timerRunning && !waitingForDoorClose) {
      timerRunning = true;
      alarmTimerStart = millis();
      Serial.println("Magnetic switch opened. Starting timer.");
    }
  }

  if (timerRunning && (millis() - alarmTimerStart >= 2000)) {
    timerRunning = false;
    alarmState = true;
    Serial.println("Timer expired. Alarm triggered.");
  }

  if (disarmTimerRunning && (millis() - disarmTimerStart >= 5000)) {
    disarmTimerRunning = false;
    waitingForDoorClose = false;
    passwordEntered = false;
    Serial.println("Disarm timer expired.");
    if (digitalRead(magneticSwitchPin) == HIGH) {
      Serial.println("Door still open. System in monitoring mode, waiting for door to close.");
    } else {
      Serial.println("Door closed. System rearmed.");
    }
  }

  if (waitingForDoorClose && digitalRead(magneticSwitchPin) == LOW) {
    waitingForDoorClose = false;
    disarmTimerRunning = false;
    passwordEntered = false; 
    Serial.println("Door closed. System rearmed.");
  }

  if (alarmState && passwordEntered) {
    alarmState = false;
    passwordEntered = false; 
    disarmTimerRunning = true;
    disarmTimerStart = millis();
    waitingForDoorClose = true;
    Serial.println("Alarm turned off with correct password. Waiting for door to close or timer to expire.");
  }

  if (alarmState) {
    digitalWrite(ledPin, HIGH);
    digitalWrite(buzzerPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
    digitalWrite(buzzerPin, LOW);
  }
}
