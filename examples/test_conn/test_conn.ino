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
    Serial.println("=====Deregister from network=====");
    if(quectel.deregisterFromNetwork())
    {
        Serial.println("Done!");
    }
    delay(1000);
    Serial.println("=====Set bands to 8 and 20=====");
    uint8_t bands[2] = {8, 20};
    if(quectel.setManualBand(2,bands))
    {
        Serial.println("Done!");
    }
    delay(1000);
    Serial.println("=====Register to specific network=====");
    if(quectel.manualRegisterToNetwork("23003"))
    {
        Serial.println("Done!");
    }
    delay(1000);
    Serial.println("======TEST DONE======");
    quectel.setDeepSleep(1);
}

void loop()
{

}