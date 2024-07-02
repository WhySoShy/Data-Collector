#include <ArduinoJson.h>

#ifndef ELEMENTS
#define ELEMENTS

struct Elements {       
    public:
        struct Time {
            public:
                // We use an char array instead of the direct data types, beacuse this uses less memory.
                // We allocate 3 bytes for it 2 for the numbers and 1 for the null terminator.
                char second [3];
                char minute [3];
                char hour   [3];
                char day    [3];
                char month  [3];
        };

        Time time;
        float  temperature;
        float  humidity;
        float  gas;
        
        static JsonDocument ConstructElement(JsonDocument& json, Elements& element, int& index, char* year) {
            json[index]["time"] = FormatTime(element.time, year);
            json[index]["values"][0]["value"] = element.temperature;
            json[index]["values"][1]["value"] = element.humidity;
            json[index]["values"][2]["value"] = element.gas;

            json[index]["values"][0]["name"] = "temperature";
            json[index]["values"][1]["name"] = "humidity";
            json[index]["values"][2]["name"] = "propane";
            return json; 
        }

        static std::string FormatTime(Time& time, char* year) {
            std::string concat;
            concat.append(year);        concat += "-";
            concat.append(time.month);  concat += "-";
            concat.append(time.day);    concat += "T";
            concat.append(time.hour);   concat += ":";
            concat.append(time.minute); concat += ":";
            concat.append(time.second); 
            return concat;
        }
};

#endif

