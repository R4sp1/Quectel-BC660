#include <Quectel_BC660.h>
#include <OneWire.h>                                        // https://www.arduino.cc/reference/en/libraries/onewire/
#include <DallasTemperature.h>                              // https://github.com/milesburton/Arduino-Temperature-Control-Library

#define SERIAL_PORT Serial2                                 // Define hardware serial port for Quectel BC66 module (ESP32 Serial2 pins: RX=GPIO16, TX=GPIO17)
#define TPIN 32                                             // DS18B20 data pin

float temp;

QuectelBC660 quectel = QuectelBC660(5, false);               // Initialize Quectel BC660 library, first parameter define module wake up pin, second parameter toggles debug mode (true for debug on, false for debug off)
OneWire oneWire(TPIN);                                      // Initialize OneWire library
DallasTemperature oneWireTemp(&oneWire);                    // Initialize DallasTemperature library

void setup() 
{
    Serial.begin(115200);                                   // Initialize serial port
    
    Serial.println("Quectel MQTT client example");
    Serial.println("===================");
    
    quectel.begin(&SERIAL_PORT);                            // Initialize Quectel BC660 module
    oneWireTemp.begin();                                    // Initialize DS18B20 sensor

    while(!quectel.getRegistrationStatus(5)){                // Wait until module is registered to network
        Serial.println("Waiting for network registration...");
        delay(5000);
    }
    Serial.println("Module is successfully registered to network");

    oneWireTemp.requestTemperatures();                      // Request temperature from DS18B20 sensor
    temp = oneWireTemp.getTempCByIndex(0);                 // Get temperature from DS18B20 sensor
    Serial.println("\nTemp: " + String(temp) + " °C");     // Print temperature to serial monitor

    Serial.println("\nTurn off deep sleep mode");
    if(quectel.setDeepSleep()){                             // Turn of deep sleep mode
        Serial.println("\tDeep sleep mode turned off");
    }
    else{
        Serial.println("\tDeep sleep mode turn off failed");
    }
    delay(1000);

    Serial.println("\nOpen MQTT connection");
    if(quectel.openMQTT("0.0.0.0")){	                    // Open UDP socket, replace 0.0.0.0 with your host IP adress and 0 with your PORT number
        Serial.println("\tMQTT connection opened");
    }
    else{
        Serial.println("\tFailed to open MQTT connection");
    }
    delay(1000);

    Serial.println("\nConnect to MQTT broker");
    if(quectel.connectMQTT("Test-123456")){                 // Connect to MQTT broker, first parameter is client ID
        Serial.println("\tConnected to MQTT broker");
    }
    else{
        Serial.println("\tFailed to connect to MQTT broker");
    }
    delay(1000);

    Serial.println("\nPublish temperature reading to MQTT broker");
    String lastTemp = String(temp);
    int lastTempLength = lastTemp.length();
    if(quectel.publishMQTT(lastTemp.c_str(), lastTempLength, "MQTT/TOPIC")){  // Publish temperature reading to MQTT broker, first parameter is data to be sent, second parameter is data length, third parameter is MQTT topic
        Serial.println("\tTemperature reading published to MQTT broker");
    }
    else{
        Serial.println("\tFailed to publish temperature reading to MQTT broker");
    }
    delay(1000);

    Serial.println("\nClose MQTT connection");
    if(quectel.closeMQTT()){                                // Close MQTT connection
        Serial.println("\tMQTT connection closed");
    }
    else{
        Serial.println("\tFailed to close MQTT connection");
    }
    delay(1000);

    Serial.println("\nTurn on deep sleep mode");
    if(quectel.setDeepSleep(1)){                            // Turn on deep sleep mode, first parameter is deep sleep time in seconds
        Serial.println("\tDeep sleep mode turned on");
    }
    else{
        Serial.println("\tFailed to turn on deep sleep mode");
    }
    delay(1000);
    
    Serial.println("\nGoing to sleep for 30 seconds...");
    delay(10);
    esp_sleep_enable_timer_wakeup(30000000);                // Deep sleep for 30 seconds
    esp_deep_sleep_start();
}

void loop()
{
    
}