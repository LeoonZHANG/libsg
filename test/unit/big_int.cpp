#include <sg/math/big_int.h>
#include <sg/math/big_float.h>
#include <gtest/gtest.h>

class test_big_int : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        _a = sg_big_int_alloc();
        _b = sg_big_int_alloc();
        _c = sg_big_int_alloc();
        _d = sg_big_int_alloc();
    }

    virtual void TearDown()
    {
        sg_big_int_free(_a);
        sg_big_int_free(_b);
        sg_big_int_free(_c);
        sg_big_int_free(_d);
    }

    sg_big_int_t* _a;
    sg_big_int_t* _b;
    sg_big_int_t* _c;
    sg_big_int_t* _d;
};

TEST_F(test_big_int, transform_binary)
{
    unsigned int bin[2] = { 0x0000000f, 0xf0000000 };
    sg_big_int_set_bin(_a, (const unsigned char *)bin, sizeof(bin));
    sg_vlbuf_t* buf = sg_vlbuf_create();
    sg_big_int_get_bin(_a, buf);
    ASSERT_TRUE(buf->size == sizeof(bin) && memcmp(buf->mem, &bin, sizeof(bin)) == 0);
    sg_vlbuf_destroy(buf);
}

TEST_F(test_big_int, transform_regular_integer)
{
    int i;
    enum sg_c_int_type type;

    i = 1024;
    sg_big_int_set_c_int(_a, &i, SGCINTTYPE_SINT);
    sg_big_int_try_to_get_c_int_type(_a, &type);
    ASSERT_EQ(type, SGCINTTYPE_SSHORT);

    i = 65534;
    sg_big_int_set_c_int(_a, &i, SGCINTTYPE_SINT);
    sg_big_int_try_to_get_c_int_type(_a, &type);
    ASSERT_EQ(type, SGCINTTYPE_USHORT);

    i = 65536;
    sg_big_int_set_c_int(_a, &i, SGCINTTYPE_SINT);
    sg_big_int_try_to_get_c_int_type(_a, &type);
    ASSERT_EQ(type, SGCINTTYPE_SINT);

    int j;
    sg_big_int_get_c_int(_a, &j, type);
    ASSERT_EQ(i, j);
}

TEST_F(test_big_int, transform_regular_float)
{
    double d = 12342332344.0, e;
    sg_big_int_set_c_float(_a, &d, SGCFLOATTYPE_SDOUBLE);
    sg_big_int_get_c_float(_a, &e, SGCFLOATTYPE_SDOUBLE);
    ASSERT_DOUBLE_EQ(d, e);

    float f = 234.0f, g;
    sg_big_int_set_c_float(_a, &f, SGCFLOATTYPE_SFLOAT);
    sg_big_int_get_c_float(_a, &g, SGCFLOATTYPE_SFLOAT);
    ASSERT_FLOAT_EQ(f, g);
}

TEST_F(test_big_int, assign_big_int)
{
    sg_big_int_set_str(_a, "1003000800980000", SGNUMSYS_DEC);
    sg_big_int_set_big_int(_b, _a);
    ASSERT_TRUE(sg_big_int_cmp(_a, _b) == SGBIGINTCMP_EQUALS);
}

TEST_F(test_big_int, assign_big_float)
{
    sg_big_float_t* f = sg_big_float_alloc();
    unsigned long long l;

    sg_big_float_set_str(f, "1003000800980.12345", SGNUMSYS_DEC);
    sg_big_int_set_big_float(_a, f);
    sg_big_int_get_c_int(_a, &l, SGCINTTYPE_SINT64);
    ASSERT_EQ(l, 1003000800980);

    sg_big_float_set_str(f, "-1003000800980.12345", SGNUMSYS_DEC);
    sg_big_int_set_big_float(_a, f);
    sg_big_int_get_c_int(_a, &l, SGCINTTYPE_SINT64);
    ASSERT_EQ(l, -1003000800980);

    sg_big_float_free(f);
}

TEST_F(test_big_int, tranform_string)
{
    const char* src;
    sg_vlstr_t* str = sg_vlstralloc();

    src = "1024000000";
    sg_big_int_set_str(_a, src, SGNUMSYS_DEC);
    sg_big_int_get_str(_a, SGNUMSYS_DEC, str);
    ASSERT_STREQ(sg_vlstrraw(str), src);

    src = "-3480000000";
    sg_big_int_set_str(_a, src, SGNUMSYS_DEC);
    sg_big_int_get_str(_a, SGNUMSYS_DEC, str);
    ASSERT_STREQ(sg_vlstrraw(str), src);

    src = "3480a0b0c0";
    sg_big_int_set_str(_a, src, SGNUMSYS_HEX);
    sg_big_int_get_str(_a, SGNUMSYS_HEX, str);
    ASSERT_STREQ(sg_vlstrraw(str), src);

    src = "110010011";
    sg_big_int_set_str(_a, src, SGNUMSYS_BIN);
    sg_big_int_get_str(_a, SGNUMSYS_BIN, str);
    ASSERT_STREQ(sg_vlstrraw(str), src);

    sg_vlstrfree(&str);
}

TEST_F(test_big_int, add)
{
    sg_big_int_set_str(_a, "1003000800980000", SGNUMSYS_DEC);
    sg_big_int_set_str(_b, "3007000100050000", SGNUMSYS_DEC);
    sg_big_int_set_str(_d, "4010000901030000", SGNUMSYS_DEC);
    sg_big_int_add(_a, _b, _c);
    ASSERT_TRUE(sg_big_int_cmp(_c, _d) == SGBIGINTCMP_EQUALS);
}

TEST_F(test_big_int, sub)
{
    sg_big_int_set_str(_a, "4010000901030000", SGNUMSYS_DEC);
    sg_big_int_set_str(_b, "3007000100050000", SGNUMSYS_DEC);
    sg_big_int_set_str(_d, "1003000800980000", SGNUMSYS_DEC);
    sg_big_int_sub(_a, _b, _c);
    ASSERT_TRUE(sg_big_int_cmp(_c, _d) == SGBIGINTCMP_EQUALS);

    sg_big_int_set_str(_a, "3007000100050000", SGNUMSYS_DEC);
    sg_big_int_set_str(_b, "4010000901030000", SGNUMSYS_DEC);
    sg_big_int_set_str(_d, "-1003000800980000", SGNUMSYS_DEC);
    sg_big_int_sub(_a, _b, _c);
    ASSERT_TRUE(sg_big_int_cmp(_c, _d) == SGBIGINTCMP_EQUALS);
}

TEST_F(test_big_int, multiply)
{
    sg_big_int_set_str(_a, "3000102000030000", SGNUMSYS_DEC);
    sg_big_int_set_str(_b, "3", SGNUMSYS_DEC);
    sg_big_int_set_str(_d, "9000306000090000", SGNUMSYS_DEC);
    sg_big_int_mul(_a, _b, _c);
    ASSERT_TRUE(sg_big_int_cmp(_c, _d) == SGBIGINTCMP_EQUALS)
        << "on positive x positive";

    sg_big_int_set_str(_a, "-3000102000030000", SGNUMSYS_DEC);
    sg_big_int_set_str(_d, "-9000306000090000", SGNUMSYS_DEC);
    sg_big_int_mul(_a, _b, _c);
    ASSERT_TRUE(sg_big_int_cmp(_c, _d) == SGBIGINTCMP_EQUALS)
        << "on negative x positive";

    sg_big_int_set_str(_b, "-3", SGNUMSYS_DEC);
    sg_big_int_set_str(_d, "9000306000090000", SGNUMSYS_DEC);
    sg_big_int_mul(_a, _b, _c);
    ASSERT_TRUE(sg_big_int_cmp(_c, _d) == SGBIGINTCMP_EQUALS)
        << "on negative x negative";
}

TEST_F(test_big_int, divide)
{
    sg_big_int_set_str(_a, "9000306000090000", SGNUMSYS_DEC);
    sg_big_int_set_str(_b, "3000102000030000", SGNUMSYS_DEC);
    sg_big_int_set_str(_d, "3", SGNUMSYS_DEC);
    sg_big_int_div(_a, _b, _c);
    ASSERT_TRUE(sg_big_int_cmp(_c, _d) == SGBIGINTCMP_EQUALS);

    sg_big_int_set_str(_b, "3000102000030999", SGNUMSYS_DEC);
    sg_big_int_set_str(_d, "3", SGNUMSYS_DEC);
    sg_big_int_div(_a, _b, _c);
    ASSERT_TRUE(sg_big_int_cmp(_c, _d) == SGBIGINTCMP_EQUALS);

    sg_big_int_set_str(_a, "9000306000090000", SGNUMSYS_DEC);
    sg_big_int_set_str(_b, "-3000102000030000", SGNUMSYS_DEC);
    sg_big_int_set_str(_d, "-3", SGNUMSYS_DEC);
    sg_big_int_div(_a, _b, _c);
    ASSERT_TRUE(sg_big_int_cmp(_c, _d) == SGBIGINTCMP_EQUALS);

}

TEST_F(test_big_int, mod)
{
    sg_big_int_set_str(_a, "9000306000090000", SGNUMSYS_DEC);
    sg_big_int_set_str(_b, "3000102000030000", SGNUMSYS_DEC);
    sg_big_int_set_str(_d, "0", SGNUMSYS_DEC);
    sg_big_int_mod(_a, _b, _c);
    ASSERT_TRUE(sg_big_int_cmp(_c, _d) == SGBIGINTCMP_EQUALS)
        << "on no remainer mod";

    sg_big_int_set_str(_a, "9000306000090003", SGNUMSYS_DEC);
    sg_big_int_set_str(_d, "3", SGNUMSYS_DEC);
    sg_big_int_mod(_a, _b, _c);
    ASSERT_TRUE(sg_big_int_cmp(_c, _d) == SGBIGINTCMP_EQUALS)
        << "on positive remainer mod";

    sg_big_int_set_str(_a, "9000306000089993", SGNUMSYS_DEC);
    sg_big_int_set_str(_d, "3000102000029993", SGNUMSYS_DEC);
    sg_big_int_mod(_a, _b, _c);
    ASSERT_TRUE(sg_big_int_cmp(_c, _d) == SGBIGINTCMP_EQUALS)
        << "on negative remainer mod";
}

TEST_F(test_big_int, sqrt)
{
    sg_big_int_set_str(_a, "1000000000000000", SGNUMSYS_DEC);
    sg_big_int_set_str(_d, "31622776", SGNUMSYS_DEC);
    sg_big_int_sqrt(_a, _c);
    ASSERT_TRUE(sg_big_int_cmp(_c, _d) == SGBIGINTCMP_EQUALS);
}

TEST_F(test_big_int, shift)
{
    sg_big_int_set_str(_a, "256", SGNUMSYS_DEC);
    sg_big_int_set_str(_d, "65536", SGNUMSYS_DEC);
    sg_big_int_shift(_a, true, 8, _c);
    ASSERT_TRUE(sg_big_int_cmp(_c, _d) == SGBIGINTCMP_EQUALS)
        << "c = a << 8";

    sg_big_int_set_str(_a, "256", SGNUMSYS_DEC);
    sg_big_int_set_str(_d, "1", SGNUMSYS_DEC);
    sg_big_int_shift(_a, false, 8, _c);
    ASSERT_TRUE(sg_big_int_cmp(_c, _d) == SGBIGINTCMP_EQUALS)
        << "c = a >> 8";
}

TEST_F(test_big_int, compare)
{
    sg_big_int_set_str(_a, "9000306000090000", SGNUMSYS_DEC);
    sg_big_int_set_str(_b, "3000102000030000", SGNUMSYS_DEC);
    ASSERT_TRUE(sg_big_int_cmp(_a, _b) == SGBIGINTCMP_A_BIGGER)
        << "on a > b";

    sg_big_int_set_str(_a, "-3000102000030000", SGNUMSYS_DEC);
    sg_big_int_set_str(_b, "-3000102000030000", SGNUMSYS_DEC);
    ASSERT_TRUE(sg_big_int_cmp(_a, _b) == SGBIGINTCMP_EQUALS)
        << "on a = b";

    sg_big_int_set_str(_a, "-9000306000090000", SGNUMSYS_DEC);
    sg_big_int_set_str(_b, "-3000102000030000", SGNUMSYS_DEC);
    ASSERT_TRUE(sg_big_int_cmp(_a, _b) == SGBIGINTCMP_B_BIGGER)
        << "on a < b";
}

TEST_F(test_big_int, compare_with_0)
{
    sg_big_int_set_str(_a, "3000102000030000", SGNUMSYS_DEC);
    ASSERT_TRUE(sg_big_int_cmp_zero(_a) == 1)
        << "on a > 0";
    sg_big_int_set_str(_a, "0", SGNUMSYS_DEC);
    ASSERT_TRUE(sg_big_int_cmp_zero(_a) == 0)
        << "on a = 0";
    sg_big_int_set_str(_a, "-3000102000030000", SGNUMSYS_DEC);
    ASSERT_TRUE(sg_big_int_cmp_zero(_a) == -1)
        << "on a < 0";
}

TEST_F(test_big_int, check_number_string)
{
    ASSERT_TRUE(sg_big_int_check_num_str("124235", SGNUMSYS_DEC));
    ASSERT_TRUE(sg_big_int_check_num_str("-124235", SGNUMSYS_DEC));
    ASSERT_TRUE(sg_big_int_check_num_str("a83ec8", SGNUMSYS_HEX));
    ASSERT_TRUE(sg_big_int_check_num_str("-a83ec8", SGNUMSYS_HEX));
    ASSERT_TRUE(sg_big_int_check_num_str("101100110", SGNUMSYS_BIN));
    ASSERT_TRUE(sg_big_int_check_num_str("-101100110", SGNUMSYS_BIN));

    ASSERT_FALSE(sg_big_int_check_num_str("124235", SGNUMSYS_BIN));
    ASSERT_FALSE(sg_big_int_check_num_str("12a235", SGNUMSYS_DEC));
    ASSERT_FALSE(sg_big_int_check_num_str("12g235", SGNUMSYS_HEX));
}
