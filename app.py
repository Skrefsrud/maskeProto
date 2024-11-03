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
                # Prompt the user for a new frequency value
                data = input("Enter new blink frequency in Hz (or 'exit' to quit): ")
                if data.lower() == 'exit':
                    break

                # Validate input
                try:
                    frequency = float(data)
                    if frequency <= 0:
                        print("Please enter a positive number.")
                        continue
                except ValueError:
                    print("Please enter a valid number.")
                    continue

                # Convert frequency to delay in milliseconds
                period_seconds = 1 / frequency
                half_period_ms = int((period_seconds / 2) * 1000)

                # Ensure the delay is at least 1 ms
                if half_period_ms < 1:
                    half_period_ms = 1

                # Send the delay to the ESP32
                delay_str = str(half_period_ms)
                await client.write_gatt_char(CHARACTERISTIC_UUID_RX, delay_str.encode('utf-8'))
                print(f"Sent delay of {half_period_ms} ms for {frequency} Hz blinking.")

    except Exception as e:
        print(f"An error occurred: {e}")

if __name__ == "__main__":
    asyncio.run(main())
