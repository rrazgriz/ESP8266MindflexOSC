# ESP8266MindflexOSC

This is firmware for the ESP8266 based on https://github.com/Thorinair/Lulu but modified and simplified a bit for my needs. It uses the Brain library to read data from a Neurosky-powered Mindflex headset, and sends it over the network via OSC. It's intended to be connected to VRChat to control avatar parameters.

# Requirements
- [Arduino IDE set up to deploy to ESP8266](https://github.com/esp8266/Arduino)
- [Brain library](https://github.com/kitschpatrol/Brain)
- [ESP8266 OSC Library](https://github.com/stahlnow/OSCLib-for-ESP8266)

# Hardware
- Mindflex model P2639 (modified with GND and TX outputs)
- ESP8266 (Powered by USB battery)

The Mindflex was modified as described at https://frontiernerds.com/brain-hack and connected to the GND and TX pins of the ESP8266 instead of to an arduino uno. Purportedly, the system shouldn't be coupled to grid/mains, as the grounding can cause issues with getting readings.

# Setup
1. Clone or download repository
2. Open `ESP8266MindflexOSC` sketch in Arduino IDE
   - If necessary, clone/download the requirements (Brain/OSC Lib) and add using `Sketch -> Include Library -> Add .ZIP Library...`
4. Copy `ESP8266MindflexOSC_Config_EXAMPLE.h` and rename it `ESP8266MindflexOSC_Config.h`. Edit `SSID`, `PSK`, `IP_3`/`IP_4` (last two fields of local IP to send to) and any other configuration parameters as needed.
5. Build and Upload the sketch to the ESP8266. Use software like Protokol to ensure that data is being transmitted correctly. 
   - You may need to add a firewall rule allowing UDP traffic on the configured ports (9000, 9010 by default) from the ESP8266's IP address.

# Usage
By default, this program outputs a bundle with three values to the following addresses (configurable):
- `/avatar/parameters/EEGAttention` - "Attention" value from Neurosky, normalized to 0-1 (float)
- `/avatar/parameters/EEGMeditation` - "Meditation" value from Neurosky, normalized to 0-1 (float)
- `/avatar/parameters/EEGSignal` - Signal validity value, True (1) if signal is < 1 (valid), False (0) otherwise
  
In VRChat, you can add the parameters `EEGAttention` and `EEGMeditation` to your avatar's synced parameter list, and use them directly as you would any other float. This can be [smoothed](https://hai-vr.notion.site/Avatars-3-0-Animated-Animator-Parameters-and-Smoothing-f128c71dd3184c2bb61a4cff8296ada5#aeb2d0d54edf41e1a846818657dfc1b7) in order to interpolate between values instead of stepping from one to the next. 

You can use the boolean `EEGSignal` to display whether a valid signal is present. This is recommended as the EEG connection can be a bit finicky. 

Optionally, it can be configured to send the paramaters as "Binary Parameters", in the form of multiple "booleans" (int 0/1) for each main value. For example, sending `/avatar/parameters/EEGAttention` as a 3-bit binary parameter would send values over `/avatar/parameters/EEGAttention1`, `/avatar/parameters/EEGAttention2`, and  `/avatar/parameters/EEGAttention4`. For a 3-bit setup, there would be 2^3 = 8 different values, corresponding as such:

| `EEGAttention1` | `EEGAttention2` | `EEGAttention4` | Value Range |
| --- | ----------- | --- | --- |
| `0` | `0` | `0` | `0.000 - 0.125` |
| `1` | `0` | `0` | `0.125 - 0.250` |
| `0` | `1` | `0` | `0.250 - 0.375` |
| `1` | `1` | `0` | `0.375 - 0.500` |
| `0` | `0` | `1` | `0.500 - 0.625` |
| `1` | `0` | `1` | `0.625 - 0.750` |
| `0` | `1` | `1` | `0.750 - 0.875` |
| `1` | `1` | `1` | `0.875 - 1.000` |

In VRChat, these can be used as Boolean parameters to reduce the amount of parameter memory used. You can convert from binary parameters back to floats in multiple ways:

- Create discrete animations for each potential binary state. This makes it easy to create smooth transitions (using transition time), but is obnoxious to update if the amount of binary bits changes.
- Use an animation that ranges from the desired values at 0 and 1, setting each state's speed to 0, and setting the transition offset to the location you'd like 
- Use Avatars 3.0 state behaviors to drive a local parameter for each boolean state combination. This works, but in order to smoothly interpolate between values, it's necessary to [smooth the local float value](https://hai-vr.notion.site/Avatars-3-0-Animated-Animator-Parameters-and-Smoothing-f128c71dd3184c2bb61a4cff8296ada5#aeb2d0d54edf41e1a846818657dfc1b7).
- Use [Animated Animator Parameters](https://hai-vr.notion.site/Avatars-3-0-Animated-Animator-Parameters-and-Smoothing-f128c71dd3184c2bb61a4cff8296ada5) to animate a float value in the animator directly. This works similarly to the above, and can be smoothed in the same way.


### Debugging
If `DEBUG_ENABLE` is set to `true`, an additional bundle will be sent to a different port (9010 by default) at the address `eegdata`. This data is in the format:

`Signal strength (200-0, 0 is max), Attention (0-1), Meditation (0-1), Delta, Theta, LowAlpha, HighAlpha, LowBeta, HighBeta, LowGamma, MidGamma`

I'm not sure what the scales are for the eeg waves (delta, theta, etc.); they're directly from the Neurosky transmission.
