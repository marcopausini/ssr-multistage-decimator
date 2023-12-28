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
        % Fixed-point parameters
        coef_t
        datain_t
        mult_t
        dataout_t
        acc_t
        ap_data = struct('w', 16, 'f', 15, 'type', 'signed'); % Data type for input and output signals
        ap_coef = struct('w', 18, 'f', 17, 'type', 'signed'); % Data type for coefficients
        seed = 123; % seed for random number generator
    end

    methods
        %% Constructor
        function obj = MatlabTestBench(DecimationFactor, SignalType)
            % -------------------------
            % Fixed-point parameters
            % -------------------------
            obj.coef_t.w = 18; % word length
            obj.coef_t.f = 17; % fraction length
            obj.datain_t.w = 16; % word length
            obj.datain_t.f = 15; % fraction length
            obj.mult_t.w = 34; % word length
            obj.mult_t.f = 32; % word length
            obj.acc_t.w = 40;
            obj.acc_t.f = 32;
            obj.dataout_t.w = 16; % word length
            obj.dataout_t.f = 14; % fraction length
            % -----------------------------
            % get the test case parameters
            % -----------------------------
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
            % load test vectors if not available
            if isempty(obj.Input)
                obj = loadInput(obj);
            end
            if isempty(obj.Output)
                obj = loadReferenceOutput(obj);
            end
            % load the C simulation outputs
            obj = loadOutputCsim(obj);
            % compare the C simulation outputs with the reference outputs
            obj = compareResults(obj);

            % compare frequency and power of the C simulation outputs with the reference outputs
            obj = compareSignalFreqPower(obj);

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
            input = convertToFxpnt(input, 's', obj.datain_t.w, obj.datain_t.f);
            obj.Input = input;
            % save input signal as a .mat file
            save(fullfile(obj.TestCaseDir, 'input.mat'), 'input');
        end

        %% Generate reference output signal
        function obj = generateReferenceOutput(obj)
            % Implementation of generateReferenceOutput method
            output = obj.Filter(obj.Input);
            % convert to fixed point
            output = convertToFxpnt(output, 's', obj.dataout_t.w, obj.dataout_t.f);
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
            x = x * 2 ^ obj.datain_t.f;
            % split and interleave real and imaginary parts
            input = zeros(size(x, 1), 2 * size(x, 2));
            input(:, 1:2:end) = real(x);
            input(:, 2:2:end) = imag(x);
            % save input test vector file for the C simulation
            dlmwrite([obj.TestCaseDir, '/input_test_vector.txt'], input, 'delimiter', '\t');
            % parallelize output signal = each row contains obj.Parallelel/obj.DecimationFactor samples
            y = reshape(obj.Output, ceil(obj.Parallelism / obj.DecimFactor), []);
            y = transpose(y);
            % convert to integer
            y = y * 2 ^ obj.dataout_t.f;
            % split and interleave real and imaginary parts
            yy = zeros(size(y, 1), 2 * size(y, 2));
            yy(:, 1:2:end) = real(y);
            yy(:, 2:2:end) = imag(y);
            % pad with zeros to get 2*obj.Parallelism columns
            output = zeros(size(yy, 1), 2 * obj.Parallelism);
            output(:, 1:size(yy, 2)) = yy;
            % save output test vector file for the C simulation
            dlmwrite([obj.TestCaseDir, '/output_ref.txt'], output, 'delimiter', '\t');
        end

        function obj = loadInput(obj)
            % Implementation of loadInput method
            % load input signal
            data = load(fullfile(obj.TestCaseDir, 'input.mat'));
            obj.Input = data.input;
        end

        function obj = loadReferenceOutput(obj)
            % Implementation of loadReferenceOutput method
            % load output signal
            data = load(fullfile(obj.TestCaseDir, 'output.mat'));
            obj.Output = data.output;
        end

        function obj = loadOutputCsim(obj)
            % Implementation of loadOutputCsim method
            % load output signal from C simulation
            data = load(fullfile(obj.TestCaseDir, 'output_csim.txt'));
            % first column is tvalid
            tvalid = data(:,1);
            obj.TvalidC = tvalid;
            % get only valid samples
            data = data(:,2:end);
            data = data(tvalid == 1, :);
            % convert to complex
            y = data(:, 1:2:end) + 1i * data(:, 2:2:end);
            % scale to [-1, 1]
            y = y ./ 2 ^ obj.dataout_t.f;
            decFactor = obj.DecimFactor;
            switch decFactor
                case 1
                    numCols = 8;
                case 2
                    numCols = 4;
                case 4
                    numCols = 2;
                case 8
                    numCols = 1;
                case 16
                    numCols = 1;
                case 32
                    numCols = 1;
                case 64
                    numCols = 1;
            end
            % get only valid samples
            y = y(:, 1:numCols);
            % read matrix row by row into column vector
            y = y.';
            y = y(:);
            % save output signal from C simulation
            obj.OutputC = y;
        end

        function obj = compareResults(obj)
            % Ensure that both signals are available
            if isempty(obj.Output) || isempty(obj.OutputC)
                error('Both Output and OutputC must be set before comparison.');
            end

            % Determine the threshold for acceptable error (this will be design-specific)
            errorThreshold = 2; % Define an appropriate threshold value

            % Print the header with the added 'Verdict' column
            fprintf('\n%-20s %-10s %-7s\n', 'Num Errors', 'Max Error', 'Verdict');

            % get the output and reshape it as a column vector
            y = obj.Output.*2^obj.dataout_t.f;
            y = y(:);
            % get the C output and reshape it as a column vector
            yc = obj.OutputC.*2^obj.dataout_t.f;
            yc = yc(:);
            % Perform comparison - this is an example using absolute error
            if length(y) ~= length(yc)
                fprintf('The length of the reference output and the C output are different.\n');
            else
                errorMetric = abs(y - yc);
                % Number of errors
                numErrors = sum(errorMetric > errorThreshold);
                % Determine verdict
                if numErrors > 0
                    verdict = 'FAIL';
                else
                    verdict = 'PASS';
                end
                % Print the results with column alignment including the 'Verdict'
                fprintf('%-20d %-10.2f %-7s\n', numErrors, max(errorMetric), verdict);
                % Plot the error, if errors are detected and number of errors exceeds 1000
                if numErrors > 1
                    % Ask the user if he wants to plot the error
                    prompt = 'Do you want to plot the error? Y/N [Y]: ';
                    str = input(prompt, 's');
                    if strcmp(str, 'Y') || strcmp(str, 'y') || isempty(str)
                        figure;
                        plot(errorMetric);
                        title('Error between Reference Output and RTL Simulation Output');
                        xlabel('Sample Number');
                        ylabel('Absolute Error');
                        figure;
                        subplot(2, 1, 1); plot(real(y)); hold on; plot(real(yc),'r'); hold off; title('Real Part');
                        subplot(2, 1, 2); plot(imag(y)); hold on; plot(imag(yc),'r'); hold off; title('Imaginary Part');
                    end
                end
            end
        end

        %% New Method to compare C/RTL outputs with reference outputs
        function obj = compareSignalFreqPower(obj)
            % Ensure that both signals are available
            if isempty(obj.Output) || isempty(obj.OutputC)
                error('Both Output and OutputC must be set before comparison.');
            end

            % Print the header with added 'powerDiff' and 'freqDiff'
            fprintf('\n%-15s %-15s %-15s %-15s %-15s %-15s %-15s %-15s\n', ...
                'powerInput [dB]', 'powerRef [dB]', 'powerC [dB]', ...
                'powerDiff [dB]', 'FreqInput [MHz]', 'FreqRef [MHz]', 'FreqC [MHz]', 'freqDiff [MHz]');

            % Compute the power of the input in dB
            powerInput = 10 * log10(mean(abs(obj.Input) .^ 2));
            % Compute the power of the reference output in dBFS
            powerRef = 10 * log10(mean(abs(obj.Output) .^ 2));
            % Compute the power of the C output in dB
            powerC = 10 * log10(mean(abs(obj.OutputC) .^ 2));
            % Compute the power difference in dB
            powerDiff = powerRef - powerC;
            % Compute the frequency of the input in MHz
            freqInput = meanfreq(obj.Input, obj.InputSampleRate);
            % Compute the frequency of the reference output in MHz
            freqRef = meanfreq(obj.Output, obj.OutputSampleRate);
            % Compute the frequency of the C output in MHz
            freqC = meanfreq(obj.OutputC, obj.OutputSampleRate);
            % Compute the frequency difference in MHz
            freqDiff = freqRef - freqC;
            % Print the results with column alignment including 'powerDiff' and 'freqDiff'
            fprintf('%-15.2f %-15.2f %-15.2f %-15.2f %-15.2f %-15.2f %-15.2f %-15.2f\n', ...
                powerInput, powerRef, powerC, powerDiff, freqInput, freqRef, freqC, freqDiff);

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
