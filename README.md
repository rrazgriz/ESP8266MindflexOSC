# ESP8266MindflexOSC

This is an OSC sender firmware based on https://github.com/Thorinair/Lulu but modified and simplified a bit for my needs. It uses the Brain library to read data from a Neurosky-powered Mindflex headset, and sends it over the network via OSC. It's intended to be connected to VRChat to control avatar parameters.

# Requirements
- [Arduino IDE set up to deploy to ESP8266](https://github.com/esp8266/Arduino)
- [Brain library](https://github.com/kitschpatrol/Brain)
- [ESP8266 OSC Library](https://github.com/stahlnow/OSCLib-for-ESP8266)

# Hardware
- Mindflex model P2639 (modified with GND and TX outputs)
- ESP8266 (Powered by USB battery)

The Mindflex was modified as described at https://frontiernerds.com/brain-hack and connected to the GND and TX pins of the ESP8266 instead of to an arduino uno. Ideally, the system shouldn't be coupled to grid/mains, as the grounding can cause issues with getting readings.

# Setup
1. Clone or download repository
2. Open `ESP8266MindflexOSC` sketch in Arduino IDE
   - If necessary, clone/download the requirements (Brain/OSC Lib) and add using `Sketch -> Include Library -> Add .ZIP Library...`
4. Copy `ESP8266MindflexOSC_Config_EXAMPLE.h` and rename it `ESP8266MindflexOSC_Config.h`. Edit `SSID`, `PSK`, `IP_3`/`IP_4` (last two fields of local IP to send to) and any other configuration parameters as needed.
5. Build and Upload the sketch to the ESP8266. Use software like Protokol to ensure that data is being transmitted correctly. 
   - You may need to add a firewall rule allowing UDP traffic on the configured ports (9000, 9010 by default) from the ESP8266's IP address.

# Usage
By default, this program outputs a bundle with two values to the following addresses: 
- `/avatar/parameters/EEGAttention` - "Attention" value from Neurosky, normalized to 0-1 (float)
- `/avatar/parameters/EEGMeditation` - "Meditation" value from Neurosky, normalized to 0-1 (float)
  
In VRChat, you can add the parameters `EEGAttention` and `EEGMeditation` to your avatar's synced parameter list, and use them directly as you would any other float.

Optionally, it can be configured to send four additional parameters, in the form of two "booleans" (int 0/1) for each main value:

- `/avatar/parameters/EEGAttention1` and `/avatar/parameters/EEGAttention2`
- `/avatar/parameters/EEGMeditation1` and `/avatar/parameters/EEGMeditation2`

| Value 1 | Value 2 | Float Value Range |
| --- | ----------- | --- |
| `0` | `0` | `0.00 - 0.25` |
| `1` | `0` | `0.25 - 0.50` |
| `0` | `1` | `0.50 - 0.75` |
| `1` | `1` | `0.75 - 1.00` |

In VRChat, these can be used as Boolean parameters to reduce the amount of parameter memory used. You can use two per parameter for four steps, or one per paremeter (`EEGAttention2`, `EEGMeditation2`) for a simple low/high (less than 0.5/greater than 0.5). These parameters can be used to drive effects, or to drive local floats to emulate the functionality of sending floats directly while using less memory, at the expense of precision. 

If `DEBUG_ENABLE` is set to `true`, an additional bundle will be sent to a different port (9010 by default) at the address `eegdata`. This data is in the format:

`Signal strength (200-0, 0 is max), Attention (0-1), Meditation (0-1), Delta, Theta, LowAlpha, HighAlpha, LowBeta, HighBeta, LowGamma, MidGamma`

I'm not sure what the scales are for the eeg waves (delta, theta, etc.); they're directly from the Neurosky transmission.


