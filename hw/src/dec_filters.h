/**
 * @file dec_filters.h
 *
 * @brief Decimation (Half-band) filters implementing the SSR multi-stage decimator
 *
 * - dec2_ssr8: 1280 -> 640 (decimation factor = 2, SSR = 8)
 * - dec2_ssr4: 640 -> 320 (decimation factor = 4, SSR = 4)
 * - dec2_ssr2: 320 -> 160 (decimation factor = 8, SSR = 2)
 * - dec2       160 -> 80 (decimation factor = 16, SSR = 1)
 * - dec2       80 -> 40 (decimation factor = 32, SSR = 1)
 * - dec2       40 -> 20 (decimation factor = 64, SSR = 1)
 *
 *
 * @author marco.pausini@gmail.com
 * @date 2023-11-xy
 * @version 0.1
 *
 */

#include "ssr_multistage_decimator.h"
#include "mac_engines.h"

/*
const double coeff_vec[31] = {
    -0.001503, 0.000000, 0.003822, 0.000000, -0.008293, 0.000000,
    0.015862, 0.000000, -0.028404, 0.000000, 0.050323, 0.000000,
    -0.097603, 0.000000, 0.315392, 0.500000, 0.315392, 0.000000,
    -0.097603, 0.000000, 0.050323, 0.000000, -0.028404, 0.000000,
    0.015862, 0.000000, -0.008293, 0.000000, 0.003822, 0.000000,
    -0.001503};
*/

void dec2_ssr8(bool tvalid_i, cdatain_vec_t<8> tdata_i, bool &tvalid_o, cdata_vec_t<4> &tdata_o)
{


    tvalid_o = tvalid_i;

    for (int i = 0; i < 4; i++)
    #pragma HLS unroll
    {
        tdata_o.re[i] = tdata_i.re[2 * i];
        tdata_o.im[i] = tdata_i.im[2 * i];
    }

    // run the filter
  //  filter.run(xi, yi);
   // filter.run(xq, yq);
    
};

void dec2_ssr4(bool tvalid_i, cdata_vec_t<4> tdata_i, bool &tvalid_o, cdata_vec_t<2> &tdata_o)
{

    tvalid_o = tvalid_i;

    for (int i = 0; i < 2; i++)
#pragma HLS unroll
    {
        tdata_o.re[i] = tdata_i.re[2 * i];
        tdata_o.im[i] = tdata_i.im[2 * i];
    }

    // run the filter
    //  filter.run(xi, yi);
    // filter.run(xq, yq);
};

// ---------------------------------------------------------------------------------------------
// dec2_ssr2: 320 -> 160 ( overal decimation factor = 8, SSR = 2)
// 2 inputs per clock cycle are processed in parallel using polyphase decomposition
// Y(z) = Y0(z^2) + z^-1 Y1(z^2), where:
// * Y0(z^2) = P0(z^2) X0(z^2) + (z^-2)P1(z^2) X1(z^2)
// * Y1(z^2) = P1(z^2) X0(z^2) + P0(z^2) X1(z^2)
// only 1 output is computed per clock cycle - the other is not computed because it needs to be discarded,
// since the decimation factor is 2
// ---------------------------------------------------------------------------------------------
void dec2_ssr2(bool tvalid_i, cdata_vec_t<2> tdata_i, bool &tvalid_o, cdata_vec_t<1> &tdata_o)
{

#pragma HLS INLINE off

    constexpr unsigned int num_coef = 16;
    // polyphase decomposition coefficients
    const coef_int_t coeff_vec0[num_coef] = {-197, 501, -1087, 2079, -3723, 6596, -12793, 41339, 41339, -12793, 6596, -3723, 2079, -1087, 501, -197};
    const coef_int_t coeff_vec1[num_coef] = {0, 0, 0, 0, 0, 0, 0, 0,65536, 0, 0, 0, 0, 0, 0, 0, 0};

    // shift registers to align valid to the module output (1 extra clock for the final sum)
    static ap_shift_reg<bool, (num_coef+1)+1> vld_shftreg;

    // ---------------------------------------------
    // control the shift register of the mac engine 
    // ----------------------------------------------
    static bool skip = false;
    // whether the filter memory is updated with the sample at its input, or it is maintained in the current state
    bool toshift_v = tvalid_i;
    // valid output 
    bool tvalid_v = (tvalid_i && !skip);

    // align the valid signal to the output
    tvalid_o = vld_shftreg.shift(tvalid_v, num_coef + 1 - 1);

    // input sample
    cdata_t tdata_vi[2];
    // X0(z^2):  x(n), x(n-2), x(n-4), ....
    tdata_vi[0].re = tdata_i.re[1];
    tdata_vi[0].im = tdata_i.im[1];
    // X1(z^2): x(n-1), x(n-3), x(n-5), ....
    tdata_vi[1].re = tdata_i.re[0];
    tdata_vi[1].im = tdata_i.im[0];

    // compute the output P1(z^2) X0(z^2) 
    cacc_t acc0 = multi_mac_systolic<8, num_coef>(toshift_v, tdata_vi[0], coeff_vec1);
    // compute the output P0(z^2) X1(z^2)
    cacc_t acc1 = multi_mac_systolic<9, num_coef>(toshift_v, tdata_vi[1], coeff_vec0);

    // output sample - sum and cast to the output data type
    cacc_t acc = {acc0.re + acc1.re, acc0.im + acc1.im};

    tdata_o.re[0] = acc.re;
    tdata_o.im[0] = acc.im;

}

template <int instance_id>
void dec2_ssr1(bool tvalid_i, cdata_vec_t<1> tdata_i, bool &tvalid_o, cdata_vec_t<1> &tdata_o)
{

#pragma HLS INLINE off

    constexpr unsigned int num_coef = 31;
    // prototype filter coefficients
    const coef_int_t coeff_vec[num_coef] = {-197, 0, 501, 0, -1087, 0, 2079, 0, -3723, 0, 6596, 0, -12793, 0, 41339, 65536, 41339, 0, -12793, 0, 6596, 0, -3723, 0, 2079, 0, -1087, 0, 501, 0, -197};
    // shift registers to align valid and beamid signal to the module output
    static ap_shift_reg<bool, num_coef+1> vld_shftreg;

    // ----------------------------------------------
    // control the shift register of the mac engine 
    // ----------------------------------------------
    static bool skip = false;
    // whether the filter memory is updated with the sample at its input, or it is maintained in the current state
    bool toshift_v = tvalid_i;
    // valid output 
    bool tvalid_v = (tvalid_i && !skip);

    if (tvalid_v)
    {
        // when the output is valid, the next output will be discarded
        skip = true;
    }
    else if (tvalid_i)
    {
        // the output was not valid (skipped), then next output will be valid
        skip = false;
    }
    // align the valid signal to the output
    tvalid_o = vld_shftreg.shift(tvalid_v, num_coef +1 - 1);

    // input sample
    cdata_t tdata_vi;
    tdata_vi.re = tdata_i.re[0];
    tdata_vi.im = tdata_i.im[0];

    // compute the output
    cacc_t acc = multi_mac_systolic<instance_id, num_coef>(toshift_v, tdata_vi, coeff_vec);

    // output sample - cast to the output data type
    tdata_o.re[0] = acc.re;
    tdata_o.im[0] = acc.im;
}

// used for debugging
template <int instance_id>
void hbf(bool tvalid_i, cdata_vec_t<1> tdata_i, bool &tvalid_o, cdata_vec_t<1> &tdata_o)
{
    
#pragma HLS INLINE off

    constexpr unsigned int num_coef = 31;
    // prototype filter coefficients
    const coef_int_t coeff_vec[num_coef] = {-197, 0, 501, 0, -1087, 0, 2079, 0, -3723, 0, 6596, 0, -12793, 0, 41339, 65536, 41339, 0, -12793, 0, 6596, 0, -3723, 0, 2079, 0, -1087, 0, 501, 0, -197};
    // shift registers to align valid and beamid signal to the module output
    static ap_shift_reg<bool, num_coef + 1> vld_shftreg;

    // ----------------------------------------------
    // control the shift register of the mac engine 
    // ----------------------------------------------
    static bool skip = false;
    // whether the filter memory is updated with the sample at its input, or it is maintained in the current state
    bool toshift_v = tvalid_i;
    // valid output 
    bool tvalid_v = (tvalid_i && !skip);

    if (tvalid_v)
    {
        // when the output is valid, the next output will be discarded
        skip = false;
    }
    else if (tvalid_i)
    {
        // the output was not valid (skipped), then next output will be valid
        skip = false;
    }
    // align the valid signal to the output
    tvalid_o = vld_shftreg.shift(tvalid_v, num_coef + 1 - 1);

    // input sample
    cdata_t tdata_vi;
    tdata_vi.re = tdata_i.re[0];
    tdata_vi.im = tdata_i.im[0];

    // compute the output
    cacc_t acc = multi_mac_systolic<instance_id, num_coef>(toshift_v, tdata_vi, coeff_vec);

    // output sample - cast to the output data type
    tdata_o.re[0] = acc.re;
    tdata_o.im[0] = acc.im;
}
