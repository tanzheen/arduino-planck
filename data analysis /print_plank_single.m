% Function to display all Planck's constants found in the text
function display_all_planck_constants(text_data)
    % Extract Planck's constants using regular expression
    pattern = "Planck's constant \(h\): (\d+\.\d+)";
    matches = regexp(text_data, pattern, 'tokens');
    
    % Convert cell array of strings to numeric array
    constants = cellfun(@(x) str2double(x{1}), matches);
    
    % Calculate error
    error = (abs(constants * (10^-34) - 6.62607015e-34)/6.62607015e-34)*100;
    
    % Create numbering for the measurements
    measurement_numbers = (1:length(constants))';
    
    % Combine measurement numbers, constants, and error
    display_data = [measurement_numbers, constants', error'];
    
    % Create figure window
    figure('Name', "All Planck's Constants", 'Position', [100 100 600 600]);
    
    % Create table with values (using proper column formats)
    uitable('Data', display_data, ...
        'Position', [20 20 560 560], ...
        'ColumnName', {'Measurement #', "Planck's Constant", "Error (%)"}, ...
        'ColumnWidth', {100, 200, 200}, ...
        'ColumnFormat', {'numeric', 'numeric', 'numeric'});
    
    % Print values to command window as well
    fprintf('\nAll Planck''s constant values found:\n');
    fprintf('%-15s %-20s %-15s\n', 'Measurement #', 'Value', 'Error (%)');
    fprintf('------------------------------------------------\n');
    for i = 1:length(constants)
        fprintf('%-15d %-20.10f %-15.4f\n', i, constants(i), error(i));
    end
    fprintf('\nTotal number of measurements: %d\n', length(constants));
    fprintf('Average error: %.4f%%\n', mean(error));
end

% Example usage:
% Read the text file
fileID = fopen('arduino_dual OPAMP new laser.txt', 'r');
text_data = fscanf(fileID, '%c');
fclose(fileID);

% Call the function
display_all_planck_constants(text_data);