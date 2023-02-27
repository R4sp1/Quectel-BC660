#include <Quectel_BC660.h>

#define SERIAL_PORT Serial2

QuectelBC660 quectel = QuectelBC660(5, true);

void setup() 
{
	Serial.begin(115200);
	Serial.println("Quectel test ready fun");
	Serial.println("===================");
	quectel.begin(&SERIAL_PORT);
    if(quectel.registered(5))
    {
        Serial.println("Module is registered to network");
    }
    
}

void loop()
{

}