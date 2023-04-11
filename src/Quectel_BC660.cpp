#include <Arduino.h>
#include <time.h>
#include "Quectel_BC660.h"

// Constructor
QuectelBC660::QuectelBC660(int8_t wakeUpPin, bool debug){
    _wakeUpPin = wakeUpPin;
    _debug = debug;

    if(_wakeUpPin != NOT){
        pinMode(_wakeUpPin, OUTPUT);
    }
}

// Initialization (only HardwareSerial is supported for now)
bool QuectelBC660::begin(HardwareSerial *uart){
    _uart = uart;
    _uart->begin(115200);

    wakeUp();

    if(_sleepMode == NULL){
        updateSleepMode();
    }

    if(!sendAndCheckReply("ATE0", _OK, 1000)){
        return false;
    }
    return true;
}

// Status and information
const char* QuectelBC660::getFirmwareVersion(){
	return _firmwareVersion;
}

int8_t QuectelBC660::getRSSI(){
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
    if (sendAndWaitForReply("AT+CSQ", 1000, 3)){
        char * token = strtok(_buffer, " ");
        if (token){
            token = strtok(nullptr, ",");
            if (token){
                char* ptr;
                uint8_t rssi_queried;
                rssi_queried =  strtol(token, &ptr, 10);
                if (rssi_queried == 99){
                    return rssi_queried;
                }
                else{
                    return (-113)+(rssi_queried*2);
                }
            }
        }
    }
    return 0;
}

uint8_t QuectelBC660::getBER(){
    // Response: +CSQ: <rssi>,<ber>
    // BER: Integer type. Channel bit error rate (in percent).
                // 0-7 = RxQual values RXQUAL_0–RXQUAL_7 as defined in 3GPP TS 45.008
                // 99 = Not known or not detectable

    // Reply is:
    // +CSQ: 14,2
    //
    // OK
    wakeUp();
    if (sendAndWaitForReply("AT+CSQ", 1000, 3)){
        char * token = strtok(_buffer, " ");
        if (token){
            token = strtok(nullptr, ",");
            if (token){
                token = strtok(nullptr, "\n");
                if (token){
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

const char* QuectelBC660::getDateAndTime(){
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
    if (sendAndWaitForReply("AT+CCLK?", 1000, 3)){
        char * token = strtok(_buffer, "\n");
        if (token)
        if (strlen(token) > 7){
            strcpy(_dateAndTime, token + 7);
            return _dateAndTime;
        }
    }
    return "ERROR";
}

uint8_t QuectelBC660::getStatusCode(){
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
    if (sendAndWaitForReply("AT+CEREG?", 1000, 3)){
        char * token = strtok(_buffer, " ");
        if (token){
            token = strtok(nullptr, ",");
            if (token){
                token = strtok(nullptr, "\n");
                if (token){
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

bool QuectelBC660::setDeepSleep(uint8_t sleepMode){
    _sleepMode = sleepMode;
    wakeUp();
    if(_sleepMode == 1){
        if(_debug != false){
        Serial.println("\nEnabling light sleep and deep sleep!");
        }
        sendAndCheckReply("AT+QSCLK=1", _OK, 1000);
        return true;
    }
    else if(_sleepMode == 2){
        if(_debug != false){
        Serial.println("Enabling light sleep only!");
        }
        sendAndCheckReply("AT+QSCLK=2", _OK, 1000);
        return true;
    }
    else{
        if(_debug != false){
        Serial.println("Disabling sleep modes!");
        }
        sendAndCheckReply("AT+QSCLK=0", _OK, 1000);
        return true;
    }
    return false;
}

bool QuectelBC660::wakeUp(){
    if(_debug != false){
        Serial.print("\n(Wakeup: ");
    }
    /*if(_sleepMode == NULL){
        if(_wakeUpPin != NOT){
            digitalWrite(_wakeUpPin, HIGH);
            delay(300);
            digitalWrite(_wakeUpPin, LOW);
            delay(100);
        }
        else{
            sendAndCheckReply("AT", _OK, 1000);
            delay(100);
        }
        updateSleepMode();
        return true;
    }*/
    if(_sleepMode != 0){
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
            return true;
        } 
        else{
            if(_debug != false){
            Serial.println("AT command!)");
            }
            sendAndCheckReply("AT", _OK, 1000);
            delay(100);
            return true;
        }
    }
    else{
        if(_debug != false){
            Serial.println("sleep mode disabled!)");
        }
        return false;
    }
}

// eDRX and PSM timer settings
const char* QuectelBC660::getPSM(){
    // Reply is:
    // +CPSMS: <mode>,,,<requested_periodic_TAU>,<requested_active_time>
    // 
    // OK
    wakeUp();
    if (sendAndWaitForReply("AT+CPSMS?", 1000, 3)){
        char * token = strtok(_buffer, "\n");
        if (token)
        if (strlen(token) > 8){
            strcpy(_psm, token);
            return _psm;
        }
    }
    return "ERROR";
}

bool QuectelBC660::setPSM(const char* requested_periodic_TAU, const char* requested_active_time, uint8_t mode){
    // AT+CPSMS=<mode>[,,,<requested_periodic_TAU>[,<requested_active_time>]]
    // mode: Integer type
                // 0 = Disable PSM
                // 1 = Enable PSM
                // 2 = Disable the use of PSM and discard all parameters for PSM or, if avaliable, reset to the default values

    // requested_periodic_TAU: String type
                // One byte in an 8-bit format. Requested extended periodic TAU value (T3412) to be allocated to the UE in E-UTRAN.
                // Most significant bit (MSB) is coded first. For example: "01000111" equals 70 hours
                // Bits 8 to 6 defines the timer value unit as follows:
                // Bits
                // 8 7 6
                // 0 0 0 = value is incremented in multiples of 10 minutes
                // 0 0 1 = value is incremented in multiples of 1 hour
                // 0 1 0 = value is incremented in multiples of 10 hours
                // 0 1 1 = value is incremented in multiples of 2 seconds
                // 1 0 0 = value is incremented in multiples of 30 seconds
                // 1 0 1 = value is incremented in multiples of 1 minute
                // 1 1 0 = value is incremented in multiples of 320 hours
                // 1 1 1 = value indicates that the timer is deactivated

    // requested_active_time: String type
                // One byte in an 8-bit format. Requested active time value (T3324) to be allocated to the UE in E-UTRAN.
                // Most significant bit (MSB) is coded first. For example: "00100100" equals 4 minutes.
                // Bits 8 to 6 defines the timer value unit as follows:
                // Bits
                // 8 7 6
                // 0 0 0 = value is incremented in multiples of 2 seconds
                // 0 0 1 = value is incremented in multiples of 1 minute
                // 0 1 0 = value is incremented in multiples of 6 minutes
                // 1 1 1 = value indicates that the timer is deactivated


    sprintf(_buffer, "AT+CPSMS=%d,,,\"%s\",\"%s\"", mode, requested_periodic_TAU, requested_active_time);
    wakeUp();
    return sendAndCheckReply(_buffer, _OK, 1000);
}

// Network functions
bool QuectelBC660::setDefaultAPN(const char* PDP_type, const char* APN, const char* username, const char* password, uint8_t auth_type, uint32_t timeout){
    // Write the default PDP context to the module
    // AT+QCGDEFCONT=<PDP_type>,<APN>[,<username>,<password>[,<auth_type>]]
    // PDP_type: String type (IP, IPV6, IPV4V6, Non-IP)

    if(auth_type =! 0){
        sprintf(_buffer, "AT+QCGDEFCONT=\"%s\",\"%s\",\"%s\",\"%s\",%d", PDP_type, APN, username, password, auth_type);
    } 
    else{
        sprintf(_buffer, "AT+QCGDEFCONT=\"%s\",\"%s\"", PDP_type, APN);
    }
    wakeUp();
    return sendAndCheckReply(_buffer, _OK, timeout);
}


bool QuectelBC660::getRegistrationStatus(uint8_t noOfTries, uint32_t delayBetweenTries){
    for(uint8_t i = 0; i < noOfTries; i++){
        uint8_t statusCode = getStatusCode();
        if(_debug != false){
            Serial.print("\nStatus code: ");
            Serial.println(statusCode);
        }
        if(statusCode == 1 || statusCode == 5){
            return true;
        }
        else{
            delay(delayBetweenTries);
        }
    }
    return false;
}

bool QuectelBC660::deregisterFromNetwork(uint32_t timeout){
    // Write command: AT+COPS=2
    // Mode: 0 = automatic, 1 = manual operator sel, 2 = manualy deregister from network, 3 = Set <format> not shown in read command response, 4 = Manual/automatic selected. If manual selection fails, automatic mode(<mode>=0) is entered
    wakeUp();
    if (sendAndWaitFor("AT+COPS=2", _OK, timeout)){
        return true;
    }
    return false;
}

bool QuectelBC660::autoRegisterToNetwork(uint32_t timeout){
    // Write command: AT+COPS=0
    // Mode: 0 = automatic, 1 = manual operator sel, 2 = manualy deregister from network, 3 = Set <format> not shown in read command response, 4 = Manual/automatic selected. If manual selection fails, automatic mode(<mode>=0) is entered
    wakeUp();
    if (sendAndWaitFor("AT+COPS=0", _IP, timeout)){
        return true;
    }
    return false;
}

bool QuectelBC660::manualRegisterToNetwork(const char* operatorName, uint8_t mode, uint8_t format, uint32_t timeout){
    // Write command: AT+COPS=1,<format>,<oper> or AT+COPS=4,<format>,<oper> - if <mode> is not specified, default is 4, if <format> is not specified, default is 2
    // Mode: 0 = automatic, 1 = manual operator sel, 2 = manualy deregister from network, 3 = Set <format> not shown in read command response, 4 = Manual/automatic selected. If manual selection fails, automatic mode(<mode>=0) is entered
    // <format>: 0 = long alphanumeric, 1 = short alphanumeric, 2 = numeric
    // <oper>: Operator name or numeric code
    wakeUp();
    sprintf(_buffer, "AT+COPS=%d,%d,\"%s\"", mode, format, operatorName);
    if (sendAndWaitFor(_buffer, _IP, timeout)){
        return true;
    }
    return false;
}


bool QuectelBC660::setAutoBand(bool deregistred, uint32_t timeout){
    wakeUp();
    sprintf(_buffer, "AT+QBAND=0");
    if(deregistred){    
        if (sendAndWaitFor(_buffer, _OK, timeout)){
            return true;
        }
    }
    else{
        if (sendAndWaitFor(_buffer, _IP, timeout)){
            return true;
        }
    }
    return false;
}

bool QuectelBC660::setManualBand(uint8_t numOfBands, uint8_t *bands, bool deregistred, uint32_t timeout){
    wakeUp();
    sprintf(_buffer, "AT+QBAND=%d", numOfBands);
    for(uint8_t i = 0; i < numOfBands; i++){
        sprintf(_buffer, "%s,%d", _buffer, bands[i]);
    }
    if(deregistred){    
        if (sendAndWaitFor(_buffer, _OK, timeout)){
            return true;
        }
    }
    else{
        if (sendAndWaitFor(_buffer, _IP, timeout)){
            return true;
        }
    }
    return false;
}

// MQTT functions
bool QuectelBC660::openMQTT(const char* host, uint16_t port, uint8_t TCPconnectID){
    _TCPconnectID = TCPconnectID;

    // Write command: AT+QMTOPEN=<TCP_connectID>,<host_name>,<port>

    sprintf(_buffer, "AT+QMTOPEN=%d,\"%s\",%d", _TCPconnectID, host, port);

    // Reply is:
    // OK
    // 
    // +QMTOPEN: 0,0
    wakeUp();
    if(sendAndWaitForReply(_buffer, 5000, 3)){
        char * token = strtok(_buffer, ",");
        if (token){
            token = strtok(nullptr, "\n");
            if (token){
                char* ptr;
                uint8_t stat;
                stat =  strtol(token, &ptr, 10);

                if (stat == 0){
                    if(_debug != false){
                    Serial.print("\nMQTT open succeeded, Stat: ");
                    Serial.println(stat);
                    }
                    return true;
                }
                else{
                    if(_debug != false){
                    Serial.print("\nMQTT open failed, Stat: ");
                    Serial.println(stat);
                    }
                    return false;
                }
            }
        }
    }
    if(_debug != false){
        Serial.print("\nMQTT open failed, different error occured!");
    }
    return false;
}

bool QuectelBC660::closeMQTT(){
    // Write command: AT+QMTCLOSE=<TCP_connectID>   

    sprintf(_buffer, "AT+QMTCLOSE=%d", _TCPconnectID);
    if (!sendAndCheckReply(_buffer, _OK, 5000)){
        if(_debug != false){
            Serial.println("\nFailed to close MQTT connection!");
        }
        return false;
    }
    if(_debug != false){
        Serial.println("\nMQTT connection closed successfully!");
    }
    return true;
}

bool QuectelBC660::connectMQTT(const char* clientID){
    // Write command: AT+QMTCONN=<TCP_connectID>,<clientID>

    sprintf(_buffer, "AT+QMTCONN=%d,\"%s\"", _TCPconnectID, clientID);

    // Reply is:
    // OK
    // 
    // +QMTCONN: 0,0,0
    wakeUp();
    if(sendAndWaitForReply(_buffer, 5000, 3)){
        return true;
    }
    return false;
}

bool QuectelBC660::publishMQTT(const char* msg, uint16_t msgLen, const char* topic, uint16_t msgID, uint8_t QoS, uint8_t retain){
    // AT+QMTPUB=<TCP_connectID>,<msgID>,<QoS>,<retain>,<topic>,<msg_len>,<msg>

    sprintf(_buffer, "AT+QMTPUB=%d,%d,%d,%d,\"%s\",%d,\"%s\"", _TCPconnectID, msgID, QoS, retain, topic, msgLen, msg);

    // Reply is:
    // OK
    // 
    // +QMTPUB: 0,0,0
    wakeUp();
    if(sendAndWaitForReply(_buffer, 5000, 3)){
        return true;
    }
    return false;
}

// UDP functions
bool QuectelBC660::openUDP(const char* host, uint16_t port, uint8_t TCPconnectID){
    _TCPconnectID = TCPconnectID;
    strcpy(_host, host);
    _port = port;
    sprintf(_buffer, "AT+QIOPEN=0,%d,\"UDP\",\"%s\",%d", _TCPconnectID, _host, _port);
    wakeUp();
    if(sendAndWaitForReply(_buffer, 60000, 3)){
        char * token = strtok(_buffer, ",");
        if (token){
            token = strtok(nullptr, "\n");
            if (token){
                char* ptr;
                uint8_t stat;
                stat =  strtol(token, &ptr, 10);

                if (stat == 0){
                    if(_debug != false){
                    Serial.print("\nUDP client connected successfully, Stat: ");
                    Serial.println(stat);
                    }
                    return true;
                }
                else{
                    if(_debug != false){
                    Serial.print("\nUDP client connection failed, Stat: ");
                    Serial.println(stat);
                    }
                    return false;
                }
            }
        }
    }
    if(_debug != false){
        Serial.print("\nUDP client connection failed, different error occured!");
    }
    return false;
}

bool QuectelBC660::closeUDP(){
    sprintf(_buffer, "AT+QICLOSE=%d", _TCPconnectID);
    if (!sendAndCheckReply(_buffer, _OK, 1000)){
        if(_debug != false){
            Serial.println("\nFailed to close UDP connection!");
        }
        return false;
    }
    if(_debug != false){
        Serial.println("\nUDP connection closed successfully!");
    }
    return true;
}

bool QuectelBC660::sendDataUDP(const char* msg, uint16_t msgLen){
    sprintf(_buffer, "AT+QISEND=%d,%d", _TCPconnectID, msgLen);
    if (!sendAndWaitFor(_buffer, ">", 5000)){
        if(_debug != false){
            Serial.print("\nError occured before data send command");
        }
        return false;
    }
    if(_debug != false){
        Serial.print("\n --> msg: ");
        Serial.print(msg);
        Serial.print(" , size: ");
        Serial.println(msgLen);
    }
    _uart->write(msg, msgLen);
    if (readReply(5000, 3) && strstr(_buffer, "SEND OK")){
        return true;
    }
    if(_debug != false){
        Serial.print("\nSend failed");
    }
    return false;
}

// Engineering data functions
void QuectelBC660::getData(){
    // Engineering data
    // +QENG: 0,<sc_EARFCN>,<sc_EARFCN_offset>,<sc_pci>,<sc_cellID>,[<sc_RSRP>],[<sc_RSRQ>],[<sc_RSSI>],[<sc_SINR>],<sc_band>,<sc_TAC>,[<sc_ECL>],[<sc_Tx_pwr>],<operation_mode>
    wakeUp();
    if (sendAndWaitForReply("AT+QENG=0", 1000, 3)){
        char * token = strtok(_buffer, ",");
        for(int i = 0; i < 4; i++){
            token = strtok(nullptr, ",");
        }
        token = strtok(nullptr, ",");
        if(token){
            char* ptr;
            engineeringData.RSRP =  strtol(token, &ptr, 10);
        }
        token = strtok(nullptr, ",");
        if(token){
            char* ptr;
            engineeringData.RSRQ =  strtol(token, &ptr, 10);
        }
        token = strtok(nullptr, ",");
        if(token){
            char* ptr;
            engineeringData.RSSI =  strtol(token, &ptr, 10);
        }
        token = strtok(nullptr, ",");
        if(token){
            char* ptr;
            engineeringData.SINR =  strtol(token, &ptr, 10);
        }
    }

    // Firmware  version
    wakeUp();
    if (sendAndWaitForReply("AT+CGMR", 1000, 3)){
		// response is:
        // Revision: BC660KGLAAR01A01
        // 
        // OK

        char * token = strtok(_buffer, "\n");
        if (token){
            strcpy(engineeringData.firmwareVersion, token + 10);
        }
    }

    // Date and time
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
    if (sendAndWaitForReply("AT+CCLK?", 1000, 3)){
        char * token1 = strtok(_buffer, "\n");
        if(token1){
            char test[3];
            strcpy(test, token1 + 24);
            char sign[1];
            sign[0] = test[0];
            char timezone[2];
            // strcpy(sign, token + 24);
            strcpy(timezone, test + 1);
            if(strcmp(sign, "+")){
                char* ptr;
                engineeringData.timezone = strtol(timezone, &ptr, 10)/4;
            }
            else{
                char* ptr;
                engineeringData.timezone = 0 - strtol(timezone, &ptr, 10)/4;
            }
        }
        char * token = strtok(_buffer, " ");
        token = strtok(nullptr, "/");
        if (token){
            char* ptr;
            t.tm_year = 100 + strtol(token, &ptr, 10); // +100 because we have only two last digits -> 2000 + xx = 20xx but we must substract 1900 for epoch time
        }
        token = strtok(nullptr, "/");
        if(token){
            char* ptr;
            t.tm_mon = strtol(token, &ptr, 10) - 1;
        }
        token = strtok(nullptr, ",");
        if(token){
            char* ptr;
            t.tm_mday = strtol(token, &ptr, 10);
        }
        token = strtok(nullptr, ":");
        if(token){
            char* ptr;
            t.tm_hour = strtol(token, &ptr, 10) - engineeringData.timezone;
        }
        token = strtok(nullptr, ":");
        if(token){
            char* ptr;
            t.tm_min = strtol(token, &ptr, 10);
        }
        token = strtok(nullptr, "+-");
        if(token){
            char* ptr;
            t.tm_sec = strtol(token, &ptr, 10);
        }
        engineeringData.epoch = mktime(&t);
    }
}

// Replay management functions
bool QuectelBC660::sendAndWaitForReply(const char* command, uint32_t timeout, uint8_t lines){
    flush();
	if(_debug != false){
        Serial.print("\n --> ");
        Serial.println(command);
    }
    _uart->println(command);
    return readReply(timeout, lines);
}

bool QuectelBC660::sendAndWaitFor(const char* command, const char* reply, uint32_t timeout){
    uint16_t index = 0;

    flush();
	if(_debug != false){
        Serial.print("\n --> ");
        Serial.println(command);
    }
    _uart->println(command);
    while (timeout--){
        if (index > 254){
            break;
        }
        while (_uart->available()){
            char c = _uart->read();
            if (c == '\r'){
                continue;
            }
            if (c == '\n' && index == 0){
                // Ignore first \n.
                continue;
            }
            _buffer[index++] = c;
        }

        if (strstr(_buffer, reply)){
            if(_debug != false){
            Serial.println("Match found");
            }
            break;
        }
        if (timeout <= 0){
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

bool QuectelBC660::sendAndCheckReply(const char* command, const char* reply, uint32_t timeout){
    sendAndWaitForReply(command, timeout);
    return (strstr(_buffer, reply) != nullptr);
}

bool QuectelBC660::readReply(uint32_t timeout, uint8_t lines){
    uint16_t index = 0;
    uint16_t linesFound = 0;

    while (timeout--){
	if (index > 254){
	    break;
	}
	while (_uart->available()){
	    char c = _uart->read();
	    if (c == '\r'){
		continue;
	    }
	    if (c == '\n' && index == 0){
		// Ignore first \n.
		continue;
	    }
	    _buffer[index++] = c;
	    if (c == '\n'){
		linesFound++;
	    }
	    if (linesFound >= lines){
		break;
	    }
	}

	if (linesFound >=lines){
	    break;
	}

	if (timeout <= 0){
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

// Flush serial buffer
void QuectelBC660::flush(){
    while (_uart->available()){
        _uart->read();
    }
}

void QuectelBC660::updateSleepMode(){
    if(sendAndWaitForReply("AT+QSCLK?", 1000, 3)){
        char * token = strtok(_buffer, " ");
        if (token){
            token = strtok(nullptr, "\n");
            if(token){
                char* ptr;
                _sleepMode =  strtol(token, &ptr, 10);
            }
        }
    }
}