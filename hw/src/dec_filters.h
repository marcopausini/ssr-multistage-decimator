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
//
// Y(z), H(z) and X(z) are written as:
// Y(z) = Y0(z^8) + z^-1 Y1(z^8) + z^-2 Y2(z^8) + z^-3Y3(z^8) + z^-4Y4(z^8) + z^-5Y5(z^8) + z^-6Y6(z^8) + z^-7Y7(z^8)
// H(z) = P0(z^8) + z^-1 P1(z^8) + z^-2 P2(z^8) + z^-3P3(z^8) + z^-4P4(z^8) + z^-5P5(z^8) + z^-6P6(z^8) + z^-7P7(z^8)
// X(z) = X0(z^8) + z^-1 X1(z^8) + z^-2 X2(z^8) + z^-3X3(z^8) + z^-4X4(z^8) + z^-5X5(z^8) + z^-6X6(z^8) + z^-7X7(z^8)
//
// Each polyphase component is given as: (omitting the term z^8 for clarity)
// * Y0 = P0 X0 + (z^-8){P7 X1 + P6 X2 + P5 X3 + P4 X4 + P3 X5 + P2 X6 + P1 X7}
// * Y1 = P1 X0 + P0 X1 + (z^-8){P7 X2 + P6 X3 + P5 X4 + P4 X5 + P3 X6 + P2 X7}
// * Y2 = P2 X0 + P1 X1 + P0 X2 + (z^-8){P7 X3 + P6 X4 + P5 X5 + P4 X6 + P3 X7}
// * Y3 = P3 X0 + P2 X1 + P1 X2 + P0 X3 + (z^-8){P7 X4 + P6 X5 + P5 X6 + P4 X7}
// * Y4 = P4 X0 + P3 X1 + P2 X2 + P1 X3 + P0 X4 + (z^-8){P7 X5 + P6 X6 + P5 X7}
// * Y5 = P5 X0 + P4 X1 + P3 X2 + P2 X3 + P1 X4 + P0 X5 + (z^-8){P7 X6 + P6 X7}
// * Y6 = P6 X0 + P5 X1 + P4 X2 + P3 X3 + P2 X4 + P1 X5 + P0 X6 + (z^-8)P7 X7
// * Y7 = P7 X0 + P6 X1 + P5 X2 + P4 X3 + P3 X4 + P2 X5 + P1 X6 + P0 X7
//
// only 4 output are computed per clock cycle - the other are not computed because they are discarded by the decimation process
//
// Y0(z^8) = ... y(0), y(8), ... = tdata_o[0],  X0(z^8) = ... x(0), x(8), ... = tdata_i[0]
// Y1(z^8) = ... y(1), y(9), ... = tdata_o[1],  X1(z^8) = ... x(1), x(9), ... = tdata_i[1]
// Y2(z^8) = ... y(2), y(10), ... = tdata_o[2], X2(z^8) = ... x(2), x(10), ... = tdata_i[2]
// Y3(z^8) = ... y(3), y(11), ... = tdata_o[3], X3(z^8) = ... x(3), x(11), ... = tdata_i[3]
// Y4(z^8) = ... y(4), y(12), ... = tdata_o[4], X4(z^8) = ... x(4), x(12), ... = tdata_i[4]
// Y5(z^8) = ... y(5), y(13), ... = tdata_o[5], X5(z^8) = ... x(5), x(13), ... = tdata_i[5]
// Y6(z^8) = ... y(6), y(14), ... = tdata_o[6], X6(z^8) = ... x(6), x(14), ... = tdata_i[6]
// Y7(z^8) = ... y(7), y(15), ... = tdata_o[7], X7(z^8) = ... x(7), x(15), ... = tdata_i[7]
// ---------------------------------------------------------------------------------------------

void dec2_ssr8(bool tvalid_i, cdatain_vec_t<8> tdata_i, bool &tvalid_o, cdata_vec_t<8> &tdata_o)
{

#pragma HLS INLINE off

    constexpr unsigned int num_coef = 4;
    const coef_int_t coeff_vec[8][num_coef] = {
        {-197, -3723, 41339, 2079},
        {0, 0, 0, 0},
        {501, 6596, -12793, -1087},
        {0, 0, 0, 0},
        {-1087, -12793, 6596, 501},
        {0, 0, 0, 0},
        {2079, 41339, -3723, -197},
        {0, 65536, 0, 0}};

    constexpr unsigned int latency_phase_combiner = 1;
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

    // input samples - cast data types
    cdata_t tdata_vi[8];
    for (int i = 0; i < 8; ++i)
#pragma HLS UNROLL
    {
        tdata_vi[i].re = tdata_i.re[i];
        tdata_vi[i].im = tdata_i.im[i];
    }

    cacc_t acc[8];

    // ------------------------------------------------------
    // ------ !!! ----------------- !!! --------------------
    // not possible to create a loop because template instantiation
    // is not allowed with a non-constant expression 'i'!
    // unroll all loops manually!!!
    // ------ !!! ----------------- !!! --------------------
    // ------------------------------------------------------

    // compute tdata_o[0] = Y0(z^8) = P0 X0 + (z^-8){P7 X1 + P6 X2 + P5 X3 + P4 X4 + P3 X5 + P2 X6 + P1 X7}
    cacc_t acc0[8];
    acc0[0] = multi_mac_systolic<0, num_coef>(toshift_v, tdata_vi[0], coeff_vec[0]); // P0(z^8) X0(z^8)
    acc0[1] = multi_mac_systolic<1, num_coef>(toshift_v, tdata_vi[1], coeff_vec[7]); // P7(z^8) X1(z^8)
    acc0[2] = multi_mac_systolic<2, num_coef>(toshift_v, tdata_vi[2], coeff_vec[6]); // P6(z^8) X2(z^8)
    acc0[3] = multi_mac_systolic<3, num_coef>(toshift_v, tdata_vi[3], coeff_vec[5]); // P5(z^8) X3(z^8)
    acc0[4] = multi_mac_systolic<4, num_coef>(toshift_v, tdata_vi[4], coeff_vec[4]); // P4(z^8) X4(z^8)
    acc0[5] = multi_mac_systolic<5, num_coef>(toshift_v, tdata_vi[5], coeff_vec[3]); // P3(z^8) X5(z^8)
    acc0[6] = multi_mac_systolic<6, num_coef>(toshift_v, tdata_vi[6], coeff_vec[2]); // P2(z^8) X6(z^8)
    acc0[7] = multi_mac_systolic<7, num_coef>(toshift_v, tdata_vi[7], coeff_vec[1]); // P1(z^8) X7(z^8)

    acc[0] = phase_combiner<0, 8, 1, 7>(acc0);

    // compute tdata_o[2] = Y2(z^8) = P2 X0 + P1 X1 + P0 X2 + (z^-8){P7 X3 + P6 X4 + P5 X5 + P4 X6 + P3 X7}
    cacc_t acc2[8];
    acc2[0] = multi_mac_systolic<16, num_coef>(toshift_v, tdata_vi[0], coeff_vec[2]); // P2(z^8) X0(z^8)
    acc2[1] = multi_mac_systolic<17, num_coef>(toshift_v, tdata_vi[1], coeff_vec[1]); // P1(z^8) X1(z^8)
    acc2[2] = multi_mac_systolic<18, num_coef>(toshift_v, tdata_vi[2], coeff_vec[0]); // P0(z^8) X2(z^8)
    acc2[3] = multi_mac_systolic<19, num_coef>(toshift_v, tdata_vi[3], coeff_vec[7]); // P7(z^8) X3(z^8)
    acc2[4] = multi_mac_systolic<20, num_coef>(toshift_v, tdata_vi[4], coeff_vec[6]); // P6(z^8) X4(z^8)
    acc2[5] = multi_mac_systolic<21, num_coef>(toshift_v, tdata_vi[5], coeff_vec[5]); // P5(z^8) X5(z^8)
    acc2[6] = multi_mac_systolic<22, num_coef>(toshift_v, tdata_vi[6], coeff_vec[4]); // P4(z^8) X6(z^8)
    acc2[7] = multi_mac_systolic<23, num_coef>(toshift_v, tdata_vi[7], coeff_vec[3]); // P3(z^8) X7(z^8)

    acc[2] = phase_combiner<5, 8, 3, 5>(acc2);

    // compute tdata_o[4] = Y4(z^8) = P4 X0 + P3 X1 + P2 X2 + P1 X3 + P0 X4 + (z^-8){P7 X5 + P6 X6 + P5 X7}
    cacc_t acc4[8];
    acc4[0] = multi_mac_systolic<32, num_coef>(toshift_v, tdata_vi[0], coeff_vec[4]); // P4(z^8) X0(z^8)
    acc4[1] = multi_mac_systolic<33, num_coef>(toshift_v, tdata_vi[1], coeff_vec[3]); // P3(z^8) X1(z^8)
    acc4[2] = multi_mac_systolic<34, num_coef>(toshift_v, tdata_vi[2], coeff_vec[2]); // P2(z^8) X2(z^8)
    acc4[3] = multi_mac_systolic<35, num_coef>(toshift_v, tdata_vi[3], coeff_vec[1]); // P1(z^8) X3(z^8)
    acc4[4] = multi_mac_systolic<36, num_coef>(toshift_v, tdata_vi[4], coeff_vec[0]); // P0(z^8) X4(z^8)
    acc4[5] = multi_mac_systolic<37, num_coef>(toshift_v, tdata_vi[5], coeff_vec[7]); // P7(z^8) X5(z^8)
    acc4[6] = multi_mac_systolic<38, num_coef>(toshift_v, tdata_vi[6], coeff_vec[6]); // P6(z^8) X6(z^8)
    acc4[7] = multi_mac_systolic<39, num_coef>(toshift_v, tdata_vi[7], coeff_vec[5]); // P5(z^8) X7(z^8)

    acc[4] = phase_combiner<3, 8, 5, 3>(acc4);

    // compute tdata_o[6] = Y6(z^8) = P6 X0 + P5 X1 + P4 X2 + P3 X3 + P2 X4 + P1 X5 + P0 X6 + (z^-8)P7 X7
    cacc_t acc6[8];
    acc6[0] = multi_mac_systolic<48, num_coef>(toshift_v, tdata_vi[0], coeff_vec[6]); // P6(z^8) X0(z^8)
    acc6[1] = multi_mac_systolic<49, num_coef>(toshift_v, tdata_vi[1], coeff_vec[5]); // P5(z^8) X1(z^8)
    acc6[2] = multi_mac_systolic<50, num_coef>(toshift_v, tdata_vi[2], coeff_vec[4]); // P4(z^8) X2(z^8)
    acc6[3] = multi_mac_systolic<51, num_coef>(toshift_v, tdata_vi[3], coeff_vec[3]); // P3(z^8) X3(z^8)
    acc6[4] = multi_mac_systolic<52, num_coef>(toshift_v, tdata_vi[4], coeff_vec[2]); // P2(z^8) X4(z^8)
    acc6[5] = multi_mac_systolic<53, num_coef>(toshift_v, tdata_vi[5], coeff_vec[1]); // P1(z^8) X5(z^8)
    acc6[6] = multi_mac_systolic<54, num_coef>(toshift_v, tdata_vi[6], coeff_vec[0]); // P0(z^8) X6(z^8)
    acc6[7] = multi_mac_systolic<55, num_coef>(toshift_v, tdata_vi[7], coeff_vec[7]); // P7(z^8) X7(z^8)

    acc[6] = phase_combiner<1, 8, 7, 1>(acc6);

#ifdef _DEBUG_
    // compute tdata_o[1] = Y1(z^8) = P1 X0 + P0 X1  + (z^-8){P7 X2 + P6 X3 + P5 X4 + P4 X5 + P3 X6 + P2 X7}
    cacc_t acc1[8];
    acc1[0] = multi_mac_systolic<8, num_coef>(toshift_v, tdata_vi[0], coeff_vec[1]);  // P1(z^8) X0(z^8)
    acc1[1] = multi_mac_systolic<9, num_coef>(toshift_v, tdata_vi[1], coeff_vec[0]);  // P0(z^8) X1(z^8)
    acc1[2] = multi_mac_systolic<10, num_coef>(toshift_v, tdata_vi[2], coeff_vec[7]); // P7(z^8) X2(z^8)
    acc1[3] = multi_mac_systolic<11, num_coef>(toshift_v, tdata_vi[3], coeff_vec[6]); // P6(z^8) X3(z^8)
    acc1[4] = multi_mac_systolic<12, num_coef>(toshift_v, tdata_vi[4], coeff_vec[5]); // P5(z^8) X4(z^8)
    acc1[5] = multi_mac_systolic<13, num_coef>(toshift_v, tdata_vi[5], coeff_vec[4]); // P4(z^8) X5(z^8)
    acc1[6] = multi_mac_systolic<14, num_coef>(toshift_v, tdata_vi[6], coeff_vec[3]); // P3(z^8) X6(z^8)
    acc1[7] = multi_mac_systolic<15, num_coef>(toshift_v, tdata_vi[7], coeff_vec[2]); // P2(z^8) X7(z^8)

    acc[1] = phase_combiner<6, 8, 2, 6>(acc1);

    // compute tdata_o[3] = Y3(z^8) = P3 X0 + P2 X1 + P1 X2 + P0 X3 + (z^-8){P7 X4 + P6 X5 + P5 X6 + P4 X7}
    cacc_t acc3[8];
    acc3[0] = multi_mac_systolic<24, num_coef>(toshift_v, tdata_vi[0], coeff_vec[3]); // P3(z^8) X0(z^8)
    acc3[1] = multi_mac_systolic<25, num_coef>(toshift_v, tdata_vi[1], coeff_vec[2]); // P2(z^8) X1(z^8)
    acc3[2] = multi_mac_systolic<26, num_coef>(toshift_v, tdata_vi[2], coeff_vec[1]); // P1(z^8) X2(z^8)
    acc3[3] = multi_mac_systolic<27, num_coef>(toshift_v, tdata_vi[3], coeff_vec[0]); // P0(z^8) X3(z^8)
    acc3[4] = multi_mac_systolic<28, num_coef>(toshift_v, tdata_vi[4], coeff_vec[7]); // P7(z^8) X4(z^8)
    acc3[5] = multi_mac_systolic<29, num_coef>(toshift_v, tdata_vi[5], coeff_vec[6]); // P6(z^8) X5(z^8)
    acc3[6] = multi_mac_systolic<30, num_coef>(toshift_v, tdata_vi[6], coeff_vec[5]); // P5(z^8) X6(z^8)
    acc3[7] = multi_mac_systolic<31, num_coef>(toshift_v, tdata_vi[7], coeff_vec[4]); // P4(z^8) X7(z^8)

    acc[3] = phase_combiner<4, 8, 4, 4>(acc3);

    // compute tdata_o[5] = Y5(z^8) = P5 X0 + P4 X1 + P3 X2 + P2 X3 + P1 X4 + P0 X5 + (z^-8){P7 X6 + P6 X7}
    cacc_t acc5[8];
    acc5[0] = multi_mac_systolic<40, num_coef>(toshift_v, tdata_vi[0], coeff_vec[5]); // P5(z^8) X0(z^8)
    acc5[1] = multi_mac_systolic<41, num_coef>(toshift_v, tdata_vi[1], coeff_vec[4]); // P4(z^8) X1(z^8)
    acc5[2] = multi_mac_systolic<42, num_coef>(toshift_v, tdata_vi[2], coeff_vec[3]); // P3(z^8) X2(z^8)
    acc5[3] = multi_mac_systolic<43, num_coef>(toshift_v, tdata_vi[3], coeff_vec[2]); // P2(z^8) X3(z^8)
    acc5[4] = multi_mac_systolic<44, num_coef>(toshift_v, tdata_vi[4], coeff_vec[1]); // P1(z^8) X4(z^8)
    acc5[5] = multi_mac_systolic<45, num_coef>(toshift_v, tdata_vi[5], coeff_vec[0]); // P0(z^8) X5(z^8)
    acc5[6] = multi_mac_systolic<46, num_coef>(toshift_v, tdata_vi[6], coeff_vec[7]); // P7(z^8) X6(z^8)
    acc5[7] = multi_mac_systolic<47, num_coef>(toshift_v, tdata_vi[7], coeff_vec[6]); // P6(z^8) X7(z^8)

    acc[5] = phase_combiner<2, 8, 6, 2>(acc5);

    // compute tdata_o[7] = Y7(z^8) = P7 X0 + P6 X1 + P5 X2 + P4 X3 + P3 X4 + P2 X5 + P1 X6 + P0 X7
    cacc_t acc7[8];
    acc7[0] = multi_mac_systolic<56, num_coef>(toshift_v, tdata_vi[0], coeff_vec[7]); // P7(z^8) X0(z^8)
    acc7[1] = multi_mac_systolic<57, num_coef>(toshift_v, tdata_vi[1], coeff_vec[6]); // P6(z^8) X1(z^8)
    acc7[2] = multi_mac_systolic<58, num_coef>(toshift_v, tdata_vi[2], coeff_vec[5]); // P5(z^8) X2(z^8)
    acc7[3] = multi_mac_systolic<59, num_coef>(toshift_v, tdata_vi[3], coeff_vec[4]); // P4(z^8) X3(z^8)
    acc7[4] = multi_mac_systolic<60, num_coef>(toshift_v, tdata_vi[4], coeff_vec[3]); // P3(z^8) X4(z^8)
    acc7[5] = multi_mac_systolic<61, num_coef>(toshift_v, tdata_vi[5], coeff_vec[2]); // P2(z^8) X5(z^8)
    acc7[6] = multi_mac_systolic<62, num_coef>(toshift_v, tdata_vi[6], coeff_vec[1]); // P1(z^8) X6(z^8)
    acc7[7] = multi_mac_systolic<63, num_coef>(toshift_v, tdata_vi[7], coeff_vec[0]); // P0(z^8) X7(z^8)

    acc[7] = phase_combiner<0, 8, 8, 0>(acc7);

    // assign outputs
    for (int i = 0; i < 8; ++i)
    {
        tdata_o.re[i] = acc[i].re;
        tdata_o.im[i] = acc[i].im;
    }

#else

    for (int i = 0; i < 4; ++i)
#pragma HLS UNROLL
    {
        tdata_o.re[i] = acc[2 * i].re;
        tdata_o.im[i] = acc[2 * i].im;
        tdata_i.re[i+4] = 0;
        tdata_i.im[i+4] = 0;
    }

#endif
}

// ---------------------------------------------------------------------------------------------
// dec2_ssr4: 640 -> 320 ( overal decimation factor = 4, SSR = 4)
// 4 inputs per clock cycle are processed in parallel using polyphase decomposition
//
// Y(z) = Y0(z^4) + z^-1 Y1(z^4) + z^-2 Y2(z^4) + z^-3Y3(z^4)
// H(z) = P0(z^4) + z^-1 P1(z^4) + z^-2 P2(z^4) + z^-3P3(z^4)
// X(z) = X0(z^4) + z^-1 X1(z^4) + z^-2 X2(z^4) + z^-3X3(z^4)
//
// Each polyphase component is given as: (omitting the term z^4 for clarity)
// * Y0 = P0 X0 + (z^-4){P3 X1 + P2 X2 + P1 X3}
// * Y1 = P1 X0 + P0 X1 + (z^-4){P3 X2 + P2 X3}
// * Y2 = P2 X0 + P1 X1 + P0 X2 + (z^-4)P3 X3
// * Y3 = P3 X0 + P2 X1 + P1 X2 + P0 X3
//
// only 2 output are computed per clock cycle - the other are not computed because they are discarded by the decimation process
//
// Y0(z^4) = ... y(0), y(4) ... = tdata_o[0], X0(z^4) = ... x(0), x(4), .... = tdata_i[0]
// Y1(z^4) = ... y(1), y(5) ... = tdata_o[1], X1(z^4) = ... x(1), x(5), .... = tdata_i[1]
// Y2(z^4) = ... y(2), y(6) ... = tdata_o[2], X2(z^4) = ... x(2), x(6), .... = tdata_i[2]
// Y3(z^4) = ... y(3), y(7) ... = tdata_o[3], X3(z^4) = ... x(3), x(7), .... = tdata_i[3]
// ---------------------------------------------------------------------------------------------

void dec2_ssr4(bool tvalid_i, cdata_vec_t<4> tdata_i, bool &tvalid_o, cdata_vec_t<4> &tdata_o)
{

#pragma HLS INLINE off

    constexpr unsigned int num_coef = 8;
    // polyphase decomposition coefficients
    const coef_int_t coeff_vec0[num_coef] = {-197, -1087, -3723, -12793, 41339, 6596,  2079,   501};
    const coef_int_t coeff_vec1[num_coef] = {   0,     0,     0,     0,      0,    0,     0,     0};
    const coef_int_t coeff_vec2[num_coef] = {  501, 2079,  6596, 41339, -12793, -3723, -1087,  -197};
    const coef_int_t coeff_vec3[num_coef] = {    0,    0,     0, 65536,      0,     0,     0,     0};

    constexpr unsigned int latency_phase_combiner = 1;
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

    // input sample
    cdata_t tdata_vi[4];

    for (int i = 0; i < 4; ++i)
#pragma HLS UNROLL
    {
        tdata_vi[i].re = tdata_i.re[i];
        tdata_vi[i].im = tdata_i.im[i];
    }

    cacc_t acc[4];

    // compute tdata_o[0] = Y0(z^4) = P0 X0 + (z^-4){P3 X1 + P2 X2 + P1 X3}
    cacc_t acc0[4];
    acc0[0] = multi_mac_systolic<0, num_coef>(toshift_v, tdata_vi[0], coeff_vec0); // P0(z^4) X0(z^4)
    acc0[1] = multi_mac_systolic<1, num_coef>(toshift_v, tdata_vi[1], coeff_vec3); // P3(z^4) X1(z^4)
    acc0[2] = multi_mac_systolic<2, num_coef>(toshift_v, tdata_vi[2], coeff_vec2); // P2(z^4) X2(z^4)
    acc0[3] = multi_mac_systolic<3, num_coef>(toshift_v, tdata_vi[3], coeff_vec1); // P1(z^4) X3(z^4)

    acc[0] = phase_combiner<0, 4, 1, 3>(acc0);

    // compute tdata_o[2] = Y2(z^4) = P2 X0 + P1 X1 + P0 X2 + (z^-4){P3 X3}
    cacc_t acc2[4];
    acc2[0] = multi_mac_systolic<8, num_coef>(toshift_v, tdata_vi[0], coeff_vec2); // P2(z^4) X0(z^4)
    acc2[1] = multi_mac_systolic<9, num_coef>(toshift_v, tdata_vi[1], coeff_vec1); // P1(z^4) X1(z^4)
    acc2[2] = multi_mac_systolic<10, num_coef>(toshift_v, tdata_vi[2], coeff_vec0); // P0(z^4) X2(z^4)
    acc2[3] = multi_mac_systolic<11, num_coef>(toshift_v, tdata_vi[3], coeff_vec3); // P3(z^4) X3(z^4)

    acc[2] = phase_combiner<2, 4, 3, 1>(acc2);

    for (int i = 0; i < 2; ++i)
    {
        tdata_o.re[i] = acc[2*i].re;
        tdata_o.im[i] = acc[2*i].im;
        tdata_i.re[i+2] = 0;
        tdata_i.im[i+2] = 0;
    }

};

// ---------------------------------------------------------------------------------------------
// dec2_ssr2: 320 -> 160 ( overal decimation factor = 8, SSR = 2)
// 2 inputs per clock cycle are processed in parallel using polyphase decomposition
//
// Y(z) = Y0(z^2) + z^-1 Y1(z^2)
// H(z) = P0(z^2) + z^-1 P1(z^2)
// X(z) = X0(z^2) + z^-1 X1(z^2)
//
// Each polyphase component is given as: (omitting the term z^2 for clarity)
// * Y0 = P0 X0 + (z^-2)P1 X1
// * Y1 = P1 X0 + P0 X1
//
// only 1 output is computed per clock cycle - the other is not computed because it needs to be discarded,
// since the decimation factor is 2
//
// Y0(z^2) = ... y(0), y(2), y(4), y(6), ... = tdata_o[0], X0(z^2) = ... x(0), x(2), x(4), x(6), ... = tdata_i[0]
// Y1(z^2) = ... y(1), y(3), y(5), y(7), ... = tdata_o[1], X1(z^2) = ... x(1), x(3), x(5), x(7), ... = tdata_i[1]
// ---------------------------------------------------------------------------------------------
void dec2_ssr2(bool tvalid_i, cdata_vec_t<2> tdata_i, bool &tvalid_o, cdata_vec_t<2> &tdata_o)
{

#pragma HLS INLINE off

    constexpr unsigned int num_coef = 16;
    // polyphase decomposition coefficients
    const coef_int_t coeff_vec0[num_coef] = {-197, 501, -1087, 2079, -3723, 6596, -12793, 41339, 41339, -12793, 6596, -3723, 2079, -1087, 501, -197};
    const coef_int_t coeff_vec1[num_coef] = {0, 0, 0, 0, 0, 0, 0, 65536, 0, 0, 0, 0, 0, 0, 0, 0};

    constexpr unsigned int latency_phase_combiner = 1;
    constexpr unsigned int latency = num_coef + latency_phase_combiner;

    // shift registers to align valid to the module output (consider if 1 extra clock is needed for the final sum)
    static ap_shift_reg<bool, (latency)> vld_shftreg;

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

    // input sample
    cdata_t tdata_vi[2];
    
    for (int i = 0; i < 2; ++i)
#pragma HLS UNROLL
    {
        tdata_vi[i].re = tdata_i.re[i];
        tdata_vi[i].im = tdata_i.im[i];
    }

    cacc_t acc[2];

    // compute tdata_o[0] = Y0(z^2) = P0 X0 + (z^-2)P1 X1
    cacc_t acc0[2];
    acc0[0] = multi_mac_systolic<0, num_coef>(toshift_v, tdata_vi[0], coeff_vec0); // P0(z^2) X0(z^2)
    acc0[1] = multi_mac_systolic<1, num_coef>(toshift_v, tdata_vi[1], coeff_vec1); // P1(z^2) X1(z^2)

    acc[0] = phase_combiner<0, 2, 1, 1>(acc0);


    tdata_o.re[0] = acc[0].re;
    tdata_o.im[0] = acc[0].im;
    tdata_o.re[1] = 0;
    tdata_o.im[1] = 0;

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
