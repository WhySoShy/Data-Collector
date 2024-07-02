#include "../header/Cloud.h"

void Cloud::addNetwork(const char* ssid, const char* pass) {
    // Add the networks to the multidimensional array.
    this->_networks.push_back(Network(ssid, pass));
}

// Tries to reconnect to the networks in the vector array, and collects data from the sensors if the network is not present while reconnecting.
bool Cloud::tryConnection(int index, bool isInit) {
    bool isConnected = false;
    unsigned long delayMillis = 0;
    int tries = 0;

    Network network = this->_networks.at(index);
    WiFi.begin(network._name, network._content);

    while(!isConnected && tries <= NETWORK_TRIES) {
        if (millis() - delayMillis > 500) {
            delayMillis = millis();
            tries++;
        }

        if (!isInit && shouldRunRequest())
            addFiFo();

        isConnected = WiFi.status() == WL_CONNECTED;
        // It needs a small delay, if not it will crash.
        delay(1);
    }

    return isConnected;
}

/// Connects to networks added the the Vector array, throws an exception if no networks are present.
void Cloud::ensureConnection(bool isInit) { 
    // If there is no networks added, continue in a loop.
    if (this->_networks.empty()) {
        Serial.print("No networks added, running inf loop.");
        for(;;);
    }

    while (true) {
        // Check if we have a internet connection.
        if (WiFi.status() == WL_CONNECTED) 
            return;

        if (tryConnection(this->_connectedNetworkIndex, isInit)) 
            break;

        this->_connectedNetworkIndex++;

        if (this->_connectedNetworkIndex >= this->_networks.size())
            // Reset the index if no network was found.
            this->_connectedNetworkIndex = 0;
    }
}

// Send requests to the different networks
void Cloud::sendHTTPRequest() {
    // If it is preparing a JSON document, then it should do a request and it maybe would take the found data, with it.
    if (this->_prepareJson)
        return;

    /*
        Ensure that there is a internet connection for us to use.
        If there ins't it will try to reconnect to the WIFI, but still save the data from the sensors and the timestamp when it was read. 
    */ 
    ensureConnection(false);

    if (this->_endpoints.empty() || this->_queue.empty()) 
        return;

    bool continueQueue;

    do {
        std::tuple<std::string, bool> json = constructJson(true);
        continueQueue = get<1>(json);

        // Loop over all the endpoints, to deliver data to all the API's requested.
        for(unsigned int i = 0; i < this->_endpoints.size(); i++) {
            // Post the data to the requested URLs.
            Serial.println("Sending request");
            this->_endpoints.at(i).requestPOST(get<0>(json).c_str());
        }

        // Continue to loop, until the vector of elements is empty or there isn't enough in the vector, for it to be worth the extra loop.
    } while(continueQueue);
}
void Cloud::addFiFo() {
    Serial.println("Reading request");
    // Get the new data from our sensors
    this->_sensor.update();

    Elements element;
    element.gas = this->_sensor.gas;
    element.humidity = this->_sensor.humidity;
    element.temperature = this->_sensor.temperature;
    element.time = this->_timeSync.getTime();

    // Remove the first element (oldest data), to get new data.
    if ((const int) this->_queue.size() >= MAX_ELEMENTS) {
        this->_queue.erase(this->_queue.begin());
    }

    this->_queue.push_back(element);
    Serial.println("Required read and pushed");
}


std::tuple<std::string, bool> Cloud::constructJson(bool erase) {
    // If there is any elements left from the previous construction, then it will be cleaned the next time it should send a POST request.
    char* year = this->_timeSync.getYear();
    this->_prepareJson = true;
    JsonDocument json;
    int num = 0;

    while (true) {        
        if (this->_queue.empty() || this->_queue.size() <= (unsigned int)num || num >= MAX_CONSTRUCTION_ELEMENTS) break;

        // Get the first element in the vector if the vector should be erased, else get elements at index num.
        Elements& element = this->_queue.at(num);
        Elements::ConstructElement(json, element, num, year); // Add the element to the JSON document.

        num++;
    }
    
    /*
        The elements has to be deleted here, instead of dynamically deleting it.
        If you dynamically delete the elements, it does not delete all the elements it should.
        Therfor the use of _prepareJson, to ensure that it only does 1 json doc at the time.
    */
    std::vector<Elements>::iterator begin = this->_queue.begin();
    this->_queue.erase(begin, begin + num);
    // year has been allocated using malloc, therefor we have to free up the memory.
    free(year);
    this->_prepareJson = false;
    bool shouldReQueue = this->_queue.size() % MAX_CONSTRUCTION_ELEMENTS != 0;

    return { json.as<std::string>(), shouldReQueue };
}