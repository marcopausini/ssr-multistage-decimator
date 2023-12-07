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
void ssr_multistage_decimator(hls::stream<cdata_vec_t> &in, hls::stream<cdata_vec_t> &out, dec_factor_t dec_factor){    

    // intermediate streams
    cdata_vec_t input;
    input = in.read();
    //
    // decimation factor = 2
    //
    hls::stream<cdata_vec_t> in_dec2;
    in_dec2.write(input);
    hls::stream<cdata_vec4_t> out_dec2;
    ssr_dec2(in_dec2, out_dec2);
    cdata_vec4_t output4 = out_dec2.read();
    cdata_vec_t output8;
    for (int i = 0; i < 4; i++)
    {
        output8[i] = output4[i];
    }
    out.write(output8);

    //hls::stream<cdata_vec2_t> out_dec4;

    // output
    
/*    switch (dec_factor)
    {
    case 1:
        
        out.write(input);
        break;
    case 2:
        // filter chain
       

        output4 = out_dec2.read();
        for (int i = 0; i < 4; i++)
        {
            output8[i] = output4[i];
        }
        out.write(output8);
        break;
    default:
        throw std::invalid_argument("Invalid decimation factor");
    }
    */
}