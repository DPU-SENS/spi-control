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

#include <fcntl.h>
#include "spi_class.h"

using std::string;

static bool type_loaded = false;

DEFUN_DLD (spi, args, nargout, 
        "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{spi} = } spi ([@var{path}], [@var{address}])\n \
\n\
Open spi interface.\n \
\n\
@var{cs} - Desired chip select (0 or 1). If omitted defaults to 0. @*\
@var{flags} - Flags for setting spi parameters. TO DO: usage details\n \
\n\
The spi() shall return instance of @var{octave_spi} class as the result @var{spi}.\n \
@end deftypefn")
{
    if (!type_loaded)
    {
        octave_spi::register_type();
        type_loaded = true;
    }
    // Do not open interface if return value is not assigned
    if (nargout != 1)
    {
        print_usage();
        return octave_value();
    }
    // Default value: cs 0
    int dev = 0;
    string flags;
    // Parse the function arguments (chip select, flags)
    // is_float_type() or'ed to allow ("", 123), without using ("", int32(123)), we only take "int_value"
    if (args.length() > 0 && (args(0).is_integer_type() || args(0).is_float_type())) dev = args(0).int_value();
    // flags
    if (args.length() > 1) {
        if (args(1).is_string()) {
            flags = args(1).string_value();
        } else {
            print_usage();
            return octave_value();
        }
    }
    
    octave_spi* retval = new octave_spi();
    // Open the interface
    if (retval->open(dev, flags) < 0)
        return octave_value();

    return octave_value(retval);
}
