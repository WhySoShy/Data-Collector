#include "../header/sensor.h"

void Sensor::calibrate() {
    float calibrateResult = 0;
    for(int i = 1; i <= 10; i++, 
            this->mq2.update(),
            calibrateResult += mq2.calibrate(CLEAN_AIR_RATIO)
    );
    this->mq2.setR0(calibrateResult/10);
}

void Sensor::update() {
    readSht();
    readMQ2();
}

void Sensor::begin(MQ2Type type) {
    beginSht();
    beginMQ2(type);
}

void Sensor::beginSht() {
  sht.setHeatTimeout(5);
  sht.begin();
  Wire.begin();
}

void Sensor::beginMQ2(MQ2Type type) {
  this->mq2.setRegressionMethod(1);

  std::tuple<float, float> values = getMQ2Values(type);

  this->mq2.setA(std::get<0>(values));
  this->mq2.setB(std::get<1>(values));
  this->mq2.init();

  calibrate();
}

void Sensor::readSht() {
  this->sht.heatOn();
  this->sht.read();

  humidity = this->sht.getHumidity();
  temperature = this->sht.getTemperature();

  this->sht.heatOff();
}

void Sensor::readMQ2() {
  this->mq2.update();

  gas = this->mq2.readSensor();
}