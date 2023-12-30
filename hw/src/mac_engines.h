/**
 * @file mac_engines.h
 * @brief implementation of multiply-accumulate filters
 *        (see FIR Compiler PG149, v7.2, Filter Architecture and UG073)
 *
 * @details
 *  - multi_mac: conventional Direct Form Type 1 Tapped Delay Line FIR filter architecture,
 *               parallel implementation, summatios of products, one DSP per each tap
 *               the output of multipliers are summed up by a tree of adders - require extra logic outside the DSP
 *               sub-optimal for high clock frequencies -
 *               consider using the systolic architecture for better performance
 * 
 * - multi_mac_systolic: systolic implementation of the Direct Form Type 1 Tapped Delay Line FIR filter architecture,
 *
 * - mac_single_tap: a single multiplier, used in the polyphase decomposition of the Half-Band filters
 *
 * - multi_mac_hbf: efficient implementation of the Half-Band filters exploting the zero coefficients
 *
 * @note
 * Use template to create unique instances of the function for each different value of the template argument
 * (otherwise the tool will attempt to share the data shift register, creating a intra-loop dependency)
 *  - A static variable in a template function is duplicated for each different value of the template arguments.
 *  - We use the static keyword to instruct Vitis HLS tool tp map the variable to a persistent hardware resource.
 *
 * @author marco.pausini@ast-science.com
 * @date 2023-11-xy
 * @version 0.1
 *
 */

#ifndef MAC_ENGINES_H_
#define MAC_ENGINES_H_

#include "ssr_multistage_decimator.h"

template <int instance_id, int num_coef>
cacc_t multi_mac(bool toshift_i, cdata_t x_i, coef_t h[num_coef])
{

    // shift register for input data
    static cdata_t data_sreg[num_coef];
    cacc_t acc = {0, 0};
    cdata_t x;

// loop for all the taps
MULTMACLOOP:
    // all the MAC run in parallel
    for (int i = num_coef - 1; i >= 1; i--)
    {
        acc.re += data_sreg[i].re * h[i];
        acc.im += data_sreg[i].im * h[i];
        // update the delay line
        data_sreg[i] = toshift_i ? data_sreg[i - 1] : data_sreg[i];
    }
    // multiply the first tap
    acc.re += data_sreg[0].re * h[0];
    acc.im += data_sreg[0].im * h[0];
    // update the shift register only when the input is valid
    data_sreg[0] = toshift_i ? x_i : data_sreg[0];

    return acc;
}

template <int instance_id, int num_coef>
cacc_t multi_mac_systolic(bool toshift_i, cdata_t x_i, const coef_int_t coef_vec[num_coef])
{

#pragma HLS INLINE off

    // shift register for input data
    static cdata_t data_sreg[num_coef];
    
    // DSP48E1 signals
    static cdata_t x_r[num_coef];
    coef_t h;
    cacc_t mult;
    static cacc_t acc_r[num_coef];
    
    // control the tapped delay line
    static bool toshift_r[num_coef];
    
    // mux to select input to the data shift register
    cdata_t x_mux;

// loop for all the taps
MULTMACLOOP:
    // all the MAC run in parallel
    for (int i = num_coef - 1; i >= 1; i--)
    {
        h.range() = coef_vec[i];
        // multiplier with registered output
        mult.re = x_r[i].re * h;
        mult.im = x_r[i].im * h;
        // one clock delay for the accumulator
        acc_r[i].re = acc_r[i - 1].re + mult.re;
        acc_r[i].im = acc_r[i - 1].im + mult.im;

        // read data from the shift register
        x_r[i] = data_sreg[i];

        // if not shift, then write back the data read from shift register
        x_mux = toshift_r[i - 1] ? x_r[i - 1] : x_r[i];

        // shift all values up one and load x_mux into location 0
        data_sreg[i] = x_mux; // toshift_i ? x_r[i - 1] : data_sreg[i];

        toshift_r[i] = toshift_r[i - 1];
    }

    // multiply the first tap
    h.range() = coef_vec[0];
    acc_r[0].re = x_r[0].re * h;
    acc_r[0].im = x_r[0].im * h;

    x_r[0] = data_sreg[0];

    toshift_r[0] = toshift_i;

    x_mux = toshift_r[0] ? x_i : x_r[0];

    // update the shift register only when the input is valid
    data_sreg[0] = x_mux;
    
    return (acc_r[num_coef - 1]);
}

template <int instance_id>
cacc_t phase_combiner_2(cacc_t ph0, cacc_t ph1)
{
    // shift registers for input data
    static cacc_t x_r0[1];
    static cacc_t x_r1[2];

    cacc_t acc = {0, 0};

    // combiner
    acc.re = x_r0[0].re + x_r1[1].re;
    acc.im = x_r0[0].im + x_r1[1].im;

    x_r0[0] = ph0;
    x_r1[0] = ph1;
    x_r1[1] = x_r1[0];

    return acc;  

}

#endif /* MAC_ENGINES_H_ */