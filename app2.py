import asyncio
from bleak import BleakClient, BleakScanner

# UUIDs must match those in the ESP32 code
SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
CHARACTERISTIC_UUID_RX = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"

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

            while True:
                # Prompt the user for new frequency and brightness values
                freq_input = input("Enter blink frequency in Hz (or 'exit' to quit): ")
                if freq_input.lower() == 'exit':
                    break

                brightness_input = input("Enter brightness percentage (0-100): ")
                if brightness_input.lower() == 'exit':
                    break

                # Validate frequency input
                try:
                    frequency = float(freq_input)
                    if frequency <= 0:
                        print("Please enter a positive frequency.")
                        continue
                except ValueError:
                    print("Please enter a valid number for frequency.")
                    continue

                # Validate brightness input
                try:
                    brightness_percent = float(brightness_input)
                    if brightness_percent < 0 or brightness_percent > 100:
                        print("Please enter a brightness percentage between 0 and 100.")
                        continue
                except ValueError:
                    print("Please enter a valid number for brightness.")
                    continue

                # Convert frequency to delay in milliseconds
                period_seconds = 1 / frequency
                half_period_ms = int((period_seconds / 2) * 1000)

                # Ensure the delay is at least 1 ms
                if half_period_ms < 1:
                    half_period_ms = 1

                # Convert brightness percentage to 0-255 scale
                brightness_value = int((brightness_percent / 100) * 255)
                brightness_value = max(0, min(brightness_value, 255))  # Clamp value between 0 and 255

                # Prepare the data string
                data_str = f"{half_period_ms},{brightness_value}"

                # Send the data to the ESP32
                await client.write_gatt_char(CHARACTERISTIC_UUID_RX, data_str.encode('utf-8'))
                print(f"Sent delay of {half_period_ms} ms and brightness {brightness_value} (0-255 scale).")

    except Exception as e:
        print(f"An error occurred: {e}")

if __name__ == "__main__":
    asyncio.run(main())
