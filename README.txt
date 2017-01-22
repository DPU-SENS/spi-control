SPI-Control read me:

1. Installation

Download git and clone the repository:
sudo apt-get install git
git clone https://github.com/DPU-SENS/spi-control

Make tarball from source:
tar -cvzf spi-control.tar.gz /path/to/spi-control/*

Install package in octave:
pkg install spi-control.tar.gz


2. Usage

spi(int chip_select, string flags) -
Opens an spi interface on chip select 0 (default) or 1 (argument 1 > 0).
flags (entered as a string) can be used to change the spi parameters from default values:
	-o set bit order: 0 MSB first (default setting), or 1 LSB first
	-b set bits per word: 0 8bpw (default), 1 16bpw (this doesn’t seem to be working)
	-s set max tx speed: uint32 giving MAXIMUM clock speed (see below) min: 30.5KHz, max: 125MHz
	-m set data mode: 0 SPI_MODE_0, 1 SPI_MODE_1, 2 SPI_MODE_2, 3 SPI_MODE_3
SPI_MODE_0 (0,0) CPOL = 0, CPHA = 0, Clock idle low, data clocked in on rising edge, output data on falling edge
SPI_MODE_1 (0,1) CPOL = 0, CPHA = 1, Clock idle low, data clocked in on falling edge, output data on rising edge
SPI_MODE_2 (1,0) CPOL = 1, CPHA = 0, Clock idle high, data clocked in on falling edge, output data on rising edge
SPI_MODE_3 (1,1) CPOL = 1, CPHA = 1, Clock idle high, data clocked in on rising, edge output data on falling edge
Clock speed is entered as the maximum acceptable value. Actual clock speed will be set based on the following allowed values:
SPI_CLK = 250000000/[2*(x+1)] where x is an integer
** note ** I haven’t tested entering incorrect strings very extensively, so typing crazy things may result in a crash

spi_write(spi interface, uint8* data) -
writes data (buffer up to 32 bytes) to specified spi device and returns the response overwritten onto data.

spi_close() - 
closes specified spi device.


3. Examples

open spi device on chip select 0 with default parameters:
spi0=spi()
OR
spi0=spi(0)
OR 
spi0=spi(0,”-o 0 -b 0 -s 0 -m 0”)

open spi device on chip select 1 with default parameters:
spi1=spi(1)

open spi device on chip select 1 with LSB first, 16 bpw, clock speed 11.363MHz, SPI_MODE_2:
spi1=spi(1,”-o 1 -b 1 -s 12000000 -m 2”)

write 3 bytes to previously opened spi device on chip select 0:
data=uint8([9 53 4])
spi_write(spi0,data)

close previously opened spi device on chip select 0:
spi_close(spi0)

Read MCP3008 CH0 command: 0b000000011000000000000000 = 0x018000 = [9 53 4]


