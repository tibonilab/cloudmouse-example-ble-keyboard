# BLE Volume Controller Example

Bluetooth HID media controller implementation for CloudMouse devices. Control your computer's volume wirelessly using the rotary encoder.

Built on top of the [CloudMouse SDK](https://github.com/cloudmouse-co/cloudmouse-sdk) boilerplate.

## Features

- **Volume Control**: Rotate encoder to adjust system volume up/down
- **Mute Toggle**: Press encoder button to toggle mute
- **Visual Feedback**: On-screen display shows current action (VOLUME UP/DOWN, TOGGLE MUTE)
- **Cross-Platform**: Works as standard Bluetooth HID keyboard

## Hardware Requirements

This example requires a **CloudMouse device** with:
- ESP32-S3 module
- ILI9488 display (480x320)
- Rotary encoder with push button
- Standard CloudMouse PCB

The example is designed to work out-of-the-box with CloudMouse hardware - no additional components needed!

## How It Works

The CloudMouse connects to your computer as a Bluetooth HID keyboard and sends media control commands:

- **Rotate clockwise** → Volume UP
- **Rotate counter-clockwise** → Volume DOWN  
- **Press button** → Toggle MUTE

The display shows real-time feedback of each action for 1 second before returning to idle.

## Pairing

1. Power on your CloudMouse device
2. On your computer, open Bluetooth settings and scan for devices
3. Look for **"CM-XXXXXXXX"** (where X is your device's unique ID)
4. Click connect/pair
5. The device will appear as a Bluetooth keyboard
6. Start controlling volume with the encoder! 🎉

**Note**: Pairing with phones/tablets is not currently supported.

## Compatibility

**Tested on:**
- ✅ Linux (Ubuntu/Mint)

**Should work on:**
- Windows 10/11
- macOS

## Implementation Overview

This example demonstrates how to integrate BLE HID functionality into the CloudMouse SDK architecture.

### Architecture
```
┌──────────────┐
│   Encoder    │  Physical input
└──────┬───────┘
       │ events
       ↓
┌──────────────┐
│     Core     │  Event coordinator (runs on Core 0)
└──┬────────┬──┘
   │        │
   ↓        ↓
┌─────────┐ ┌──────────┐
│   BLE   │ │ Display  │  Outputs
│ Manager │ │ Manager  │
└─────────┘ └──────────┘
```

### Key Components Modified

**1. BluetoothManager** (`lib/network/BluetoothManager.*`)
- New manager class wrapping ESP32-BLE-Keyboard library
- Handles BLE connection lifecycle
- Processes encoder events and sends HID commands
- **Critical**: Calls `releaseAll()` after connection to initialize HID descriptor properly

**2. Core** (`lib/core/Core.*`)
- Integrated BluetoothManager into coordination loop
- Routes encoder events to both BLE and display
- Debounces display feedback to prevent lag during fast rotations

**3. Events** (`lib/core/Events.h`)
- Added volume feedback events: `VOLUME_UP_FEEDBACK`, `VOLUME_DOWN_FEEDBACK`, `VOLUME_MUTE_FEEDBACK`

**4. DisplayManager** (`lib/hardware/DisplayManager.*`)
- New `renderVolumeFeedback()` method for displaying volume actions
- Auto-timeout after 1 second returns to idle screen

### Critical Implementation Detail

After BLE connection is established, you **must** call `bleKeyboard->releaseAll()` to properly initialize the HID descriptor:
```cpp
if (connected && currentState != BluetoothState::CONNECTED) {
    setState(BluetoothState::CONNECTED);
    bleKeyboard->releaseAll();  // ← CRITICAL!
    connectionEstablishedTime = millis();
    Serial.println("🔵 Device connected!");
}
```

Without this call, the first HID commands may be misinterpreted by the operating system (e.g., opening file manager or calculator instead of changing volume).

### Event Processing

The `BluetoothManager::handleEncoderEvents()` method translates encoder events into HID media keys:
```cpp
void BluetoothManager::handleEncoderEvents(const Event& event) {
    if (!isConnected()) return;
    
    switch(event.type) {
        case EventType::ENCODER_ROTATION:
            int delta = event.getRotationDelta();
            if (delta > 0) {
                bleKeyboard->write(KEY_MEDIA_VOLUME_UP);
            } else if (delta < 0) {
                bleKeyboard->write(KEY_MEDIA_VOLUME_DOWN);
            }
            break;
            
        case EventType::ENCODER_CLICK:
            bleKeyboard->write(KEY_MEDIA_MUTE);
            break;
    }
}
```

## Performance

- **Response time**: <50ms from rotation to volume change
- **Range**: ~10 meters (typical indoor environment)
- **Display feedback**: Debounced to max 1 update per 150ms during fast rotations

## Troubleshooting

### Device not visible in Bluetooth settings
- Check serial monitor for `🔵 Advertising...` message
- Verify Bluetooth is enabled on your computer
- Try restarting the CloudMouse device

### First command opens wrong application (file manager, calculator)
- This should not happen if `releaseAll()` is called after connection
- If it persists, disconnect and reconnect the device
- Check that you're using the correct version of ESP32-BLE-Keyboard library

### Volume control not working
- Ensure device shows as "Connected" in Bluetooth settings
- Check serial monitor for `🔵 Device connected!` message
- Some applications need focus to accept media keys
- Try with system volume first to verify HID is working

### Display feedback is laggy
- This is normal during very fast rotations
- Display updates are debounced to maintain BLE responsiveness
- BLE commands are always sent immediately regardless of display state

## Building This Example

This example is the CloudMouse SDK boilerplate with BLE functionality integrated. To use it:

1. Clone this repository
2. Open in Arduino IDE or PlatformIO
3. Configure your `DeviceConfig.h` (PCB version, etc.)
4. Upload to your CloudMouse device
5. Pair with your computer and start controlling volume!

See the [CloudMouse SDK documentation](https://docs.cloudmouse.co) for detailed build instructions.

## Dependencies

- [CloudMouse SDK](https://github.com/cloudmouse-co/cloudmouse-sdk) - Base firmware framework
- [ESP32-BLE-Keyboard](https://github.com/T-vK/ESP32-BLE-Keyboard) - BLE HID library (NimBLE version)

## License

Same as CloudMouse SDK - see main repository for details.

## Support

- **Discord**: [discord.gg/cloudmouse](https://discord.gg/cloudmouse)
- **Docs**: [docs.cloudmouse.co](https://docs.cloudmouse.co)
- **Issues**: [GitHub Issues](https://github.com/cloudmouse-co/cloudmouse-sdk/issues)

---

**Built with ❤️ for the CloudMouse community**