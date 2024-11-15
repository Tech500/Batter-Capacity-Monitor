/*

   Base on Microsoft Copilot generation C++ Code
   Code debugged and Modified by Wlliam Lucid 11/15/2024 @ 11:22 AM

*/


#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Wire.h>
#include <ThingSpeak.h>
#include <AdvancedSerial.h>

const char* ssid = "R2D2";
const char* password = "sissy4357";
const char* myWriteAPIKey = "E12345678";  // Dummy API key
const long myChannelNumber = 12345678;    // Dummy channel number

AdvancedSerial as;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);

WiFiClient client;

float initial_capacity_mah = 2000;  // Example: 2000mAh battery
float used_capacity_mah = 0;
unsigned long previousMillis = 0;

int interruptPin = 14;
volatile bool event = false;

void alert() {
    event = true;
    detachInterrupt(digitalPinToInterrupt(interruptPin));
}

void setup() {
    as.begin(115200);

    
    /* Uncomment the following line to disable the output. By defalut the ouput is on. */
    // aSerial.off();
    Serial.println("Starting setup...");

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    timeClient.begin();
    //ThingSpeak.begin(client);
    Wire.begin();

    attachInterrupt(digitalPinToInterrupt(interruptPin), alert, FALLING);
    Serial.println("Setup complete.");
}

void loop() {
    timeClient.update();
    unsigned long currentMillis = millis();

    if (timeClient.getMinutes() % 2 == 0 && timeClient.getSeconds() == 0 && currentMillis - previousMillis >= 60000) {
        previousMillis = currentMillis;

        // Randomized dummy values for testing
        float busVoltage = random(110, 130) / 10.0; // Random voltage between 11.0V and 13.0V
        float current = random(10, 50) / 10.0;      // Random current between 1.0A and 5.0A
        float power = busVoltage * current;
        float remaining_capacity = calculateRemainingCapacity(current);

        // Using advancedSerial methods for cleaner output
        as.println("Bus Voltage: " + String(busVoltage) + " V");
        as.println("Current: " + String(current) + " mA");
        as.println("Power: " + String(power) + " mW");
        as.println("Remaining Capacity: " + String(remaining_capacity) + " mAh");

        /*
        ThingSpeak.setField(1, busVoltage);
        ThingSpeak.setField(2, current);
        ThingSpeak.setField(3, power);
        ThingSpeak.setField(4, remaining_capacity);

        // Write fields to ThingSpeak
        int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
        if (x == 200) {
            as.println("Channel update successful.");
        } else {
            as.println("Problem updating channel. HTTP error code " + String(x));
        }
        */

        if (busVoltage < 3.3) {  // Set your low voltage threshold
            as.println("ALERT: Low Voltage!");
        }
        

        // Plot data on the Serial Plotter (plain numeric output for plotter)
        
        /*
        as.print(busVoltage);
        as.print(" ");
        as.print(current);
        as.print(" ");
        as.print(power);
        as.print(" ");
        */
        
        as.println(remaining_capacity); // Adjust capacity for better plot scale
    }

    delay(1000);  // Loop delay
}

float calculateRemainingCapacity(float current) {
    static unsigned long last_update = millis();
    unsigned long now = millis();
    unsigned long elapsedMillis = now - last_update;

    // Ensure elapsedMillis is at least 1 millisecond to prevent division by zero
    if (elapsedMillis < 1) {
        elapsedMillis = 1;
    }

    float hours_elapsed = elapsedMillis / 3600000.0;  // Convert milliseconds to hours
    last_update = now;

    used_capacity_mah += current * hours_elapsed;
    return initial_capacity_mah - used_capacity_mah;
}
