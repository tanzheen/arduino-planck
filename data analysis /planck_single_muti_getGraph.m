% Open the text file for reading
fileID = fopen('arduino_dual OPAMP new laser.txt');
if fileID == -1
    error('Could not open file. Check if file exists in current directory.');
end

% Create output directory if it doesn't exist
output_dir = 'led_analysis_results';
if ~exist(output_dir, 'dir')
    mkdir(output_dir);
end

% Initialize arrays to store extracted data
load_voltage_1 = [];
current_1 = [];

% Regular expressions to extract voltage and current values
voltage1_pattern = 'Bus Voltage:\s+([0-9.]+)\s+V';
current1_pattern = 'Current:\s+([-]?[0-9.]+)\s+mA';

% Read the file line by line
while ~feof(fileID)
    line = fgetl(fileID);
    
    % Extract Load Voltage 1
    tokens = regexp(line, voltage1_pattern, 'tokens');
    if ~isempty(tokens)
        load_voltage_1 = [load_voltage_1; str2double(tokens{1})];
    end
    
    % Extract Current 1
    tokens = regexp(line, current1_pattern, 'tokens');
    if ~isempty(tokens)
        current_1 = [current_1; str2double(tokens{1})];
    end
end

% Close the file
fclose(fileID);

% Find start of new iterations when current drops close to 0
threshold = 1; % Current threshold in mA to detect new iteration
start_indices = [1]; % First iteration starts at index 1

for i = 2:length(current_1)
    if current_1(i-1) > threshold && current_1(i) <= threshold
        start_indices = [start_indices; i];
    end
end
end_indices = [start_indices(2:end)-1; length(current_1)];

% Arrays to store results
threshold_voltages = [];
planck_constants = [];
errors = [];
valid_iterations = [];
total_iterations = 0;  % Counter for total iterations including skipped ones

% Process each iteration
for iter = 1:length(start_indices)
    % Extract data for this iteration
    idx_range = start_indices(iter):end_indices(iter);
    voltage_iter = load_voltage_1(idx_range);
    current_iter = current_1(idx_range);
    
    % Increment total iterations counter
    total_iterations = total_iterations + 1;
    
    % Check if max current reaches 45mA
    if max(current_iter) < 14
        fprintf('Skipping iteration %d - max current too low (%.2f mA)\n', total_iterations, max(current_iter));
        continue;  % Skip this iteration but keep counting
    end
    
    valid_iterations = [valid_iterations; total_iterations];
    
    % Create new figure for this iteration
    fig = figure('Position', [100, 100, 800, 600]);
    plot(voltage_iter, current_iter, '-o');
    hold on;
    
    % Select linear region
    linear_region = (current_iter > 5) & (current_iter < 45);
    linear_voltage = voltage_iter(linear_region);
    linear_current = current_iter(linear_region);
    
    % Perform linear regression
    p = polyfit(linear_voltage, linear_current, 1);
    
    % Generate fitted line
    x_fit = linspace(min(voltage_iter), max(voltage_iter), 100);
    y_fit = polyval(p, x_fit);
    plot(x_fit, y_fit, 'r-', 'LineWidth', 1);
    
    % Find x-intercept
    x_intercept = -p(2) / p(1);
    plot(x_intercept, 0, 'bx', 'MarkerSize', 5);
    text(x_intercept, 0, sprintf('Intercept = %.2f V', x_intercept), 'VerticalAlignment', 'bottom');
    
    % Calculate Planck
    V1 = x_intercept;
    e = 1.6022e-19;
    c = 2.998e8;
    lambda = 663e-9;
    h = 6.62607015e-34;
    h_calc = e*V1*lambda/c;
    error_percent = (abs(h-h_calc)/h)*100;
    
    % Store results
    threshold_voltages = [threshold_voltages; x_intercept];
    planck_constants = [planck_constants; h_calc];
    errors = [errors; error_percent];
    
    % Finish the plot
    grid on;
    ylim([0 50]);
    xlabel('Voltage (V)');
    ylabel('Current (mA)');
    title(sprintf('LED I-V Characteristics - Iteration %d', total_iterations));
    legend('Original Data', 'Linear Fit', 'X-Intercept', 'Location', 'northwest');
    
    % Save only PNG
    print(fig, fullfile(output_dir, sprintf('iteration_%d.png', total_iterations)), '-dpng');
    close(fig);
end

