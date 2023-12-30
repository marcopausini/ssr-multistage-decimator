/**
 * @file ssr_multistage_decimator.cpp
 *
 * @brief Source file for the super-sample rate multistage decimator.
 *
 *
 * @author marco.pausini@gmail.com
 * @date 2023-11-xy
 * @version 0.1
 *
 */

#include "ssr_multistage_decimator.h"
#include "dec_filters.h"

template <int N>
void copy_data(cdata_vec_t<N> &tdata_i, cdataout_vec_t<ssr> &tdata_o)
{
    for (int i = 0; i < N; ++i)
    {
#pragma HLS UNROLL
        tdata_o.re[i] = tdata_i.re[i];
        tdata_o.im[i] = tdata_i.im[i];
    }
}

template <int N>
void read_data(cdata_vec_t<2*N> &tdata_i, cdata_vec_t<N> &tdata_o)
{
    for (int i = 0; i < N; ++i)
    #pragma HLS UNROLL
    {
        tdata_o.re[i] = tdata_i.re[i];
        tdata_o.im[i] = tdata_i.im[i];
    }
}

void copy_data(cdatain_vec_t<ssr> &tdata_i, cdataout_vec_t<ssr> &tdata_o)
{
    for (int i = 0; i < ssr; ++i)
    {
#pragma HLS UNROLL
        tdata_o.re[i] = tdata_i.re[i];
        tdata_o.im[i] = tdata_i.im[i];
    }
}

// FIR filter parameters
void ssr_multistage_decimator(dec_factor_t dec_factor, bool tvalid_i, cdatain_vec_t<ssr> tdata_i, bool &tvalid_o, cdataout_vec_t<ssr> &tdata_o)
{

//  add pragams for the decimation factor
#pragma HLS INTERFACE s_axilite port = dec_factor bundle = control

    
    // first filter stage (decimation factor = 2)
    bool tvalid_dec2;
    cdata_vec_t<8> tdata_o_dec2;
    dec2_ssr8(tvalid_i, tdata_i, tvalid_dec2, tdata_o_dec2);

    // second filter stage (decimation factor = 4)
    bool tvalid_dec4;
    cdata_vec_t<4> tdata_i_dec4;
    cdata_vec_t<4> tdata_o_dec4;
    read_data<4>(tdata_o_dec2, tdata_i_dec4);
    dec2_ssr4(tvalid_dec2, tdata_i_dec4, tvalid_dec4, tdata_o_dec4);

    // third filter stage (decimation factor = 8)
    bool tvalid_dec8;
    cdata_vec_t<2> tdata_i_dec8;
    cdata_vec_t<2> tdata_o_dec8;
    read_data<2>(tdata_o_dec4, tdata_i_dec8);
    dec2_ssr2(tvalid_dec4, tdata_i_dec8, tvalid_dec8, tdata_o_dec8);

    // fourth filter stage (decimation factor = 16)
    bool tvalid_dec16;
    cdata_vec_t<1> tdata_i_dec16;
    cdata_vec_t<1> tdata_dec16;
    read_data<1>(tdata_o_dec8, tdata_i_dec16);
    dec2_ssr1<16>(tvalid_dec8, tdata_i_dec16, tvalid_dec16, tdata_dec16);
    // debug
    //hbf<0>(tvalid_dec8, tdata_dec8, tvalid_dec16, tdata_dec16);

    // fifth filter stage (decimation factor = 32)
    bool tvalid_dec32;
    cdata_vec_t<1> tdata_dec32;
    dec2_ssr1<32>(tvalid_dec16, tdata_dec16, tvalid_dec32, tdata_dec32);

    // sixth filter stage (decimation factor = 64)
    bool tvalid_dec64;
    cdata_vec_t<1> tdata_dec64;
    dec2_ssr1<64>(tvalid_dec32, tdata_dec32, tvalid_dec64, tdata_dec64);

    // get the output data
    if (dec_factor == 1)
    {
        tvalid_o = tvalid_i;
        copy_data(tdata_i, tdata_o);
    }
    else if (dec_factor == 2)
    {
        tvalid_o = tvalid_dec2;
        copy_data<8>(tdata_o_dec2, tdata_o);
    }
    else if (dec_factor == 4)
    {
        tvalid_o = tvalid_dec4;
        copy_data<4>(tdata_o_dec4, tdata_o);
    }
    else if (dec_factor == 8)
    {
        tvalid_o = tvalid_dec8;
        copy_data<2>(tdata_o_dec8, tdata_o);
    }
    else if (dec_factor == 16)
    {
        tvalid_o = tvalid_dec16;
        copy_data<1>(tdata_dec16, tdata_o);
    }
    else if (dec_factor == 32)
    {
        tvalid_o = tvalid_dec32;
        copy_data<1>(tdata_dec32, tdata_o);
    }
    else if (dec_factor == 64)
    {
        tvalid_o = tvalid_dec64;
        copy_data<1>(tdata_dec64, tdata_o);
    }
    else
    {
        tvalid_o = false;
    }
}