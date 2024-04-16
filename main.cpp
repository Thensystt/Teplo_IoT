#include <SoftwareSerial.h>

SoftwareSerial SIM800L(8, 9);  // RX, TX
#define led 13
void setup() {
    Serial.begin(9600);   // Start serial communication with the serial monitor
    SIM800L.begin(9600);  // Start communication with SIM800L
    pinMode(led, OUTPUT);
    Serial.println("Checking connection to SIM800L module...");
    delay(5000);

    // Check connection to SIM800L module
    if (sendCommand("AT\r\n", 2000)) {
        Serial.println("SIM800L module connected");
    } else {
        Serial.println("Error connecting to SIM800L module");
        digitalWrite(led, HIGH);
        while (1)
            ;
    }

    // Check connection to network
    if (sendCommand("AT+CSQ\r\n", 2000)) {
        Serial.println("SIM800L module connected to network");
    } else {
        Serial.println("SIM800L module not connected to network");
    }

    // Configure Access Point Name (APN)
    if (sendCommand("AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r\n", 5000)) {
        Serial.println("Connection type set");
    } else {
        Serial.println("Error setting connection type");
    }

    if (sendCommand("AT+SAPBR=3,1,\"APN\",\"internet\"\r\n", 5000)) {
        Serial.println("APN set");
    } else {
        Serial.println("Error setting APN");
    }

    // Open GPRS connection
    if (sendCommand("AT+SAPBR=1,1\r\n", 30000)) {  // Increased timeout to 30 seconds
        Serial.println("GPRS connection opened");
        digitalWrite(led, LOW);

        // Send data to ThingsBoard
        sendTelemetryToThingsBoard();
    } else {
        Serial.println("Error opening GPRS connection");
    }
}

void loop() {
}

bool sendCommand(String command, int timeout) {
    String response = "";

    SIM800L.print(command);

    long int time = millis();
    while ((time + timeout) > millis()) {
        while (SIM800L.available()) {
            char c = SIM800L.read();
            response += c;
        }
    }

    Serial.print(response);

    if (response.indexOf("OK") != -1) {
        return true;
    }
    return false;
}

void sendTelemetryToThingsBoard() {
    String payload = "{\"temperature\":25}";  // Modify this payload as needed

    // Send HTTP POST request to ThingsBoard
    SIM800L.print("AT+HTTPINIT\r\n");
    delay(1000);

    SIM800L.print("AT+HTTPPARA=\"CID\",1\r\n");
    delay(1000);

    SIM800L.print("AT+HTTPPARA=\"URL\",\"http://thingsboard.cloud/api/v1/VGIzalADkbRbGyHzmldo/telemetry\"\r\n");
    delay(1000);

    SIM800L.print("AT+HTTPPARA=\"CONTENT\",\"application/json\"\r\n");
    delay(1000);

    SIM800L.print("AT+HTTPDATA=" + String(payload.length()) + ",5000\r\n");
    delay(1000);

    SIM800L.print(payload);
    delay(1000);

    SIM800L.print("AT+HTTPACTION=1\r\n");
    delay(20000);  // Increased delay to 20 seconds

    SIM800L.print("AT+HTTPREAD\r\n");
    delay(5000);  // Adjust delay according to expected response time

    SIM800L.print("AT+HTTPTERM\r\n");
}