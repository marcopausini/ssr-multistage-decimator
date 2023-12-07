decimationFactors = [1, 2, 4, 8, 16, 32, 64];
signalType = 'exponential';
usecase = 'a';

fprintf('Usecase: %s\n', usecase);

% Depending on the use case, create test bench object outside the loop
% This avoids unnecessary checks and object creations
if usecase == 'a' || usecase == 'b'
    for idx = 1:length(decimationFactors)
        decimationFactor = decimationFactors(idx);
        fprintf('Decimation factor: %d\n', decimationFactor);
        fprintf('Signal type: %s\n\n', signalType);

        % Clear previous test bench object
        if exist('tb', 'var')
            clear tb;
        end

        % Create new test bench object once per iteration
        tb = MatlabTestBench(decimationFactor, signalType);

        % Perform different operations based on the use case
        if usecase == 'a'
            % Generate test vectors
            tb.generateTestVectors();
        elseif usecase == 'b'
            % Validate C simulation
            tb.validateCsim();
        end
    end
end
