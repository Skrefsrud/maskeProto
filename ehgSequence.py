import asyncio
from bleak import BleakClient, BleakScanner

# UUIDs must match those in the ESP32 code
SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
CHARACTERISTIC_UUID_RX = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"

async def send_sequence(client, brightness, fade_speed):
    """
    Sends the predefined sequence of commands to the ESP32.
    """
    sequence = [
        {"frequency": 5, "duration": 60},  # 1 minute of 5 Hz blinking
        {"frequency": 0, "duration": 30},  # 30 seconds off
        {"frequency": 6, "duration": 60},  # 1 minute of 6 Hz blinking
        {"frequency": 0, "duration": 30},  # 30 seconds off
        {"frequency": 7, "duration": 60},  # 1 minute of 7 Hz blinking
    ]

    for step in sequence:
        frequency = step["frequency"]
        duration = step["duration"]

        # Convert frequency to period in milliseconds
        period_ms = int((1 / frequency) * 1000) if frequency > 0 else 0

        # Convert brightness percentage to 0-255 scale
        brightness_value = int((brightness / 100) * 255)

        # Clamp values to their respective ranges
        brightness_value = max(0, min(brightness_value, 255))
        fade_speed_value = max(0, min(int(fade_speed), 100))

        # Prepare the data string
        data_str = f"{period_ms},{brightness_value},{fade_speed_value}"

        # Send the data to the ESP32
        await client.write_gatt_char(CHARACTERISTIC_UUID_RX, data_str.encode('utf-8'))
        print(f"Sent period {period_ms} ms, brightness {brightness_value} (0-255), fade speed {fade_speed_value} (0-100).")

        # Wait for the duration of this step
        await asyncio.sleep(duration)

async def main():
    print("Scanning for BLE devices...")
    devices = await BleakScanner.discover(timeout=10.0)

    esp32_device = None
    for device in devices:
        if device.name == "ESP32_BLE":
            esp32_device = device
            break

    if esp32_device is None:
        print("ESP32_BLE device not found.")
        return

    print(f"Found ESP32_BLE device: {esp32_device.address}")

    try:
        async with BleakClient(esp32_device.address) as client:
            if not await client.is_connected():
                print("Failed to connect to the device.")
                return

            print(f"Connected to {esp32_device.address}")

            # Prompt user for brightness and fade speed
            brightness_input = input("Enter brightness percentage (0-100): ")
            fade_speed_input = input("Enter fade speed percentage (0-100): ")

            try:
                brightness = float(brightness_input)
                fade_speed = float(fade_speed_input)
            except ValueError:
                print("Invalid input. Please enter numbers only.")
                return

            # Clamp values to acceptable ranges
            brightness = max(0, min(brightness, 100))
            fade_speed = max(0, min(fade_speed, 100))

            print("Waiting for go signal...")
            input("Press Enter to start the sequence...")

            # Send the sequence to the ESP32
            await send_sequence(client, brightness, fade_speed)

            print("Sequence completed.")

    except Exception as e:
        print(f"An error occurred: {e}")

if __name__ == "__main__":
    asyncio.run(main())
