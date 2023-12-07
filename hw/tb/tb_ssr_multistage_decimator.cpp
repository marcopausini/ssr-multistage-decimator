/**
 * @file tb_ssr_multistage_decimator.cpp
 * @brief Testbench for the ssr multistage decimator.
 *
 * Detailed explanation about what this file contains or does.
 *
 * @author marco.pausini@gmail.com
 * @date 2023-11-xy
 * @version 0.1
 *
 */

#include <fstream>
#include <iostream>
#include <cstdio>
#include <vector>
#include <string>

#include "../src/ssr_multistage_decimator.h"

// Function prototype.
void readParameterFile(const std::string &parameterFilePath, dec_factor_t &dec_factor);

int main(void)
{

    printf("\nA Testbench for the ssr multistage decimator\n\n");

    // Path to the parameters file
    std::string parameterFilePath = "work/parameters.csv";

    // Decimation factor
    dec_factor_t dec_factor;

    // Read the parameters file
    readParameterFile(parameterFilePath, dec_factor);

    // Open the input test vector file
    std::ifstream inputFile("work/input_test_vector.txt");

    // Open the file to write the simulation results
    std::ofstream outputFile("work/output_csim.txt");

    // flags to control the loop
    bool endOfFileReached = false;

    // Read the input test vector file line by line
    std::string line;

    // Declare the input and output streams
    hls::stream<cdata_vec_t> inputStream;
    hls::stream<cdata_vec_t> outputStream;

    // Declare the input samples
    cdata_vec_t inputSamples;

    // loop over the input test vector file
    while (!endOfFileReached)
    {
        
        // Read a line from the input test vector file
        std::getline(inputFile, line);
        // Check if the end of the file has been reached
        if (inputFile.eof())
        {
            endOfFileReached = true;
            continue;
        }

        // Parse the line
        sscanf(line.c_str(), "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
               &inputSamples[0].re, &inputSamples[0].im,
               &inputSamples[1].re, &inputSamples[1].im,
               &inputSamples[2].re, &inputSamples[2].im,
               &inputSamples[3].re, &inputSamples[3].im,
               &inputSamples[4].re, &inputSamples[4].im,
               &inputSamples[5].re, &inputSamples[5].im,
               &inputSamples[6].re, &inputSamples[6].im,
               &inputSamples[7].re, &inputSamples[7].im);

        // Call the DUT
        inputStream.write(inputSamples);
        ssr_multistage_decimator(inputStream, outputStream, dec_factor);
        // Read the output test vector
        cdata_vec_t outputSample = outputStream.read();

        // Write the output sample to the output file
        outputFile << std::setw(6) << outputSample[0].re << " " << std::setw(6) << outputSample[0].im << " "
                   << std::setw(6) << outputSample[1].re << " " << std::setw(6) << outputSample[1].im << " "
                   << std::setw(6) << outputSample[2].re << " " << std::setw(6) << outputSample[2].im << " "
                   << std::setw(6) << outputSample[3].re << " " << std::setw(6) << outputSample[3].im << " "
                   << std::setw(6) << outputSample[4].re << " " << std::setw(6) << outputSample[4].im << " "
                   << std::setw(6) << outputSample[5].re << " " << std::setw(6) << outputSample[5].im << " "
                   << std::setw(6) << outputSample[6].re << " " << std::setw(6) << outputSample[6].im << " "
                   << std::setw(6) << outputSample[7].re << " " << std::setw(6) << outputSample[7].im << std::endl;
    }
}

void readParameterFile(const std::string &parameterFilePath, dec_factor_t &dec_factor)
{
    // Open the file
    std::ifstream parameterFile(parameterFilePath);

    // Check if the file is open
    if (!parameterFile.is_open())
    {
        std::cerr << "Error: could not open the file " << parameterFilePath << std::endl;
        exit(EXIT_FAILURE);
    }

    // Read the file line by line
    std::string line;
    while (std::getline(parameterFile, line))
    {
        // Skip empty lines
        if (line.empty())
            continue;

        // Skip comments
        if (line[0] == '#')
            continue;

        // First value is the decimation factor
        dec_factor = std::stoi(line);

        // Print the read values
        std::cout << "Decimation factor: " << dec_factor << std::endl;
    }

    // Close the file
    parameterFile.close();
}