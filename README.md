spi-config
==========

spi board configuration without having to recompile the kernel

Compiling
---------
make
make install

Usage:
------
```modprobe spi-config devices=<devicedev1>,<devicedev2>,...,<devicedev16>```

and a device is defined as follows:
```<bus>:<cs>:<driver>:<speed>:<irq>:<extra args for driver setup - requires special code>```

so the following:
```modprobe spi-config devices=0:0:mcp251x:10000000:25,0:1:spidev:2000000```

will configure:
* on SPI0.0 a mcp251x device with max_speed of 10MHz
* on SPI0.1 a spidev device with a max_speed of 2MHz

