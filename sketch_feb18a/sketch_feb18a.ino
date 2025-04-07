#include <WiFi.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN 16       
#define LED_COUNT 100   


const char* ssid = "RGB ovladani";
const char* password = "";

WiFiServer server(80);
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

bool ledState = false; 
int currentR = 255, currentG = 255, currentB = 255; 

void setup() {
    Serial.begin(115200);
    strip.begin();
    strip.show(); 
    
    
    WiFi.softAP(ssid, password);
    Serial.println("Access Point started");
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());

    server.begin();
}

void loop() {
    WiFiClient client = server.available();
    if (client) {
        Serial.println("Client connected");
        String request = "";

        while (client.connected()) {
            if (client.available()) {
                char c = client.read();
                request += c;
                if (c == '\n') {
                    if (request.indexOf("GET /color?") >= 0) {
                        currentR = getValue(request, "r=");
                        currentG = getValue(request, "g=");
                        currentB = getValue(request, "b=");
                        if (ledState) setColor(currentR, currentG, currentB);
                    } 
                    else if (request.indexOf("GET /on") >= 0) {
                        ledState = true;
                        setColor(currentR, currentG, currentB);
                    } 
                    else if (request.indexOf("GET /off") >= 0) {
                        ledState = false;
                        setColor(0, 0, 0);
                    }
                    else if (request.indexOf("GET /reset") >= 0) { 
                        ledState = true; 
                        setColor(0, 0, 0); 
                    }
                    client.println(sendWebPage());
                    break;
                }
            }
        }
        delay(10);
        client.stop();
        Serial.println("Client disconnected");
    }
}

int getValue(String data, String key) {
    int startIndex = data.indexOf(key);
    if (startIndex == -1) return 0;
    int endIndex = data.indexOf("&", startIndex);
    if (endIndex == -1) endIndex = data.indexOf(" ", startIndex);
    return data.substring(startIndex + key.length(), endIndex).toInt();
}

void setColor(int r, int g, int b) {
    for (int i = 0; i < LED_COUNT; i++) {
        strip.setPixelColor(i, strip.Color(r, g, b));
    }
    strip.show();
}

String sendWebPage() {
    return String("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n") +
           "<!DOCTYPE html><html><head><title>RGB ovladac</title>" +
           "<style>body{text-align:center;font-family:sans-serif;} " +
           ".color-picker{width:150px;height:150px;border:none;} " +
           ".btn{font-size:20px;padding:10px;margin:10px;cursor:pointer;}</style></head>" +
           "<body><h1>Zvolte barvu</h1>" +
           "<input type='color' class='color-picker' id='colorPicker' value='#ffffff'><br>" +
           "<button class='btn' style='background:green;color:white;' onclick=\"fetch('/on')\">Zapnout</button>" +
           "<button class='btn' style='background:red;color:white;' onclick=\"fetch('/off')\">Vypnout</button>" +
           "<button class='btn' style='background:white;color:black;' onclick=\"fetch('/reset')\">reset barev</button>" +
           "<script>document.getElementById('colorPicker').addEventListener('input', function(){ " +
           "let c = this.value; fetch('/color?r='+parseInt(c.substr(1,2),16)+'&g='+parseInt(c.substr(3,2),16)+'&b='+parseInt(c.substr(5,2),16)); });" +
           "</script></body></html>";
}