import pandas as pd
import matplotlib.pyplot as plt
import re  # Import regex to clean non-numerical characters

# Function to clean and extract the data from the text file
def extract_data(filename):
    with open(filename, 'r') as file:
        lines = file.readlines()

    data = {
        'Digi_pot_level': [],
        'Bus_Voltage_1': [],
        'Current_1': [],
        'Bus_Voltage_2': [],
        'Current_2': []
    }

    # Function to remove non-numeric characters (except dot and minus)
    def clean_value(value):
        cleaned = re.sub(r'[^\d.-]', '', value)
        print(cleaned)
        return cleaned

    for i in range(0, len(lines), 5):  # Assuming each set of data spans 5 lines
        try:
            print(lines[i])
            
            # Clean up and extract each value, remove labels like 'Bus Voltage 1:', etc.
            data['Digi_pot_level'].append(int(clean_value(lines[i].split(':')[1].strip())))  # Digi_pot_level
            data['Bus_Voltage_1'].append(float(clean_value(lines[i+1].split(':')[1].strip())))  # Bus Voltage 1
            data['Current_1'].append(float(clean_value(lines[i+2].split(':')[1].strip())))  # Current 1
            data['Bus_Voltage_2'].append(float(clean_value(lines[i+3].split(':')[1].strip())))  # Bus Voltage 2
            data['Current_2'].append(float(clean_value(lines[i+4].split(':')[1].strip())))  # Current 2
        except (IndexError, ValueError) as e:
            print(f"Error processing line {i}: {e}")
            break  # Break if lines are not complete or conversion fails

    return pd.DataFrame(data)

# Plotting function for I-V curves
def plot_iv_curves(df):
    plt.figure(figsize=(10, 6))
    plt.plot(df['Bus_Voltage_1'], df['Current_1'], label='Laser Diode 1 I-V Curve', color='b')
    plt.plot(df['Bus_Voltage_2'], df['Current_2'], label='Laser Diode 2 I-V Curve', color='r')
    plt.xlabel('Voltage (V)')
    plt.ylabel('Current (mA)')
    plt.title('I-V Curves for Two Laser Diodes')
    plt.legend()
    plt.grid(True)
    plt.show()

# Main code execution
filename = 'arduino_dual_LED.txt'  # Replace with the correct path to your text file
df = extract_data(filename)  # Extract the data into a DataFrame
print(df)
plot_iv_curves(df)  # Plot the I-V curves