spi-config
==========

spi board configuration without having to recompile the kernel

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
* pd = platform data (hex list - first number is the length of the structure)

so the following:

```modprobe spi-config devices=\
bus=0:cs=0:modalias=mcp2515:speed=10000000:gpioirq=25:pd=140024f4000220,\
bus=0:cs=0:modalias=mcp2515:speed=6000000:gpioirq=22:pd=14002d31000200
```

will configure:
* on SPI0.0 a mcp251x device with max_speed of 10MHz with IRQ on GPIO25 and platform data that reflects: 16MHz crystal and Interrupt flags with IRQF_TRIGGER_FALLING|IRQF_ONESHOT
* on SPI0.1 a mcp251x device with max_speed of 6MHz with IRQ on GPIO22 and platform data that reflects: 20MHz crystal and Interrupt flags with IRQF_TRIGGER_FALLING

