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
 * Copyright (C) 2009 - 2017, ruki All rights reserved.
 *
 * @author      ruki
 * @file        uint32.c
 * @ingroup     container
 *
 */

/* //////////////////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "prefix.h"
#include "hash.h"

/* //////////////////////////////////////////////////////////////////////////////////////
 * implementation
 */
static tb_size_t tb_element_uint32_hash(tb_element_ref_t element, tb_cpointer_t data, tb_size_t mask, tb_size_t index)
{
    return tb_element_hash_uint32(tb_p2u32(data), mask, index);
}
static tb_long_t tb_element_uint32_comp(tb_element_ref_t element, tb_cpointer_t ldata, tb_cpointer_t rdata)
{
    // compare it
    return ((tb_p2u32(ldata) < tb_p2u32(rdata))? -1 : (tb_p2u32(ldata) > tb_p2u32(rdata)));
}
static tb_pointer_t tb_element_uint32_data(tb_element_ref_t element, tb_cpointer_t buff)
{
    // check
    tb_assert_and_check_return_val(buff, tb_null);

    // the element data
    return tb_u2p(*((tb_uint32_t*)buff));
}
static tb_char_t const* tb_element_uint32_cstr(tb_element_ref_t element, tb_cpointer_t data, tb_char_t* cstr, tb_size_t maxn)
{
    // check
    tb_assert_and_check_return_val(element && cstr, "");

    // format string
    tb_long_t n = tb_snprintf(cstr, maxn, "%u", (tb_uint32_t)(tb_size_t)data);
    if (n >= 0 && n < (tb_long_t)maxn) cstr[n] = '\0';

    // ok?
    return (tb_char_t const*)cstr;
}
static tb_void_t tb_element_uint32_free(tb_element_ref_t element, tb_pointer_t buff)
{
    // check
    tb_assert_and_check_return(buff);

    // clear
    *((tb_uint32_t*)buff) = 0;
}
static tb_void_t tb_element_uint32_copy(tb_element_ref_t element, tb_pointer_t buff, tb_cpointer_t data)
{
    // check
    tb_assert_and_check_return(buff);

    // copy element
    *((tb_uint32_t*)buff) = tb_p2u32(data);
}
static tb_void_t tb_element_uint32_nfree(tb_element_ref_t element, tb_pointer_t buff, tb_size_t size)
{
    // check
    tb_assert_and_check_return(buff);

    // clear elements
    if (size) tb_memset(buff, 0, size * sizeof(tb_uint32_t));
}
static tb_void_t tb_element_uint32_ncopy(tb_element_ref_t element, tb_pointer_t buff, tb_cpointer_t data, tb_size_t size)
{
    // check
    tb_assert_and_check_return(buff);

    // copy elements
    tb_memset_u16(buff, tb_p2u32(data), size);
}

/* //////////////////////////////////////////////////////////////////////////////////////
 * interfaces
 */
tb_element_t tb_element_uint32()
{
    // init element
    tb_element_t element = {0};
    element.type   = TB_ELEMENT_TYPE_UINT32;
    element.flag   = 0;
    element.hash   = tb_element_uint32_hash;
    element.comp   = tb_element_uint32_comp;
    element.data   = tb_element_uint32_data;
    element.cstr   = tb_element_uint32_cstr;
    element.free   = tb_element_uint32_free;
    element.dupl   = tb_element_uint32_copy;
    element.repl   = tb_element_uint32_copy;
    element.copy   = tb_element_uint32_copy;
    element.nfree  = tb_element_uint32_nfree;
    element.ndupl  = tb_element_uint32_ncopy;
    element.nrepl  = tb_element_uint32_ncopy;
    element.ncopy  = tb_element_uint32_ncopy;
    element.size   = sizeof(tb_uint32_t);

    // ok?
    return element;
}
