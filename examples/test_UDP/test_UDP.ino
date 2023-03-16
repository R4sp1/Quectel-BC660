#include <Quectel_BC660.h>

#define SERIAL_PORT Serial2

QuectelBC660 quectel = QuectelBC660(5, true);

void setup() 
{
	Serial.begin(115200);
	Serial.println("Quectel UDP connection test");
	Serial.println("===================");
	quectel.begin(&SERIAL_PORT);
    if(quectel.registered(5))
    {
        Serial.println("Module is registered to network");
    }
    quectel.setDeepSleep();
    Serial.println("======UDP SEND======");
    quectel.openUDP("0.0.0.0", 0);	// Replace 0.0.0.0 with your host IP adress and 0 with your PORT number
    delay(1000);
    quectel.sendDataUDP("Hello world!", 12);
    delay(1000);
    quectel.closeUDP();
    delay(1000);
    Serial.println("======UDP SEND DONE======");
    quectel.setDeepSleep(1);
}

void loop()
{

}
