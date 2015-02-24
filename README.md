spi-config
==========

spi board configuration without having to recompile the kernel

NOTE: it is no longer recommended to use spi-config with the newer kernels 
that have device-tree enabled.

For an example how to configure via device-tree overlays please look at:
https://github.com/raspberrypi/linux/issues/848

this includes:
* clocks
* interrupts
* gpio directions
* spi registrations

Compiling
---------
```
make
make install
```

Usage:
------
loading the module:

```modprobe spi-config devices=<devicedev1>,<devicedev2>,...,<devicedev16>```

and a ```<devicedev>``` is defined as a list of : separated key=value pairs

possible keys are:
* bus = the bus number
* cs = the chip select
* modalias = the driver to assign/use
* irq = the irq to assign
* irqgpio = the GPIO pin of the irq
* mode = the SPI mode
* pd = platform data length to allocate
* pdx-<offset> = sets the hex values at byte-offset <offset> of the platform data (hex string - 2 chars per byte!!!)
* pds64-<offset> = sets the s64-value at byte-offset <offset> of the platform data 
* pdu64-<offset> = sets the u64-value at byte-offset <offset> of the platform data 
* pds32-<offset> = sets the s32-value at byte-offset <offset> of the platform data 
* pdu32-<offset> = sets the u32-value at byte-offset <offset> of the platform data 
* pds16-<offset> = sets the u16-value at byte-offset <offset> of the platform data 
* pdu16-<offset> = sets the s16-value at byte-offset <offset> of the platform data 
* pds8-<offset> = sets the u8-value at byte-offset <offset> of the platform data 
* pdu8-<offset> = sets the u8-value at byte-offset <offset> of the platform data 
* pdp-<offset> = sets a pointer to platform_data+<value> at byte-offset <offset> of the platform data 
* force_release = forces a release of a spi device if it has NOT been configured by this module 
  this action taints the kernel!!! Also this is defined without a =<value>.

<value> and <offset> can typically get prefixed with 0x for hex and 0 for octal.

So the following:

```
modprobe spi-config devices=\
bus=0:cs=0:modalias=mcp2515:speed=10000000:gpioirq=25:pd=20:pdu32-0=16000000:pdu32-4=0x2002,\
bus=0:cs=1:modalias=mcp2515:speed=6000000:gpioirq=22:pd=0x14:pdu32-0=20000000:pdu32-4=0x02
```

will configure:
* on SPI0.0 a mcp251x device with max_speed of 10MHz with IRQ on GPIO25 and platform data that reflects: 16MHz crystal and Interrupt flags with IRQF_TRIGGER_FALLING|IRQF_ONESHOT
* on SPI0.1 a mcp251x device with max_speed of 6MHz with IRQ on GPIO22 and platform data that reflects: 20MHz crystal and Interrupt flags with IRQF_TRIGGER_FALLING
