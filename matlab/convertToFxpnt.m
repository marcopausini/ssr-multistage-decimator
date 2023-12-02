function [output] = convertToFxpnt(in, sign_mode, w, f, rounding_mode, saturation_mode)
    % convert floating point input to fixed point
    % --------------------------------------------------------------
    %
    % in: floating point input
    %
    % sign_mode: 'u' -> unsigned
    %            's' -> signed
    %
    % w : word length, i.e. total number of bits
    % f : fraction length
    %
    % unsigned  w.f (range: [0       : (1/2^f) : 2^(w-f) - 1/2^f]
    %   signed sw.f (range: [-2^(w-f-1): (1/2^f) : 2^(w-f-1) - 1/2^f]
    %
    % rounding_mode:
    % [default] 'round'           - round to the nearest integers (ties towards  infinity)
    %           'round_tietozero' - round to the nearest integers (ties toward zero)
    %           'ceil'            - round towards positive infinity
    %           'floor'           - round towards negative infinity
    %           'zero'            - round towards zero
    %
    % saturation_mode:
    %           'none'        - No saturation is performed and the value is truncated on the MSB side
    % [default] 'asymmetric'  - Saturation rounds an n-bit signed value in the range [- ( 2^(n-1) ) : +2^(n-1) - 1 ].
    %                           For example if n=8, the range would be [-128:127].
    %           'symmetric'   - Symmetric saturation rounds an n-bit signed value in the range [- ( 2^(n-1) -1 ) : +2^(n-1) - 1 ].
    %                           For example if n=8, the range would be [-127:127].
    %
    % Author: Marco Pausini
    %         marco.pausini@ast-science.com
    %
    % --------------------------------------------------------------

    %     persistent satCounter;
    %
    %     if isempty(satCounter)
    %         satCounter = 0;
    %         set(satCounter,'PersistentMemory',true);
    %     end

    if (nargin == 4)
        rounding = 'round';
        saturation = 'asymmetric';
    elseif (nargin == 5)
        rounding = rounding_mode;
        saturation = 'asymmetric';
    elseif (nargin == 6)
        rounding = rounding_mode;
        saturation = saturation_mode;
    end;

    % resolution
    res = 1/2 ^ f;

    % compute range
    switch saturation
        case 'symmetric'
            if (sign_mode == 'u')
                max_ = 2 ^ (w - f) -1/2 ^ f;
                min_ = 0;
            elseif (sign_mode == 's')
                max_ = 2 ^ (w - f - 1) -1/2 ^ f;
                min_ = -max_;
            end
        case 'asymmetric'
            if (sign_mode == 'u')
                max_ = 2 ^ (w - f) -1/2 ^ f;
                min_ = 0;
            elseif (sign_mode == 's')
                max_ = 2 ^ (w - f - 1) -1/2 ^ f;
                min_ = -2 ^ (w - f - 1);
            end
        case 'none'
            if (sign_mode == 'u')
                max_ = 2 ^ (w - f);
            elseif (sign_mode == 's')
                error('saturation mode not implemented');
            end
        otherwise
            error('saturation mode does not exist');
    end

    % quantize
    switch rounding
        case 'round'
            output = round(in / res) * res;
        case 'round_tietozero'
            if isreal(in)
                output = sign(in) .* floor(abs(in / res) + 0.5) * res;
            else
                output = sign(real(in)) .* floor(abs(real(in) / res) + 0.5) * res + 1i * sign(imag(in)) .* floor(abs(imag(in) / res) + 0.5) * res;
            end
        case 'ceil'
            output = ceil(in / res) * res;
        case 'zero'
            output = fix(in / res) * res;
        case 'floor'
            output = floor(in / res) * res;
        otherwise
            error('rounding mode does not exist');
    end

    if strcmp(saturation, 'none')
        output = mod(real(output),max_) + 1i * mod(imag(output),max_);
    else
        % saturate if necessary
        if isreal(output)
            output(output > max_) = max_;
            output(output < min_) = min_;
        else
            re_out = real(output);
            im_out = imag(output);
            re_out(re_out > max_) = max_;
            re_out(re_out < min_) = min_;
            im_out(im_out > max_) = max_;
            im_out(im_out < min_) = min_;
            output = re_out + 1i * im_out;
        end
    end

    %satCounter = satCounter + real(output
    %count = 0;
end
