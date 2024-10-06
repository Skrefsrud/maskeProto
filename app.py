import asyncio
from bleak import BleakClient, BleakScanner

# UUIDs must match those in the ESP32 code
SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
CHARACTERISTIC_UUID_RX = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"


async def send_data(address, data):
    async with BleakClient(address) as client:
        if not client.is_connected:
            print("Failed to connect to the device.")
            return

        print(f"Connected to {address}")
        await client.write_gatt_char(CHARACTERISTIC_UUID_RX, data.encode('utf-8'))
        print(f"Sent: {data}")


async def main():
    print("Scanning for BLE devices...")
    devices = await BleakScanner.discover()

    esp32_device = None
    for device in devices:
        if device.name == "ESP32_BLE":
            esp32_device = device
            break

    if esp32_device is None:
        print("ESP32_BLE device not found.")
        return

    await send_data(esp32_device.address, "Hello ESP32!")

if __name__ == "__main__":
    asyncio.run(main())
