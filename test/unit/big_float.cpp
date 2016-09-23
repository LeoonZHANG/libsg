#include <sg/math/big_float.h>
#include <sg/math/big_int.h>
#include <gtest/gtest.h>

class test_big_float : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        _a = sg_big_float_alloc();
        _b = sg_big_float_alloc();
        _c = sg_big_float_alloc();
        _d = sg_big_float_alloc();
    }

    virtual void TearDown()
    {
        sg_big_float_free(_a);
        sg_big_float_free(_b);
        sg_big_float_free(_c);
        sg_big_float_free(_d);
    }

    sg_big_float_t* _a;
    sg_big_float_t* _b;
    sg_big_float_t* _c;
    sg_big_float_t* _d;
};

TEST_F(test_big_float, transform_regular_integer)
{
    int i;
    enum sg_c_int_type type;

    i = 1024;
    sg_big_float_set_c_int(_a, &i, SGCINTTYPE_SINT);
    sg_big_float_try_to_get_c_int_type(_a, &type);
    ASSERT_EQ(type, SGCINTTYPE_SSHORT);

    i = 65534;
    sg_big_float_set_c_int(_a, &i, SGCINTTYPE_SINT);
    sg_big_float_try_to_get_c_int_type(_a, &type);
    ASSERT_EQ(type, SGCINTTYPE_USHORT);

    i = 65536;
    sg_big_float_set_c_int(_a, &i, SGCINTTYPE_SINT);
    sg_big_float_try_to_get_c_int_type(_a, &type);
    ASSERT_EQ(type, SGCINTTYPE_SINT);

    int j;
    sg_big_float_get_c_int(_a, &j, type);
    ASSERT_EQ(i, j);
}

TEST_F(test_big_float, transform_regular_float)
{
    double d = 12342332344.0, e;
    ASSERT_EQ(0, sg_big_float_set_c_float(_a, &d, SGCFLOATTYPE_SDOUBLE));
    ASSERT_EQ(0, sg_big_float_get_c_float(_a, &e, SGCFLOATTYPE_SDOUBLE));
    ASSERT_DOUBLE_EQ(d, e);

    float f = 1123423e6, g;
    ASSERT_EQ(0, sg_big_float_set_c_float(_a, &f, SGCFLOATTYPE_SFLOAT));
    ASSERT_EQ(0, sg_big_float_get_c_float(_a, &g, SGCFLOATTYPE_SFLOAT));
    ASSERT_FLOAT_EQ(f, g);
}

TEST_F(test_big_float, assign_big_integer)
{
    ASSERT_EQ(0, sg_big_float_set_str(_a, "1003000800980000", SGNUMSYS_DEC));

    sg_big_int_t* big_int = sg_big_int_alloc();
    ASSERT_EQ(0, sg_big_int_set_str(big_int, "1003000800980000", SGNUMSYS_DEC));
    ASSERT_EQ(0, sg_big_float_set_big_int(_b, big_int));
    sg_big_int_free(big_int);

    ASSERT_TRUE(sg_big_float_cmp(_a, _b) == SGBIGFLOATCMP_EQUALS);
}

TEST_F(test_big_float, assign_big_float)
{
    ASSERT_EQ(0, sg_big_float_set_str(_a, "100300080098.1234e100", SGNUMSYS_DEC));
    ASSERT_EQ(0, sg_big_float_set_big_float(_b, _a));
    ASSERT_TRUE(sg_big_float_cmp(_a, _b) == SGBIGFLOATCMP_EQUALS);
}

TEST_F(test_big_float, tranform_string)
{
    const char* src;
    sg_vsstr_t* str = sg_vsstr_alloc();

    src = "1024000000";
    ASSERT_EQ(0, sg_big_float_set_str(_a, src, SGNUMSYS_DEC));
    ASSERT_EQ(0, sg_big_float_get_str(_a, SGNUMSYS_DEC, str));
    ASSERT_STREQ(sg_vsstr_raw(str), "1024e6");

    src = "1024e6";
    ASSERT_EQ(0, sg_big_float_set_str(_a, src, SGNUMSYS_DEC));
    ASSERT_EQ(0, sg_big_float_get_str(_a, SGNUMSYS_DEC, str));
    ASSERT_STREQ(sg_vsstr_raw(str), "1024e6");

    src = "-1024e6";
    ASSERT_EQ(0, sg_big_float_set_str(_a, src, SGNUMSYS_DEC));
    ASSERT_EQ(0, sg_big_float_get_str(_a, SGNUMSYS_DEC, str));
    ASSERT_STREQ(sg_vsstr_raw(str), "-1024e6");

    src = "-3480000000";
    ASSERT_EQ(0, sg_big_float_set_str(_a, src, SGNUMSYS_DEC));
    ASSERT_EQ(0, sg_big_float_get_str(_a, SGNUMSYS_DEC, str));
    ASSERT_STREQ(sg_vsstr_raw(str), "-348e7");

    sg_vsstr_free(&str);
}

TEST_F(test_big_float, add)
{
    sg_big_float_set_str(_a, "1022222003080098.1234", SGNUMSYS_DEC);
    sg_big_float_set_str(_b, "1022222007010005.1234", SGNUMSYS_DEC);
    sg_big_float_add(_a, _b, _c);

    unsigned long long ui;
    sg_big_float_get_c_int(_c, &ui, SGCINTTYPE_UINT64);
    ASSERT_EQ(ui, 2044444010090103);
}

TEST_F(test_big_float, subtract)
{
    int64_t si64;

    sg_big_float_set_str(_a, "4010000901030000.4289", SGNUMSYS_DEC);
    sg_big_float_set_str(_b, "3007000100050000.2345", SGNUMSYS_DEC);
    sg_big_float_sub(_a, _b, _c);

    sg_big_float_get_c_int(_c, &si64, SGCINTTYPE_SINT64);
    ASSERT_EQ(si64, 1003000800980000);

    sg_big_float_set_str(_a, "3007000100050000.2345", SGNUMSYS_DEC);
    sg_big_float_set_str(_b, "4010000901030000.4289", SGNUMSYS_DEC);
    sg_big_float_sub(_a, _b, _c);

    sg_big_float_get_c_int(_c, &si64, SGCINTTYPE_SINT64);
    ASSERT_EQ(si64, -1003000800980000);
}

TEST_F(test_big_float, multiply)
{
    int64_t si64;
    sg_big_float_set_str(_a, "3000102000030000.1122", SGNUMSYS_DEC);
    sg_big_float_set_str(_b, "3", SGNUMSYS_DEC);
    sg_big_float_mul(_a, _b, _c);
    sg_big_float_get_c_int(_c, &si64, SGCINTTYPE_SINT64);
    ASSERT_EQ(si64, 9000306000090000)
        << "on positive x positive";

    sg_big_float_set_str(_a, "-300010200001234.30000", SGNUMSYS_DEC);
    sg_big_float_set_str(_b, "3", SGNUMSYS_DEC);
    sg_big_float_mul(_a, _b, _c);
    sg_big_float_get_c_int(_c, &si64, SGCINTTYPE_SINT64);
    ASSERT_EQ(si64, -900030600003702)
        << "on negative x positive";

    sg_big_float_set_str(_a, "-300010200000000.30000", SGNUMSYS_DEC);
    sg_big_float_set_str(_b, "-3", SGNUMSYS_DEC);
    sg_big_float_mul(_a, _b, _c);
    sg_big_float_get_c_int(_c, &si64, SGCINTTYPE_SINT64);
    ASSERT_EQ(si64, 900030600000000)
        << "on negative x negative";
}

TEST_F(test_big_float, divide)
{
    int64_t si64;
    sg_big_float_set_str(_a, "900030600000009.0066", SGNUMSYS_DEC);
    sg_big_float_set_str(_b, "300010200000003.0022", SGNUMSYS_DEC);
    sg_big_float_div(_a, _b, _c);
    sg_big_float_get_c_int(_c, &si64, SGCINTTYPE_SINT64);
    ASSERT_EQ(si64, 3);

    sg_big_float_set_str(_a, "900030600000009.0088", SGNUMSYS_DEC);
    sg_big_float_set_str(_b, "300010200000003.0022", SGNUMSYS_DEC);
    sg_big_float_div(_a, _b, _c);
    sg_big_float_get_c_int(_c, &si64, SGCINTTYPE_SINT64);
    ASSERT_EQ(si64, 3);

    sg_big_float_set_str(_a, "900030600000009.0044", SGNUMSYS_DEC);
    sg_big_float_set_str(_b, "300010200000003.0022", SGNUMSYS_DEC);
    sg_big_float_div(_a, _b, _c);
    sg_big_float_get_c_int(_c, &si64, SGCINTTYPE_SINT64);
    ASSERT_EQ(si64, 2);

    sg_big_float_set_str(_a,  "900030600000009.0003", SGNUMSYS_DEC);
    sg_big_float_set_str(_b, "-300010200000003.0001", SGNUMSYS_DEC);
    sg_big_float_div(_a, _b, _c);
    sg_big_float_get_c_int(_c, &si64, SGCINTTYPE_SINT64);
    ASSERT_EQ(si64, -3);
}

TEST_F(test_big_float, sqrt)
{
    sg_big_float_set_str(_a, "1000000000000000.192843", SGNUMSYS_DEC);
    sg_big_float_set_str(_d, "31622776", SGNUMSYS_DEC);
    sg_big_float_sqrt(_a, _c);
    int64_t si64;
    sg_big_float_get_c_int(_c, &si64, SGCINTTYPE_SINT64);
    ASSERT_EQ(si64, 31622776);
}

TEST_F(test_big_float, compare)
{
    sg_big_float_set_str(_a, "9000306000090000.000001", SGNUMSYS_DEC);
    sg_big_float_set_str(_b, "3000102000030000.999991", SGNUMSYS_DEC);
    ASSERT_TRUE(sg_big_float_cmp(_a, _b) == SGBIGFLOATCMP_A_BIGGER)
        << "on a > b";

    sg_big_float_set_str(_a, "-3000102000030000.123456", SGNUMSYS_DEC);
    sg_big_float_set_str(_b, "-3000102000030000.123456", SGNUMSYS_DEC);
    ASSERT_TRUE(sg_big_float_cmp(_a, _b) == SGBIGFLOATCMP_EQUALS)
        << "on a = b";

    sg_big_float_set_str(_a, "-9000306000090000.1111", SGNUMSYS_DEC);
    sg_big_float_set_str(_b, "-3000102000030000.9999", SGNUMSYS_DEC);
    ASSERT_TRUE(sg_big_float_cmp(_a, _b) == SGBIGFLOATCMP_B_BIGGER)
        << "on a < b";
}

TEST_F(test_big_float, compare_with_0)
{
    sg_big_float_set_str(_a, "3000102000030000.12341243", SGNUMSYS_DEC);
    ASSERT_TRUE(sg_big_float_cmp_zero(_a) == 1)
        << "on a > 0";
    sg_big_float_set_str(_a, "0", SGNUMSYS_DEC);
    ASSERT_TRUE(sg_big_float_cmp_zero(_a) == 0)
        << "on a = 0";
    sg_big_float_set_str(_a, "-3000102000030000.12341243", SGNUMSYS_DEC);
    ASSERT_TRUE(sg_big_float_cmp_zero(_a) == -1)
        << "on a < 0";
}

TEST_F(test_big_float, check_number_string)
{
    ASSERT_TRUE(sg_big_float_check_num_str("124235.123", SGNUMSYS_DEC));
    ASSERT_TRUE(sg_big_float_check_num_str("-124235.534", SGNUMSYS_DEC));
    ASSERT_TRUE(sg_big_float_check_num_str("a83ec8.a0c1", SGNUMSYS_HEX));
    ASSERT_TRUE(sg_big_float_check_num_str("-a83ec8.a0c1", SGNUMSYS_HEX));
    ASSERT_TRUE(sg_big_float_check_num_str("101100110.1001", SGNUMSYS_BIN));
    ASSERT_TRUE(sg_big_float_check_num_str("-101100110.1001", SGNUMSYS_BIN));

    ASSERT_FALSE(sg_big_float_check_num_str("124235.1001", SGNUMSYS_BIN));
    ASSERT_FALSE(sg_big_float_check_num_str("12a235.1001", SGNUMSYS_DEC));
    ASSERT_FALSE(sg_big_float_check_num_str("12g235.1001", SGNUMSYS_HEX));
}

