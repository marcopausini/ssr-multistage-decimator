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

// FIR filter parameters
void ssr_multistage_decimator(hls::stream<cdata_t<ssr>> &in, hls::stream<cdata_t<ssr>> &out, dec_factor_t dec_factor)
{
/*
 * An hls::stream<> on the top-level interface is by default implemented with an ap_fifo interface for the AMD Vivado™ IP flow
 * If an hls::stream is used inside the design function and synthesized into hardware, it is implemented as a FIFO with a default depth of 2
 * Important: Ensure hls::stream variables are correctly sized when used in the default non-DATAFLOW regions.
 * If an hls::stream is used to transfer data between tasks (sub-functions or loops), you should implement the tasks in a DATAFLOW region where data streams from one task to the next.
 * A stream can also be specified as hls::stream<Type, Depth>, where Depth indicates the depth of the FIFO needed
 * For multirate designs in which the implementation requires a FIFO with a depth greater than 2,
 * you must determine (and set using the STREAM directive) the depth necessary for the RTL simulation to complete.
 */

//  add pragams for the decimation factor
#pragma HLS INTERFACE s_axilite port = dec_factor bundle = control

    // output samples
    cdata_t<ssr> output;

    // first filter stage
    hls::stream<cdata_t<ssr / 2>> out_dec2("out_dec2_stream");
    cdata_t<ssr / 2> output_dec2;

    switch (dec_factor)
    {
    case 1:
        out.write(in.read());
        break;
    case 2:
        //
        hbf_ssr8(in, out_dec2);
        output_dec2 = out_dec2.read();
        
        for (int i = 0; i < ssr / 2; i++)
        // Unroll this loop to copy all elements in parallel
        #pragma HLS unroll
        {
            output.re[i] = output_dec2.re[i];
            output.im[i] = output_dec2.im[i];
        }
        out.write(output);
        break;
    default:
        out.write(in.read());
        break;
    }
}