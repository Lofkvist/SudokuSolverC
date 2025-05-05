import numpy as np

base = 5
filename = f"../grids/grid_{base*base}x{base*base}.dat"
threads = [1, 2, 4, 8, 16, 32]
max_depth = 200 if (base == 5) else (400 if (base == 6) else 1000)
max_tasks = [147822, 822273]
depth_switching_point = [round(x) for x in np.linspace(2, 200, 10)]
MINIMUM_TASK_COUNT = [round(x) for x in np.linspace(2, 200, 10)]
BATCH_SIZE = 230

