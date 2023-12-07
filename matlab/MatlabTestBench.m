classdef MatlabTestBench
    properties
        % Configurable public properties
        DecimFactor % Decimation factor
        SignalType % Signal type
        % Signals
        Input % Generated input test vector
        Output % Expected reference output for validation
        OutputC % Output from the C simulation
        TvalidC % Tvalid from the C simulation
        OutputRTL % Output from the RTL simulation
    end

    properties (Access = private)
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
            obj = computeProperties(obj);
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

        %% Compute properties
        function obj = computeProperties(obj)
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
        end
    end
end
