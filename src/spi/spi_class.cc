// Copyright (C) 2016   Abe Burleigh  <ktrout.ab@gmail.com>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.


// This octave pkg is based on the
// Instrument Control Toolbox for GNU Octave by Andrius Sutas <andrius.sutas@gmail.com>
// and adapted for spi

#include <octave/oct.h>

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <vector>

#include <fcntl.h>				//Needed for SPI
#include <sys/ioctl.h>			//Needed for SPI
#include <linux/spi/spidev.h>	//Needed for SPI
#include <unistd.h>             //Needed for SPI

#include <errno.h>
#include <sys/mman.h>
#include <time.h>

using std::string;

#include "spi_class.h"

DEFINE_OCTAVE_ALLOCATOR (octave_spi);
DEFINE_OV_TYPEID_FUNCTIONS_AND_DATA (octave_spi, "octave_spi", "octave_spi");

// Setting to 0 will suppress printf statements
static int debug = 0;

octave_spi::octave_spi()
{
    this->fd = -1;
}

octave_spi::~octave_spi()
{
    this->close();
}

char** split(char *s, const char delim) {
    int x = 0, i = 0;
    char *pch = strchr(s, delim);
    while (pch != NULL) {
        i++;
        pch = strchr(pch+1, delim);
    }
    if (debug) printf("Split  %s  x %i\n", s, i);
    char **el = new char*[i];
    for (char *tok = strtok(s, &delim); tok != NULL; tok = strtok(NULL, &delim)) {
        if (debug) printf("%s\n", tok);
        el[x] = tok;
        x++;
    }
    return el;
}

int octave_spi::parse_flags(std::string flags)
{
    if (debug) printf("Parsing flags: %s\n", flags.c_str());
    char **params = split((char *)flags.c_str(), '-');
//    for (int i = 0; i < (sizeof(params) / sizeof(params[0])); i++) {
    for (int i = 0; i < sizeof(params); i++) {
        if (debug) printf("param: %s\n", params[i]);
        char **args = split(params[i], ' ');
        // if-else through all possible flags to set parameters
        int argVal = atoi(args[1]);
        if (argVal < 0) {
            // Invalid argument.
            error("invalid argument: %s\n", args[1]);
            return -1;
        }
        if (strcmp(args[0], "o") == 0) {        // bit order (0 : MSB, 1 : LSB)
            if (debug) printf("bit order: %i\n", argVal);
            uint8_t bitOrder;
            switch (argVal) {
                case 0 :
                    bitOrder = 0x00;
                    break;
                case 1 :
                    bitOrder = 0x01;
                    break;
                default :
                    // Invalid bit order.
                    error("invalid bit order: -o %i\nChoose 0 (MSB first) or 1 (LSB first)\n", argVal);
                    return -1;
            }
            this->bo = bitOrder;
        } else if (strcmp(args[0], "b") == 0) { // bits per word (0 : 8, 1 : 16) **untested**
            if (debug) printf("bits per word: %i\n", argVal);
            uint8_t bitsPerWord;
            switch (argVal) {
                case 0 :
                    bitsPerWord = 0x00;
                    break;
                case 1 :
                    bitsPerWord = 16;
                    break;
                default :
                    // Invalid bits per word.
                    error("invalid bits per word: -b %i\nChoose 0 (8bits), 1 (16bits)\n", argVal);
                    return -1;
            }
            this->bpw = bitsPerWord;
        } else if (strcmp(args[0], "s") == 0) { // max speed (uint32_t =< 125MHz)
            if (debug) printf("speed: %i\n", argVal);
            uint32_t speed;
            if (argVal <= BCM2835_CORE_CLK_HZ/2) {
                speed = (uint32_t)argVal;
            } else {
                // Invalid max speed.
                error("invalid max speed: -s %i\nChoose a value <= 125000000\n", argVal);
                return -1;
            }
            this->sp = speed;
        } else if (strcmp(args[0], "m") == 0) { // data mode (0 : SPI_MODE_0, 1 : SPI_MODE_1, 2 : SPI_MODE_2, 3 : SPI_MODE_3)
            if (debug) printf("data mode: %i\n", argVal);
            uint8_t dataMode;
            switch (argVal) {
                case 0 :
                    dataMode = SPI_MODE_0;
                    break;
                case 1 :
                    dataMode = SPI_MODE_1;
                    break;
                case 2 :
                    dataMode = SPI_MODE_2;
                    break;
                case 3 :
                    dataMode = SPI_MODE_3;
                    break;
                default :
                    // Invalid data mode.
                    error("invalid data mode: -m %i\nChoose 0 (SPI_MODE_0), 1 (SPI_MODE_1), 2 (SPI_MODE_2), 3 (SPI_MODE_3)\n", argVal);
                    return -1;
            }
            this->dm = dataMode;
        } else {                                // invalid param
            error("invalid flag specified: -%s\n", args[0]);
            return -1;
        }
    }
}

int octave_spi::open(int dev, string flags)
{
    if (dev) {
        this->fd = ::open(std::string("/dev/spidev0.1").c_str(), O_RDWR);
    } else {
        this->fd = ::open(std::string("/dev/spidev0.0").c_str(), O_RDWR);
    }
    // open the spi device
    if (this->get_fd() > 0) {
        this->bo = 0x00;
        this->bpw = 0x00;
        this->sp = 0x00000000;
        this->dm = 0x00;
        // parse parameter flags if specified
        if (!flags.empty()) {
            if (parse_flags(flags) < 0) {
                this->close();
                return -1;
            }
        }
        this->cs = dev;
        if (debug) printf("chip select %i\n", this->get_chip_select());
        if (spi_init() < 0) {
            error("spi_init failed. Are you running as root?\n");
            this->close();
            return -1;
        }
    } else {
        error("spi: Error opening the interface: %s\n", strerror(errno));
        return -1;
    }
    return this->get_fd();
}

int octave_spi::get_fd()
{
    return this->fd;
}

int octave_spi::fd_check()
{
    if (this->get_fd() < 0) {
        error("spi: Interface must be open\n");
        return -1;
    }
    return 0;
}

int octave_spi::set_bit_order(uint8_t order)
{
    if (this->fd_check() < 0) return -1;
    if (debug) printf("Set bit order 0x%02X\n", order);
    // Set the bit order
    int ret = ioctl(this->get_fd(), SPI_IOC_WR_LSB_FIRST, &order);
    if (ret == -1) {
        error("error setting bit order\n");
        return -1;
    }
    get_bit_order();
    return 0;
}

uint8_t octave_spi::get_bit_order()
{
    if (this->fd_check() < 0) return -1;
    // get the bit order
    uint8_t order;
    int ret = ioctl(this->get_fd(), SPI_IOC_RD_LSB_FIRST, &order);
    if (ret == -1) {
        error("error getting bit order\n");
        return -1;
    }
    if (debug) printf("bit order = 0x%02X\n", order);
    this->bo = order;
    return this->bo;
}

int octave_spi::set_bits_per_word(uint8_t bits)
{
    if (this->fd_check() < 0) return -1;
    if (debug) printf("Set bits per word 0x%02X\n", bits);
    // Set the Bits per Word
    int ret = ioctl(this->get_fd(), SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1) {
        error("error setting bpw\n");
        return -1;
    }
    get_bits_per_word();
    return 0;
}

uint8_t octave_spi::get_bits_per_word()
{
    if (this->fd_check() < 0) return -1;
    // get bits per word
    uint8_t bits;
    int ret = ioctl(this->get_fd(), SPI_IOC_RD_BITS_PER_WORD, &bits);
    if (ret == -1) {
        error("error getting bpw\n");
        return -1;
    }
    if (debug) printf("bits per word = 0x%02X\n", bits);
    this->bo = bits;
    return this->bo;
}

int octave_spi::set_max_speed(uint32_t speed)
{
    if (this->fd_check() < 0) return -1;
    if (debug) printf("Set clk speed 0x%02X\n", speed);
    // Set the max speed
    int ret = ioctl(this->get_fd(), SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1){
        error("error setting clk speed\n");
        return -1;
    }
    get_max_speed();
    return 0;
}

uint32_t octave_spi::get_max_speed()
{
    if (this->fd_check() < 0) return -1;
    // get max speed
    uint32_t speed;
    int ret = ioctl(this->get_fd(), SPI_IOC_RD_MAX_SPEED_HZ, &speed);
    if (ret == -1) {
        error("error getting clk div\n");
        return -1;
    }
    if (debug) printf("clk speed = 0x%08X\n", speed);
    this->sp = speed;
    return this->sp;
}

int octave_spi::set_data_mode(uint8_t mode)
{
    if (this->fd_check() < 0) return -1;
    if (debug) printf("Set data mode 0x%02X\n", mode);
    // Set SPI data mode
    int ret = ioctl(this->get_fd(), SPI_IOC_WR_MODE, &mode);
    if (ret == -1) {
        error("error setting bpw\n");
        return -1;
    }
    get_data_mode();
    return 0;
}

uint8_t octave_spi::get_data_mode()
{
    if (this->fd_check() < 0) return -1;
    // get data mode
    uint8_t mode;
    int ret = ioctl(this->get_fd(), SPI_IOC_RD_MODE, &mode);
    if (ret == -1) {
        error("error getting data mode\n");
        return -1;
    }
    if (debug) printf("data mode = 0x%02X\n", mode);
    this->dm = mode;
    return this->dm;
}

int octave_spi::get_chip_select()
{
    if (this->fd_check() < 0) return -1;
    if (debug) printf("chip select = %i\n", this->cs);
    return this->cs;
}

// TO DO: fully implement delay
static uint16_t delay = 0;
//    sleep(1);
//    usleep(1000000);

// Write len bytes to slave and read len bytes back from slave
int octave_spi::write(uint8_t *buf, unsigned int len)
{
    if (this->fd_check() < 0) return -1;
    if (debug) {
        printf("SPI tx %d bytes\n0x", len);
        for (int x = 0; x < len; x++)
            printf("%02X", buf[x]);
        printf("\n");
    }
    int i = 0;
    int retVal = -1;

    spi_ioc_transfer spi = {};
    spi.tx_buf = (unsigned long)buf;
    spi.rx_buf = (unsigned long)buf;
    spi.len = len;
    spi.delay_usecs = delay;
    spi.speed_hz = this->get_max_speed();
    spi.bits_per_word = this->get_bits_per_word();

    retVal = ioctl(this->get_fd(), SPI_IOC_MESSAGE(1), &spi) ;

    if (retVal < 0) {
        error("ioctl error transmitting spi data");
        return -1;
    }
    // buffer will now be filled with the data that was read from the slave
    if (debug) {
        printf("SPI rx:\n0x");
        for (int x = 0; x < len; x++)
            printf("%02X", buf[x]);
        printf("\n");
    }
    return retVal;
}

// for now read will work exactly as write (master must write in order to read)
int octave_spi::read(uint8_t *buf, unsigned int len)
{
    write(buf, len);
    return 0;
}

int octave_spi::close()
{
    spi_close();
    int retval = -1;
    if (this->get_fd() > 0)
    {
        retval = ::close(this->get_fd());
        this->fd = -1;
    }
    return retval;
}

void octave_spi::print(std::ostream& os, bool pr_as_read_syntax ) const
{
    print_raw(os, pr_as_read_syntax);
    newline(os);
}

void octave_spi::print_raw(std::ostream& os, bool pr_as_read_syntax) const
{
    os << this->fd;
}

// SPI initialization
//SPI_MODE_0 (0,0) CPOL = 0, CPHA = 0, Clock idle low, data clocked in on rising edge, output data (change) on falling edge
//SPI_MODE_1 (0,1) CPOL = 0, CPHA = 1, Clock idle low, data clocked in on falling edge, output data (change) on rising edge
//SPI_MODE_2 (1,0) CPOL = 1, CPHA = 0, Clock idle high, data clocked in on falling edge, output data (change) on rising edge
//SPI_MODE_3 (1,1) CPOL = 1, CPHA = 1, Clock idle high, data clocked in on rising, edge output data (change) on falling edge
int octave_spi::spi_init(void)
{
    printf("spi_init\n");
    int ret;
    if (this->fd_check() < 0) return -1;
    
    uint8_t bitOrder = 0x00, bitsPerWord = 0x00, dataMode = 0x00;
    uint32_t maxSpeed = 0x00000000;

    // bit order (default MSB)
    if (this->bo > 0) bitOrder = this->bo;
    if (set_bit_order(bitOrder) < 0) return -1;
    // bits per word (default 8)
    if (this->bpw > 0) bitsPerWord = this->bpw;
    if (set_bits_per_word(bitsPerWord) < 0) return -1;
    // clock speed (Max 125MHz [in practice much less], Min 30516Hz, default 65536Hz)
    if (this->sp > 0) maxSpeed = this->sp;
    if (set_max_speed(maxSpeed) < 0) return -1;
    // data mode (default idle low, rising edge)
    if (this->dm > 0) dataMode = this->dm;
    if (set_data_mode(dataMode) < 0) return -1;

    // TO DO: implement full duplex transfer aka test bits per word
    // TO DO: 32bit SPI mode register to adjust parameters dynamically ??
    
    return 1;
}

// Close this library and deallocate everything
int octave_spi::spi_close(void)
{
    // Set all the SPI0 pins back to input
    printf("SPI end\n");

    // TO DO: reset SPI pins ??
    
    return 1; // Success
}
