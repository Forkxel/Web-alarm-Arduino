#include <WiFi.h>
#include <WiFiServer.h>

const char* ssid = "Pavel - iPhone";
const char* password = "chalkaww"; 
 
const int ledPin = 2; 
const int buzzerPin = 5;
const int magneticSwitchPin = 8;

String correctPassword = "1234";
String enteredPassword = "";
bool passwordEntered = false;
unsigned long alarmTimerStart = 0;
bool timerRunning = false;
 
int counter = 0;
String buttonState = "/OFF";
bool alarmState = false;

WiFiServer server(80);

void setup() {
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(magneticSwitchPin, INPUT_PULLUP);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to Wi-Fi...");
  }

  Serial.println("Connected to Wi-Fi");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
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
      enteredPassword.trim(); // Remove any trailing whitespace or newline
      if (enteredPassword == correctPassword) {
        passwordEntered = true;
        timerRunning = false;
        alarmState = false;
        Serial.println("Correct password entered. Alarm disarmed.");
      } else {
        passwordEntered = false;
        Serial.println("Incorrect password entered.");
      }
    }

    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println();
    client.println("<!DOCTYPE HTML>");
    client.println("<html lang=\"en\">");
    client.println("<head>");
    client.println("<meta charset=\"UTF-8\">");
    client.println("<title>Alarm</title>");
    client.println("<style>");
    client.println("*{box-sizing: border-box; text-align: center; font-size: 1.5em}");
    client.println("h1 {font-size: 2em; color: darkblue;}");
    client.println("</style>");
    client.println("</head>");
    client.println("<body>");
    client.println("<h1>Controller for alarm</h1>");
    client.println("<form action=\"\" method=\"GET\">");
    client.println("<label for=\"password\">Enter Password:</label><br>");
    client.println("<input type=\"text\" id=\"password\" name=\"password\"><br><br>");
    client.println("<input type=\"submit\" value=\"Submit\">");
    client.println("</form>");
    client.println("</body>");
    client.println("</html>");
    
    client.stop();

     if (!passwordEntered && !timerRunning) {
      timerRunning = true;
      alarmTimerStart = millis();
      Serial.println("No password entered or incorrect password. Starting timer.");
    }
  }

  // Continuously check the state of the magnetic switch
  if (digitalRead(magneticSwitchPin) == HIGH) {
    if (!passwordEntered && !timerRunning) {
      timerRunning = true;
      alarmTimerStart = millis();
      Serial.println("Magnetic switch opened. Starting timer.");
    }
  } else {
    if (!alarmState && timerRunning) {
      timerRunning = false;
      Serial.println("Magnetic switch closed. Timer reset.");
    }
  }

  // Check if the timer has expired
  if (timerRunning && (millis() - alarmTimerStart >= 10000)) {
    if (digitalRead(magneticSwitchPin) == HIGH) { // Ensure the switch is still open
      alarmState = true; // Trigger the alarm
      timerRunning = false; // Stop the timer
      Serial.println("Timer expired. Alarm triggered.");
    } else {
      timerRunning = false; // Reset timer if switch is closed
      Serial.println("Timer reset as magnetic switch was closed.");
    }
  }

  // Allow the alarm to be turned off with the correct password
  if (alarmState && passwordEntered) {
    alarmState = false;
    Serial.println("Alarm turned off with correct password.");
  }

  if (alarmState) {
    digitalWrite(ledPin, HIGH);
    digitalWrite(buzzerPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
    digitalWrite(buzzerPin, LOW);
  }
}
/*
void loop() {
  WiFiClient client = server.available();
  if (client) {
    String request = client.readStringUntil('\r');
    Serial.println(request);
    client.flush();

    if (request.indexOf("/ON") != -1) {
      if (digitalRead(magneticSwitchPin) == HIGH) {
        alarmState = true;
      }
    }

    if (request.indexOf("/OFF") != -1) {
      if (digitalRead(magneticSwitchPin) == LOW) {
        alarmState = false;
      }
    }

    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println();
    client.println("<!DOCTYPE HTML>");
    client.println("<html lang=\"en\">");
    client.println("<head>");
    client.println("<meta charset=\"UTF-8\">");
    client.println("<title>Alarm</title>");
    client.println("<style>");
    client.println("*{box-sizing: border-box; text-align: center; font-size: 1.5em}");
    client.println("h1 {font-size: 2em; color: darkblue;}");
    client.println("</style>");
    client.println("</head>");
    client.println("<body>");
    client.println("<h1>Controller for alarm</h1>");
    client.println("<p><a href=\"/ON\"><button>Zapnout</button></a></p>");
    client.println("<p><a href=\"/OFF\"><button>Vypnout</button></a></p>");
    client.println("</body>");
    client.println("</html>");
    
    client.stop();

  if (alarmState) {
    digitalWrite(ledPin, HIGH);
    digitalWrite(buzzerPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
    digitalWrite(buzzerPin, LOW);
  }
    
  }
}
*/