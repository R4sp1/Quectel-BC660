#ifndef __Quectel_BC660_h__
#define __Quectel_BC660_h__

#include "Arduino.h"

#define NOT -1
#define ONE_SEC 1000
#define FIVE_SEC 5000
#define ONE_MIN 60000
#define FIVE_MIN 300000
#define TEN_MIN 600000


class QuectelBC660 {
    public:
        // Constructor
        QuectelBC660(int8_t wakeUpPin = NOT, bool debug = false);

        // Initialization (only HardwareSerial is supported for now)
        bool begin(HardwareSerial *uart);

        // Status and information
        const char* getFirmwareVersion();
        const char* getDateAndTime();
        int8_t getRSSI();
        uint8_t getBER();
        uint8_t getStatusCode();
        const char* getStatus();
        bool setDeepSleep(uint8_t sleepMode = 0);
        bool wakeUp();

        // eDRX and PSM timers
        const char* getPSM();
        bool setPSM(const char* requested_periodic_TAU, const char* requested_active_time, uint8_t mode = 1);

        // Network
        bool setUEFun(uint8_t fun = 1);
        bool setDefaultAPN(const char* PDP_type, const char* APN, const char* username = "", const char* password = "", uint8_t auth_type = 0, uint32_t timeout = FIVE_MIN);
        bool getRegistrationStatus(uint8_t noOfTries = 1, uint32_t delayBetweenTries = FIVE_SEC);
        bool deregisterFromNetwork(uint32_t timeout = FIVE_MIN);
        bool autoRegisterToNetwork(uint32_t timeout = FIVE_MIN);
        bool manualRegisterToNetwork(const char* oper, uint8_t mode = 4, uint8_t format = 2, uint32_t timeout = FIVE_MIN);
        bool setAutoBand(bool deregistred = true, uint32_t timeout = FIVE_MIN);
        bool setManualBand(uint8_t numOfBands, uint8_t *bands, bool deregistred = true, uint32_t timeout = FIVE_MIN);
        
        
        // MQTT
        bool openMQTT(const char* host, uint16_t port = 1883, uint8_t TCPconnectID = 0);
        bool closeMQTT();
        bool connectMQTT(const char* clientID);
        bool publishMQTT(const char* msg, uint16_t msgLen, const char* topic, uint16_t msgID = 0, uint8_t QoS = 0, uint8_t retain = 0);

        // UDP socket
        bool openUDP(const char* host, uint16_t port, uint8_t TCPconnectID = 0);
        bool closeUDP();
        bool sendDataUDP(const char* msg, uint16_t msgLen);
        bool sendDataUDPn(const char* msg, uint16_t msgLen, uint8_t raiMode = 0);

        // Engineering data
        struct engineeringStruct{
            int8_t RSRP;
            int8_t RSRQ;
            int8_t RSSI;
            int8_t SINR;
            char firmwareVersion[20];
            time_t epoch;
            int16_t timezone;
        };
        engineeringStruct engineeringData;
        void getData();

        // Flush the serial buffer
        void flush();
    private:
        // Reply management
        bool sendAndWaitForReply(const char* command, uint32_t timeout = ONE_SEC, uint8_t lines = 1);
        bool sendAndWaitFor(const char* command, const char* reply, uint32_t timeout); 
        bool sendAndCheckReply(const char* command, const char* reply, uint32_t timeout = ONE_SEC);
        bool readReply(uint32_t timeout = ONE_SEC, uint8_t lines = 1);

        // TODO: updateSleepMode() is not working as expected yet
        void updateSleepMode();

        // Private variables
        int8_t _wakeUpPin;
        bool _debug;
        HardwareSerial *_uart;
        uint8_t _sleepMode;
        uint8_t _TCPconnectID;
        char _buffer[255];
        char _command[32];
        char _firmwareVersion[20];
        char _dateAndTime[40];
        char _psm[40];
        char _host[40];
        uint16_t _port;
        struct tm t = {0};
        
        // Private constants
        const char* _AT = "AT";
        const char* _OK = "OK";
        const char* _ERROR = "ERROR";
        const char* _IP = "+IP:";
};

#endif