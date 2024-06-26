/**
 * @file ssr_multistage_decimator.cpp
 *
 * @brief Top file for the super-sample rate multistage decimator.
 *
 *  Cascade of half-band decimator-by-2 filters.
 *  The first 3 stages are Super-Sample Rate (SSR) filters, processing multiple samples per clock cycle.
 *  The parallel processing is achieved by means of polyphase decomposition.
 *
 * - dec2_ssr8: 1280 -> 640 (decimation factor = 2, SSR = 8)
 * - dec2_ssr4: 640 -> 320 (decimation factor = 4, SSR = 4)
 * - dec2_ssr2: 320 -> 160 (decimation factor = 8, SSR = 2)
 * - dec2       160 -> 80 (decimation factor = 16, SSR = 1)
 * - dec2       80 -> 40 (decimation factor = 32, SSR = 1)
 * - dec2       40 -> 20 (decimation factor = 64, SSR = 1)
 *
 * The module accepts a block of 8 complex samples per clock cycle,
 * and produces a block of 1,2,4, or 8 (by-pass) complex samples per clock cycle.
 *
 * @author marco.pausini@gmail.com
 * @date 2023-11-xy
 * @version 0.1
 *
 */

#include "ssr_multistage_decimator.h"
#include "dec_filters.h"

 /**
  * @brief write decimator output data to output port
  *
  */
template <int N>
cdataout_vec_t<ssr> copy_data(cdata_vec_t<N> tdata_i)
{
    cdataout_vec_t<ssr> tdata_o;
    for (int i = 0; i < N; ++i) {
    #pragma HLS UNROLL
        tdata_o.re[i] = tdata_i.re[i];
        tdata_o.im[i] = tdata_i.im[i];
    }
    return tdata_o;
}

/**
 * @brief read decimator input data from previous stage
 *
 */
template <int N>
cdata_vec_t<N> read_data(cdata_vec_t<2 * N> tdata_i)
{
    cdata_vec_t<N> tdata_o;
    for (int i = 0; i < N; ++i)
    #pragma HLS UNROLL
    {
        tdata_o.re[i] = tdata_i.re[i];
        tdata_o.im[i] = tdata_i.im[i];
    }
    return tdata_o;
}

/**
 * @brief copy input data to output port (by-pass)
 *
 */
cdataout_vec_t<ssr> copy_data(cdatain_vec_t<ssr> tdata_i)
{
    cdataout_vec_t<ssr> tdata_o;
    for (int i = 0; i < ssr; ++i) {
    #pragma HLS UNROLL
        tdata_o.re[i] = tdata_i.re[i];
        tdata_o.im[i] = tdata_i.im[i];
    }
    return tdata_o;
}

// 
/**
 * @brief Performs multistage decimation on the input data.
 *
 * This function takes the input data and applies multistage decimation with the given decimation factor.
 * The input data is a vector of complex data samples, and the output data is also a vector of complex data samples.
 *
 * @param dec_factor The decimation factor to be applied.
 * @param tvalid_i The validity flag of the input data.
 * @param tdata_i The input data vector.
 * @param tvalid_o The validity flag of the output data.
 * @param tdata_o The output data vector.
 */
void ssr_multistage_decimator(dec_factor_t dec_factor, bool tvalid_i, cdatain_vec_t<ssr> tdata_i, bool& tvalid_o, cdataout_vec_t<ssr>& tdata_o)
{


    // ----------------------------------------------------
    // first filter stage (decimation factor = 2)
    // ----------------------------------------------------
    bool tvalid_dec2;
    cdata_vec_t<8> tdata_o_dec2;
    dec2_ssr8(tvalid_i, tdata_i, tvalid_dec2, tdata_o_dec2);

    // ----------------------------------------------------
    // second filter stage (decimation factor = 4)
    // ----------------------------------------------------
    bool tvalid_dec4;
    cdata_vec_t<4> tdata_i_dec4;
    cdata_vec_t<4> tdata_o_dec4;
    tdata_i_dec4 = read_data<4>(tdata_o_dec2);
    dec2_ssr4(tvalid_dec2, tdata_i_dec4, tvalid_dec4, tdata_o_dec4);

    // ----------------------------------------------------
    // third filter stage (decimation factor = 8)
    // ----------------------------------------------------
    bool tvalid_dec8;
    cdata_vec_t<2> tdata_i_dec8;
    cdata_vec_t<2> tdata_o_dec8;
    tdata_i_dec8 = read_data<2>(tdata_o_dec4);
    dec2_ssr2(tvalid_dec4, tdata_i_dec8, tvalid_dec8, tdata_o_dec8);

    // ----------------------------------------------------
    // fourth filter stage (decimation factor = 16)
    // ----------------------------------------------------
    bool tvalid_dec16;
    cdata_vec_t<1> tdata_i_dec16;
    cdata_vec_t<1> tdata_dec16;
    tdata_i_dec16 = read_data<1>(tdata_o_dec8);
    dec2_ssr1<16>(tvalid_dec8, tdata_i_dec16, tvalid_dec16, tdata_dec16);

    // ----------------------------------------------------
    // fifth filter stage (decimation factor = 32)
    // ----------------------------------------------------
    bool tvalid_dec32;
    cdata_vec_t<1> tdata_dec32;
    dec2_ssr1<32>(tvalid_dec16, tdata_dec16, tvalid_dec32, tdata_dec32);

    // ----------------------------------------------------
    // sixth filter stage (decimation factor = 64)
    // ----------------------------------------------------
    bool tvalid_dec64;
    cdata_vec_t<1> tdata_dec64;
    dec2_ssr1<64>(tvalid_dec32, tdata_dec32, tvalid_dec64, tdata_dec64);

    // ----------------------------------------------------
    // select the output data based on the decimation factor
    // ----------------------------------------------------
    if (dec_factor == 1) {
        tvalid_o = tvalid_i;
        tdata_o = copy_data(tdata_i);
    } else if (dec_factor == 2) {
        tvalid_o = tvalid_dec2;
        tdata_o = copy_data<8>(tdata_o_dec2);
    } else if (dec_factor == 4) {
        tvalid_o = tvalid_dec4;
        tdata_o =  copy_data<4>(tdata_o_dec4);
    } else if (dec_factor == 8) {
        tvalid_o = tvalid_dec8;
        tdata_o = copy_data<2>(tdata_o_dec8);
    } else if (dec_factor == 16) {
        tvalid_o = tvalid_dec16;
        tdata_o = copy_data<1>(tdata_dec16);
    } else if (dec_factor == 32) {
        tvalid_o = tvalid_dec32;
        tdata_o = copy_data<1>(tdata_dec32);
    } else if (dec_factor == 64) {
        tvalid_o = tvalid_dec64;
        tdata_o = copy_data<1>(tdata_dec64);
    } else {
        tvalid_o = false;
        for (int i = 0; i < ssr; ++i) {
        #pragma HLS UNROLL
            tdata_o.re[i] = 0;
            tdata_o.im[i] = 0;
        }
    }
}