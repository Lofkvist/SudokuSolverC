import pandas as pd
import re

# Define the file path
log_file = 'res.log'  # Replace with the path to your .log file

# Initialize empty lists to store the parsed data
depths = []
tasks = []
batches = []
run_numbers = []
times = []

# Read the log file
with open(log_file, 'r') as file:
    lines = file.readlines()

# Regular expression patterns to capture depth, tasks, batch, run, and time
depth_task_batch_pattern = re.compile(r"DEPTH=(\d+) TASKS=(\d+) BATCH=(\d+)")
run_pattern = re.compile(r"Run (\d+)")
time_pattern = re.compile(r"(\d+\.\d{6})")

# Iterate through the lines in the log file
current_depth = current_tasks = current_batch = None

for line in lines:
    # Check for DEPTH, TASKS, BATCH information
    match = depth_task_batch_pattern.search(line)
    if match:
        current_depth = int(match.group(1))
        current_tasks = int(match.group(2))
        current_batch = int(match.group(3))
    
    # Check for run number
    run_match = run_pattern.search(line)
    if run_match:
        run_number = int(run_match.group(1))
    
    # Check for time information
    time_match = time_pattern.search(line)
    if time_match:
        time_value = float(time_match.group(1))
        
        # Append data to lists
        depths.append(current_depth)
        tasks.append(current_tasks)
        batches.append(current_batch)
        run_numbers.append(run_number)
        times.append(time_value)

# Create a pandas DataFrame
df = pd.DataFrame({
    'DEPTH': depths,
    'TASKS': tasks,
    'BATCH': batches,
    'Run': run_numbers,
    'Time': times
})

# Display the DataFrame
print(df)

print("Smallest execution time:", df['Time'].min())
print(df.loc[df['Time'].idxmin()])

