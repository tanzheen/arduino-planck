% Open the text file for reading
fileID = fopen('arduino_dual_LED 4.txt', 'r');

% Initialize arrays to store extracted data
load_voltage_1 = [];
current_1 = [];
load_voltage_2 = [];
current_2 = [];

% Regular expressions to extract voltage and current values
voltage1_pattern = 'Load Voltage 1:\s+([0-9.]+)\s+V';
current1_pattern = 'Current 1:\s+([-]?[0-9.]+)\s+mA';  % Allow negative values
voltage2_pattern = 'Load Voltage 2:\s+([0-9.]+)\s+V';
current2_pattern = 'Current 2:\s+([-]?[0-9.]+)\s+mA';  % Allow negative values

% Read the file line by line
while ~feof(fileID)
    line = fgetl(fileID);  % Read the current line
    
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

    % Extract Load Voltage 2
    tokens = regexp(line, voltage2_pattern, 'tokens');
    if ~isempty(tokens)
        load_voltage_2 = [load_voltage_2; str2double(tokens{1})];
    end
    
    % Extract Current 2
    tokens = regexp(line, current2_pattern, 'tokens');
    if ~isempty(tokens)
        current_2 = [current_2; str2double(tokens{1})]; 
    end
end

% Close the file
fclose(fileID);

%% Plot Graphs
% Plot Load Voltage 1 vs Current 1
figure;
subplot(1, 2, 1);  % Create a subplot for the first plot
plot(load_voltage_1, current_1, '-o');
ylabel('Current 1 (mA)');
xlabel('Load Voltage 1 (V)');
title('Load Voltage 1 vs Current 1');
hold on;
grid on;

% Select the linear region of the data 
linear_region_1 = (current_1 > 11) & (current_1 < 45);  % Using typ threshold current of 11mA and max operatign current of 45mA from 660nm datasheet
linear_voltage_1 = load_voltage_1(linear_region_1);
linear_current_1 = current_1(linear_region_1);

% Perform linear regression on the selected data points
p = polyfit(linear_voltage_1, linear_current_1, 1);  % p(1) is the slope, p(2) is the intercept

% Generate x values for the fitted line (including extrapolation)
x_fit = linspace(min(load_voltage_1), max(load_voltage_1), 100);  % Generate more x values for a smooth line
y_fit = polyval(p, x_fit);  % Compute the corresponding y values

% Plot the fitted line
plot(x_fit, y_fit, 'r-', 'LineWidth', 1); % Red line for linear fit
ylim([0 50])

% Extrapolate to find the x-intercept (when current = 0)
x_intercept_1 = -p(2) / p(1);  % Solving for x when y = 0 (y = mx + b -> 0 = mx + b)

% Plot the x-intercept on the graph
plot(x_intercept_1, 0, 'bx', 'MarkerSize', 5);  % Mark the intercept with a blue 'x'
text(x_intercept_1, 0, sprintf('Intercept = %.2f V', x_intercept_1), 'VerticalAlignment', 'bottom');

% Calculate Planck
V1 = x_intercept_1;
e = 1.6022e-19;
c = 2.998e8;
lambda = 650e-9;
h = 6.62607015e-34;
h_calc_1 = e*V1*lambda/c;
error_1 = (abs(h-h_calc_1)/h)*100;

% Finish the plot
grid on;
legend('Original Data', 'Linear Fit', 'X-Intercept', Location='northwest');
hold off;

% Plot Load Voltage 2 vs Current 2
subplot(1, 2, 2);  % Create a subplot for the second plot
plot(load_voltage_2, current_2, '-o');
ylabel('Current 2 (A)');
xlabel('Load Voltage 2 (V)');
title('Load Voltage 2 vs Current 2');
hold on;
grid on;

% Select the linear region of the data 
linear_region_2 = (current_2 > 11) & (current_2< 45);  % Using typ threshold current of 11mA and max operatign current of 45mA from 660nm datasheet
linear_voltage_2 = load_voltage_2(linear_region_2);
linear_current_2 = current_2(linear_region_2);

% Perform linear regression on the selected data points
p2 = polyfit(linear_voltage_2, linear_current_2, 1);  % p(1) is the slope, p(2) is the intercept

% Generate x values for the fitted line (including extrapolation)
x_fit_2 = linspace(min(load_voltage_2), max(load_voltage_2), 100);  % Generate more x values for a smooth line
y_fit_1 = polyval(p2, x_fit_2);  % Compute the corresponding y values

% Plot the fitted line
plot(x_fit_2, y_fit_1, 'r-', 'LineWidth', 1); % Red line for linear fit
ylim([0 50])

% Extrapolate to find the x-intercept (when current = 0)
x_intercept_2 = -p2(2) / p2(1);  % Solving for x when y = 0 (y = mx + b -> 0 = mx + b)

% Plot the x-intercept on the graph
plot(x_intercept_2, 0, 'bx', 'MarkerSize', 5);  % Mark the intercept with a blue 'x'
text(x_intercept_2, 0, sprintf('Intercept = %.2f V', x_intercept_2), 'VerticalAlignment', 'bottom');

%% Calculate Electrical Power (P = V * I)
% For both Laser Diodes
power_1 = load_voltage_1 .* current_1;  % Electrical power for diode 1 (W or mW)
power_2 = load_voltage_2 .* current_2;  % Electrical power for diode 2 (W or mW)

%% Plot Power vs Current for Laser Diode 1
figure;
subplot(1, 2, 1);  % Create a subplot for the first plot
plot(current_1, power_1, '-o');
ylabel('Electrical Power 1 (W)');
xlabel('Current 1 (mA)');
title('Power vs Current for Laser Diode 1');
grid on;

% Perform a linear fit for the power curve
p_power_1 = polyfit(current_1, power_1, 1);  % Linear fit for Power vs Current
y_fit_power_1 = polyval(p_power_1, current_1);

% Plot the linear fit
hold on;
plot(current_1, y_fit_power_1, 'r-', 'LineWidth', 1);
legend('Power Data', 'Linear Fit');
hold off;

% Plot Power vs Current for Laser Diode 2
subplot(1, 2, 2);  % Create a subplot for the second plot
plot(current_2, power_2, '-o');
ylabel('Electrical Power 2 (W)');
xlabel('Current 2 (mA)');
title('Power vs Current for Laser Diode 2');
grid on;

% Perform a linear fit for the power curve
p_power_2 = polyfit(current_2, power_2, 1);  % Linear fit for Power vs Current
y_fit_power_2 = polyval(p_power_2, current_2);

% Plot the linear fit
hold on;
plot(current_2, y_fit_power_2, 'r-', 'LineWidth', 1);
legend('Power Data', 'Linear Fit');
hold off;

% Display the results
disp(['Linear fit for Power vs Current (Diode 1): Slope = ', num2str(p_power_1(1)), ' W/mA']);
disp(['Linear fit for Power vs Current (Diode 2): Slope = ', num2str(p_power_2(1)), ' W/mA']);

% Calculate Planck
V2 = x_intercept_2;
h_calc_2 = e*V2*lambda/c;
error_2 = (abs(h-h_calc_2)/h)*100;


% Finish the plot
grid on;
legend('Original Data', 'Linear Fit', 'X-Intercept', Location='northwest');
hold off;

% Display Results
disp(['Threshold V1: ', num2str(x_intercept_1)]);
disp(['Plancks Constant 1: ', num2str(h_calc_1)]);
disp(['Error 1: ', num2str(error_1), '%'])

% Display Results
disp(['Threshold V2: ', num2str(x_intercept_2)]);
disp(['Plancks Constant 2: ', num2str(h_calc_2)]);
disp(['Error 2: ', num2str(error_2), '%'])

% Calculate Averaged Planck
planck = (h_calc_1 + h_calc_2)/2;
error = (abs(h-planck)/h)*100;

disp(['Plancks Constant (Ave): ', num2str(planck)]);
disp(['Overall Error: ', num2str(error), '%'])