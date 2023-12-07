% usecase:
% a - create new test bench object and generate test vectors
% b - use current test bench object if exist and validate C simulation


% Inform the user of the options available for use case
fprintf('Select use case:\n');
fprintf('(a) Create a new test bench object and generate test vectors.\n');
fprintf('(b) Use current test bench object if it exists and validate C simulation.\n');

% Continue asking until valid input is given for use case
validUseCase = false;
while ~validUseCase
    usecase = input('Enter choice (a/b): ', 's');
    switch usecase
        case 'a'
            % Valid input received; proceed to ask for decimation factor and signal type.
            validUseCase = true;
        case 'b'
            % Valid input received; proceed to ask for decimation factor and signal type.
            validUseCase = true;
        otherwise
            fprintf('Invalid selection. Please enter "a" or "b".\n');
    end
end

% Allowed values for decimation factor
allowedDecimationFactors = [1, 2, 4, 8, 16, 32, 64];

% Continue asking until a valid decimation factor is given
validDecimationFactor = false;
while ~validDecimationFactor
    decimationFactor = input('Enter decimation factor (1, 2, 4, 8, 16, 32, 64): ');
    
    if any(decimationFactor == allowedDecimationFactors)
        validDecimationFactor = true;
    else
        fprintf('Invalid decimation factor. Please enter a value from the allowed set.\n');
    end
end

% Allowed values for signal type
allowedSignalTypes = [1, 2];
signalTypes = {'exponential', 'chirp'};

% Continue asking until a valid signal type is given
validSignalType = false;
while ~validSignalType
    idx = input('Enter signal type (1 for exponential tone, 2 for chirp): ');
    
    if any(idx == allowedSignalTypes)
        validSignalType = true;
    else
        fprintf('Invalid signal type. Please enter 1 for exponential tone or 2 for chirp.\n');
    end
end

% Create new test bench object if use case (a) is selected
if usecase == 'a'
    close all
    clc
    if exist('tb', 'var')
        fprintf('test bench object already exist. Clear it and create a new one\n');
        clear tb
    else
        fprintf('create a new test bench object\n');
    end
    % create new test bench object and generate test vectors
    tb = MatlabTestBench(decimationFactor, signalTypes{idx});
    fprintf('generate test vectors\n');
    tb = tb.generateTestVectors()
elseif usecase == 'b'
    % check if test bench object exists
    if ~exist('tb', 'var')
        fprintf('no test bench object found. Create a new onw\n');
        tb = MatlabTestBench(testcase)
    end
    % validate C simulation
    fprintf('validate C simulation\n');
    tb = tb.validateCsim();
end
%