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
 *  Prototype filter coefficients:
 *  [-197, 0, 501, 0, -1087, 0, 2079, 0, -3723, 0, 6596, 0, -12793, 0, 41339, 
 *   65536, 41339, 0, -12793, 0, 6596, 0, -3723, 0, 2079, 0, -1087, 0, 501, 0, -197;]
 *
 * const double coeff_vec[31] = 
 * { -0.001503, 0.000000, 0.003822, 0.000000, -0.008293, 0.000000, 0.015862, 0.000000, -0.028404, 0.000000, 0.050323, 0.000000, -0.097603, 0.000000, 0.315392,
 *  0.500000, 0.315392, 0.000000,-0.097603, 0.000000, 0.050323, 0.000000, -0.028404, 0.000000,0.015862, 0.000000, -0.008293, 0.000000, 0.003822, 0.000000,-0.001503};
 *
 *
 * @author marco.pausini@gmail.com
 * @date 2023-11-xy
 * @version 0.1
 *
 */

#include "ssr_multistage_decimator.h"
#include "mac_engines.h"


// ---------------------------------------------------------------------------------------------
// dec2_ssr8: 1280 -> 640 ( overal decimation factor = 2, SSR = 8)
// 8 inputs per clock cycle are processed in parallel using polyphase decomposition
// Y(z) = Y0(z^8) + z^-1 Y1(z^8) + z^-2 Y2(z^8) + z^-3Y3(z^8) + z^-4Y4(z^8) + z^-5Y5(z^8) + z^-6Y6(z^8) + z^-7Y7(z^8), where:
// * Y0(z^8) = P0(z^8) X0(z^8) + (z^-8){P7(z^8) X1(z^8) + P6(z^8) X2(z^8) + P5(z^8) X3(z^8) + P4(z^8) X4(z^8) + P3(z^8) X5(z^8) + P2(z^8) X6(z^8) + P1(z^8) X7(z^8)}
// * Y1(z^8) = P1(z^8) X0(z^8) + P0(z^8) X1(z^8) + (z^-8){P7(z^8) X2(z^8) + P6(z^8) X3(z^8) + P5(z^8) X4(z^8) + P4(z^8) X5(z^8) + P3(z^8) X6(z^8) + P2(z^8) X7(z^8)}
// * Y2(z^8) = P2(z^8) X0(z^8) + P1(z^8) X1(z^8) + P0(z^8) X2(z^8) + (z^-8){P7(z^8) X3(z^8) + P6(z^8) X4(z^8) + P5(z^8) X5(z^8) + P4(z^8) X6(z^8) + P3(z^8) X7(z^8)}
// * Y3(z^8) = P3(z^8) X0(z^8) + P2(z^8) X1(z^8) + P1(z^8) X2(z^8) + P0(z^8) X3(z^8) + (z^-8){P7(z^8) X4(z^8) + P6(z^8) X5(z^8) + P5(z^8) X6(z^8) + P4(z^8) X7(z^8)}
// * Y4(z^8) = P4(z^8) X0(z^8) + P3(z^8) X1(z^8) + P2(z^8) X2(z^8) + P1(z^8) X3(z^8) + P0(z^8) X4(z^8) + (z^-8){P7(z^8) X5(z^8) + P6(z^8) X6(z^8) + P5(z^8) X7(z^8)}
// * Y5(z^8) = P5(z^8) X0(z^8) + P4(z^8) X1(z^8) + P3(z^8) X2(z^8) + P2(z^8) X3(z^8) + P1(z^8) X4(z^8) + P0(z^8) X5(z^8) + (z^-8){P7(z^8) X6(z^8) + P6(z^8) X7(z^8)}
// * Y6(z^8) = P6(z^8) X0(z^8) + P5(z^8) X1(z^8) + P4(z^8) X2(z^8) + P3(z^8) X3(z^8) + P2(z^8) X4(z^8) + P1(z^8) X5(z^8) + P0(z^8) X6(z^8) + (z^-8)P7(z^8) X7(z^8)
// * Y7(z^8) = P7(z^8) X0(z^8) + P6(z^8) X1(z^8) + P5(z^8) X2(z^8) + P4(z^8) X3(z^8) + P3(z^8) X4(z^8) + P2(z^8) X5(z^8) + P1(z^8) X6(z^8) + P0(z^8) X7(z^8)
//
// only 4 output are computed per clock cycle - the other are not computed because they are discarded,
//
// Y0(z^8) = tdata_o[7], X0(z^8) = tdata_i[7]
// Y1(z^8) = tdata_o[6], X1(z^8) = tdata_i[6]
// Y2(z^8) = tdata_o[5], X2(z^8) = tdata_i[5]
// Y3(z^8) = tdata_o[4], X3(z^8) = tdata_i[4]
// Y4(z^8) = tdata_o[3], X4(z^8) = tdata_i[3]
// Y5(z^8) = tdata_o[2], X5(z^8) = tdata_i[2]
// Y6(z^8) = tdata_o[1], X6(z^8) = tdata_i[1]
// Y7(z^8) = tdata_o[0], X7(z^8) = tdata_i[0]
// ---------------------------------------------------------------------------------------------

void dec2_ssr8(bool tvalid_i, cdatain_vec_t<8> tdata_i, bool &tvalid_o, cdata_vec_t<8> &tdata_o)
{

#pragma HLS INLINE off

    constexpr unsigned int num_coef = 4;
    const coef_int_t coeff_vec[8][num_coef] = {
        { -197,  -3723,  41339,  2079},
        {    0,      0,      0,     0},
        {  501,   6596, -12793, -1087},
        {    0,      0,      0,     0},
        {-1087, -12793,   6596,   501},
        {    0,      0,      0,     0},
        { 2079,  41339,  -3723,  -197},
        {    0,  65536,      0,     0}
    };
    
    constexpr unsigned int latency_phase_combiner = 2;
    constexpr unsigned int latency = num_coef + latency_phase_combiner;

    // shift registers to align valid to the module output (consider if 1 extra clock is needed for the final sum)
    static ap_shift_reg<bool, latency> vld_shftreg;

    // ---------------------------------------------
    // control the shift register of the mac engine
    // ----------------------------------------------
    static bool skip = false;
    // whether the filter memory is updated with the sample at its input, or it is maintained in the current state
    bool toshift_v = tvalid_i;
    // valid output
    bool tvalid_v = (tvalid_i && !skip);

    // align the valid signal to the output
    tvalid_o = vld_shftreg.shift(tvalid_v, latency - 1);

    // input samples
    cdata_t tdata_vi[8];
    // tdata_vi[0] = X0(z^8):  x(n), x(n-8), x(n-16), ....
    // tdata_vi[1] = X1(z^8): x(n-1), x(n-9), x(n-17), ....
    // ...
    // tdata_vi[7] = X7(z^8): x(n-7), x(n-15), x(n-23), ....
    for (int i = 0; i < 8; ++i)
    #pragma HLS UNROLL
    {
        tdata_vi[i].re = tdata_i.re[7 - i];
        tdata_vi[i].im = tdata_i.im[7 - i];
    }
    
    cacc_t acc[8];

    // ------------------------------------------------------
    // not possible to create a loop because template instantiation is not allowed with a non-constant expression 'i'!
    // unroll all loops manually!!!
    // ------------------------------------------------------

    // compute tdata_o[7] = Y0(z^8) = P0(z^8) X0(z^8) 
    //                                + (z^-8){P7(z^8) X1(z^8) + P6(z^8) X2(z^8) + P5(z^8) X3(z^8) + P4(z^8) X4(z^8) + P3(z^8) X5(z^8) + P2(z^8) X6(z^8) + P1(z^8) X7(z^8)}
    cacc_t acc7[8];
    acc7[0] = multi_mac_systolic<0, num_coef>(toshift_v, tdata_vi[0], coeff_vec[0]); // P0(z^8) X0(z^8)
    acc7[1] = multi_mac_systolic<1, num_coef>(toshift_v, tdata_vi[1], coeff_vec[7]); // P7(z^8) X1(z^8)
    acc7[2] = multi_mac_systolic<2, num_coef>(toshift_v, tdata_vi[2], coeff_vec[6]); // P6(z^8) X2(z^8)
    acc7[3] = multi_mac_systolic<3, num_coef>(toshift_v, tdata_vi[3], coeff_vec[5]); // P5(z^8) X3(z^8)
    acc7[4] = multi_mac_systolic<4, num_coef>(toshift_v, tdata_vi[4], coeff_vec[4]); // P4(z^8) X4(z^8)
    acc7[5] = multi_mac_systolic<5, num_coef>(toshift_v, tdata_vi[5], coeff_vec[3]); // P3(z^8) X5(z^8)
    acc7[6] = multi_mac_systolic<6, num_coef>(toshift_v, tdata_vi[6], coeff_vec[2]); // P2(z^8) X6(z^8)
    acc7[7] = multi_mac_systolic<7, num_coef>(toshift_v, tdata_vi[7], coeff_vec[1]); // P1(z^8) X7(z^8)

    acc[7] = phase_combiner<7,8,1,7>(acc7);
    //tdata_o.re[7] = acc[7].re;
    //tdata_o.im[7] = acc[7].im;
    tdata_o.re[1] = acc[7].re;
    tdata_o.im[1] = acc[7].im;

    // compute tdata_o[6] = Y1(z^8) = P1(z^8) X0(z^8) + P0(z^8) X1(z^8) 
    //                               + (z^-8){P7(z^8) X2(z^8) + P6(z^8) X3(z^8) + P5(z^8) X4(z^8) + P4(z^8) X5(z^8) + P3(z^8) X6(z^8) + P2(z^8) X7(z^8)}
    cacc_t acc6[8];
    acc6[0] = multi_mac_systolic<8, num_coef>(toshift_v, tdata_vi[0], coeff_vec[1]); // P1(z^8) X0(z^8)
    acc6[1] = multi_mac_systolic<9, num_coef>(toshift_v, tdata_vi[1], coeff_vec[0]); // P0(z^8) X1(z^8)
    acc6[2] = multi_mac_systolic<10, num_coef>(toshift_v, tdata_vi[2], coeff_vec[7]); // P7(z^8) X2(z^8)
    acc6[3] = multi_mac_systolic<11, num_coef>(toshift_v, tdata_vi[3], coeff_vec[6]); // P6(z^8) X3(z^8)
    acc6[4] = multi_mac_systolic<12, num_coef>(toshift_v, tdata_vi[4], coeff_vec[5]); // P5(z^8) X4(z^8)
    acc6[5] = multi_mac_systolic<13, num_coef>(toshift_v, tdata_vi[5], coeff_vec[4]); // P4(z^8) X5(z^8)
    acc6[6] = multi_mac_systolic<14, num_coef>(toshift_v, tdata_vi[6], coeff_vec[3]); // P3(z^8) X6(z^8)
    acc6[7] = multi_mac_systolic<15, num_coef>(toshift_v, tdata_vi[7], coeff_vec[2]); // P2(z^8) X7(z^8)

    acc[6] = phase_combiner<6,8,2,6>(acc6);

    //tdata_o.re[6] = acc[6].re;
    //tdata_o.im[6] = acc[6].im;
    tdata_o.re[2] = acc[6].re;
    tdata_o.im[2] = acc[6].im;


    // compute tdata_o[5] = Y2(z^8) = P2(z^8) X0(z^8) + P1(z^8) X1(z^8) + P0(z^8) X2(z^8) 
    //                                + (z^-8){P7(z^8) X3(z^8) + P6(z^8) X4(z^8) + P5(z^8) X5(z^8) + P4(z^8) X6(z^8) + P3(z^8) X7(z^8)}
    cacc_t acc5[8];
    acc5[0] = multi_mac_systolic<16, num_coef>(toshift_v, tdata_vi[0], coeff_vec[2]); // P2(z^8) X0(z^8)
    acc5[1] = multi_mac_systolic<17, num_coef>(toshift_v, tdata_vi[1], coeff_vec[1]); // P1(z^8) X1(z^8)
    acc5[2] = multi_mac_systolic<18, num_coef>(toshift_v, tdata_vi[2], coeff_vec[0]); // P0(z^8) X2(z^8)
    acc5[3] = multi_mac_systolic<19, num_coef>(toshift_v, tdata_vi[3], coeff_vec[7]); // P7(z^8) X3(z^8)
    acc5[4] = multi_mac_systolic<20, num_coef>(toshift_v, tdata_vi[4], coeff_vec[6]); // P6(z^8) X4(z^8)
    acc5[5] = multi_mac_systolic<21, num_coef>(toshift_v, tdata_vi[5], coeff_vec[5]); // P5(z^8) X5(z^8)
    acc5[6] = multi_mac_systolic<22, num_coef>(toshift_v, tdata_vi[6], coeff_vec[4]); // P4(z^8) X6(z^8)
    acc5[7] = multi_mac_systolic<23, num_coef>(toshift_v, tdata_vi[7], coeff_vec[3]); // P3(z^8) X7(z^8)

    acc[5] = phase_combiner<5,8,3,5>(acc5);
    //tdata_o.re[5] = acc[5].re;
    //tdata_o.im[5] = acc[5].im;
    tdata_o.re[3] = acc[5].re;
    tdata_o.im[3] = acc[5].im;

    // compute tdata_o[4] = Y3(z^8) = P3(z^8) X0(z^8) + P2(z^8) X1(z^8) + P1(z^8) X2(z^8) + P0(z^8) X3(z^8) 
    //                                + (z^-8){P7(z^8) X4(z^8) + P6(z^8) X5(z^8) + P5(z^8) X6(z^8) + P4(z^8) X7(z^8)}
    cacc_t acc4[8];
    acc4[0] = multi_mac_systolic<24, num_coef>(toshift_v, tdata_vi[0], coeff_vec[3]); // P3(z^8) X0(z^8)
    acc4[1] = multi_mac_systolic<25, num_coef>(toshift_v, tdata_vi[1], coeff_vec[2]); // P2(z^8) X1(z^8)
    acc4[2] = multi_mac_systolic<26, num_coef>(toshift_v, tdata_vi[2], coeff_vec[1]); // P1(z^8) X2(z^8)
    acc4[3] = multi_mac_systolic<27, num_coef>(toshift_v, tdata_vi[3], coeff_vec[0]); // P0(z^8) X3(z^8)
    acc4[4] = multi_mac_systolic<28, num_coef>(toshift_v, tdata_vi[4], coeff_vec[7]); // P7(z^8) X4(z^8)
    acc4[5] = multi_mac_systolic<29, num_coef>(toshift_v, tdata_vi[5], coeff_vec[6]); // P6(z^8) X5(z^8)
    acc4[6] = multi_mac_systolic<30, num_coef>(toshift_v, tdata_vi[6], coeff_vec[5]); // P5(z^8) X6(z^8)
    acc4[7] = multi_mac_systolic<31, num_coef>(toshift_v, tdata_vi[7], coeff_vec[4]); // P4(z^8) X7(z^8)

    acc[4] = phase_combiner<4,8,4,4>(acc4);
    tdata_o.re[4] = acc[4].re;
    tdata_o.im[4] = acc[4].im;

    // compute tdata_o[3] = Y4(z^8) = P4(z^8) X0(z^8) + P3(z^8) X1(z^8) + P2(z^8) X2(z^8) + P1(z^8) X3(z^8) + P0(z^8) X4(z^8) 
    //                                + (z^-8){P7(z^8) X5(z^8) + P6(z^8) X6(z^8) + P5(z^8) X7(z^8)}
    cacc_t acc3[8];
    acc3[0] = multi_mac_systolic<32, num_coef>(toshift_v, tdata_vi[0], coeff_vec[4]); // P4(z^8) X0(z^8)
    acc3[1] = multi_mac_systolic<33, num_coef>(toshift_v, tdata_vi[1], coeff_vec[3]); // P3(z^8) X1(z^8)
    acc3[2] = multi_mac_systolic<34, num_coef>(toshift_v, tdata_vi[2], coeff_vec[2]); // P2(z^8) X2(z^8)
    acc3[3] = multi_mac_systolic<35, num_coef>(toshift_v, tdata_vi[3], coeff_vec[1]); // P1(z^8) X3(z^8)
    acc3[4] = multi_mac_systolic<36, num_coef>(toshift_v, tdata_vi[4], coeff_vec[0]); // P0(z^8) X4(z^8)
    acc3[5] = multi_mac_systolic<37, num_coef>(toshift_v, tdata_vi[5], coeff_vec[7]); // P7(z^8) X5(z^8)
    acc3[6] = multi_mac_systolic<38, num_coef>(toshift_v, tdata_vi[6], coeff_vec[6]); // P6(z^8) X6(z^8)
    acc3[7] = multi_mac_systolic<39, num_coef>(toshift_v, tdata_vi[7], coeff_vec[5]); // P5(z^8) X7(z^8)

    acc[3] = phase_combiner<3,8,5,3>(acc3);
    //tdata_o.re[3] = acc[3].re;
    //tdata_o.im[3] = acc[3].im;
    tdata_o.re[5] = acc[3].re;
    tdata_o.im[5] = acc[3].im;


    // compute tdata_o[2] = Y5(z^8) = P5(z^8) X0(z^8) + P4(z^8) X1(z^8) + P3(z^8) X2(z^8) + P2(z^8) X3(z^8) + P1(z^8) X4(z^8) + P0(z^8) X5(z^8) 
    //                                + (z^-8){P7(z^8) X6(z^8) + P6(z^8) X7(z^8)}
    cacc_t acc2[8];
    acc2[0] = multi_mac_systolic<40, num_coef>(toshift_v, tdata_vi[0], coeff_vec[5]); // P5(z^8) X0(z^8)
    acc2[1] = multi_mac_systolic<41, num_coef>(toshift_v, tdata_vi[1], coeff_vec[4]); // P4(z^8) X1(z^8)
    acc2[2] = multi_mac_systolic<42, num_coef>(toshift_v, tdata_vi[2], coeff_vec[3]); // P3(z^8) X2(z^8)
    acc2[3] = multi_mac_systolic<43, num_coef>(toshift_v, tdata_vi[3], coeff_vec[2]); // P2(z^8) X3(z^8)
    acc2[4] = multi_mac_systolic<44, num_coef>(toshift_v, tdata_vi[4], coeff_vec[1]); // P1(z^8) X4(z^8)
    acc2[5] = multi_mac_systolic<45, num_coef>(toshift_v, tdata_vi[5], coeff_vec[0]); // P0(z^8) X5(z^8)
    acc2[6] = multi_mac_systolic<46, num_coef>(toshift_v, tdata_vi[6], coeff_vec[7]); // P7(z^8) X6(z^8)
    acc2[7] = multi_mac_systolic<47, num_coef>(toshift_v, tdata_vi[7], coeff_vec[6]); // P6(z^8) X7(z^8)

    acc[2] = phase_combiner<2,8,6,2>(acc2);
    //tdata_o.re[2] = acc[2].re;
    //tdata_o.im[2] = acc[2].im;
    tdata_o.re[6] = acc[2].re;
    tdata_o.im[6] = acc[2].im;

    // compute tdata_o[1] = Y6(z^8) = P6(z^8) X0(z^8) + P5(z^8) X1(z^8) + P4(z^8) X2(z^8) + P3(z^8) X3(z^8) + P2(z^8) X4(z^8) + P1(z^8) X5(z^8) + P0(z^8) X6(z^8) 
    //                               + (z^-8)P7(z^8) X7(z^8)
    cacc_t acc1[8];
    acc1[0] = multi_mac_systolic<48, num_coef>(toshift_v, tdata_vi[0], coeff_vec[6]); // P6(z^8) X0(z^8)
    acc1[1] = multi_mac_systolic<49, num_coef>(toshift_v, tdata_vi[1], coeff_vec[5]); // P5(z^8) X1(z^8)
    acc1[2] = multi_mac_systolic<50, num_coef>(toshift_v, tdata_vi[2], coeff_vec[4]); // P4(z^8) X2(z^8)
    acc1[3] = multi_mac_systolic<51, num_coef>(toshift_v, tdata_vi[3], coeff_vec[3]); // P3(z^8) X3(z^8)
    acc1[4] = multi_mac_systolic<52, num_coef>(toshift_v, tdata_vi[4], coeff_vec[2]); // P2(z^8) X4(z^8)
    acc1[5] = multi_mac_systolic<53, num_coef>(toshift_v, tdata_vi[5], coeff_vec[1]); // P1(z^8) X5(z^8)
    acc1[6] = multi_mac_systolic<54, num_coef>(toshift_v, tdata_vi[6], coeff_vec[0]); // P0(z^8) X6(z^8)
    acc1[7] = multi_mac_systolic<55, num_coef>(toshift_v, tdata_vi[7], coeff_vec[7]); // P7(z^8) X7(z^8)

    acc[1] = phase_combiner<1,8,7,1>(acc1);
    //tdata_o.re[1] = acc[1].re;
    //tdata_o.im[1] = acc[1].im;
    tdata_o.re[7] = acc[1].re;
    tdata_o.im[7] = acc[1].im;

    // compute tdata_o[0] = Y7(z^8) = P7(z^8) X0(z^8) + P6(z^8) X1(z^8) + P5(z^8) X2(z^8) + P4(z^8) X3(z^8) + P3(z^8) X4(z^8) + P2(z^8) X5(z^8) + P1(z^8) X6(z^8) + P0(z^8) X7(z^8)
    cacc_t acc0[8];
    acc0[0] = multi_mac_systolic<56, num_coef>(toshift_v, tdata_vi[0], coeff_vec[7]); // P7(z^8) X0(z^8)
    acc0[1] = multi_mac_systolic<57, num_coef>(toshift_v, tdata_vi[1], coeff_vec[6]); // P6(z^8) X1(z^8)
    acc0[2] = multi_mac_systolic<58, num_coef>(toshift_v, tdata_vi[2], coeff_vec[5]); // P5(z^8) X2(z^8)
    acc0[3] = multi_mac_systolic<59, num_coef>(toshift_v, tdata_vi[3], coeff_vec[4]); // P4(z^8) X3(z^8)
    acc0[4] = multi_mac_systolic<60, num_coef>(toshift_v, tdata_vi[4], coeff_vec[3]); // P3(z^8) X4(z^8)
    acc0[5] = multi_mac_systolic<61, num_coef>(toshift_v, tdata_vi[5], coeff_vec[2]); // P2(z^8) X5(z^8)
    acc0[6] = multi_mac_systolic<62, num_coef>(toshift_v, tdata_vi[6], coeff_vec[1]); // P1(z^8) X6(z^8)
    acc0[7] = multi_mac_systolic<63, num_coef>(toshift_v, tdata_vi[7], coeff_vec[0]); // P0(z^8) X7(z^8)

    acc[0] = phase_combiner<0,8,8,0>(acc0);
    tdata_o.re[0] = acc[0].re;
    tdata_o.im[0] = acc[0].im;

}

// ---------------------------------------------------------------------------------------------
// dec2_ssr4: 640 -> 320 ( overal decimation factor = 4, SSR = 4)
// 4 inputs per clock cycle are processed in parallel using polyphase decomposition
// Y(z) = Y0(z^4) + z^-1 Y1(z^4) + z^-2 Y2(z^4) + z^-3Y3(z^4), where:
// * Y0(z^4) = P0(z^4) X0(z^4) +                                                    (z^-4){P3(z^4) X1(z^4) + P2(z^4) X2(z^4) + P1(z^4) X3(z^4)}
// * Y1(z^4) = P1(z^4) X0(z^4) + P0(z^4) X1(z^4) +                                  (z^-4){P3(z^4) X2(z^4) + P2(z^4) X3(z^4)}
// * Y2(z^4) = P2(z^4) X0(z^4) + P1(z^4) X1(z^4) + P0(z^4) X2(z^4) +                (z^-4)P3(z^4) X3(z^4)
// * Y3(z^4) = P3(z^4) X0(z^4) + P2(z^4) X1(z^4) + P1(z^4) X2(z^4) + P0(z^4) X3(z^4)
// only 2 output are computed per clock cycle - the other are not computed because they are discarded,
// since the decimation factor is 2
//
// Y0(z^4) = tdata_o[3], X0(z^4) = tdata_i[3]
// Y1(z^4) = tdata_o[2], X1(z^4) = tdata_i[2]
// Y2(z^4) = tdata_o[1], X2(z^4) = tdata_i[1]
// Y3(z^4) = tdata_o[0], X3(z^4) = tdata_i[0]
// ---------------------------------------------------------------------------------------------
void dec2_ssr4(bool tvalid_i, cdata_vec_t<4> tdata_i, bool &tvalid_o, cdata_vec_t<4> &tdata_o)
{

#pragma HLS INLINE off

    constexpr unsigned int num_coef = 8;
    // CHECK: polyphase decomposition coefficients
    // polyphase decomposition coefficients
    const coef_int_t coeff_vec0[num_coef] = {-197, -1087, -3723, -12793,  41339,  6596, 2079,   501};
    const coef_int_t coeff_vec1[num_coef] = {   0,     0,     0,      0,      0,     0,    0,     0};
    const coef_int_t coeff_vec2[num_coef] = { 501,  2079,  6596,  41339, -12793, -3723, -1087, -197};
    const coef_int_t coeff_vec3[num_coef] = {   0,     0,     0,  65536,      0,     0,     0,    0};

    // shift registers to align valid to the module output (consider if 1 extra clock is needed for the final sum)
    static ap_shift_reg<bool, (num_coef + 1)> vld_shftreg;

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
    cdata_t tdata_vi[4];
    // X0(z^4):  x(n), x(n-4), x(n-8), ....
    tdata_vi[0].re = tdata_i.re[3];
    tdata_vi[0].im = tdata_i.im[3];
    // X1(z^4): x(n-1), x(n-5), x(n-9), ....
    tdata_vi[1].re = tdata_i.re[2];
    tdata_vi[1].im = tdata_i.im[2];
    // X2(z^4): x(n-2), x(n-6), x(n-10), ....
    tdata_vi[2].re = tdata_i.re[1];
    tdata_vi[2].im = tdata_i.im[1];
    // X3(z^4): x(n-3), x(n-7), x(n-11), ....
    tdata_vi[3].re = tdata_i.re[0];
    tdata_vi[3].im = tdata_i.im[0];
};

// ---------------------------------------------------------------------------------------------
// dec2_ssr2: 320 -> 160 ( overal decimation factor = 8, SSR = 2)
// 2 inputs per clock cycle are processed in parallel using polyphase decomposition
// Y(z) = Y0(z^2) + z^-1 Y1(z^2), where:
// * Y0(z^2) = P0(z^2) X0(z^2) + (z^-2)P1(z^2) X1(z^2)
// * Y1(z^2) = P1(z^2) X0(z^2) +       P0(z^2) X1(z^2)
// only 1 output is computed per clock cycle - the other is not computed because it needs to be discarded,
// since the decimation factor is 2
//
// Y0(z^2) = tdata_o[1], X0(z^2) = tdata_i[1]
// Y1(z^2) = tdata_o[0], X1(z^2) = tdata_i[0]
// ---------------------------------------------------------------------------------------------
void dec2_ssr2(bool tvalid_i, cdata_vec_t<2> tdata_i, bool &tvalid_o, cdata_vec_t<2> &tdata_o)
{

#pragma HLS INLINE off

    constexpr unsigned int num_coef = 16;
    // polyphase decomposition coefficients
    const coef_int_t coeff_vec0[num_coef] = {-197, 501, -1087, 2079, -3723, 6596, -12793, 41339, 41339, -12793, 6596, -3723, 2079, -1087, 501, -197};
    const coef_int_t coeff_vec1[num_coef] = {   0,   0,     0,    0,     0,    0,      0, 65536,     0,      0,    0,     0,    0,     0,   0,    0};

    // shift registers to align valid to the module output (consider if 1 extra clock is needed for the final sum)
    static ap_shift_reg<bool, (num_coef + 1)> vld_shftreg;

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

    // compute the output Y1(z^2) = P1(z^2) X0(z^2) + P0(z^2) X1(z^2)
    cacc_t acc0 = multi_mac_systolic<8, num_coef>(toshift_v, tdata_vi[0], coeff_vec1); // P1(z^2) X0(z^2)
    cacc_t acc1 = multi_mac_systolic<9, num_coef>(toshift_v, tdata_vi[1], coeff_vec0); // P0(z^2) X1(z^2)

    // output sample - sum and cast to the output data type
    cacc_t acc = {acc0.re + acc1.re, acc0.im + acc1.im};

    tdata_o.re[0] = acc.re;
    tdata_o.im[0] = acc.im;

#ifdef _DEBUG_
    // DEBUG:  compute the ouput  Y0(z^2) = P0(z^2) X0(z^2) + (z^-2)P1(z^2) X1(z^2)
    cacc_t acc2 = multi_mac_systolic<10, num_coef>(toshift_v, tdata_vi[0], coeff_vec0); // P0(z^2) X0(z^2)
    cacc_t acc3 = multi_mac_systolic<11, num_coef>(toshift_v, tdata_vi[1], coeff_vec1); // P1(z^2) X1(z^2)

    // phase combiner adds one clock cycle of latency - delay tdata_o[0] by one clock cycle
    cacc_t acc_dbg = phase_combiner_2<2>(acc2, acc3);

    tdata_o.re[1] = acc_dbg.re;
    tdata_o.im[1] = acc_dbg.im;
#else

    tdata_o.re[1] = 0;
    tdata_o.im[1] = 0;

#endif
}

template <int instance_id>
void dec2_ssr1(bool tvalid_i, cdata_vec_t<1> tdata_i, bool &tvalid_o, cdata_vec_t<1> &tdata_o)
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
        skip = true;
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
