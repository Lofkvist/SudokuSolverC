import pandas as pd
import matplotlib.pyplot as plt

# Läs CSV
csv_file = './experiments/result_base5.csv'
df = pd.read_csv(csv_file)

# Rename columns
df = df.rename(columns={
    'NUM_THREADS': 'THREADS',
    'BASE_DEPTH': 'DEPTH',
    'MINIMUM_TASK_COUNT': 'TASKS',
    'RUN_NUMBER': 'Run',
    'RUNTIME': 'Time'
})

# --- DEPTH ---
grouped_depth = df.groupby('DEPTH')['Time'].agg(['mean', 'var']).reset_index()
print("Grouped by DEPTH:")
print(grouped_depth)

plt.errorbar(
    grouped_depth['DEPTH'],
    grouped_depth['mean'],
    yerr=grouped_depth['var']**0.5,
    fmt='-o'
)
plt.xlabel('DEPTH')
plt.ylabel('Average Time (s)')
plt.title('Average Execution Time per DEPTH')
plt.grid(True)
plt.savefig('./experiments/execution_time_per_depth.png', dpi=300)
plt.clf()  # rensa figuren

# Depth med lägst snitt-tid
min_depth_row = grouped_depth.loc[grouped_depth['mean'].idxmin()]
print("Depth with lowest average time:")
print(min_depth_row)


# --- TASKS ---
grouped_tasks = df.groupby('TASKS')['Time'].agg(['mean', 'var']).reset_index()
print("Grouped by TASKS:")
print(grouped_tasks)

plt.errorbar(
    grouped_tasks['TASKS'],
    grouped_tasks['mean'],
    yerr=grouped_tasks['var']**0.5,
    fmt='-o'
)
plt.xlabel('TASKS')
plt.ylabel('Average Time (s)')
plt.title('Average Execution Time per TASKS')
plt.grid(True)
plt.savefig('./experiments/execution_time_per_tasks.png', dpi=300)
plt.clf()

# Tasks med lägst snitt-tid
min_tasks_row = grouped_tasks.loc[grouped_tasks['mean'].idxmin()]
print("TASKS with lowest average time:")
print(min_tasks_row)


# --- THREADS ---
grouped_threads = df.groupby('THREADS')['Time'].agg(['mean', 'var']).reset_index()
print("Grouped by THREADS:")
print(grouped_threads)

plt.errorbar(
    grouped_threads['THREADS'],
    grouped_threads['mean'],
    yerr=grouped_threads['var']**0.5,
    fmt='-o'
)
plt.xlabel('NUM_THREADS')
plt.ylabel('Average Time (s)')
plt.title('Average Execution Time per NUM_THREADS')
plt.grid(True)
plt.savefig('./experiments/execution_time_per_threads.png', dpi=300)
plt.clf()

# Threads med lägst snitt-tid
min_threads_row = grouped_threads.loc[grouped_threads['mean'].idxmin()]
print("THREADS with lowest average time:")
print(min_threads_row)
