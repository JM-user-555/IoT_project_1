#include <WiFi.h>

void PrintWifiEncryption(int encType){
  switch (encType)
    {
    case WIFI_AUTH_OPEN:
        Serial.print("open");
        break;
    case WIFI_AUTH_WEP:
        Serial.print("WEP");
        break;
    case WIFI_AUTH_WPA_PSK:
        Serial.print("WPA");
        break;
    case WIFI_AUTH_WPA2_PSK:
        Serial.print("WPA2");
        break;
    case WIFI_AUTH_WPA_WPA2_PSK:
        Serial.print("WPA+WPA2");
        break;
    case WIFI_AUTH_WPA2_ENTERPRISE:
        Serial.print("WPA2-EAP");
        break;
    case WIFI_AUTH_WPA3_PSK:
        Serial.print("WPA3");
        break;
    case WIFI_AUTH_WPA2_WPA3_PSK:
        Serial.print("WPA2+WPA3");
        break;
    case WIFI_AUTH_WAPI_PSK:
        Serial.print("WAPI");
        break;
    default:
        Serial.print("unknown");
    }
}

bool WifiScan(const char* ssid){
    bool networkFound=false;
    Serial.println("Scan start");   
   // WiFi.scanNetworks will return the number of networks found.
    int n = WiFi.scanNetworks();
    Serial.println("Scan done");
    if (n == 0) {
        Serial.println("no networks found");
    } else {        
      for (int i = 0; i < n; ++i) {
          // Print SSID and RSSI for each network found          
          delay(10);
          if (strcmp(WiFi.SSID(i).c_str(),ssid)==0){
            Serial.println("Network found:");
            Serial.print(WiFi.SSID(i).c_str());
            Serial.print(" | ");
            PrintWifiEncryption(WiFi.encryptionType(i));
            Serial.println();          
            networkFound=true;
            return networkFound;
          }
      }
    }
    Serial.println("");
    // Delete the scan result to free memory for code below.
    WiFi.scanDelete();
    return networkFound;
}

//try to connect to a wifi once
bool ConnectWifi(const char* ssid,const char* pass){
  bool connected=false;
  WiFi.begin(ssid,pass);  //connect to ssid
  Serial.println("Connecting");
  byte n=32;
  while ((WiFi.status()!=WL_CONNECTED)&& (n>0)){
    n--;
    delay(250);
    Serial.print(".");
  }
  if (WiFi.status()==WL_CONNECTED)
  {
    connected=true;
    Serial.println("\nConnected with IP:"+ WiFi.localIP().toString());
    //return connected;
  }else{
    Serial.println("\nUnable to connect!");
    WiFi.disconnect(true); 
  }
  return connected;
}

//try to connect 3 times
bool ConnectToWifi(const char* ssid,const char* pass){
  byte i=0;
  while (i++<3){
    if (WifiScan(ssid)){
      if (ConnectWifi(ssid,pass))
        return true;      
    }
  }
  return false;
}