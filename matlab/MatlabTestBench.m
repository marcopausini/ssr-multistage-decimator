classdef MatlabTestBench
    properties
        % Configurable public properties
        DecimFactor % Decimation factor
        SignalType % Signal type
        % Signals
        Filter
        Input % Generated input test vector
        Output % Expected reference output for validation
        OutputC % Output from the C simulation
        TvalidC % Tvalid from the C simulation
        OutputRTL % Output from the RTL simulation
    end

    properties (Access = private)
        TestCaseDir % Test case directory
        % Internal properties for computed values
        SignalDuration % Duration of the signal in seconds
        SignalFrequency % Frequency of the input test signal (exponential tone)
        SignalBandwidth % Bandwidth of the input test signal (chirp)
        OutputSampleRate % Desired output sample rate
        NumInputSamples % Number of input samples
        % Constant properties
        HardwareClockRate = 160; % Hardware clock rate
        InputSampleRate = 1280; % Input sample rate
        Parallelism = 8; % Parallelism Factor or Hardware Oversampling Rate
        NumOutputSamples = 64; % Number of output samples - must be a multiple of Parallelism
        Amplitude = 0.7; % Amplitude of the input test signal
        ap_data = struct('w', 16, 'f', 15, 'type', 'signed'); % Data type for input and output signals
        ap_coef = struct('w', 18, 'f', 17, 'type', 'signed'); % Data type for coefficients
        seed = 123; % seed for random number generator
    end

    methods
        %% Constructor
        function obj = MatlabTestBench(DecimationFactor, SignalType)
            obj.DecimFactor = DecimationFactor;
            obj.SignalType = SignalType;
            obj = setTestCase(obj);
            obj = loadDecimationFilter(obj);
        end

        %% generate Test Vectors
        function obj = generateTestVectors(obj)
            % generate input signals
            obj = generateInput(obj);
            % generate reference output signals
            obj = generateReferenceOutput(obj);
            % create input/output test vector file  for the C/RTL simulation
            obj = generateSimTestVectors(obj);
            % plot and save spectrogram of the input and output signals
           % obj = plotSpectrogram(obj);
           obj = plotPowerSpectrum(obj);
        end

        %% Validate C Simulation
        function obj = validateCsim(obj)
            % Implementation of validateCsim method
            % This function would compare the object's Output property to
            % OutputC to validate that the C simulation is working as
            % expected.

            % Pseudocode:
            % if all(abs(obj.Output - obj.OutputC) < some_tolerance)
            %     fprintf('C Simulation Validation Passed\n');
            % else
            %     fprintf('C Simulation Validation Failed\n');
            % end

            % An actual implementation would define 'some_tolerance' and
            % use appropriate numeric comparisons.
        end
    end

    %% Private methods
    methods (Access = private)

        %% Compute properties
        function obj = setTestCase(obj)
            % arbitrary shift to comnpute the signal frequency from bandwidth
            shift = 3/5;
            % compute paratemers based on decimation factor
            d = obj.DecimFactor;
            switch d
                case 1
                    obj.OutputSampleRate = obj.InputSampleRate;
                    obj.SignalBandwidth = [-500, 500];
                case 2
                    obj.OutputSampleRate = obj.InputSampleRate / 2;
                    obj.SignalBandwidth = [-250, 250];
                case 4
                    obj.OutputSampleRate = obj.InputSampleRate / 4;
                    obj.SignalBandwidth = [-125, 125];
                case 8
                    obj.OutputSampleRate = obj.InputSampleRate / 8;
                    obj.SignalBandwidth = [-62.5, 62.5];
                case 16
                    obj.OutputSampleRate = obj.InputSampleRate / 16;
                    obj.SignalBandwidth = [-31.25, 31.25];
                case 32
                    obj.OutputSampleRate = obj.InputSampleRate / 32;
                    obj.SignalBandwidth = [-15.625, 15.625];
                case 64
                    obj.OutputSampleRate = obj.InputSampleRate / 64;
                    obj.SignalBandwidth = [-7.8125, 7.8125];
            end
            obj.SignalFrequency = obj.SignalBandwidth(2) * shift;
            obj.NumInputSamples = obj.NumOutputSamples * obj.DecimFactor;
            obj.SignalDuration = obj.NumInputSamples / obj.InputSampleRate;
            % test case directory
            obj.TestCaseDir = ['../data/testcase_', sprintf('decim_%d', obj.DecimFactor), sprintf('_signal_%s', obj.SignalType)];
            % create directory if it does not exist
            if ~exist(obj.TestCaseDir, 'dir')
                mkdir(obj.TestCaseDir);
            end
            % write decimation factor file
            fid = fopen(fullfile(obj.TestCaseDir, 'parameters.csv'), 'w');
            fprintf(fid, '#decim_factor\n%d\n', obj.DecimFactor);
            fclose(fid);
        end

        %% Load decimation filter based on decimation factor
        function obj = loadDecimationFilter(obj)
            % Implementation of loadDecimationFilter method
            d = obj.DecimFactor;
            switch d
                case 1
                    obj.Filter = dsp.FIRFilter('Numerator', 1);
                case 2
                    data = load('filter_dec2.mat');
                    obj.Filter = data.dec2;
                case 4
                    data = load('filter_dec4.mat');
                    obj.Filter = data.dec4;
                case 8
                    data = load('filter_dec8.mat');
                    obj.Filter = data.dec8;
                case 16
                    data = load('filter_dec16.mat');    
                    obj.Filter = data.dec16;
                case 32
                    data = load('filter_dec32.mat');
                    obj.Filter = data.dec32;
                case 64
                    data = load('filter_dec64.mat');
                    obj.Filter = data.dec64;
            end
        end

        %% Generate input test signal
        function obj = generateInput(obj)
            % Implementation of generateInput method
            t = linspace(0, obj.SignalDuration, obj.SignalDuration * obj.InputSampleRate);
            % we want column vectors
            t = transpose(t);
            s = obj.SignalType;
            switch s
                case 'chirp'
                    phi = 0;
                    realPart = obj.Amplitude * chirp(t, obj.SignalBandwidth(1), obj.SignalDuration, obj.SignalBandwidth(2), 'linear', phi);
                    imagPart = obj.Amplitude * chirp(t, obj.SignalBandwidth(1), obj.SignalDuration, obj.SignalBandwidth(2), 'linear', phi + pi / 2);
                    input = realPart + 1i * imagPart;
                case 'exponential'
                    input = obj.Amplitude * exp(1i * 2 * pi * obj.SignalFrequency * t);
                case 'random'
                    if isprop(obj, 'seed') % Check if seed property exists
                        rng(obj.seed); % Set random generator seed
                    end
                    obj.NumInputSamples = length(t); % Ensure NumInputSamples property matches time vector length
                    input = obj.Amplitude * randn(1, obj.NumInputSamples);
                otherwise
                    error('Signal type "%s" not supported. Valid types: chirp, exponential, random', obj.SignalType);
            end
            % convert to fixed point
            input = convertToFxpnt(input, 's', obj.ap_data.w, obj.ap_data.f);
            obj.Input = input;
            % save input signal as a .mat file
            save(fullfile(obj.TestCaseDir, 'input.mat'), 'input');
        end

        %% Generate reference output signal
        function obj = generateReferenceOutput(obj)
            % Implementation of generateReferenceOutput method
            output = obj.Filter(obj.Input);
            % convert to fixed point
            output = convertToFxpnt(output, 's', obj.ap_data.w, obj.ap_data.f);
            obj.Output = output;
            % save output signal as a .mat file
            save(fullfile(obj.TestCaseDir, 'output.mat'), 'output');
        end
        
        %% Generate input/output test vector file  for the C/RTL simulation
        function obj = generateSimTestVectors(obj)
            % Implementation of generateSimTestVectors method
            % parallelize input and output signals = each row contains obj.Parallelism samples 
            x = reshape(obj.Input, obj.Parallelism, []);
            x = transpose(x);
            % convert to integer
            x = x * 2^obj.ap_data.f;
            % split and interleave real and imaginary parts
            input = zeros(size(x,1), 2*size(x,2));
            input(:,1:2:end) = real(x);
            input(:,2:2:end) = imag(x);
            % save input test vector file for the C simulation
            dlmwrite([obj.TestCaseDir,'/input_test_vector.txt'], input, 'delimiter', '\t');
            % parallelize output signal = each row contains obj.Parallelel/obj.DecimationFactor samples
            y = reshape(obj.Output, ceil(obj.Parallelism/obj.DecimFactor), []);
            y = transpose(y);
            % convert to integer
            y = y * 2^obj.ap_data.f;
            % split and interleave real and imaginary parts
            yy = zeros(size(y,1), 2*size(y,2));
            yy(:,1:2:end) = real(y);
            yy(:,2:2:end) = imag(y);
            % pad with zeros to get 2*obj.Parallelism columns
            output = zeros(size(yy,1), 2*obj.Parallelism);
            output(:,1:size(yy,2)) = yy;
            % save output test vector file for the C simulation
            dlmwrite([obj.TestCaseDir,'/output_test_vector.txt'], output, 'delimiter', '\t');
        end
        
        %% plot and save power spectrum of the input and output signals
        function obj = plotPowerSpectrum(obj)
            figure;
            opts = "'power'";
            data1 = obj.Input;
            fs1 = obj.InputSampleRate;
            data2 = obj.Output;
            fs2 = obj.OutputSampleRate;
            dbmin = -100;
            dbmax = 0;
            subplot(2, 1, 1);
            eval(sprintf('pspectrum(data1,fs1,%s);', opts));
            caxis([dbmin dbmax]);
            grid on;
            subplot(2, 1, 2);
            eval(sprintf('pspectrum(data2,fs2,%s);', opts));
            caxis([dbmin dbmax]);
            grid on;

            saveas(gcf, fullfile(obj.TestCaseDir, 'input_output_power_spectrum.png'));
        end

        %% plot and save spectrogram of the input and output signals
        function obj = plotSpectrogram(obj)
            % Implementation of plotSpectrogram method
            
            figure;
            opts = "'spectrogram','OverlapPercent',99";
            data1 = obj.Input;
            fs1 = obj.InputSampleRate;
            data2 = obj.Output;
            fs2 = obj.OutputSampleRate;
            dbmin = -100;
            dbmax = 0;
            subplot(2, 1, 1);
            eval(sprintf('pspectrum(data1,fs1,%s);', opts));
            caxis([dbmin dbmax]);
            xlabel('Time (s)');
            ylabel('Frequency (Hz)');
            grid on;
            subplot(2, 1, 2);
            eval(sprintf('pspectrum(data2,fs2,%s);', opts));
            caxis([dbmin dbmax]);
            xlabel('Time (s)');
            ylabel('Frequency (Hz)');
            grid on;

            
            saveas(gcf, fullfile(obj.TestCaseDir, 'input_output_spectrogram.png'));
        end
    end
end
