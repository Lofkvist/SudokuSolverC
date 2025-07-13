import pandas as pd
import matplotlib.pyplot as plt
import os

# -------- FUNCTIONS --------

def plot_speedups_twin_axes(df, base, out_dir):
    """
    Plots absolute and normalized speedup in the same figure with dual y-axes.
    """

    plot_df = df.dropna(subset=["ABSOLUTE_SPEEDUP", "NORMALIZED_SPEEDUP"])

    if plot_df.empty:
        print(f"No data to plot speedups for base {base}. Skipping.")
        return

    x_data = plot_df["NUM_THREADS"].values
    abs_speedup = plot_df["ABSOLUTE_SPEEDUP"].values
    norm_speedup = plot_df["NORMALIZED_SPEEDUP"].values

    fig, ax1 = plt.subplots()

    color1 = "tab:blue"
    color2 = "tab:red"

    ax1.set_xlabel("Number of Threads")
    ax1.set_ylabel("Absolute Speedup", color=color1)
    ax1.plot(
        x_data,
        abs_speedup,
        marker='o',
        color=color1,
        label="Absolute Speedup"
    )
    ax1.tick_params(axis='y', labelcolor=color1)
    ax1.set_xscale("log", base=2)

    ax2 = ax1.twinx()
    ax2.set_ylabel("Normalized Speedup", color=color2)
    ax1.grid(True, which='both', linestyle='--', linewidth=0.5)
    ax2.grid(False)  # Keep grid only on ax1, usually clearer
    ax2.plot(
        x_data,
        norm_speedup,
        marker='s',
        color=color2,
        label="Normalized Speedup"
    )
    ax2.tick_params(axis='y', labelcolor=color2)

    plt.title(f"$n = {base}$: Absolute vs. Normalized Speedup")
    fig.tight_layout()

    filename = f"base{base}_speedup_ABS_and_NORM.png"
    out_path = os.path.join(out_dir, filename)
    plt.savefig(out_path)
    plt.close()



def plot_efficiency(df, base, out_dir):
    """
    Plots Efficiency vs NUM_THREADS for final data
    """
    plot_df = df.dropna(subset=["EFFICIENCY"])

    if plot_df.empty:
        print(f"No data to plot efficiency for base {base}. Skipping.")
        return

    x_data = plot_df["NUM_THREADS"].values
    efficiency_data = plot_df["EFFICIENCY"].values

    plt.figure()
    plt.plot(x_data, efficiency_data, marker='s', color='orange', label="Efficiency")
    plt.xlabel("Number of Threads")
    plt.ylabel("Efficiency")
    plt.xscale("log", base=2)
    plt.title(f"Base = {base}: Efficiency vs Threads")
    plt.grid()
    plt.legend()

    filename = f"base{base}_efficiency.png"
    out_path = os.path.join(out_dir, filename)
    plt.savefig(out_path)
    plt.close()


def analyze_final_results(speedup_file, data_dir, final_dir, serial_times):
    """
    Processes speedup_results.csv and computes:
        - absolute speedup
        - normalized speedup
        - efficiency
    """
    path = os.path.join(data_dir, speedup_file)
    df = pd.read_csv(path)

    df["RUN_TIME"] = pd.to_numeric(df["RUN_TIME"], errors="coerce")
    df = df.dropna(subset=["RUN_TIME"])

    base_values = df["BASE"].unique()

    for base in base_values:
        df_base = df[df["BASE"] == base]

        # Find minimum runtime per NUM_THREADS
        grouped = (
            df_base.groupby("NUM_THREADS")["RUN_TIME"]
            .min()
            .reset_index()
            .rename(columns={"RUN_TIME": "MIN_RUNTIME"})
        )

        # Find measured min runtime for 1 thread
        one_thread_row = grouped[grouped["NUM_THREADS"] == 1]

        if one_thread_row.empty:
            print(f"Base {base}: No data for 1 thread. Skipping normalized speedup.")
            continue

        measured_serial_runtime = one_thread_row["MIN_RUNTIME"].values[0]

        # Lookup provided serial runtime for absolute speedup
        serial_runtime_fixed = serial_times.get(base)

        if serial_runtime_fixed is None:
            print(f"Base {base}: No provided serial runtime. Skipping absolute speedup.")
            continue

        # Compute absolute and normalized speedup
        grouped["ABSOLUTE_SPEEDUP"] = serial_runtime_fixed / grouped["MIN_RUNTIME"]
        grouped["NORMALIZED_SPEEDUP"] = measured_serial_runtime / grouped["MIN_RUNTIME"]
        grouped["EFFICIENCY"] = grouped["NORMALIZED_SPEEDUP"] / grouped["NUM_THREADS"]
        
        print(f"\n=== Results for n = {base} ===")
        print(grouped.to_string(index=False))
        print("\n")


        # Plot results
        plot_speedups_twin_axes(grouped, base, final_dir)

        plot_efficiency(grouped, base, final_dir)


# -------- MAIN --------

def main():
    data_dir = "./results"
    final_dir = "./plots/speedup_efficiency"

    speedup_file = "speedup_results.csv"

    # Your provided serial runtimes for absolute speedup
    serial_times = {
        5: 0.000293,
        6: 4.138064,
        8: 37.989444,
    }

    os.makedirs(final_dir, exist_ok=True)

    analyze_final_results(speedup_file, data_dir, final_dir, serial_times)

    print("Final performance plots saved to:", final_dir)


if __name__ == "__main__":
    main()
