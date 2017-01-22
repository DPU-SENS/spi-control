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
// along with this program; if not, see <http://www.gnu.org/licenses/>.s


// This octave pkg is based on the
// Instrument Control Toolbox for GNU Octave by Andrius Sutas <andrius.sutas@gmail.com>
// and adapted for spi

#ifndef SPI_CLASS_H
#define SPI_CLASS_H

#include <octave/oct.h>
#include <string>
#include <stdio.h>
#include <stdint.h>

using std::string;

class octave_spi : public octave_base_value
{
public:
    octave_spi();
    ~octave_spi();

    int open(int /* chip select (0 or 1) */, string /* parameter flags */);
    int close();    
    int get_fd();
    int fd_check();
    
    // bit order (0x00 = MSB)
    int set_bit_order(uint8_t /* bit order */);
    uint8_t get_bit_order();
    // bits per word (0x00 = 8bpw)
    int set_bits_per_word(uint8_t /* bits per word */);
    uint8_t get_bits_per_word();
    // maximum clock speed ("default" is 65536)
    int set_max_speed(uint32_t /* clock speed */);
    uint32_t get_max_speed();
    // data mode (0x00 = SPI_MODE_0)
    int set_data_mode(uint8_t /* data mode */);
    uint8_t get_data_mode();
    // chip select
    int get_chip_select();
    // Simple spi commands
    int write(uint8_t* /* buffer */, unsigned int /* buffer size */);
    int read(uint8_t* /* buffer */, unsigned int /* buffer size */);

    // Overloaded base functions
    double spi_value() const
    { return (double)this->fd; }
    virtual double scalar_value (bool frc_str_conv = false) const
    { return (double)this->fd; }
    void print(std::ostream& os, bool pr_as_read_syntax = false) const;
    void print_raw(std::ostream& os, bool pr_as_read_syntax) const;
    // Properties
    bool is_constant (void) const { return true; }
    bool is_defined (void) const { return true; }
    bool print_as_scalar (void) const { return true; }

private:
    int fd;
    uint8_t bo;
    uint8_t bpw;
    uint32_t sp;
    uint8_t dm;
    int cs;
    
    int spi_init(void);
    int spi_close(void);
    int parse_flags(string /* parameter flags */);
    
    DECLARE_OCTAVE_ALLOCATOR
    DECLARE_OV_TYPEID_FUNCTIONS_AND_DATA
};

#define BCM2835_CORE_CLK_HZ     250000000
/* spi defines from spidev.h */
//#define SPI_CPHA                0x01
//#define SPI_CPOL                0x02
//#define SPI_MODE_0              (0|0)
//#define SPI_MODE_1              (0|SPI_CPHA)
//#define SPI_MODE_2              (SPI_CPOL|0)
//#define SPI_MODE_3              (SPI_CPOL|SPI_CPHA)
//#define SPI_CS_HIGH             0x04                  // Default CS Low is 0x00
//#define SPI_LSB_FIRST           0x08                  // Default MSB is 0x00
//#define SPI_3WIRE               0x10
//#define SPI_LOOP                0x20
//#define SPI_NO_CS               0x40
//#define SPI_READY               0x80
//#define SPI_TX_DUAL             0x100
//#define SPI_TX_QUAD             0x200
//#define SPI_RX_DUAL             0x400
//#define SPI_RX_QUAD             0x800

#endif
