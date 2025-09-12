import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from collections import Counter
from scipy.stats import chisquare
import random

# === 配置 ===
CSV_FILE = "lottery.csv"   # 你的开奖数据文件
RED_RANGE = 33             # 红球范围（1~33）
BLUE_RANGE = 16            # 蓝球范围（1~16）
NUM_RED = 6                # 每期红球数量
NUM_BLUE = 1               # 每期蓝球数量

# === 读取数据 ===
df = pd.read_csv(CSV_FILE)

# 提取红球和蓝球
reds = df[[f"red{i}" for i in range(1, NUM_RED+1)]].values.flatten()
blues = df["blue"].values

# === 1. 单号码频率统计 ===
red_counts = Counter(reds)
blue_counts = Counter(blues)

# 理论期望（均匀分布）
expected_red = len(reds) / RED_RANGE
expected_blue = len(blues) / BLUE_RANGE

# 卡方检验
chi_red = chisquare(list(red_counts.values()))
chi_blue = chisquare(list(blue_counts.values()))

print("===== 卡方检验结果 =====")
print(f"红球 χ²={chi_red.statistic:.2f}, p={chi_red.pvalue:.4f}")
print(f"蓝球 χ²={chi_blue.statistic:.2f}, p={chi_blue.pvalue:.4f}")
print("p<0.05 表示号码分布显著偏离均匀分布")

# === 2. 可视化号码频率 ===
plt.figure(figsize=(12,5))
plt.bar(range(1, RED_RANGE+1), [red_counts.get(i,0) for i in range(1, RED_RANGE+1)])
plt.axhline(expected_red, color="r", linestyle="--", label="理论期望")
plt.title("红球出现频率")
plt.xlabel("号码")
plt.ylabel("次数")
plt.legend()
plt.show()

plt.figure(figsize=(8,4))
plt.bar(range(1, BLUE_RANGE+1), [blue_counts.get(i,0) for i in range(1, BLUE_RANGE+1)], color="blue")
plt.axhline(expected_blue, color="r", linestyle="--", label="理论期望")
plt.title("蓝球出现频率")
plt.xlabel("号码")
plt.ylabel("次数")
plt.legend()
plt.show()

# === 3. 和值/跨度检验 ===
sums = df[[f"red{i}" for i in range(1, NUM_RED+1)]].sum(axis=1)
spans = df[[f"red{i}" for i in range(1, NUM_RED+1)]].max(axis=1) - df[[f"red{i}" for i in range(1, NUM_RED+1)]].min(axis=1)

plt.figure(figsize=(10,5))
plt.hist(sums, bins=30, color="g", alpha=0.7)
plt.title("红球和值分布")
plt.xlabel("和值")
plt.ylabel("出现次数")
plt.show()

plt.figure(figsize=(10,5))
plt.hist(spans, bins=20, color="orange", alpha=0.7)
plt.title("红球跨度分布")
plt.xlabel("跨度")
plt.ylabel("出现次数")
plt.show()

# === 4. 独立性检验（游程检验）===
def runs_test(sequence):
    median = np.median(sequence)
    signs = ["+" if x >= median else "-" for x in sequence]
    runs = 1
    for i in range(1, len(signs)):
        if signs[i] != signs[i-1]:
            runs += 1
    n1 = signs.count("+")
    n2 = signs.count("-")
    expected_runs = 1 + 2*n1*n2/(n1+n2)
    var_runs = (2*n1*n2*(2*n1*n2-n1-n2))/(((n1+n2)**2)*(n1+n2-1))
    z = (runs - expected_runs)/np.sqrt(var_runs)
    return runs, expected_runs, z

runs, expected, z = runs_test(sums)
print("===== 游程检验 =====")
print(f"实际 runs={runs}, 期望={expected:.2f}, z={z:.2f}")
print("z 在 -1.96 ~ 1.96 之间 → 数据基本独立")

# === 5. Monte Carlo 验证 ===
def monte_carlo_draws(num_trials=10000):
    freq = np.zeros(RED_RANGE+1)
    for _ in range(num_trials):
        draw = random.sample(range(1, RED_RANGE+1), NUM_RED)
        for d in draw:
            freq[d] += 1
    return freq

sim_freq = monte_carlo_draws()
plt.figure(figsize=(12,5))
plt.plot(range(1, RED_RANGE+1), sim_freq[1:], label="Monte Carlo 模拟")
plt.plot(range(1, RED_RANGE+1), [red_counts.get(i,0) for i in range(1, RED_RANGE+1)], label="实际数据")
plt.title("模拟 vs 实际（红球分布）")
plt.xlabel("号码")
plt.ylabel("出现次数")
plt.legend()
plt.show()
