#ifndef __Quectel_BC660_h__
#define __Quectel_BC660_h__

#include "Arduino.h"

#define NOT -1

class QuectelBC660 {
    public:
        QuectelBC660(int8_t wakeUpPin = NOT, bool debug = false);
        bool begin(HardwareSerial *uart);

        bool wakeUp();

        const char* getFirmwareVersion();
        const char* getDateAndTime();
        int8_t getRSSI();
        uint8_t getBER();
        uint8_t getStatusCode();
        const char* getStatus();
        bool registered(uint8_t noOfTries = 1, uint32_t delayBetweenTries = 5000);
        bool setOperator(uint8_t mode = 0);
        
        bool setDeepSleep(uint8_t sleepMode = 0);

        bool openMQTT(const char* host, uint16_t port = 1883, uint8_t TCPconnectID = 0);
        bool closeMQTT();
        bool connectMQTT(const char* clientID);
        bool publishMQTT(const char* msg, uint16_t msgLen, const char* topic, uint16_t msgID = 0, uint8_t QoS = 0, uint8_t retain = 0);

        bool openUDP(const char* host, uint16_t port, uint8_t TCPconnectID = 0);
        bool closeUDP();
        bool sendDataUDP(const char* msg, uint16_t msgLen);

        struct engineeringStruct
        {
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
        

        void flush();

    private:
        bool sendAndWaitForReply(const char* command, uint16_t timeout = 1000, uint8_t lines = 1);
        bool sendAndWaitFor(const char* command, const char* reply, uint16_t timeout); 
        bool sendAndCheckReply(const char* command, const char* reply, uint16_t timeout = 1000);
        bool readReply(uint16_t timeout = 1000, uint8_t lines = 1);

        void updateSleepMode();

        int8_t _wakeUpPin;
        bool _debug;
        HardwareSerial *_uart;
        uint8_t _sleepMode;
        uint8_t _TCPconnectID;


        char _buffer[255];
        char _command[32];
        char _firmwareVersion[20];
        char _dateAndTime[40];
        char _host[40];
        uint16_t _port;
        struct tm t = {0};
        

        const char* _AT = "AT";
        const char* _OK = "OK";
        const char* _ERROR = "ERROR";

        
};

#endif