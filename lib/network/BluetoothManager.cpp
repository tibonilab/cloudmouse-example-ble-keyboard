/**
 * CloudMouse SDK - Bluetooth Connection Manager Implementation
 *
 * Pure connection lifecycle management for BLE.
 * NO application logic - that belongs in application layer using BleKeyboard.
 */

#include "BluetoothManager.h"

namespace CloudMouse::Network
{
    // ============================================================================
    // CONNECTION LIFECYCLE
    // ============================================================================

    BluetoothManager::BluetoothManager()
    {
        // Generate unique device name using MAC address
        deviceName = generateDeviceName();
    }

    void BluetoothManager::init()
    {
        Serial.println("🔵 Initializing BluetoothManager...");

        setState(BluetoothState::INITIALIZING);

        // Create BLE keyboard instance with device-specific name
        // Note: CloudMouse is desk-powered, no battery reporting needed
        bleKeyboard = new BleKeyboard(deviceName.c_str(), manufacturer.c_str());

        // Start BLE HID service and begin advertising
        bleKeyboard->begin();

        initialized = true;
        setState(BluetoothState::ADVERTISING);

        Serial.printf("✅ Bluetooth initialized: %s\n", deviceName.c_str());
        Serial.println("🔵 Advertising... Waiting for connection");
    }

    void BluetoothManager::update()
    {
        if (!initialized)
            return;

        // Monitor connection state changes
        bool connected = bleKeyboard->isConnected();

        // Detect connection established
        if (connected && currentState != BluetoothState::CONNECTED)
        {
            setState(BluetoothState::CONNECTED);
            Serial.println("🔵 Device connected!");

            // Release all keys (benign operation that forces HID sync)
            bleKeyboard->releaseAll();
        }
        // Detect disconnection
        else if (!connected && currentState == BluetoothState::CONNECTED)
        {
            setState(BluetoothState::DISCONNECTED);
            Serial.println("🔵 Device disconnected");

            // Auto-restart advertising after disconnect
            setState(BluetoothState::ADVERTISING);
            Serial.println("🔵 Advertising... Waiting for reconnection");
        }
    }

    void BluetoothManager::shutdown()
    {
        if (!initialized)
            return;

        Serial.println("🔵 Shutting down Bluetooth...");

        // Release BLE keyboard instance
        if (bleKeyboard)
        {
            delete bleKeyboard;
            bleKeyboard = nullptr;
        }

        initialized = false;
        setState(BluetoothState::IDLE);

        Serial.println("✅ Bluetooth shutdown complete");
    }

    // ============================================================================
    // CONNECTION STATUS
    // ============================================================================

    bool BluetoothManager::isConnected() const
    {
        return initialized && bleKeyboard && bleKeyboard->isConnected();
    }

    bool BluetoothManager::isAdvertising() const
    {
        return initialized && currentState == BluetoothState::ADVERTISING;
    }

    // ============================================================================
    // PRIVATE METHODS
    // ============================================================================

    void BluetoothManager::setState(BluetoothState newState)
    {
        if (currentState == newState)
            return;

        currentState = newState;

        // Log state transitions
        const char *stateNames[] = {
            "IDLE",
            "INITIALIZING",
            "ADVERTISING",
            "CONNECTED",
            "DISCONNECTED",
            "ERROR"};

        Serial.printf("🔵 Bluetooth State: %s\n", stateNames[(int)newState]);
    }

    String BluetoothManager::generateDeviceName()
    {
        // Use same pattern as WiFi AP name for consistency
        // Format: "CM-XXXXXXXX" where X is last 4 bytes of MAC
        return "CM-" + DeviceID::getDeviceID();
    }

    void BluetoothManager::handleEncoderEvents(const Event &event)
    {
        // Only process if BLE is connected
        if (!isConnected())
        {
            return;
        }

        switch (event.type)
        {
        case EventType::ENCODER_ROTATION:
        {
            // Get rotation direction
            int delta = event.value;

            if (delta > 0)
            {
                // Clockwise rotation = Volume UP
                bleKeyboard->write(KEY_MEDIA_VOLUME_UP);
            }
            else if (delta < 0)
            {
                // Counter-clockwise rotation = Volume DOWN
                bleKeyboard->write(KEY_MEDIA_VOLUME_DOWN);
            }
            break;
        }

        case EventType::ENCODER_CLICK:
            // Click = Toggle Mute
            bleKeyboard->write(KEY_MEDIA_MUTE);
            break;

        default:
            // Ignore other events
            break;
        }
    }

} // namespace CloudMouse::Network