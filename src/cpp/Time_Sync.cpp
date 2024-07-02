#include "../header/Time_Sync.h"

void Time_Sync::syncTime() {
    // Configure the time zone and the server pool we should get the time zone from.
    configTzTime(TIME_ZONE, NTP_SERVER);

    // Synchronize the time from the network.
    trySynchronize();
}


Elements::Time Time_Sync::getTime() {
    time_t now = time(nullptr);
    tm* localTime = localtime(&now);

    Elements::Time time;
    sprintf(time.month, "%02d", (unsigned int)localTime->tm_mon + 1);
    sprintf(time.day, "%02d", localTime->tm_mday);
    sprintf(time.hour, "%02d", localTime->tm_hour);
    sprintf(time.minute, "%02d", localTime->tm_min);
    sprintf(time.second, "%02d", localTime->tm_sec);

    return time;
}

void Time_Sync::trySynchronize() {
    Serial.print("Synchronizing");

    // Continue synchronizing until it has a representive value or it has done over 40 tries.
    while (time(nullptr) < 1000000000l) {
        Serial.print(".");
        delay(500);
    }

    Serial.println("Synchronized");
}
