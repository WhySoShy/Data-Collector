#include <Arduino.h>

#include "./header/Cloud.h"

#define IRRITATION_DELAY 300000   // 5 Minutes in milliseconds.
#define MAX_CONSTRUC_ELEMENTS 20  // Max 20 elements can be constructed into JSON at once.
#define MAX_ELEMENTS 576          // Max elements that can be stored in ram.

#define URL ""

Cloud cloudData(IRRITATION_DELAY, MAX_CONSTRUC_ELEMENTS, MAX_ELEMENTS);

void setup() {
  Serial.begin(9600);

  Endpoint_Client client(URL);

  cloudData.addEndpoint(client);

  cloudData.addNetwork("SSID", "PASSWORD");
  
  cloudData.begin(Sensor::Propane);
}

void loop() {
  if (!cloudData.shouldRunRequest()) return;

  cloudData.addFiFo();

  cloudData.sendHTTPRequest();
}