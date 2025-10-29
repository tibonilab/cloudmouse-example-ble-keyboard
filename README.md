# BLE Volume Controller Example

Simple BLE HID media controller using rotary encoder for volume control.

## Features

- **Volume Control**: Rotate encoder to adjust system volume up/down
- **Mute Toggle**: Press encoder button to toggle mute
- **Visual Feedback**: On-screen display shows current action
- **Cross-Platform**: Works with Windows, macOS, Linux, Android, iOS

## Hardware Requirements

- ESP32-S3 based CloudMouse device
- Rotary encoder (KY-040 or similar)
- Display (optional, for visual feedback)

## How It Works

The device connects as a Bluetooth HID keyboard and sends media control commands:
- **Clockwise rotation** → Volume UP
- **Counter-clockwise rotation** → Volume DOWN  
- **Button press** → Toggle MUTE

## Pairing

1. Power on the device
2. On your computer/phone, scan for Bluetooth devices
3. Connect to "CloudMouse-XXXXXXXX"
4. The device will appear as a Bluetooth keyboard
5. Start using the encoder to control volume!

## Implementation Details

### Core Components

**BluetoothManager** (`lib/network/BluetoothManager.*`)
- Manages BLE connection lifecycle
- Wraps ESP32-BLE-Keyboard library
- Handles HID initialization with `releaseAll()` after connection

**Event Flow**
```
Encoder → EncoderManager → Event Bus → Core → BluetoothManager → BLE HID
                                     ↓
                              DisplayManager (feedback)
```

### Key Implementation Notes

#### HID Initialization
After BLE connection, call `bleKeyboard->releaseAll()` to properly initialize the HID descriptor:
```cpp
if (connected && currentState != BluetoothState::CONNECTED) {
    setState(BluetoothState::CONNECTED);
    bleKeyboard->releaseAll();  // Critical for HID sync
    Serial.println("🔵 Device connected!");
}
```

This ensures the OS properly recognizes the device as a media controller.

#### Event Handling
The `BluetoothManager::handleEncoderEvents()` method processes encoder events and sends corresponding HID commands:
```cpp
void BluetoothManager::handleEncoderEvents(const Event& event) {
    if (!isConnected()) return;
    
    switch(event.type) {
        case EventType::ENCODER_ROTATION:
            if (event.value > 0) {
                bleKeyboard->write(KEY_MEDIA_VOLUME_UP);
            } else if (event.value < 0) {
                bleKeyboard->write(KEY_MEDIA_VOLUME_DOWN);
            }
            break;
            
        case EventType::ENCODER_CLICK:
            bleKeyboard->write(KEY_MEDIA_MUTE);
            break;
    }
}
```

#### Display Feedback (Optional)
Visual feedback shows the current action on screen for 1 second before returning to idle state.

## Troubleshooting

### Device not appearing in Bluetooth list
- Check serial monitor for "Advertising..." message
- Restart device and try again
- Make sure Bluetooth is enabled on host device

### Wrong keys being sent
- Disconnect and reconnect
- The `releaseAll()` call after connection should prevent this

### Commands not working
- Verify device is paired and connected
- Check serial monitor for "Device connected!" message
- Some media players require focus to accept media keys

## Performance

- **Latency**: <50ms from encoder rotation to volume change
- **Range**: 10m typical (depends on environment)
- **Battery**: Not applicable (desk-powered device)

## Compatibility

Tested and working on:
- ✅ Windows 10/11
- ✅ macOS Monterey+
- ✅ Linux (Ubuntu, Mint)
- ✅ Android 10+
- ✅ iOS 14+

## Dependencies

- [ESP32-BLE-Keyboard](https://github.com/T-vK/ESP32-BLE-Keyboard) - BLE HID library

## License

See main SDK license.