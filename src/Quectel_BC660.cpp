#include <Arduino.h>
#include "Quectel_BC660.h"

QuectelBC660::QuectelBC660(int8_t wakeUpPin, bool debug)
{
    _wakeUpPin = wakeUpPin;
    _debug = debug;

    if(_wakeUpPin != NOT){
        pinMode(_wakeUpPin, OUTPUT);
    }
}

bool QuectelBC660::begin(HardwareSerial *uart)
{
    _uart = uart;
    _uart->begin(115200);

    wakeUp();

    if(_sleepMode == NULL)
    {
        updateSleepMode();
    }

    if(!sendAndCheckReply("ATE0", _OK, 1000))
    {
        return false;
    }
    return true;
}

const char* QuectelBC660::getFirmwareVersion()
{
	return _firmwareVersion;
}

int8_t QuectelBC660::getRSSI()
{
    // Response: +CSQ: <rssi>,<ber>
    // RSSI: Integer type. Received signal strength level.
                // 0 = -113 dBm or less
                // 1 = -111 dBm
                // 2-30 = -109 to -53 dBm
                // 31 = -51 dBm or greater 
                // 99 = Not known or not detectable

    // Reply is:
    // +CSQ: 14,2
    //
    // OK
    wakeUp();
    if (sendAndWaitForReply("AT+CSQ", 1000, 3))
    {
        char * token = strtok(_buffer, " ");
        if (token)
        {
            token = strtok(nullptr, ",");
            if (token)
            {
                char* ptr;
                uint8_t rssi_queried;
                rssi_queried =  strtol(token, &ptr, 10);

                if (rssi_queried == 99)
                {
                    return rssi_queried;
                }
                else
                {
                    return (-113)+(rssi_queried*2);
                }
            }
        }
    }
    return 0;
}

uint8_t QuectelBC660::getBER()
{
    // Response: +CSQ: <rssi>,<ber>
    // BER: Integer type. Channel bit error rate (in percent).
                // 0-7 = RxQual values RXQUAL_0–RXQUAL_7 as defined in 3GPP TS 45.008
                // 99 = Not known or not detectable

    // Reply is:
    // +CSQ: 14,2
    //
    // OK
    wakeUp();
    if (sendAndWaitForReply("AT+CSQ", 1000, 3))
    {
        char * token = strtok(_buffer, " ");
        if (token)
        {
            token = strtok(nullptr, ",");
            if (token)
            {
                token = strtok(nullptr, "\n");
                if (token)
                {
                    char* ptr;
                    uint8_t ber_queried;
                    ber_queried =  strtol(token, &ptr, 10);

                    return ber_queried;
                }
            }
        }
    }
    return 0;
}

const char* QuectelBC660::getDateAndTime()
{
    // Response: +CCLK: <time>
    // Time: String type. The format is "YY/MM/DD,hh:mm:ss±zz", where characters indicate
    // year (two last digits), month, day, hour, minute, second and time zone (indicates
    // the difference, expressed in quarters of an hour, between the local time and GMT;
    // the range is -96 to +96.) For instance, 6th of May 2014, 22:10:00 GMT+2 hours
    // equals "14/05/06,22:10:00+08"

	// Reply is:
    // +CCLK: 20/11/03,06:25:06+32
    // 
    // OK
    wakeUp();
    if (sendAndWaitForReply("AT+CCLK?", 1000, 3))
    {
        char * token = strtok(_buffer, "\n");
        if (token)
        if (strlen(token) > 7)
        {
            strcpy(_dateAndTime, token + 7);
            return _dateAndTime;
        }
    }
    return "ERROR";
}

uint8_t QuectelBC660::getStatusCode()
{
    // Response: +CREG: <n>,<stat>
    // STAT: Integer type
                // 0 = Not registered, MT is not currently searching a new operator to register to
                // 1 = Registered, home network
                // 2 = Not registered, but MT is searching a new operator to register to
                // 3 = Registration denied
                // 4 = Unknown. (for example, out GERAN/UTRAN/E-UTRAN coverage)
                // 5 = Registered, roaming

    // Reply is:
    // +CREG: 0,1
    //
    // OK
    wakeUp();
    if (sendAndWaitForReply("AT+CEREG?", 1000, 3))
    {
        char * token = strtok(_buffer, " ");
        if (token)
        {
            token = strtok(nullptr, ",");
            if (token)
            {
                token = strtok(nullptr, "\n");
                if (token)
                {
                    char* ptr;
                    uint8_t statusCode;
                    statusCode =  strtol(token, &ptr, 10);
                    return statusCode;
                }
            }
        }
    }
    return 6;
}

const char* QuectelBC660::getStatus()
{
    uint8_t statusCode = getStatusCode();
    if(statusCode == 0)
    {
        return "0 = Not registered, MT is not currently searching a new operator to register to";
    }
    if(statusCode == 1)
    {
        return "1 = Registered, home network";
    }
    if(statusCode == 2)
    {
        return "2 = Not registered, but MT is searching a new operator to register to";
    }
    if(statusCode == 3)
    {
        return "3 = Registration denied";
    }
    if(statusCode == 4)
    {
        return "4 = Unknown. (for example, out GERAN/UTRAN/E-UTRAN coverage)";
    }
    if(statusCode == 5)
    {
        return "5 = Registered, roaming";
    }
    return "ERROR";
}

bool QuectelBC660::registered(uint8_t noOfTries, uint32_t delayBetweenTries)
{
    for(uint8_t i = 0; i < noOfTries; i++)
    {
        uint8_t statusCode = getStatusCode();
        if(_debug != false){
            Serial.print("\nStatus code: ");
            Serial.println(statusCode);
        }
        if(statusCode == 1 || statusCode == 5)
        {
            return true;
        }
        else
        {
            delay(delayBetweenTries);
        }
    }
    return false;
}

void QuectelBC660::wakeUp()
{
    if(_debug != false){
        Serial.print("\n(Wakeup: ");
    }
    if(_sleepMode == NULL)
    {
        updateSleepMode();
    }
    if(_sleepMode != 0)
    {
        if(_debug != false){
            Serial.print("Waking up module with: ");
        }
        if(_wakeUpPin != NOT){
            if(_debug != false){
            Serial.println("PSM_EINT pin!)");
            }
            digitalWrite(_wakeUpPin, HIGH);
            delay(300);
            digitalWrite(_wakeUpPin, LOW);
            delay(100);
        } else 
        {
            if(_debug != false){
            Serial.println("AT command!)");
            }
            sendAndCheckReply("AT", _OK, 1000);
            delay(100);
        }
    }
    else
    {
        if(_debug != false){
            Serial.println("Sleep mode disabled!)");
        }
    }

}

bool QuectelBC660::setDeepSleep(uint8_t sleepMode)
{
    _sleepMode = sleepMode;
    wakeUp();
    if(_sleepMode == 1)
    {
        if(_debug != false){
        Serial.println("\nEnabling light sleep and deep sleep!");
        }
        sendAndCheckReply("AT+QSCLK=1", _OK, 1000);
        return true;
    }
    else if(_sleepMode == 2)
    {
        if(_debug != false){
        Serial.println("Enabling light sleep only!");
        }
        sendAndCheckReply("AT+QSCLK=2", _OK, 1000);
        return true;
    }
    else
    {
        if(_debug != false){
        Serial.println("Disabling sleep modes!");
        }
        sendAndCheckReply("AT+QSCLK=0", _OK, 1000);
        return true;
    }
    return false;
}

bool QuectelBC660::openMQTT(const char* host, uint16_t port, uint8_t TCPconnectID)
{
    _TCPconnectID = TCPconnectID;

    // Write command: AT+QMTOPEN=<TCP_connectID>,<host_name>,<port>

    sprintf(_buffer, "AT+QMTOPEN=%d,\"%s\",%d", _TCPconnectID, host, port);

    // Reply is:
    // OK
    // 
    // +QMTOPEN: 0,0
    wakeUp();
    if(sendAndWaitForReply(_buffer, 2000, 3))
    {
        char * token = strtok(_buffer, ",");
        if (token)
        {
            token = strtok(nullptr, "\n");
            if (token)
            {
                char* ptr;
                uint8_t stat;
                stat =  strtol(token, &ptr, 10);

                if (stat == 0)
                {
                    if(_debug != false)
                    {
                    Serial.print("\nMQTT open succeeded, Stat: ");
                    Serial.println(stat);
                    }
                    return true;
                }
                else
                {
                    if(_debug != false)
                    {
                    Serial.print("\nMQTT open failed, Stat: ");
                    Serial.println(stat);
                    }
                    return false;
                }
            }
        }
    }
}

bool QuectelBC660::closeMQTT()
{
    // Write command: AT+QMTCLOSE=<TCP_connectID>   

    sprintf(_buffer, "AT+QMTCLOSE=%d", _TCPconnectID);
    if (!sendAndCheckReply(_buffer, _OK, 1000))
    {
        if(_debug != false)
        {
            Serial.println("\nFailed to close MQTT connection!");
        }
        return false;
    }
    if(_debug != false)
        {
            Serial.println("\nMQTT connection closed successfully!");
        }
    return true;
}

bool QuectelBC660::connectMQTT(const char* clientID)
{
    // Write command: AT+QMTCONN=<TCP_connectID>,<clientID>

    sprintf(_buffer, "AT+QMTCONN=%d,\"%s\"", _TCPconnectID, clientID);

    // Reply is:
    // OK
    // 
    // +QMTCONN: 0,0,0
    wakeUp();
    if(sendAndWaitForReply(_buffer, 1000, 3))
    {
        return true;
    }
    return false;
}

bool QuectelBC660::publishMQTT(const char* msg, uint16_t msgLen, const char* topic, uint16_t msgID, uint8_t QoS, uint8_t retain)
{
    // AT+QMTPUB=<TCP_connectID>,<msgID>,<QoS>,<retain>,<topic>,<msg_len>,<msg>

    sprintf(_buffer, "AT+QMTPUB=%d,%d,%d,%d,\"%s\",%d,\"%s\"", _TCPconnectID, msgID, QoS, retain, topic, msgLen, msg);

    // Reply is:
    // OK
    // 
    // +QMTPUB: 0,0,0
    wakeUp();
    if(sendAndWaitForReply(_buffer, 2000, 3))
    {
        return true;
    }
    return false;
}

bool QuectelBC660::openUDP(const char* host, uint16_t port, uint8_t TCPconnectID)
{
    _TCPconnectID = TCPconnectID;
    strcpy(_host, host);
    _port = port;
    sprintf(_buffer, "AT+QIOPEN=0,%d,\"UDP\",\"%s\",%d", _TCPconnectID, _host, _port);
    wakeUp();
    if(sendAndWaitForReply(_buffer, 60000, 3))
    {
        char * token = strtok(_buffer, ",");
        if (token)
        {
            token = strtok(nullptr, "\n");
            if (token)
            {
                char* ptr;
                uint8_t stat;
                stat =  strtol(token, &ptr, 10);

                if (stat == 0)
                {
                    if(_debug != false)
                    {
                    Serial.print("\nUDP client connected successfully, Stat: ");
                    Serial.println(stat);
                    }
                    return true;
                }
                else
                {
                    if(_debug != false)
                    {
                    Serial.print("\nUDP client connection failed, Stat: ");
                    Serial.println(stat);
                    }
                    return false;
                }
            }
        }
    }
}

bool QuectelBC660::closeUDP()
{
    sprintf(_buffer, "AT+QICLOSE=%d", _TCPconnectID);
    if (!sendAndCheckReply(_buffer, _OK, 1000))
    {
        if(_debug != false)
        {
            Serial.println("\nFailed to close UDP connection!");
        }
        return false;
    }
    if(_debug != false)
        {
            Serial.println("\nUDP connection closed successfully!");
        }
    return true;
}

bool QuectelBC660::sendDataUDP(const char* msg, uint16_t msgLen)
{
    sprintf(_buffer, "AT+QISEND=%d,\"%s\",\"%s\",%d", _TCPconnectID, _host, _port, msgLen);
    if (!sendAndWaitFor(_buffer, "> ", 5000))
    {
        if(_debug != false)
        {
            Serial.print("\nError occured before data send command");
        }
        return false;
    }
    if(_debug != false)
    {
        Serial.print("\n --> msg: ");
        Serial.print(msg);
        Serial.print(" , size: ");
        Serial.println(msgLen);
    }
    _uart->write(msg, msgLen);
    if (readReply(5000, 1) && strstr(_buffer, "SEND OK"))
    {
        return true;
    }
    if(_debug != false)
    {
        Serial.print("\nSend failed");
    }
    return false;
}

void QuectelBC660::getData(){
    wakeUp();
    // +QENG: 0,<sc_EARFCN>,<sc_EARFCN_offset>,<sc_pci>,<sc_cellID>,[<sc_RSRP>],[<sc_RSRQ>],[<sc_RSSI>],[<sc_SINR>],<sc_band>,<sc_TAC>,[<sc_ECL>],[<sc_Tx_pwr>],<operation_mode>
    if (sendAndWaitForReply("AT+QENG=0", 1000, 3))
    {
        char * token = strtok(_buffer, ",");
        for(int i = 0; i < 4; i++)
        {
            token = strtok(nullptr, ",");
        }
        token = strtok(nullptr, ",");
        if(token)
        {
                char* ptr;
                engineeringData.RSRP =  strtol(token, &ptr, 10);
        }
        token = strtok(nullptr, ",");
        if(token)
        {
                char* ptr;
                engineeringData.RSRQ =  strtol(token, &ptr, 10);
        }
        token = strtok(nullptr, ",");
        if(token)
        {
                char* ptr;
                engineeringData.RSSI =  strtol(token, &ptr, 10);
        }
        token = strtok(nullptr, ",");
        if(token)
        {
                char* ptr;
                engineeringData.SINR =  strtol(token, &ptr, 10);
        }
    }

    if (sendAndWaitForReply("AT+CGMR", 1000, 3))
    {
		// response is:
        // Revision: BC660KGLAAR01A01
        // 
        // OK

        char * token = strtok(_buffer, "\n");
        if (token)
        {
            strcpy(engineeringData.firmwareVersion, token + 10);
        }
    }

    
    /*if (sendAndWaitForReply("ATI", 1000, 5))
    {
		// response is:
		// Quectel_Ltd
    	// Quectel_BC660K-GL
        // Revision: BC660KGLAAR01A01
        // 
        // OK


        const char linefeed[] = "\n";
        char * token = strtok(_buffer, linefeed);
        if (token == nullptr || strcmp(token, "Quectel_Ltd") != 0)
        {
            if(_debug != false){
                Serial.println("\nNot a Quectel module!");
            }
        }
        token = strtok(nullptr, linefeed);
        if (token == nullptr)
        {
            if(_debug != false){
                Serial.println("\nParse error");
            }
            return false;
        }
        token = strtok(nullptr, linefeed);
        if (strlen(token) > 10)
        {
            strcpy(_firmwareVersion, token + 10);
        }
    }*/
}

bool QuectelBC660::sendAndWaitForReply(const char* command, uint16_t timeout, uint8_t lines)
{
    flush();
	if(_debug != false){
        Serial.print("\n --> ");
        Serial.println(command);
    }
    _uart->println(command);
    return readReply(timeout, lines);
}

bool QuectelBC660::sendAndWaitFor(const char* command, const char* reply, uint16_t timeout)
{
    uint16_t index = 0;

    flush();
	if(_debug != false){
        Serial.print("\n --> ");
        Serial.println(command);
    }
    _uart->println(command);
    while (timeout--)
    {
        if (index > 254)
        {
            break;
        }
        while (_uart->available())
        {
            char c = _uart->read();
            if (c == '\r')
            {
                continue;
            }
            if (c == '\n' && index == 0)
            {
                // Ignore first \n.
                continue;
            }
            _buffer[index++] = c;
        }

        if (strstr(_buffer, reply))
        {
            if(_debug != false){
            Serial.println("MAtch found");
            }
            break;
        }
        if (timeout <= 0)
        {
            if(_debug != false){
            Serial.print(" <-- (Timeout) ");
            Serial.println(_buffer);
            }
            return false;
        }
        delay(1);
    }
    _buffer[index] = 0;
    if(_debug != false){
        Serial.print(" <-- ");
        Serial.println(_buffer);
    }
    return true;
}

bool QuectelBC660::sendAndCheckReply(const char* command, const char* reply, uint16_t timeout)
{
    sendAndWaitForReply(command, timeout);
    return (strstr(_buffer, reply) != nullptr);
}

bool QuectelBC660::readReply(uint16_t timeout, uint8_t lines)
{
    uint16_t index = 0;
    uint16_t linesFound = 0;

    while (timeout--)
    {
	if (index > 254)
	{
	    break;
	}
	while (_uart->available())
	{
	    char c = _uart->read();
	    if (c == '\r')
	    {
		continue;
	    }
	    if (c == '\n' && index == 0)
	    {
		// Ignore first \n.
		continue;
	    }
	    _buffer[index++] = c;
	    if (c == '\n')
	    {
		linesFound++;
	    }
	    if (linesFound >= lines)
	    {
		break;
	    }
	}

	if (linesFound >=lines)
	{
	    break;
	}

	if (timeout <= 0)
	{
        if(_debug != false){
        Serial.print(" <-- (Timeout) ");
        Serial.println(_buffer);
        }
	    return false;
	}
	delay(1);
    }
    _buffer[index] = 0;
    if(_debug != false){
        Serial.print(" <-- ");
        Serial.println(_buffer);
    }
    return true;
}

void QuectelBC660::flush()
{
    while (_uart->available())
    {
        _uart->read();
    }
}

void QuectelBC660::updateSleepMode()
{
    if(sendAndWaitForReply("AT+QSCLK?", 1000, 3))
    {
        char * token = strtok(_buffer, " ");
        if (token)
        {
            token = strtok(nullptr, "\n");
            if(token)
            {
                char* ptr;
                _sleepMode =  strtol(token, &ptr, 10);
            }
        }
    }
}

