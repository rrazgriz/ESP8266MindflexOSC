// Copyright 2022 Razgriz
// SPDX-License-Identifier: MIT

// Based on Brain Library  
// and work by Thorinair (https://github.com/Thorinair)
// Requires:
// https://github.com/stahlnow/OSCLib-for-ESP8266
// https://github.com/kitschpatrol/Brain

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <Brain.h>
#include "ESP8266MindflexOSC_Config.h"

const char* ssid = SSID;
const char* pass = PSK;
const IPAddress outIp(IP_1,IP_2,IP_3,IP_4);        
WiFiUDP Udp;                                // A UDP instance to let us send and receive packets over UDP
Brain brain(Serial);

OSCBundle eeg_bundle;
OSCBundle debug_bundle;

// Declarations
float attention = 0;
float meditation = 0;
int attention_binary1 = 0;
int attention_binary2 = 0;
int meditation_binary1 = 0;
int meditation_binary2 = 0;
float eegsignal = 0;
byte eegattention = 0;
byte eegmeditation = 0;
unsigned long eegwave0 = 0;
unsigned long eegwave1 = 0;
unsigned long eegwave2 = 0;
unsigned long eegwave3 = 0;
unsigned long eegwave4 = 0;
unsigned long eegwave5 = 0;
unsigned long eegwave6 = 0;
unsigned long eegwave7 = 0;

void sendOSC(OSCMessage& oscMessage, float value, int port) {
    oscMessage.add(value);
    Udp.beginPacket(outIp, port);
    oscMessage.send(Udp);
    Udp.endPacket();
    oscMessage.empty();
}

void initWiFi() {
    // Connect to WiFi network
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, pass);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    
    Serial.println("Starting UDP");
    Udp.begin(LOCAL_PORT);
    Serial.print("Local port: ");
    Serial.println(Udp.localPort());
}

/* Processing Functions*/
void processInterval() {
    // Only run if we have a new update
    if (brain.update()) {
        if (DEBUG_ENABLE) {
            Serial.println(brain.readErrors());
            Serial.println(brain.readCSV());
        }

        // Read EEG data
        eegsignal = (float) brain.readSignalQuality();
        eegattention = brain.readAttention();
        eegmeditation = brain.readMeditation();
        eegwave0 = brain.readDelta();
        eegwave1 = brain.readTheta();
        eegwave2 = brain.readLowAlpha();
        eegwave3 = brain.readHighAlpha();
        eegwave4 = brain.readLowBeta();
        eegwave5 = brain.readHighBeta();
        eegwave6 = brain.readLowGamma();
        eegwave7 = brain.readMidGamma();

        // Store these for later
        attention = (float) eegattention/100;
        meditation = (float) eegmeditation/100;

        attention_binary1 = 0;
        attention_binary2 = 0;
        meditation_binary1 = 0;
        meditation_binary2 = 0;

        // Build bundle: address + value (type is automatically set)
        eeg_bundle.add(ATTENTION_PARAM_VRCHAT).add(attention);
        eeg_bundle.add(MEDITATION_PARAM_VRCHAT).add(meditation);

        // Crude float-to-binary conversion
        if (BINARY_PARAMS) { 
            if (attention > 0.75) {
                  attention_binary1 = 1;
                  attention_binary2 = 1;
  
            } else if (attention <= 0.75 && attention > 0.5) {
                  attention_binary1 = 0;
                  attention_binary2 = 1;
            } else if (attention <= 0.5 && attention > 0.25) {
                  attention_binary1 = 1;
                  attention_binary2 = 0;
            } else {
                  attention_binary1 = 0;
                  attention_binary2 = 0;
            }
        
            if (meditation > 0.75) {
                meditation_binary1 = 1;
                meditation_binary2 = 1;
            } else if (meditation <= 0.75 && meditation > 0.5) {
                meditation_binary1 = 0;
                meditation_binary2 = 1;
            } else if (meditation <= 0.5 && meditation > 0.25) {
                meditation_binary1 = 1;
                meditation_binary2 = 0;
            } else {
                meditation_binary1 = 0;
                meditation_binary2 = 0;
            }

            eeg_bundle.add(ATTENTION_PARAM_BINARY1).add(attention_binary1);
            eeg_bundle.add(ATTENTION_PARAM_BINARY2).add(attention_binary2);
            eeg_bundle.add(MEDITATION_PARAM_BINARY1).add(meditation_binary1);
            eeg_bundle.add(MEDITATION_PARAM_BINARY2).add(meditation_binary2);
        }

        // Send OSC bundle and clear
        Udp.beginPacket(outIp, VRCHAT_PORT);
        eeg_bundle.send(Udp);
        Udp.endPacket();
        eeg_bundle.empty(); 
        
        if (DEBUG_ENABLE) {
            // Format: Signal strength (200-0, 0 is max), Attention (0-1), Meditation (0-1), Delta, Theta, LowAlpha, HighAlpha, LowBeta, HighBeta, LowGamma, MidGamma
            debug_bundle.add(DEBUG_ADDRESS).add(eegsignal).add((float) eegattention).add((float) eegmeditation).add((float) eegwave0).add((float) eegwave1).add((float) eegwave2).add((float) eegwave3).add((float) eegwave4).add((float) eegwave5).add((float) eegwave6).add((float) eegwave7);

            Udp.beginPacket(outIp, DEBUG_PORT);
            debug_bundle.send(Udp);
            Udp.endPacket();
            debug_bundle.empty();      
        }
    }
}

void setup() {
    Serial.begin(9600);

    initWiFi();

    // Set initial values to zero
    eeg_bundle.add(ATTENTION_PARAM_VRCHAT).add((float) 0.0);
    eeg_bundle.add(MEDITATION_PARAM_VRCHAT).add((float) 0.0);

    if (BINARY_PARAMS) {
            eeg_bundle.add(ATTENTION_PARAM_BINARY1).add((int) 0);
            eeg_bundle.add(ATTENTION_PARAM_BINARY2).add((int) 0);
            eeg_bundle.add(MEDITATION_PARAM_BINARY1).add((int) 0);
            eeg_bundle.add(MEDITATION_PARAM_BINARY2).add((int) 0);
    }

    // Send OSC bundle and clear
    Udp.beginPacket(outIp, VRCHAT_PORT);
    eeg_bundle.send(Udp);
    Udp.endPacket();
    eeg_bundle.empty(); 
}



void loop() {
    // Wifi should try to auto reconnect
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi Not Connected...");
    } else {
      processInterval();
    }
}
