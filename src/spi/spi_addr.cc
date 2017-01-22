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

#include "spi_class.h"

static bool type_loaded = false;

//TO DO: change this to implement adjusting speed settings on the fly
DEFUN_DLD (spi_addr, args, nargout, 
"-*- texinfo -*-\n\
@deftypefn {Loadable Function} {} spi_addr (@var{spi}, @var{address})\n \
@deftypefnx {Loadable Function} {@var{addr} = } spi_addr (@var{spi})\n \
\n\
Set new or get existing spi slave device address.\n \
\n\
@var{spi} - instance of @var{octave_spi} class.@*\
@var{address} - spi slave device address of type Integer. \
The address is passed in the 7 or 10 lower bits of the argument.\n \
\n\
If @var{address} parameter is omitted, the spi_addr() shall return \
current spi slave device address as the result @var{addr}.\n \
@end deftypefn")
{
    if (!type_loaded)
    {
        octave_spi::register_type();
        type_loaded = true;
    }
    if (args.length() < 1 || args.length() > 2 || args(0).type_id() != octave_spi::static_type_id()) 
    {
        print_usage();
        return octave_value(-1);
    }
    octave_spi* spi = NULL;
    const octave_base_value& rep = args(0).get_rep();
    spi = &((octave_spi &)rep);
    // Setting new parameters
    if (args.length() > 1)
    {
        if ( !(args(1).is_integer_type() || args(1).is_float_type()) )
        {
            print_usage();
            return octave_value(-1);
        }
//        if (spi->set_addr(args(1).int_value()) < 0) {
//            error("CS not set");
//            return octave_value(-1);
//        }
        return octave_value(spi->get_chip_select());
    }

    // Returning current slave address
    return octave_value(spi->get_chip_select());
}
