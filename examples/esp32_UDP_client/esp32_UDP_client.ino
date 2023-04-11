#include <Quectel_BC660.h>

#define SERIAL_PORT Serial2                                 // Define hardware serial port for Quectel BC66 module (ESP32 Serial2 pins: RX=GPIO16, TX=GPIO17)

QuectelBC660 quectel = QuectelBC660(5, true);               // Initialize Quectel BC660 library, first parameter define module wake up pin, second parameter toggles debug mode (true for debug on, false for debug off)

void setup() 
{
    Serial.begin(115200);                                   // Initialize serial port
    Serial.println("Quectel UDP client example");
    Serial.println("===================");
    
    quectel.begin(&SERIAL_PORT);                            // Initialize Quectel BC660 module

    while(!quectel.getRegistrationStatus(5)){                // Wait until module is registered to network
        Serial.println("Waiting for network registration...");
        delay(5000);
    }
    Serial.println("Module is registered to network");

    Serial.println("Turn off deep sleep mode");
    if(quectel.setDeepSleep()){                             // Turn of deep sleep mode
        Serial.println("Deep sleep mode turned off");
    }
    else{
        Serial.println("Failed to turn off deep sleep mode");
    }

    Serial.println("Open UDP socket");
    if(quectel.openUDP("0.0.0.0", 0)){	                    // Open UDP socket, replace 0.0.0.0 with your host IP adress and 0 with your PORT number
        Serial.println("UDP socket opened");
    }
    else{
        Serial.println("Failed to open UDP socket");
    }
    delay(1000);

    Serial.println("Send data to host");
    if(quectel.sendDataUDP("Hello world!", 12)){            // Send data to host, first parameter is data, second parameter is data length
        Serial.println("Data sent");
    }
    else{
        Serial.println("Failed to send data");
    }
    delay(1000);

    Serial.println("Close UDP socket");
    if(quectel.closeUDP()){                                 // Close UDP socket
        Serial.println("UDP socket closed");
    }
    else{
        Serial.println("Failed to close UDP socket");
    }
    delay(1000);
    Serial.println("======UDP SEND DONE======");

    Serial.println("Reenable deep sleep mode");
    if(quectel.setDeepSleep(1)){                            // Turn on deep sleep mode
        Serial.println("Deep sleep mode turned on");
    }
    else{
        Serial.println("Failed to turn on deep sleep mode");
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