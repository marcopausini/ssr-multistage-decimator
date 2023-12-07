%{
/**
 * @file dec_filter_design.m
 *
 * @brief design the decimation filter
 *
 * Design, measure and plot the frequency response of the multi-stage decimation filter
 *
 * @author Marco Pausini
 * @date YYYY-MM-DD
 * @version 1.0
 */
%}

function dec_filter_design

    % quantization parameters
    ap_data.width = 16;
    ap_data.frac = 15;
    ap_coef.width = 18;
    ap_coef.frac = 17;

    % flags
    plotFlag = 0;

    % ---------------------------------
    % Anti-aliasing LPF specifications
    % ---------------------------------
    % Decimation factor
    DecimationFactor = 2;
    % Sampling rate at input
    FsIn = 1280e6;
    % Sampling rate at the output
    FsOut = FsIn / DecimationFactor;
    % Passband frequency = BW/2
    Fpass = 250e6;
    % Stopband frequency = Fpass + 2*(FsOut/2 - Fpass) = FsOut - Fpass
    Fstop = 390e6;
    % Passband ripple
    Ap = 0.01;
    % Stopband attenuation
    Ast = 60;

    % --------------------------------------
    % Half-Band Decimator Design
    % --------------------------------------
    hbParams.DecimationFactor = DecimationFactor;
    hbParams.FsIn = FsIn;
    hbParams.TransitionWidth = Fstop - Fpass;
    hbParams.StopbandAttenuation = Ast;

    % create filter specs using fdesign
    hbSpec = fdesign.decimator(hbParams.DecimationFactor, 'halfband', 'TW,Ast', hbParams.TransitionWidth, hbParams.StopbandAttenuation, hbParams.FsIn);

    % --------------------------------------
    % Multi-Stage Decimator Design
    % --------------------------------------

    % create half-band filter object
    hbFilter = design(hbSpec, 'SystemObject', true);
    % quantize filter coefficients
    hbFilter = filterQuantization(hbFilter, ap_data, ap_coef);
    % Estimate cost of implementing filter System object
    hbFilter.cost
    % Measure frequency response characteristics of filter System object
    measure(hbFilter)

    % save the filter object for later use
    save('hbFilter.mat', 'hbFilter');

    % save filter coefficients as .coe file for use in Vivado
    writeCoefficients(hbFilter, ap_coef, '../data/hbFilter.coe');
    
    % cascade filter objects to create multi-stage decimator
    dec2 = hbFilter;
    dec4 = dsp.FilterCascade(dec2, dec2);
    dec8 = dsp.FilterCascade(dec2, dec2, dec2);
    dec16 = dsp.FilterCascade(dec2, dec2, dec2, dec2);
    dec32 = dsp.FilterCascade(dec2, dec2, dec2, dec2, dec2);
    dec64 = dsp.FilterCascade(dec2, dec2, dec2, dec2, dec2, dec2);

    % save the filter objects for later use
    save('filter_dec2.mat', 'dec2');
    save('filter_dec4.mat', 'dec4');
    save('filter_dec8.mat', 'dec8');
    save('filter_dec16.mat', 'dec16');
    save('filter_dec32.mat', 'dec32');
    save('filter_dec64.mat', 'dec64');

    if plotFlag

        % Display and export the magnitude response
        decimatorPlots.dec2 = fvtool(dec2, 'Fs', [hbParams.FsIn], 'arithmetic', 'fixed');
        legend(decimatorPlots.dec2, 'decimator-by-2');
        set(get(decimatorPlots.dec2, 'CurrentAxes'), 'YLim', [-75, 5]);
        %saveas(decimatorPlots.dec2, 'decimator_frequency_response.png');
        exportgraphics(get(decimatorPlots.dec2, 'CurrentAxes'), '../doc/decimator_2_frequency_response.png', 'Resolution', 300);

        decimatorPlots.dec4 = fvtool(dec4, 'Fs', [hbParams.FsIn], 'arithmetic', 'fixed');
        legend(decimatorPlots.dec4, 'decimator-by-4');
        set(get(decimatorPlots.dec4, 'CurrentAxes'), 'YLim', [-75, 5]);
        set(get(decimatorPlots.dec4, 'CurrentAxes'), 'XScale', 'log');
        exportgraphics(get(decimatorPlots.dec4, 'CurrentAxes'), '../doc/decimator_4_frequency_response.png', 'Resolution', 300);

        decimatorPlots.dec8 = fvtool(dec8, 'Fs', [hbParams.FsIn], 'arithmetic', 'fixed');
        legend(decimatorPlots.dec8, 'decimator-by-8');
        set(get(decimatorPlots.dec8, 'CurrentAxes'), 'YLim', [-75, 5]);
        % Change the X-axis to logarithmic scale after the plot is rendered
        set(get(decimatorPlots.dec8, 'CurrentAxes'), 'XScale', 'log');
        exportgraphics(get(decimatorPlots.dec8, 'CurrentAxes'), '../doc/decimator_8_frequency_response.png', 'Resolution', 300);

        decimatorPlots.dec16 = fvtool(dec16, 'Fs', [hbParams.FsIn], 'arithmetic', 'fixed');
        legend(decimatorPlots.dec16, 'decimator-by-16');
        set(get(decimatorPlots.dec16, 'CurrentAxes'), 'YLim', [-75, 5]);
        set(get(decimatorPlots.dec16, 'CurrentAxes'), 'XScale', 'log');
        exportgraphics(get(decimatorPlots.dec16, 'CurrentAxes'), '../doc/decimator_16_frequency_response.png', 'Resolution', 300);

        decimatorPlots.dec32 = fvtool(dec32, 'Fs', [hbParams.FsIn], 'arithmetic', 'fixed');
        legend(decimatorPlots.dec32, 'decimator-by-32');
        set(get(decimatorPlots.dec32, 'CurrentAxes'), 'YLim', [-75, 5]);
        set(get(decimatorPlots.dec32, 'CurrentAxes'), 'XScale', 'log');
        exportgraphics(get(decimatorPlots.dec32, 'CurrentAxes'), '../doc/decimator_32_frequency_response.png', 'Resolution', 300);

        decimatorPlots.dec64 = fvtool(dec64, 'Fs', [hbParams.FsIn], 'arithmetic', 'fixed');
        legend(decimatorPlots.dec64, 'decimator-by-64');
        set(get(decimatorPlots.dec64, 'CurrentAxes'), 'YLim', [-75, 5]);
        set(get(decimatorPlots.dec64, 'CurrentAxes'), 'XScale', 'log');

    end

    % Determine output delay of single - rate or multirate filter (Since R2022a)
    % outputDelay(FIR18)

end

function filter = filterQuantization(filter, ap_data, ap_coef) 
    filter.FullPrecisionOverride = false;
    filter.CoefficientsDataType = 'Custom';
    filter.CustomCoefficientsDataType = numerictype([], ap_coef.width, ap_coef.frac); %18.17
    filter.ProductDataType = 'Full precision';
    filter.AccumulatorDataType = 'Full precision';
    filter.OutputDataType = 'Custom';
    filter.CustomOutputDataType = numerictype([], ap_data.width, ap_data.frac); %16.15
end

function writeCoefficients(filterObj, ap_coef, filePath)
    % This function writes the filter coefficients to a .coe file.
    % The .coe file format is typically used with Xilinx Vivado.
    
    % Extract the filter coefficients from the filter object
    coeffs = convertToFxpnt(filterObj.Numerator, 's', ap_coef.width, ap_coef.frac) * 2 ^ ap_coef.frac;
    
    % Define header lines for the .coe file
    headerLines = {
        '; Sample filter coefficient .coe file';
        'radix=10;';
        'coefdata='
    };
    
    % Open file
    fid = fopen(filePath, 'w');
    
    % Verify if the file was opened successfully
    if (fid < 0)
        error('Error opening file %s\n', filePath);
    end
    
    % Write the headers to the file
    for i = 1:length(headerLines)
        fprintf(fid, '%s\n', headerLines{i});
    end

    % Write coefficients, assuming they are in a horizontal array format
    % If your coefficients are in a different format, adjust accordingly
    for i = 1:length(coeffs)
        % Add a comma after each coefficient except the last one
        if i ~= length(coeffs)
            fprintf(fid, '%d,\n', coeffs(i));
        else
            % Do not add a comma after the last coefficient
            fprintf(fid, '%d;', coeffs(i));
        end
    end
    
    % Close the file
    fclose(fid);
end
