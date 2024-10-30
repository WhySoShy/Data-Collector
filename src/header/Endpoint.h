#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

#ifndef Endpoint_Client_D
#define Endpoint_Client_D

struct Endpoint_Client {
    public:
        // Adds the name and value to the header.
        void addHeader(const char* name, const char* content) {
            // This is added on object creation.
            if (strcmp(content, "application/json")) return;

            this->_payload.push_back(Endpoint_Header(name, content));
        }
        
        // Sends a request using HTTPS
        int requestPOST(const char* body) {
            WiFiClientSecure client;
            HTTPClient http;
            int res = -1;

            // If the buffer size is not set, it will use around 20kb of memory alone.
            // Buffer size is calculated like this: (164 * MAX_CONSTRUC_ELEMENTS) where 164 is the size of each json element constructed.
            client.setBufferSizes(3500, 500);
            
            client.setInsecure();
            if (http.begin(client, this->_host, 443, this->_parameters)) {
                addHeaders(http);
                res = http.POST(body);
            }

            // Free resources/memory
            client.stop();
            http.end();
            return res;
        }
        
        ~Endpoint_Client() { _host = nullptr; _parameters = nullptr; }
        Endpoint_Client() : _host(nullptr), _parameters(nullptr) { }

        /// @brief Removes the protocol to only keep the host (Ex: google.com)
        /// @param host Only the hostname, with or without the protocol.
        /// @param parameters 
        Endpoint_Client(const String host, const char* parameters) : _parameters(parameters) {
            int startOfHostIndex = host.indexOf(':') + 3; // Plus 3 to include the backslash and start at the first letter of host.
            const char* convertedUrl = host.c_str();

            _host = getCharsBetweenIndexes(startOfHostIndex, strlen(convertedUrl)+1, convertedUrl);
            addHeader("Content-Type", "application/json");
        }

        /// @brief Removes the protocol and split the URL into hosts (Ex: google.com) and parameters.
        /// @param url The whole url with protocol, hostname and parameters 
        Endpoint_Client(const String url) {
            int startOfHostIndex = url.indexOf(':') + 3; // Plus 3 to include the backslash and start at the first letter of host.
            int parameterIndex = url.indexOf('/', startOfHostIndex);
            const char* convertedUrl = url.c_str();

            _host = getCharsBetweenIndexes(startOfHostIndex, parameterIndex, convertedUrl);
            _parameters = getCharsBetweenIndexes(parameterIndex, strlen(convertedUrl), convertedUrl);
            addHeader("Content-Type", "application/json");
        }

    private:        
        struct Endpoint_Header {
            public:
                Endpoint_Header() : _name(nullptr), _content(nullptr) {}
                Endpoint_Header(const char* name, const char* content) : _name(name), _content(content) {} 

                // Free memory
                ~Endpoint_Header() {
                    _name = nullptr;
                    _content = nullptr;
                }

                const char* getName() { return this->_name; }
                const char* getContent() { return this->_content; }
            
            private:
                const char* _name;
                const char* _content;
        };

        const char* _host;
        const char* _parameters;
        std::vector<Endpoint_Header> _payload;

        void addHeaders(HTTPClient& client) {
            for(unsigned int i = 0; i < _payload.size(); i++)
                client.addHeader(_payload[i].getName(), _payload[i].getContent());
        }

        // Used to get the host and parameters a URL.
        static const char* getCharsBetweenIndexes(int indexStart, int indexEnd, const char* text) {
            // Allocate memory for the array.
            char* constructedText = new char[(indexEnd - indexStart) + 1];
            constructedText[sizeof(constructedText - 1)] = '\0';

            // Construct the new array.
            for(int i = indexStart, y = 0; i < indexEnd; i++, y++)
                constructedText[y] = text[i];

            return constructedText;
        }
};

#endif