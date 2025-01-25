// TODO: Replace with GPL3
/*

  Copyright (c) 2017 Martin Sustrik

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom
  the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.

*/

#ifndef LIBDILL_FASTBYTES_H_INCLUDED
#define LIBDILL_FASTBYTES_H_INCLUDED

#include <dill/bytes.h>

// shorthand definitions that need to be made explicit as they might pollute the namespace

#define	ru8	dill_ru8
#define	ru16	dill_ru16
#define	ru32	dill_ru32
#define	ru64	dill_ru64
#define	rs8	dill_rs8
#define	rs16	dill_rs16
#define	rs32	dill_rs32
#define	rs64	dill_rs64
#define	wu8	dill_wu8
#define	wu16	dill_wu16
#define	wu32	dill_wu32
#define	wu64	dill_wu64

#define swap16	dill_swap16
#define swap32	dill_swap32
#define swap64	dill_swap64

#define rs dill_rs
#define ws dill_ws

#endif
