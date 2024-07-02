#include <Arduino.h>
#include <string>
#include "Elements.h"

#ifndef SYNCTIME
#define SYNCTIME

#define NTP_SERVER "dk.pool.ntp.org"

class Time_Sync {
    public:
        void syncTime();
        Elements::Time getTime();

        // Remember to call free() when done using the returned pointer.
        char* getYear() {
            time_t now = time(nullptr);
            char* year = (char*) malloc(5);

            sprintf(year, "%4d", 1900 + localtime(&now)->tm_year);
            return year;
        }
        Time_Sync(const char* time_zone) : TIME_ZONE(time_zone) { }
        ~Time_Sync() {
            TIME_ZONE = nullptr;
        }
    private:
        void trySynchronize();
        const char* TIME_ZONE;
};

#endif