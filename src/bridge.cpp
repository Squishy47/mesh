#include <Arduino.h>
#include <painlessMesh.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <FS.h>
#include <HTTPClient.h>

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

#define   STATION_SSID     "MrMajorsWifi"
#define   STATION_PASSWORD "MrMajorsBitchinWifi"

#define   HOSTNAME "MQTT_Bridge"

#define   OTA_PART_SIZE 128 //How many bytes to send per OTA data packet

#define   HOST "http://192.168.1.166:3000/firmware/ESP8266"

HTTPClient client;

// Prototypes
void receivedCallback( const uint32_t &from, const String &msg );
void mqttCallback(char* topic, byte* payload, unsigned int length);
void changedConnectionCallback();

IPAddress getlocalIP();

IPAddress myIP(0,0,0,0);
IPAddress mqttBroker(192, 168, 1, 176);

painlessMesh  mesh;
WiFiClient wifiClient;
PubSubClient mqttClient(mqttBroker, 1883, mqttCallback, wifiClient);

void setup() {
    Serial.begin(115200);

    mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION | MESH_STATUS );  // set before init() so that you can see startup messages

    // Channel set to 6. Make sure to use the same channel for your mesh and for you other
    // network (STATION_SSID)
    mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, 5 );
    mesh.onReceive(&receivedCallback);
    mesh.onChangedConnections(&changedConnectionCallback);

    mesh.stationManual(STATION_SSID, STATION_PASSWORD, myIP);
    mesh.setHostname(HOSTNAME);
    mesh.initOTAReceive("bridge");

    // Bridge node, should (in most cases) be a root node. See [the
    // wiki](https://gitlab.com/painlessMesh/painlessMesh/wikis/Possible-challenges-in-mesh-formation)
    // for some background
    mesh.setRoot(true);
    // This node and all other nodes should ideally know the mesh contains a root, so call this on all nodes
    mesh.setContainsRoot(true);
}

void loop() {
    mesh.update();
    mqttClient.loop();

    if(myIP != getlocalIP()){
        myIP = getlocalIP();
        Serial.println("My IP is " + myIP.toString());

        if (mqttClient.connect("painlessMeshClient")) {
            mqttClient.publish("painlessMesh/from/gateway","Ready!");
            mqttClient.subscribe("painlessMesh/to/#");
        } 
    }
}


void changedConnectionCallback() {
    Serial.println("Changed connections");
}

void receivedCallback( const uint32_t &from, const String &msg ) {
    Serial.printf("bridge: Received from %u msg=%s\n", from, msg.c_str());
    String topic = "painlessMesh/from/" + String(from);
    mqttClient.publish(topic.c_str(), msg.c_str());
}

void broadcastFirmware(WiFiClient msg){
    Serial.println("OTA FIRMWARE FOUND, NOW BROADCASTING");

    int len = msg.available();

Serial.println('buffer length: ' + String(len));

    mesh.initOTASend([&msg, len](painlessmesh::plugin::ota::DataRequest pkg, char* buffer) {
    
        // msg.readBytes(buffer, OTA_PART_SIZE);


        return min((unsigned)OTA_PART_SIZE, len - (OTA_PART_SIZE * pkg.partNo));
    }, OTA_PART_SIZE);

    MD5Builder md5;
    md5.begin();
    md5.addStream(msg, len);
    md5.calculate(); 


Serial.println('md5: ' + String(md5.toString()));

    Serial.println("Sending OTA firmware...");
    mesh.offerOTA("node", "ESP8266", md5.toString(), ceil(((float)msg.available()) / OTA_PART_SIZE), false);
}


void downloadFirmware(){
    client.begin(HOST);
    
    int resp = client.GET();
    
    Serial.print("Response: ");
    Serial.println(resp);

    if(resp == 200){
        Serial.println("Updating firmware...");
        broadcastFirmware(client.getStream());
    } else {
        Serial.println("Cannot download firmware file. Only HTTP response 200: OK is supported. Double check firmware location #defined in HOST.");
    }

    client.end();
}


void mqttCallback(char* topic, uint8_t* payload, unsigned int length) {
    Serial.println(String(length));

    char* cleanPayload = (char*)malloc(length+1);
    memcpy(cleanPayload, payload, length);
    cleanPayload[length] = '\0';
    String msg = String(cleanPayload);
    free(cleanPayload);

    String targetStr = String(topic).substring(16);

    Serial.println("new mqtt msg");

    if(targetStr == "gateway") {
        if(msg == "getNodes") {
            auto nodes = mesh.getNodeList(true);
            String str;
            for (auto &&id : nodes) {
                str += String(id) + String(" ");
            }
            mqttClient.publish("painlessMesh/from/gateway", str.c_str());
        }
    }
    else if(targetStr == "broadcast") {
        mesh.sendBroadcast(msg);
    }

    else if(targetStr == "ota"){
        downloadFirmware();
    } else {
        uint32_t target = strtoul(targetStr.c_str(), NULL, 10);
        if(mesh.isConnected(target)) {
            
            Serial.println(msg);

            mesh.sendSingle(target, msg);
        }
        else {
            mqttClient.publish("painlessMesh/from/gateway", "Client not connected!");
        }
    }
}

IPAddress getlocalIP() {
    return IPAddress(mesh.getStationIP());
}



