import pandas as pd
import matplotlib.pyplot as plt
import os

# -------- FUNCTIONS --------

def build_grouped(df, field):
    """
    Groups DataFrame on field, computing:
      - list of runtimes
      - min runtime
      - avg runtime
    """
    g = df.groupby(field)["RUNTIME"].apply(list).reset_index()
    g.rename(columns={"RUNTIME": "RUNTIME_LIST"}, inplace=True)
    g["AVG_RUNTIME"] = g["RUNTIME_LIST"].apply(
        lambda x: sum(x) / len(x) if len(x) > 0 else None
    )
    g["MIN_RUNTIME"] = g["RUNTIME_LIST"].apply(
        lambda x: min(x) if len(x) > 0 else None
    )
    return g


def extract_base_from_filename(filename):
    """
    E.g. runtime_base5.csv → 5
    """
    try:
        base_str = filename.split("base")[-1].split(".")[0]
        return int(base_str)
    except Exception:
        return None


def plot_group(df, x_field, agg_method, base, out_dir):
    y_field = "MIN_RUNTIME" if agg_method == "min" else "AVG_RUNTIME"

    if y_field not in df.columns:
        print(f"Column {y_field} not found for {x_field}. Skipping plot.")
        return

    plot_df = df[[x_field, y_field]].dropna()
    if plot_df.empty:
        print(f"No data to plot for {x_field}. Skipping.")
        return

    x_data = plot_df[x_field].values
    y_data = plot_df[y_field].values

    x_data = pd.to_numeric(x_data, errors="coerce").flatten()
    y_data = pd.to_numeric(y_data, errors="coerce").flatten()

    if len(x_data) == 0 or len(y_data) == 0:
        print(f"No valid numeric data for plotting {x_field}. Skipping.")
        return

    plt.figure()
    plt.plot(
        x_data,
        y_data,
        marker='o'
    )

    xlabel_map = {
        "NUM_THREADS": "Number of Threads",
        "BASE_DEPTH": "Base Depth",
        "MINIMUM_TASK_COUNT": "Minimum Task Count"
    }

    xlabel = xlabel_map.get(x_field, x_field)

    plt.xlabel(xlabel)
    plt.ylabel(f"{agg_method.upper()} RUNTIME (s)")
    plt.title(f"Base = {base}: Runtime vs {xlabel}")
    plt.grid()

    filename = f"base{base}_runtime_vs_{x_field.lower()}_{agg_method}.png"
    out_path = os.path.join(out_dir, filename)
    plt.savefig(out_path)
    plt.close()


def plot_speedup(df, x_field, y_field, base, out_dir, normalized=False):
    plot_df = df[[x_field, y_field]].dropna()

    if plot_df.empty:
        print(f"No data to plot speedup for {x_field}. Skipping.")
        return

    x_data = plot_df[x_field].values
    y_data = plot_df[y_field].values

    x_data = pd.to_numeric(x_data, errors="coerce").flatten()
    y_data = pd.to_numeric(y_data, errors="coerce").flatten()

    if len(x_data) == 0 or len(y_data) == 0:
        print(f"No valid numeric data for speedup plot {x_field}. Skipping.")
        return

    plt.figure()
    plt.plot(
        x_data,
        y_data,
        marker='o'
    )

    xlabel = "Number of Threads" if x_field == "NUM_THREADS" else x_field

    plt.xlabel(xlabel)
    plt.ylabel("Speedup")
    plt.xscale("log", base=2)

    title = f"Base = {base}: Speedup vs {xlabel}"
    if normalized:
        title += "\n(normalized to 1-thread runtime)"
    else:
        title += "\n(vs serial runtime)"

    plt.title(title)
    plt.grid()

    suffix = "_normalized" if normalized else "_absolute"
    filename = f"base{base}_speedup_vs_{x_field.lower()}{suffix}.png"
    out_path = os.path.join(out_dir, filename)
    plt.savefig(out_path)
    plt.close()
    print(f"Saved plot: {out_path}")


def analyze_file(file, data_dir, runtime_dir, speedup_abs_dir, speedup_norm_dir, serial_times, agg_method):
    path = os.path.join(data_dir, file)
    df = pd.read_csv(path)
    df["RUNTIME"] = pd.to_numeric(df["RUNTIME"], errors="coerce")
    df = df.dropna(subset=["RUNTIME"])

    base = extract_base_from_filename(file)

    # Find best params for 16 threads
    df_16 = df[df["NUM_THREADS"] == 16]

    if not df_16.empty:
        if agg_method == "min":
            best_row = df_16.loc[df_16["RUNTIME"].idxmin()]
        else:
            agg_df = (
                df_16
                .groupby(["BASE_DEPTH", "MINIMUM_TASK_COUNT"])["RUNTIME"]
                .mean()
                .reset_index()
            )
            best_row = agg_df.loc[agg_df["RUNTIME"].idxmin()]

        best_base_depth = int(best_row["BASE_DEPTH"])
        best_min_tasks = int(best_row["MINIMUM_TASK_COUNT"])
        best_runtime = float(best_row["RUNTIME"])
        print(
            f"Base = {base} - Best config at 16 threads:"
            f" BASE_DEPTH={best_base_depth},"
            f" MINIMUM_TASK_COUNT={best_min_tasks},"
            f" Runtime={best_runtime:.6f}s"
        )
    else:
        print(f"Base = {base} - No data found for NUM_THREADS == 16.")

    # Group for plotting
    grouped_depth = build_grouped(df, "BASE_DEPTH")
    grouped_tasks = build_grouped(df, "MINIMUM_TASK_COUNT")
    grouped_threads = build_grouped(df, "NUM_THREADS")

    # Plot runtime vs parameters
    plot_group(grouped_depth, "BASE_DEPTH", agg_method, base, runtime_dir)
    plot_group(grouped_tasks, "MINIMUM_TASK_COUNT", agg_method, base, runtime_dir)
    plot_group(grouped_threads, "NUM_THREADS", agg_method, base, runtime_dir)

    # Compute absolute speedup
    serial_runtime = serial_times.get(file)
    y_field = "MIN_RUNTIME" if agg_method == "min" else "AVG_RUNTIME"

    if serial_runtime is not None:
        grouped_threads["SPEEDUP_ABSOLUTE"] = serial_runtime / grouped_threads[y_field]
        plot_speedup(
            grouped_threads,
            "NUM_THREADS",
            "SPEEDUP_ABSOLUTE",
            base,
            speedup_abs_dir,
            normalized=False
        )
    else:
        print(f"Base = {base} - No serial time known. Skipping absolute speedup plot.")

    # Compute normalized speedup
    one_thread_row = grouped_threads[grouped_threads["NUM_THREADS"] == 1]
    if not one_thread_row.empty:
        one_thread_runtime = one_thread_row[y_field].values[0]
        grouped_threads["SPEEDUP_NORMALIZED"] = one_thread_runtime / grouped_threads[y_field]

        plot_speedup(
            grouped_threads,
            "NUM_THREADS",
            "SPEEDUP_NORMALIZED",
            base,
            speedup_norm_dir,
            normalized=True
        )
    else:
        print(f"Base = {base} - No data for NUM_THREADS == 1. Skipping normalized speedup plot.")


# -------- MAIN --------

def main():
    # -------- CONFIGURATION --------
    data_dir = "./results"
    plot_dir = "./plots"
    runtime_dir = os.path.join(plot_dir, "runtime")
    speedup_abs_dir = os.path.join(plot_dir, "speedup_absolute")
    speedup_norm_dir = os.path.join(plot_dir, "speedup_normalized")

    agg_method = "min"   # or "avg"

    files = [
        "runtime_base5.csv",
        "runtime_base6.csv",
        "runtime_base8.csv",
    ]

    serial_times = {
        "runtime_base5.csv": 0.000293,
        "runtime_base6.csv": 4.138064,
        "runtime_base8.csv": 37.989444
    }

    # Create output folders
    os.makedirs(runtime_dir, exist_ok=True)
    os.makedirs(speedup_abs_dir, exist_ok=True)
    os.makedirs(speedup_norm_dir, exist_ok=True)

    for file in files:
        analyze_file(
            file,
            data_dir,
            runtime_dir,
            speedup_abs_dir,
            speedup_norm_dir,
            serial_times,
            agg_method
        )

    print("All plots saved to:", plot_dir)


if __name__ == "__main__":
    main()
