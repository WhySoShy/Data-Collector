#include <Arduino.h>
#include <SHT31.h>
#include <Wire.h>
#include <MQUnifiedsensor.h>

#ifndef CLEAN_AIR_RATIO
#define CLEAN_AIR_RATIO 9.83

class Sensor {
  public:
    enum MQ2Type {
        H2,       // A: 987.99 | B: -2.162
        LPG,      // A: 574.25 | B: -2.222
        C0,       // A: 36974  | B: -3.109
        Alcohol,  // A: 3616.1 | B: -2.675
        Propane   // A: 658.71 | B: -2.168
    };

    float humidity;
    float temperature;
    float gas;
    void calibrate();
    void update();
    void begin(MQ2Type type);

    // Gets the A & B Values that should be used to calculate the PPM.
    static std::tuple<float, float> getMQ2Values(MQ2Type type) {
      switch(type) {
        case H2:
          return std::tuple<float, float>(987.99, -2.162);
        case LPG:
          return std::tuple<float, float>(574.25, -2.222);
        case C0:
          return std::tuple<float, float>(36974, -3.109);
        case Alcohol:
          return std::tuple<float, float>(3616.1, -2.675);
        case Propane:
          return std::tuple<float, float>(658.71, -2.168);

        default: // This should never happen, but just in case it does just return -1, -1
          return std::tuple<float, float>(-1, -1);
      }
    }

    Sensor() : 
      mq2("ESP8266", 5, 10, A0, "MQ2") {}

  private:
    MQUnifiedsensor mq2;
    SHT31 sht;

    void readMQ2();
    void readSht();
    void beginSht();
    void beginMQ2(MQ2Type type);

};

#endif