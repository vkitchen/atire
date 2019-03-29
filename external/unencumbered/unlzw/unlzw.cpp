/*
  unlzw version 1.4, 22 August 2015

  Copyright (C) 2014, 2015 Mark Adler

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Mark Adler
  madler@alumni.caltech.edu
*/

/* Version history:
   1.0  28 Sep 2014  First version
   1.1   1 Oct 2014  Cast before shift of bit buffer for portability
                     Use fastest 32-bit type for bit buffer, uint_fast32_t
                     Use uint_least16_t in case a 16-bit type is not available
   1.2   3 Oct 2014  Clean up comments, consolidate return values
   1.3  20 Aug 2015  Assure no out-of-bounds access on invalid input
   1.4  22 Aug 2015  Return uncompressed data so far on error conditions
                     Be more permissive on where the input is allowed to end
 */

#include <stdlib.h>
#include <stdint.h>

/* Type for accumulating bits.  23 bits of the register are used to accumulate
   up to 16-bit symbols. */
typedef uint_fast32_t bits_t;

/* Double size_t variable n, saturating at the maximum size_t value. */
#define DOUBLE(n) \
    do { \
        size_t was = n; \
        n <<= 1; \
        if (n < was) \
            n = (size_t)0 - 1; \
    } while (0)

/* Decompress compressed data generated by the Unix compress utility (LZW
   compression, files with suffix .Z).  Decompress in[0..inlen-1] to an
   allocated buffer (*out)[0..*outlen-1].  The length of the uncompressed data
   in the allocated buffer is returned in *outlen.  unlzw() returns zero on
   success, negative if the compressed data is invalid, or 1 if out of memory.
   The negative return values are -1 for an invalid header, -2 if the first
   code is not a literal or if an invalid code is detected, and -3 if the
   stream ended in the middle of a code.  -1 means that the data was not
   produced by Unix compress, -2 generally means random or corrupted data, and
   -3 generally means prematurely terminated data.  If the decompression
   results in a proper zero-length output, then unlzw() returns zero, *outlen
   is zero, and *out is NULL.  On error, any decompressed data up to that point
   is returned using *out and *outlen. */
static int unlzw(unsigned const char *in, size_t inlen,
                 unsigned char **out, size_t *outlen)
{
    unsigned bits;              /* current number of bits per code (9..16) */
    unsigned mask;              /* mask for current bits codes = (1<<bits)-1 */
    bits_t buf;                 /* bit buffer -- holds up to 23 bits */
    unsigned left;              /* bits left in buf (0..7 after code pulled) */
    size_t next;                /* index of next input byte in in[] */
    size_t mark;                /* index where last change in bits began */
    unsigned code;              /* code, table traversal index */
    unsigned max;               /* maximum bits per code for this stream */
    unsigned flags;             /* compress flags, then block compress flag */
    unsigned end;               /* last valid entry in prefix/suffix tables */
    unsigned prev;              /* previous code */
    unsigned final;             /* last character written for previous code */
    unsigned stack;             /* next position for reversed string */
    unsigned char *put;         /* allocated output buffer */
    size_t size;                /* size of put[] allocation */
    size_t have;                /* number of bytes of data in put[] */
    int ret = 0;                /* return code */
    /* memory for unlzw() -- the first 256 entries of prefix[] and suffix[] are
       never used, so could have offset the index but it's faster to waste a
       little memory */
    uint_least16_t prefix[65536];       /* index to LZW prefix string */
    unsigned char suffix[65536];        /* one-character LZW suffix */
    unsigned char match[65280 + 2];     /* buffer for reversed match */

    /* initialize output for error returns */
    *out = NULL;
    *outlen = 0;

    /* process the header */
    if (inlen < 3 || in[0] != 0x1f || in[1] != 0x9d)
        return -1;                          /* invalid header */
    flags = in[2];
    if (flags & 0x60)
        return -1;                          /* invalid header */
    max = flags & 0x1f;
    if (max < 9 || max > 16)
        return -1;                          /* invalid header */
    if (max == 9)                           /* 9 doesn't really mean 9 */
        max = 10;
    flags &= 0x80;                          /* true if block compress */

    /* clear table, start at nine bits per symbol */
    bits = 9;
    mask = 0x1ff;
    end = flags ? 256 : 255;

    /* set up: get the first 9-bit code, which is the first decompressed byte,
       but don't create a table entry until the next code */
    if (inlen == 3)
        return 0;                           /* zero-length input is ok */
    buf = in[3];
    if (inlen == 4)
        return -3;                          /* a partial code is not ok */
    buf += in[4] << 8;
    final = prev = buf & mask;              /* code */
    buf >>= bits;
    left = 16 - bits;
    if (prev > 255)
        return -2;                          /* first code must be a literal */

    /* we have output -- allocate and set up an output buffer four times the
       size of the input (Unix compress usually compresses less than 4:1, so
       this will avoid a reallocation most of the time) */
    size = inlen;
    DOUBLE(size);
    DOUBLE(size);
    put = (unsigned char *)malloc(size);
    if (put == NULL)
        return 1;
    put[0] = final;                         /* first decompressed byte */
    have = 1;

    /* decode codes */
    mark = 3;                               /* start of compressed data */
    next = 5;                               /* consumed five bytes so far */
    stack = 0;                              /* empty stack */
    while (next < inlen) {
        /* if the table will be full after this, increment the code size */
        if (end >= mask && bits < max) {
            /* flush unused input bits and bytes to next 8*bits bit boundary
               (this is a vestigial aspect of the compressed data format
               derived from an implementation that made use of a special VAX
               machine instruction!) */
            {
                unsigned rem = (next - mark) % bits;
                if (rem) {
                    rem = bits - rem;
                    if (rem >= inlen - next)
                        break;
                    next += rem;
                }
            }
            buf = 0;
            left = 0;

            /* mark this new location for computing the next flush */
            mark = next;

            /* increment the number of bits per symbol */
            bits++;
            mask <<= 1;
            mask++;
        }

        /* get a code of bits bits */
        buf += (bits_t)(in[next++]) << left;
        left += 8;
        if (left < bits) {
            if (next == inlen) {
                ret = -3;               /* partial code (not ok) */
                break;
            }
            buf += (bits_t)(in[next++]) << left;
            left += 8;
        }
        code = buf & mask;
        buf >>= bits;
        left -= bits;

        /* process clear code (256) */
        if (code == 256 && flags) {
            /* flush unused input bits and bytes to next 8*bits bit boundary */
            {
                unsigned rem = (next - mark) % bits;
                if (rem) {
                    rem = bits - rem;
                    if (rem > inlen - next)
                        break;
                    next += rem;
                }
            }
            buf = 0;
            left = 0;

            /* mark this new location for computing the next flush */
            mark = next;

            /* go back to nine bits per symbol */
            bits = 9;                       /* initialize bits and mask */
            mask = 0x1ff;
            end = 255;                      /* empty table */
            continue;                       /* get next code */
        }

        /* process LZW code */
        {
            unsigned temp = code;           /* save the current code */

            /* special code to reuse last match */
            if (code > end) {
                /* Be picky on the allowed code here, and make sure that the
                   code we drop through (prev) will be a valid index so that
                   random input does not cause an exception. */
                if (code != end + 1 || prev > end) {
                    ret = -2;               /* invalid LZW code */
                    break;
                }
                match[stack++] = final;
                code = prev;
            }

            /* walk through linked list to generate output in reverse order */
            while (code >= 256) {
                match[stack++] = suffix[code];
                code = prefix[code];
            }
            match[stack++] = code;
            final = code;

            /* link new table entry */
            if (end < mask) {
                end++;
                prefix[end] = prev;
                suffix[end] = final;
            }

            /* set previous code for next iteration */
            prev = temp;
        }

        /* make room for the stack in the output */
        if (stack > size - have) {
            if (have + stack + 1 < have) {
                ret = 1;
                break;
            }
            do {
                DOUBLE(size);
            } while (stack > size - have);
            {
                unsigned char *mem = (unsigned char *)realloc(put, size);
                if (mem == NULL) {
                    ret = 1;
                    break;
                }
                put = mem;
            }
        }

        /* write output in forward order */
        do {
            put[have++] = match[--stack];
        } while (stack);

        /* stack is now empty (zero) for the next code */
    }

    /* return the decompressed data, first reducing the allocated memory */
    {
        unsigned char *mem = (unsigned char *)realloc(put, have);
        if (mem != NULL)
            put = mem;
    }
    *out = put;
    *outlen = have;
    return ret;
}


int unlzw(unsigned char **out, size_t *outlen, unsigned char *str, int str_len)
{
   const char *errmsg[] = {
      "Prematurely terminated compress stream",  /* -3 */
      "Corrupted compress stream",               /* -2 */
      "Not a Unix compress (.Z) stream",         /* -1 */
      "Unexpected return code",                  /* < -3 or > 1 */
      "Out of memory"                            /* 1 */
   };

      return unlzw(str, str_len, out, outlen);
}

