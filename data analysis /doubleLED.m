%% Create Table
% Initialize arrays to store data
file_names = {};
planck_constant_1 = [];
error_1 = [];
planck_constant_2 = [];
error_2 = [];
ave_planck = [];
overall_error = [];

%% Read Data
% Open the text file for reading
filename = 'arduino_dual_OPAMP2 v16.txt';
fileID = fopen(filename, 'r');
fprintf('From file: %s\n', filename);

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
% Set Current range
lowerC = 5;
upperC = 20;

% Plot Load Voltage 1 vs Current 1
figure('Name', filename); 
subplot(1, 2, 1);  % Create a subplot for the first plot
plot(load_voltage_1, current_1, '-o');
ylabel('Current 1 (mA)');
xlabel('Load Voltage 1 (V)');
title('Load Voltage 1 vs Current 1');
hold on;
grid on;

% Select the linear region of the data 
linear_region_1 = (current_1 > lowerC) & (current_1 < upperC);  % Using typ threshold current of 11mA and max operatign current of 45mA from 660nm datasheet
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
lambda = 660e-9;
h = 6.62607015e-34;
h_calc_1 = e*V1*lambda/c;
error1 = (abs(h-h_calc_1)/h)*100;

% Finish the plot
grid on;
legend('Original Data', 'Linear Fit', 'X-Intercept', Location='northwest');
hold off;

% Plot Load Voltage 2 vs Current 2
subplot(1, 2, 2);  % Create a subplot for the second plot
plot(load_voltage_2, current_2, '-o');
ylabel('Current 2 (mA)');
xlabel('Load Voltage 2 (V)');
title('Load Voltage 2 vs Current 2');
hold on;
grid on;

% Select the linear region of the data 
linear_region_2 = (current_2 > lowerC) & (current_2 < upperC);  % Using typ threshold current of 11mA and max operatign current of 45mA from 660nm datasheet
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

% Calculate Planck
V2 = x_intercept_2;
h_calc_2 = e*V2*lambda/c;
error2 = (abs(h-h_calc_2)/h)*100;

% Finish the plot
grid on;
legend('Original Data', 'Linear Fit', 'X-Intercept', Location='northwest');
hold off;

% Display Results
disp(['Threshold V1: ', num2str(x_intercept_1)]);
disp(['Plancks Constant 1: ', num2str(h_calc_1)]);
disp(['Error 1: ', num2str(error1), '%'])

% Display Results
disp(['Threshold V2: ', num2str(x_intercept_2)]);
disp(['Plancks Constant 2: ', num2str(h_calc_2)]);
disp(['Error 2: ', num2str(error2), '%'])

% Calculate Averaged Planck
planck = (h_calc_1 + h_calc_2)/2;
error = (abs(h-planck)/h)*100;

disp(['Plancks Constant (Ave): ', num2str(planck)]);
disp(['Overall Error: ', num2str(error), '%'])

%% Update Arrays
file_names{end+1, 1} = filename;
planck_constant_1(end+1, 1) = h_calc_1;
error_1(end+1, 1) = error1;
planck_constant_2(end+1, 1) = h_calc_2;
error_2(end+1, 1) = error2;
ave_planck(end+1, 1) = planck;
overall_error(end+1, 1) = error;

% %% Create Table
% T = table(file_names, planck_constant_1, error_1, planck_constant_2, error_2, ave_planck, overall_error, ...
%           'VariableNames', {'File_Name', 'Plancks_Constant_1', 'Error_1', 'Plancks_Constant_2', 'Error_2', 'Ave_Planck', 'Overall_Error'});
% 
% writetable(T, 'planck_data.xlsx');