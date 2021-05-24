#ifndef autohue_h
#define autohue_h

#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

static const char* hue_root_ca = \
    "-----BEGIN CERTIFICATE-----\n" \
    "MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n" \
    "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
    "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n" \
    "QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n" \
    "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \
    "b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n" \
    "9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n" \
    "CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n" \
    "nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n" \
    "43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n" \
    "T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n" \
    "gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n" \
    "BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n" \
    "TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n" \
    "DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n" \
    "hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n" \
    "06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n" \
    "PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n" \
    "YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n" \
    "CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\n" \
    "-----END CERTIFICATE-----\n";

class AutoHue {
private:
    String _ip;
    String _user;
    WiFiClient _wifi;
    std::shared_ptr<HttpClient> _client;
public:
    AutoHue() {
    }

    String getIp() {
        return _ip;
    }

    void setIp(String ip) {
        _client = nullptr;
        _ip = ip;
    }

    String getUser() {
        return _user;
    }

    void setUser(String user) {
        _user = user;
    }

    std::shared_ptr<HttpClient> client() {
        if (_client == nullptr)
        {
            auto client = std::make_shared<HttpClient>(_wifi, _ip, 80);
            _client = client;
            return client;
        }
        return _client;
    }

    DynamicJsonDocument huerequest(const char* aURLPath, const char* aHttpMethod, const JsonDocument& body) {
        String bodystr;
        serializeJson(body, bodystr);
        auto cl = client();
        Serial.print(aHttpMethod);
        Serial.print(" ");
        Serial.print("http://");
        Serial.print(_ip);
        Serial.print(aURLPath);
        Serial.print(" ... ");
        int err = cl->startRequest(aURLPath, aHttpMethod, "application/json", bodystr.length(), (const byte*)bodystr.c_str());
        if(err == HTTP_SUCCESS) {
            int statusCode = cl->responseStatusCode();
            Serial.print("Sucess, HTTP-Status code: ");
            Serial.println(statusCode);

            String responsestr = cl->responseBody();
            Serial.println(responsestr);

            DynamicJsonDocument response(1024);
            deserializeJson(response, responsestr);
            return response;
        }
        else {
            Serial.print("Failed, Error code: ");
            Serial.println(err);
            DynamicJsonDocument response(1024);
            response["error"] = err;
            return response;
        }
    }

    bool detectHueIp() {
        WiFiClientSecure wifi_secure;
        wifi_secure.setCACert(hue_root_ca);

        Serial.print("GET https://discovery.meethue.com/ ... ");
        auto client = HttpClient(wifi_secure, "discovery.meethue.com", 443);
        int err = client.get("/");

        if(err == HTTP_SUCCESS) {
            int statusCode = client.responseStatusCode();
            Serial.print("Sucess, HTTP-Status code: ");
            Serial.println(statusCode);
            
            String response = client.responseBody();
            Serial.println(response);

            DynamicJsonDocument doc(1024);
            deserializeJson(doc, response);

            setIp(doc[0]["internalipaddress"]);
            return true;
        }
        else {
            Serial.print("Failed, Error code: ");
            Serial.println(err);
            return false;
        }
    }

    bool requestNewUser(const String& devicetype = "esp32-autohue") {
        StaticJsonDocument<200> doc;
        doc["devicetype"] = devicetype;
        auto res = huerequest("/api", HTTP_METHOD_POST, doc);
        if(res[0].containsKey("success")) {
            setUser(res[0]["success"]["username"]);
            return true;
        }
        else {
            delay(1000);
            return false;
        }
    }
};

#endif