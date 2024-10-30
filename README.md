# Data collector
Is a project that should be able to collect data about temperature, humidity and a gas art. The data is being sent to one or multiple API's that handle the data for graphical analytics. 

## Setup

When you open the project, you can see 3 different defines with 3 pre-defined values.
```
#define IRRITATION_DELAY 300000   // 5 Minutes in milliseconds.
#define MAX_CONSTRUC_ELEMENTS 20  // Max 20 elements can be constructed into JSON at once.
#define MAX_ELEMENTS 1152         // Max elements that can be stored in ram.
```

#### IRRITATION_DELAY
Determines how much time there should go between each sensor read and each http request. <br />
*The time should be written in <strong>ms</strong>*

#### MAX_CONSTRUC_ELEMENTS
Is the max amount of elements that will be taken from the queue and converted into a JSON string.
*If you havn't read [JSON Construction](#json-construction) then i recommend u to read it.*

#### MAX_ELEMENTS
Is the max amount of elements that should be saved into the RAM, *IF there is no network connection* <br />
 > If you do not care about the offline elements or you know that it will never loose internet, just set it to <u>1</u> <br />
*If you havn't read [Elements](#elements) i would recommend you to do that*.

### Instance of Cloud
To make it as easy and fast as possible to setup, all the functionality runs through `Cloud`, therefor you need to have an instance of it created.
```cpp
// Sets the time zone to 'CET-1CEST,M3.5.0,M10.5.0/3' automatically.
Cloud cloudData(IRRITATION_DELAY, MAX_CONSTRUC_ELEMENTS, MAX_ELEMENTS);
```
or
```cpp
// Allows you to manually choose the time zone.
Cloud cloudData(IRRITATION_DELAY, MAX_CONSTRUC_ELEMENTS, MAX_ELEMENTS, TIME_ZONE);
```
*And of course have the Cloud.h included at the top.*

<details> <summary>Methods for setup()</summary>

---

### Adding a endpoint
You can add multiple endpoints with different header's, this allows you to send data to multiple API's at once. 
This should be done inside the `setup()` method.
```cpp
Endpoint_Client client("ENDPOINT");
cloudData.addEndpoint(client);
```
*`Content-Type: application/json` gets added automatically.*

### Adding a network
You can add one or more networks for the device to use, if you add multiple networks then they will try to take over when the device looses internet.
```cpp
cloudData.addNetwork("SSID", "PASSWORD");
```
You are able to connect to an open network.
```cpp
cloudData.addNetwork("SSID");
```

*If you are trying to connect to a network that uses a login portal, then there may be an issue with connecting to it.*
<u>The ESP8266 controller only supports 2.4ghz</u>

### Start configuration.
When calling `.begin(Sensor::Propane)` you are making the sensor's ready and trying to connect to a network. If the connection wasn't success then it just tries to re-connect infinitly, when there is a connection it will synchronize the time using *NTP* and reserve memory for the queue of elements. <br />
The method expects a Sensor type, this is just the type of gas you want to read on. *This affects the outcome* <br />
*This should be called after `.addEndpoint()` and `.addNetwork()`*
```cpp
cloudData.begin(Sensor::Propane);
```

</details>

<details> <summary>Methods for loop()</summary>

---

### Ensure it only reads and sends requests every IRRITATION_DELAY
The data that you dont want to be runned continuously, but should follow the delay should be added after the check.
```cpp
if (!cloudData.shouldRunRequest()) return;
```

### Read and add a new element
Read new data from the sensor and add it to the queue of elements.
```cpp
cloudData.addFiFo();
```

### Make request to API's
The method constructs up to `MAX_CONSTRUC_ELEMENTS` in JSON, and goes over each API to send the data.
*It continues until there is not enough data in it. Also this should be called after `.addFiFo()`*
```cpp
cloudData.sendHTTPRequest();
```

</details>

## Requirements with answers
- [x] Multiple networks <br />
- [x] Network recovery
 > If the device happens to lose internet, it will try to reconnect to the latest network and if it cannot connect to it will try another network while collecting data.

- [x] Able to run offline
 > Due to the limited ammount of ram on the [AZ-Envy WLAN ESP8266-12F](https://www.az-delivery.de/en/products/az-envy), the amount of data that it is able to save depends on the iteration time and what you set the MAX_ELEMENTS to.
 > If the data queue is full, it will start removing the oldest data to make space for new and fresh data.

- [x] Multiple APIs
 > You are able to add multiple API's each with different headers, to spread the data collection between multiple companies.

- [x] Time management
 > On startup it needs network, to synchronize the time with an [NTP pool](https://www.ntppool.org/en/), when synchronized it can run offline without depending on the network (unless it needs to push data to an API).

- [x] Data reading
 > It reads data from onboard sensors (MQ2 & SHT).

## Ram usage on AZ-Envy WLAN ESP8266-12F
There is about 80kb RAM available for use, therefor we should try to allocate as much of that for saved elements. 

#### ESP8266HTTPClient & WiFiClientSecure
 > The client's themself take up about *5.5-17 kilobyte* of memory, that will be released when it has sent the payload.


#### Networks
 > The network size is affected about how many characters the *SSID* and *Password* is, and of course the of networks connected amount.

*Size of the network depends on the amount of characters used.*

#### Endpoints
 > The endpoint size depends on the endpoint length *(Hostname & Parameters)*.
 > If the endpoint requires any headers, they will affect the memory amount aswell.

*Size of the endpoint/header depends on the amount of characters used.* <br />
*The protocol will be removed automatically*

#### Elements
 > Each element takes up about *27 bytes* and we should allocate as much memory as we can, each element represents data that a sensor has read.
 > Therefor the more memory we allocate, the longer it can run offline.

*The table uses an irritation delay of 300000ms. (5 minutes)*
| Amount of elements | RAM Usage   | Time     |
| :----------------: | ----------- | -------- |
| 144                | 3888 bytes  | 12 Hours |
| 288                | 7776 bytes  | 24 Hours |
| 432                | 11664 bytes | 36 Hours |
| 576                | 15552 bytes | 48 Hours |
| 720                | 19440 bytes | 60 Hours |
| 864                | 23328 bytes | 72 Hours |
| 1008               | 27216 bytes | 84 Hours |
| 1152               | 31104 bytes | 96 Hours |

#### JSON Construction
 > When sending payloads the elements are constructed into an array of JSON objects.
 > Each object in the array ranges between 113-114 bytes in size. I recommend setting this in between 10-20 Objects,
 > Beacuse the payload cannot be over 2000 Bytes in size, we have to split our data into chuncks that we send to each endpoint.

*Every part of the JSON object is automatically filled.* <br />
*TIME_STAMP format:* `YYYY-MM-DDTHH:MM:SS`
```
[
    {
        "time": "TIME_STAMP",
        "values": [
            {
                "value": FLOAT_DATA,
                "name": "temperature"
            },
            {
                "value": FLOAT_DATA,
                "name": "humidity"
            },
            {
                "value": FLOAT_DATA,
                "name": "GAS_TYPE"
            }
        ]
    }
]
```
*Remember to check if your API can handle a JSON Array.*

### RAM Available for use.
 > Below i have made a table that displays the memory usage of every 'feature', these are only for guidance so they may be incorrect.
 
| Allocation for       | Amount |
| :------------------- | :----: |
| System               | 30.5kb |
| Endpoints & Networks | 1kb    |
| JSON Construction    | 4kb    |
| JSON Buffer          | .5kb   |
| Request              | 6.5kb  |
| Request Buffer       | .5kb   | 
*Totals to 44kb usage out of 80kb. (Aprox 30kb left including buffer)*

 > I Recommend that you leave a 5kb memory buffer, to be sure. <br />
 > *Buffer's and request/json construction has not been pre-allocated*