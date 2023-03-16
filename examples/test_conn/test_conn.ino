#include <Quectel_BC660.h>

#define SERIAL_PORT Serial2

QuectelBC660 quectel = QuectelBC660(5, true);

void setup() 
{
	Serial.begin(115200);
	Serial.println("Quectel MQTT connection test");
	Serial.println("===================");
	quectel.begin(&SERIAL_PORT);
    if(quectel.registered(5))
    {
        Serial.println("Module is registered to network");
    }
    quectel.setDeepSleep();
    Serial.println("=====Deregister from network=====");
    if(quectel.setOperator(2))
    {
        Serial.println("Done!");
    }
    delay(1000);
    Serial.println("=====Set band to 20=====");
    if(quectel.setBand(true,1,20))
    {
        Serial.println("Done!");
    }
    delay(1000);
    Serial.println("=====Register to specific network=====");
    if(quectel.setOperator(1,2,"23003"))
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