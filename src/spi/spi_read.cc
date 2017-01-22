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
#include <octave/uint8NDArray.h>

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include <errno.h>
#include "spi_class.h"

static bool type_loaded = false;

DEFUN_DLD (spi_read, args, nargout, 
"-*- texinfo -*-\n\
@deftypefn {Loadable Function} {[@var{data}, @var{count}] = } spi_read (@var{spi}, @var{n})\n \
\n\
Read from spi slave device.\n \
\n\
@var{spi} - instance of @var{octave_spi} class.@*\
@var{n} - number of bytes to attempt to read of type Integer.\n \
\n\
The spi_read() shall return number of bytes successfully read in @var{count} as Integer and the bytes themselves in @var{data} as uint8 array.\n \
@end deftypefn")
{
    if (!type_loaded) {
        octave_spi::register_type();
        type_loaded = true;
    }
    if (args.length() < 1 || args.length() > 2 || args(0).type_id() != octave_spi::static_type_id()) {
        print_usage();
        return octave_value(-1);
    }
    uint8_t *buffer = NULL;
    unsigned int buffer_len = 1;
    if (args.length() > 1) {
        if (!(args(1).is_integer_type() || args(1).is_float_type())) {
            print_usage();
            return octave_value(-1);
        }
        buffer_len = args(1).int_value();
    }
    buffer = new uint8_t [buffer_len + 1];
    if (buffer == NULL) {
        error("spi_read: cannot allocate requested memory: %s\n", strerror(errno));
        return octave_value(-1);  
    }
    octave_spi* spi = NULL;
    const octave_base_value& rep = args(0).get_rep();
    spi = &((octave_spi &)rep);
    int retval;
    retval = spi->read(buffer, buffer_len);
    octave_value_list return_list;
    uint8NDArray data( dim_vector(1, retval) );
    for (int i = 0; i < retval; i++)
        data(i) = buffer[i];

    return_list(0) = data;
    return_list(1) = retval;
    delete[] buffer;

    return return_list;
}
