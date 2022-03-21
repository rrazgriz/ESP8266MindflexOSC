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

const IPAddress outIp(IP_1, IP_2, IP_3, IP_4);
WiFiUDP Udp;
Brain brain(Serial);

OSCBundle eeg_bundle;
OSCBundle debug_bundle;

float attention = 0;
float meditation = 0;
int signal_is_valid = 0;
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

char paramNameBuffer[50];
char itoaBuffer[50];
int outVal;

float clamp(float x, float a, float b)
{
    return max(a, min(b, x));
}

void bundle_binary_params(OSCBundle &bundle, char *paramName, int binaryBits, float value)
{
    // Float-to-binary conversion: 0-to-1 range
    int value_bin_conversion = (int) ( 0.999999 * clamp(value, 0.0, 1.0) * (float) pow(2, binaryBits + 1) );

    for (int i = 1; i <= binaryBits; ++i)
    {
        outVal = (value_bin_conversion & (1 << i)) >> i; // Get i-th bit of value_bin_conversion

        strcpy(paramNameBuffer, paramName);
        strcat(paramNameBuffer, itoa(pow(2, i - 1), itoaBuffer, 10));
        bundle.add(paramNameBuffer).add(itoa(outVal, itoaBuffer, 10));

        #if DEBUG_ENABLE
            Serial.print(paramNameBuffer);
            Serial.print(" ");
            Serial.println(outVal);
        #endif
    }
    #if DEBUG_ENABLE
        Serial.println(((float)value_bin_conversion) / ((float)pow(2, binaryBits + 1)));
    #endif
}

void initWiFi()
{
    // Connect to WiFi network
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(SSID);
    WiFi.begin(SSID, PSK);

    while (WiFi.status() != WL_CONNECTED)
    {
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
void processInterval()
{
    // Only run if we have a new update
    if (brain.update())
    {
        #if DEBUG_ENABLE
            Serial.println(brain.readErrors());
            Serial.println(brain.readCSV());
        #endif

        // Read EEG data
        eegsignal = (float)brain.readSignalQuality();
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
        attention = (float) eegattention / 100.0;
        meditation = (float) eegmeditation / 100.0;

        signal_is_valid = (eegsignal < 1.0) ? 1 : 0;

        // Build bundle: address + value (type is automatically set)
        eeg_bundle.add(SIGNALVALID_PARAM_VRCHAT).add(signal_is_valid);
        eeg_bundle.add(ATTENTION_PARAM_VRCHAT).add(attention);
        eeg_bundle.add(MEDITATION_PARAM_VRCHAT).add(meditation);

        #if BINARY_PARAMS
            bundle_binary_params(eeg_bundle, ATTENTION_PARAM_VRCHAT, BINARY_PARAM_BITS, attention);
            bundle_binary_params(eeg_bundle, MEDITATION_PARAM_VRCHAT, BINARY_PARAM_BITS, meditation);
        #endif

        // Send OSC bundle and clear
        Udp.beginPacket(outIp, VRCHAT_PORT);
        eeg_bundle.send(Udp);
        Udp.endPacket();
        eeg_bundle.empty();

        #if DEBUG_ENABLE
            // Format: Signal strength (200-0, 0 is max), Attention (0-1), Meditation (0-1), Delta, Theta, LowAlpha, HighAlpha, LowBeta, HighBeta, LowGamma, MidGamma
            debug_bundle
                .add(DEBUG_ADDRESS)
                .add(eegsignal)
                .add((float)eegattention)
                .add((float)eegmeditation)
                .add((float)eegwave0)
                .add((float)eegwave1)
                .add((float)eegwave2)
                .add((float)eegwave3)
                .add((float)eegwave4)
                .add((float)eegwave5)
                .add((float)eegwave6)
                .add((float)eegwave7);

            Udp.beginPacket(outIp, DEBUG_PORT);
            debug_bundle.send(Udp);
            Udp.endPacket();
            debug_bundle.empty();
        #endif
    }
}

void setup()
{
    Serial.begin(9600); // Needs to be 9600 to recieve from Neurosky

    initWiFi();

    // Zero initial values
    eeg_bundle.add(SIGNALVALID_PARAM_VRCHAT).add((int)0);
    eeg_bundle.add(ATTENTION_PARAM_VRCHAT).add((float)0.0);
    eeg_bundle.add(MEDITATION_PARAM_VRCHAT).add((float)0.0);

    #if BINARY_PARAMS
        bundle_binary_params(eeg_bundle, ATTENTION_PARAM_VRCHAT, BINARY_PARAM_BITS, 0.0);
        bundle_binary_params(eeg_bundle, MEDITATION_PARAM_VRCHAT, BINARY_PARAM_BITS, 0.0);
    #endif

    // Send OSC bundle and clear
    Udp.beginPacket(outIp, VRCHAT_PORT);
    eeg_bundle.send(Udp);
    Udp.endPacket();
    eeg_bundle.empty();
}

void loop()
{
    // Wifi should try to auto reconnect
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi Not Connected...");
        delay(500);
    }
    else
    {
        processInterval();
    }
}
