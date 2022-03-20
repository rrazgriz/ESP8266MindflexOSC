// Edit with your parameters
// Rename to ESP8266MindflexOSC_Config.h

#define SSID "your-ssid"
#define PSK "your-password"

#define IP_1 192
#define IP_2 168
#define IP_3 1
#define IP_4 123

#define DEBUG_ENABLE false
#define BINARY_PARAMS false

#define LOCAL_PORT 8888
#define VRCHAT_PORT 9000
#define DEBUG_PORT 9010

#define ATTENTION_PARAM_VRCHAT  "/avatar/parameters/EEGAttention"
#define MEDITATION_PARAM_VRCHAT "/avatar/parameters/EEGMeditation"
#define SIGNALVALID_PARAM_VRCHAT "/avatar/parameters/EEGSignal"

#define ATTENTION_PARAM_BINARY1 "/avatar/parameters/EEGAttention1"
#define ATTENTION_PARAM_BINARY2 "/avatar/parameters/EEGAttention2"
#define MEDITATION_PARAM_BINARY1 "/avatar/parameters/EEGMeditation1"
#define MEDITATION_PARAM_BINARY2 "/avatar/parameters/EEGMeditation2"

#define DEBUG_ADDRESS "eegdata"
