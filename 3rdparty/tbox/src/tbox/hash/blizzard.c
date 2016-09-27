/*!The Treasure Box Library
 * 
 * TBox is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 * 
 * TBox is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with TBox; 
 * If not, see <a href="http://www.gnu.org/licenses/"> http://www.gnu.org/licenses/</a>
 * 
 * Copyright (C) 2016, Olexander Yermakov All rights reserved.
 *
 * @author      ruki
 * @file        blizzard.c
 * @ingroup     hash
 *
 */

/* //////////////////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "blizzard.h"

/* //////////////////////////////////////////////////////////////////////////////////////
 * implementation
 */
tb_size_t tb_blizzard_make(tb_byte_t const* data, tb_size_t size, tb_size_t seed)
{
    // check
    tb_assert_and_check_return_val(data && size, 0);

    // init value
    tb_size_t value = seed;

    // generate it
    while (size--) value = (*data++) + (value << 6) + (value << 16) - value;
    return value;

    // make table
    static tb_size_t s_make = 0;
    static tb_size_t s_table[1280];
    if (!s_make)
    {
        tb_size_t i = 0;  
        tb_size_t index1 = 0;
        tb_size_t index2 = 0;
        tb_size_t seed0 = 0x00100001;
        for (index1 = 0; index1 < 0x100; index1++)  
        {   
            for (index2 = index1, i = 0; i < 5; i++, index2 += 0x100)  
            {   
                seed0 = (seed0 * 125 + 3) % 0x2aaaab; tb_size_t temp1 = (seed0 & 0xffff) << 0x10;  
                seed0 = (seed0 * 125 + 3) % 0x2aaaab; tb_size_t temp2 = (seed0 & 0xffff);
                s_table[index2] = (temp1 | temp2);   
            }   
        }

        // ok
        s_make = 1;
    }

    // init value
    tb_size_t seed1 = 0x7fed7fed;  
    tb_size_t seed2 = 0Xeeeeeeee;  
    if (seed)
    {
        seed1 = s_table[(1 << 8) + seed] ^ (seed1 + seed2);  
        seed2 = seed + seed1 + seed2 + (seed2 << 5) + 3;  
    }

    // done
    tb_size_t byte = 0;  
    while (size--)
    {
        // get one byte
        byte = *data++;  

        // 0 << 8: hash type: 0
        // 1 << 8: hash type: 1
        // 2 << 8: hash type: 2
        seed1 = s_table[(1 << 8) + byte] ^ (seed1 + seed2);  
        seed2 = byte + seed1 + seed2 + (seed2 << 5) + 3;  
    }

    // ok
    return seed1;  
}
tb_size_t tb_blizzard_make_from_cstr(tb_char_t const* cstr, tb_size_t seed)
{
    // check
    tb_assert_and_check_return_val(cstr, 0);

    // make it
    return tb_blizzard_make((tb_byte_t const*)cstr, tb_strlen(cstr), seed);
}
