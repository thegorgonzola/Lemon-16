# Lemon-16
The Lemon-16 is a small emulator for a virtual 16-bit computer.<br/>

## Features
### Processor
- 16-Bit Processor
- 16MHz Processor Speed
- L16 Processor Architecture
    - Load-Store Based

### Memory
- 32 Registers
- 65KB Stack
- 32-Bit RAM Addressing (can support up to 4GB; defaults to 2GB)
- 64-Bit Hard Drive Addressing (can support up to 9ZB; defaults to 16GB)

### Graphics
- Dedicated Graphics Processor \(counts as a device; [see below](#Devices)\)
- 32-Bit Processing and Colors
- Math Optimized

### Devices
- Bus Devices, anything like Hard Drives, Graphic Processors, Keyboards, Mice, etc.
- Supports up to 32 Devices
- "Heartbeat" Bus Architecture
    - [see below](#"Heartbeat"-Architecture)
- 

## "Heartbeat" Architecture
&emsp; &emsp;To use any bus devices, you send packets from the CPU, down the bus, to the device.
Upon recieving a packet, the bus device will do whatever it's action is, before sending a "heartbeat" signal back to the CPU, which is then stored in either register 0x1A or register 0x1B.
For example, if you send a packet for a Hard Drive to write to itself, the Hard Drive will write to itself, before sending the heartbeat back to say "I'm done writing".<br/><br/>
&emsp; &emsp;To receive data back, you perform a similar operation.  First, you request a packet, at which point the device will perform its action, before sending out another heartbeat.
> [!IMPORTANT]
> Don't forget to reset the Heartbeat registers. Devices will not check to see if the heartbeat is already switched on, and will do absolutely nothing if it already is.
