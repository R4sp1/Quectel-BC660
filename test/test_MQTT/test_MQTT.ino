#include <Quectel_BC660.h>

#define SERIAL_PORT Serial2

QuectelBC660 quectel = QuectelBC660(5, true);

void setup() 
{
	Serial.begin(115200);
	Serial.println("Quectel MQTT connection test");
	Serial.println("===================");
	quectel.begin(&SERIAL_PORT);
    if(quectel.getRegistrationStatus(5))
    {
        Serial.println("Module is registered to network");
    }
    quectel.setDeepSleep();
    Serial.println("======MQTT SEND======");
    quectel.openMQTT("0.0.0.0");	// Replace 0.0.0.0 with address of your MQTT broker
    delay(1000);
    quectel.connectMQTT("Test-123456");
    delay(1000);
    quectel.publishMQTT("Hello world!", 12, "MQTT/TOPIC");
    delay(1000);
    quectel.closeMQTT();
    delay(1000);
    Serial.println("======MQTT SEND DONE======");
    quectel.setDeepSleep(1);
}

void loop()
{

}
