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

#include <errno.h>
#include "spi_class.h"

static bool type_loaded = false;

DEFUN_DLD (spi_write, args, nargout, 
        "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{n} = } spi_write (@var{spi}, @var{data})\n \
\n\
Write data to a spi slave device.\n \
\n\
@var{spi} - instance of @var{octave_spi} class.@*\
@var{data} - data, of type uint8, to be written to the slave device.\n \
\n\
Upon successful completion, spi_write() shall return the number of bytes written and a buffer containing the returned data @var{n}.\n \
@end deftypefn")
{
    if (!type_loaded) {
        octave_spi::register_type();
        type_loaded = true;
    }
    if (args.length() != 2 || args(0).type_id() != octave_spi::static_type_id()) {
        print_usage();
        return octave_value(-1);
    }
    octave_spi* spi = NULL;
    int retval;
    const octave_base_value& rep = args(0).get_rep();
    octave_value_list return_list;
    spi = &((octave_spi &)rep);
    if (args(1).is_uint8_type ()) {
        NDArray data = args(1).array_value();
        uint8_t *buf = NULL; 
        buf = new uint8_t[data.length()];
        if (buf == NULL) {
            error("spi_write: cannot allocate requested memory: %s\n", strerror(errno));
            return octave_value(-1);  
        }
        for (int i = 0; i < data.length(); i++) buf[i] = static_cast<uint8_t>(data(i));
        retval = spi->write(buf, data.length());
        // copy data to return buffer
        uint8NDArray rdata(dim_vector(1, retval));
        for (int i = 0; i < retval; i++) rdata(i) = buf[i];
        return_list(0) = rdata;
        return_list(1) = retval;
        delete[] buf;
    } else {
        error("buffer not uint8_t\n");
        print_usage();
        return octave_value(-1);
    }
    return return_list;
}
