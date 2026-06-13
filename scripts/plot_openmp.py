import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("results/openmp_benchmark.csv")

baseline = df[df["threads"] == 1]["runtime"].iloc[0]
df["speedup"] = baseline / df["runtime"]

plt.figure()
plt.plot(df["threads"], df["speedup"], marker="o", label="Measured speedup")
plt.plot(df["threads"], df["threads"], linestyle="--", label="Ideal speedup")

plt.xlabel("OpenMP threads")
plt.ylabel("Speedup")
plt.title("OpenMP strong scaling")
plt.legend()
plt.grid(True)

plt.savefig("results/openmp_speedup.png", dpi=200)
print(df)
print("Saved results/openmp_speedup.png")