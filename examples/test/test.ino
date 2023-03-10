#include <Quectel_BC660.h>

#define SERIAL_PORT Serial2

QuectelBC660 quectel = QuectelBC660(5, true);


void setup() 
{
	Serial.begin(115200);
	Serial.println("Quectel test sketch");
	Serial.println("===================");
	Serial.println("Initializing module");
	quectel.begin(&SERIAL_PORT);
	quectel.setDeepSleep();
	Serial.print("RSSI: ");	
	Serial.println(quectel.getRSSI());
	Serial.print("BER: "); 
	Serial.println(quectel.getBER());
	Serial.print("Time: "); 
	Serial.println(quectel.getDateAndTime());
	Serial.print("Status code: "); 
	Serial.println(quectel.getStatusCode());
	Serial.print("Status: "); 
	Serial.println(quectel.getStatus());
	quectel.getData();
	Serial.print("RSRP: "); 
  	Serial.println(quectel.engineeringData.RSRP);
	Serial.print("RSRQ: "); 
  	Serial.println(quectel.engineeringData.RSRQ);
	Serial.print("RSSI: "); 
  	Serial.println(quectel.engineeringData.RSSI);
	Serial.print("SINR: "); 
  	Serial.println(quectel.engineeringData.SINR);
	Serial.print("Firmware: ");
	Serial.println(quectel.engineeringData.firmwareVersion);
	Serial.print("Epoch: ");
	Serial.println(quectel.engineeringData.epoch);
	Serial.print("Time zone: ");
	Serial.println(quectel.engineeringData.timezone);
	quectel.setDeepSleep(1);
}

void loop()
{
	delay(10000);	
	quectel.getData();
	Serial.print("RSRP: "); 
  	Serial.println(quectel.engineeringData.RSRP);
	Serial.print("RSRQ: "); 
  	Serial.println(quectel.engineeringData.RSRQ);
	Serial.print("RSSI: "); 
  	Serial.println(quectel.engineeringData.RSSI);
	Serial.print("SINR: "); 
  	Serial.println(quectel.engineeringData.SINR);
	Serial.print("Epoch: "); 
	Serial.println(quectel.engineeringData.epoch);
}