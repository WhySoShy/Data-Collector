#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#include "Time_Sync.h"
#include "Sensor.h"
#include "Endpoint.h"
#include "Elements.h"

#ifndef NETWORK_TRIES
#define NETWORK_TRIES 15

using namespace std;

class Cloud {
    public:
        struct Network {
            friend class Cloud;
            public:
                Network(const char* name, const char* content) : _name(name), _content(content) {}
                ~Network() {
                    _name = nullptr;
                    _content = nullptr;
                }
            
            private:
                const char* _name;
                const char* _content;
        };

        /// Sets the timezone to `CET-1CEST,M3.5.0,M10.5.0/3`
        Cloud(int irritationDelay, int maxConstructElements, int maxElements)
            : _timeSync("CET-1CEST,M3.5.0,M10.5.0/3"), IRRITATION_DELAY(irritationDelay), MAX_CONSTRUCTION_ELEMENTS(maxConstructElements), MAX_ELEMENTS(maxElements) {}
        
        Cloud(int irritationDelay, int maxConstructElements, int maxElements, const char* timeZone) 
            : _timeSync(timeZone), IRRITATION_DELAY(irritationDelay), MAX_CONSTRUCTION_ELEMENTS(maxConstructElements), MAX_ELEMENTS(maxElements) {}

        void begin(Sensor::MQ2Type type) {
            this->_sensor.begin(type);
            // Intialize a connection
            this->ensureConnection(true);
            // Get the current time from the server pool.
            this->_timeSync.syncTime();
            
            // Allocate memory for the queue vector
            this->_queue.reserve(MAX_ELEMENTS);

            WiFi.mode(WIFI_STA);
        }

        void ensureConnection(bool isInit);
        void sendHTTPRequest();

        // Reads new data from the sensors, and add it to the queue.
        // With the First in - First out principle.
        void addFiFo();
        void addNetwork(const char* ssid, const char* pass);
        void addEndpoint(const Endpoint_Client& payload) { 
            this->_endpoints.push_back(payload); 
        }
        
        // Decides if its time to collect new data and push it to the API's.
        const bool shouldRunRequest() {
            bool ans = millis() - this->_loopMillis > (unsigned long)this->IRRITATION_DELAY;
            
            if (ans)
                this->_loopMillis = millis();

            return ans;
        }
    
    private:
        bool tryConnection(int index, bool isInit);
        std::tuple<std::string, bool> constructJson(bool erase);
        
        // Provides information about the DateTime.
        Time_Sync   _timeSync;
        Sensor      _sensor;

        const int       IRRITATION_DELAY;
        // Maximum amount of objects that should be created when JSON construction is started.
        const int       MAX_CONSTRUCTION_ELEMENTS; // _maxConstructElements;
        const int       MAX_ELEMENTS;
        unsigned long   _loopMillis = 0;
        // Prevent overlapping of JSON construction.
        bool            _prepareJson = false;

        // The current index of the connected network.
        unsigned int    _connectedNetworkIndex = 0;

        // Current networks.
        vector<Network>     _networks;
        // Endpoints that should be caled.
        vector<Endpoint_Client>  _endpoints;
        // Elements to be constructed as json.
        vector<Elements>    _queue; 
};

#endif
