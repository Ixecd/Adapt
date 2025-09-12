import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.font_manager as fm
from collections import Counter
from scipy.stats import chisquare, normaltest
import random
import seaborn as sns
from datetime import datetime
import warnings
warnings.filterwarnings('ignore')

# === 配置中文字体和样式 ===
# 尝试设置中文字体，如果失败则使用默认字体
try:
    plt.rcParams['font.sans-serif'] = ['Arial Unicode MS', 'SimHei', 'Helvetica', 'DejaVu Sans']
    plt.rcParams['axes.unicode_minus'] = False
except:
    print("⚠️  中文字体设置失败，使用默认字体")

# 设置matplotlib样式
plt.style.use('default')
sns.set_palette("husl")

# === 配置 ===
CSV_FILE = "lottery.csv"   # 开奖数据文件
RED_RANGE = 33             # 红球范围（1~33）
BLUE_RANGE = 16            # 蓝球范围（1~16）
NUM_RED = 6                # 每期红球数量
NUM_BLUE = 1               # 每期蓝球数量

# 优化的颜色配置
COLORS = {
    'red_primary': '#E74C3C',
    'red_secondary': '#EC7063', 
    'blue_primary': '#3498DB',
    'blue_secondary': '#5DADE2',
    'accent': '#F39C12',
    'success': '#27AE60',
    'warning': '#F1C40F',
    'background': '#ECF0F1',
    'text': '#2C3E50',
    'grid': '#BDC3C7'
}

print("🎲 双色球数据分析系统 v2.0 Enhanced")
print("=" * 60)
print(f"📊 分析时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
print("=" * 60)

# === 读取数据 ===
try:
    df = pd.read_csv(CSV_FILE)
    print(f"✅ 成功读取数据: {len(df)} 期开奖记录")
except FileNotFoundError:
    print("❌ 错误: 找不到数据文件 lottery.csv")
    exit(1)

# 提取红球和蓝球
reds = df[[f"red{i}" for i in range(1, NUM_RED+1)]].values.flatten()
blues = df["blue"].values

# === 数据概览 ===
print("\n📈 数据概览:")
print(f"   总开奖期数: {len(df)}")
print(f"   红球总数: {len(reds)}")
print(f"   蓝球总数: {len(blues)}")
print(f"   数据完整性: {(1 - (np.isnan(reds).sum() + np.isnan(blues).sum()) / (len(reds) + len(blues))) * 100:.1f}%")

# === 统计分析 ===
red_counts = Counter(reds)
blue_counts = Counter(blues)

# 理论期望（均匀分布）
expected_red = len(reds) / RED_RANGE
expected_blue = len(blues) / BLUE_RANGE

# 卡方检验
chi_red = chisquare([red_counts.get(i, 0) for i in range(1, RED_RANGE+1)])
chi_blue = chisquare([blue_counts.get(i, 0) for i in range(1, BLUE_RANGE+1)])

print("\n🔍 随机性检验结果:")
print(f"   红球 χ² = {chi_red.statistic:.2f}, p值 = {chi_red.pvalue:.4f}")
print(f"   蓝球 χ² = {chi_blue.statistic:.2f}, p值 = {chi_blue.pvalue:.4f}")
if chi_red.pvalue > 0.05:
    print("   ✅ 红球分布符合随机性")
else:
    print("   ⚠️  红球分布可能存在偏差")
if chi_blue.pvalue > 0.05:
    print("   ✅ 蓝球分布符合随机性")
else:
    print("   ⚠️  蓝球分布可能存在偏差")

# === 创建优化的可视化图表 ===
# 设置全局字体大小
plt.rcParams.update({'font.size': 10})

# 创建主图表
fig = plt.figure(figsize=(24, 18))
fig.patch.set_facecolor('white')
fig.suptitle('🎲 双色球开奖数据综合分析报告 Enhanced', fontsize=28, fontweight='bold', y=0.98, color=COLORS['text'])

# 1. 红球频率分布 - 增强版
ax1 = plt.subplot(3, 4, 1)
red_freq = [red_counts.get(i, 0) for i in range(1, RED_RANGE+1)]
# 创建渐变色效果
colors_red = plt.cm.Reds(np.linspace(0.4, 0.9, len(red_freq)))
bars1 = ax1.bar(range(1, RED_RANGE+1), red_freq, color=colors_red, alpha=0.8, edgecolor='white', linewidth=1.5)
ax1.axhline(expected_red, color=COLORS['warning'], linestyle='--', linewidth=3, label=f'理论期望 ({expected_red:.1f})', alpha=0.8)
ax1.set_title('🔴 红球出现频率分布', fontsize=14, fontweight='bold', pad=20, color=COLORS['text'])
ax1.set_xlabel('号码', fontsize=12, color=COLORS['text'])
ax1.set_ylabel('出现次数', fontsize=12, color=COLORS['text'])
ax1.legend(fontsize=10)
ax1.grid(True, alpha=0.3, color=COLORS['grid'])
ax1.set_facecolor('#FAFAFA')
# 标注异常值
for i, (bar, freq) in enumerate(zip(bars1, red_freq)):
    if freq > expected_red * 1.3 or freq < expected_red * 0.7:
        ax1.text(bar.get_x() + bar.get_width()/2., freq + 0.5, f'{int(freq)}', 
                ha='center', va='bottom', fontsize=9, fontweight='bold', color=COLORS['text'])

# 2. 蓝球频率分布 - 增强版
ax2 = plt.subplot(3, 4, 2)
blue_freq = [blue_counts.get(i, 0) for i in range(1, BLUE_RANGE+1)]
colors_blue = plt.cm.Blues(np.linspace(0.4, 0.9, len(blue_freq)))
bars2 = ax2.bar(range(1, BLUE_RANGE+1), blue_freq, color=colors_blue, alpha=0.8, edgecolor='white', linewidth=1.5)
ax2.axhline(expected_blue, color=COLORS['warning'], linestyle='--', linewidth=3, label=f'理论期望 ({expected_blue:.1f})', alpha=0.8)
ax2.set_title('🔵 蓝球出现频率分布', fontsize=14, fontweight='bold', pad=20, color=COLORS['text'])
ax2.set_xlabel('号码', fontsize=12, color=COLORS['text'])
ax2.set_ylabel('出现次数', fontsize=12, color=COLORS['text'])
ax2.legend(fontsize=10)
ax2.grid(True, alpha=0.3, color=COLORS['grid'])
ax2.set_facecolor('#FAFAFA')
# 标注异常值
for i, (bar, freq) in enumerate(zip(bars2, blue_freq)):
    if freq > expected_blue * 1.3 or freq < expected_blue * 0.7:
        ax2.text(bar.get_x() + bar.get_width()/2., freq + 0.5, f'{int(freq)}', 
                ha='center', va='bottom', fontsize=9, fontweight='bold', color=COLORS['text'])

# 3. 红球和值分布 - 增强版
ax3 = plt.subplot(3, 4, 3)
sums = df[[f"red{i}" for i in range(1, NUM_RED+1)]].sum(axis=1)
n, bins, patches = ax3.hist(sums, bins=25, color=COLORS['accent'], alpha=0.7, edgecolor='white', linewidth=1.5)
# 为直方图添加渐变色
for i, patch in enumerate(patches):
    patch.set_facecolor(plt.cm.YlOrRd(i / len(patches)))
ax3.axvline(sums.mean(), color=COLORS['text'], linestyle='-', linewidth=3, label=f'均值 ({sums.mean():.1f})', alpha=0.8)
ax3.axvline(sums.median(), color=COLORS['success'], linestyle='--', linewidth=3, label=f'中位数 ({sums.median():.1f})', alpha=0.8)
ax3.set_title('📊 红球和值分布', fontsize=14, fontweight='bold', pad=20, color=COLORS['text'])
ax3.set_xlabel('和值', fontsize=12, color=COLORS['text'])
ax3.set_ylabel('频次', fontsize=12, color=COLORS['text'])
ax3.legend(fontsize=10)
ax3.grid(True, alpha=0.3, color=COLORS['grid'])
ax3.set_facecolor('#FAFAFA')

# 4. 红球跨度分布 - 增强版
ax4 = plt.subplot(3, 4, 4)
spans = df[[f"red{i}" for i in range(1, NUM_RED+1)]].max(axis=1) - df[[f"red{i}" for i in range(1, NUM_RED+1)]].min(axis=1)
n, bins, patches = ax4.hist(spans, bins=20, color=COLORS['blue_secondary'], alpha=0.7, edgecolor='white', linewidth=1.5)
# 为直方图添加渐变色
for i, patch in enumerate(patches):
    patch.set_facecolor(plt.cm.Blues(0.4 + 0.5 * i / len(patches)))
ax4.axvline(spans.mean(), color=COLORS['text'], linestyle='-', linewidth=3, label=f'均值 ({spans.mean():.1f})', alpha=0.8)
ax4.axvline(spans.median(), color=COLORS['success'], linestyle='--', linewidth=3, label=f'中位数 ({spans.median():.1f})', alpha=0.8)
ax4.set_title('📏 红球跨度分布', fontsize=14, fontweight='bold', pad=20, color=COLORS['text'])
ax4.set_xlabel('跨度', fontsize=12, color=COLORS['text'])
ax4.set_ylabel('频次', fontsize=12, color=COLORS['text'])
ax4.legend(fontsize=10)
ax4.grid(True, alpha=0.3, color=COLORS['grid'])
ax4.set_facecolor('#FAFAFA')

# 5. 奇偶比例分析 - 增强版
ax5 = plt.subplot(3, 4, 5)
odd_counts = []
for _, row in df.iterrows():
    red_nums = [row[f"red{i}"] for i in range(1, NUM_RED+1)]
    odd_count = sum(1 for x in red_nums if x % 2 == 1)
    odd_counts.append(odd_count)

odd_even_dist = Counter(odd_counts)
labels = [f'{i}奇{NUM_RED-i}偶' for i in range(NUM_RED+1)]
values = [odd_even_dist.get(i, 0) for i in range(NUM_RED+1)]
colors_pie = plt.cm.Set3(np.linspace(0, 1, len(labels)))
wedges, texts, autotexts = ax5.pie(values, labels=labels, autopct='%1.1f%%', colors=colors_pie, 
                                   startangle=90, explode=[0.05 if v == max(values) else 0 for v in values])
ax5.set_title('⚪ 红球奇偶比例分布', fontsize=14, fontweight='bold', pad=20, color=COLORS['text'])
for autotext in autotexts:
    autotext.set_color('white')
    autotext.set_fontweight('bold')
    autotext.set_fontsize(9)

# 6. 大小比例分析 - 增强版
ax6 = plt.subplot(3, 4, 6)
small_counts = []  # 1-16为小号
for _, row in df.iterrows():
    red_nums = [row[f"red{i}"] for i in range(1, NUM_RED+1)]
    small_count = sum(1 for x in red_nums if x <= 16)
    small_counts.append(small_count)

small_big_dist = Counter(small_counts)
labels = [f'{i}小{NUM_RED-i}大' for i in range(NUM_RED+1)]
values = [small_big_dist.get(i, 0) for i in range(NUM_RED+1)]
colors_pie = plt.cm.Pastel1(np.linspace(0, 1, len(labels)))
wedges, texts, autotexts = ax6.pie(values, labels=labels, autopct='%1.1f%%', colors=colors_pie, 
                                   startangle=90, explode=[0.05 if v == max(values) else 0 for v in values])
ax6.set_title('🔢 红球大小号比例分布', fontsize=14, fontweight='bold', pad=20, color=COLORS['text'])
for autotext in autotexts:
    autotext.set_color('white')
    autotext.set_fontweight('bold')
    autotext.set_fontsize(9)

# 7. Monte Carlo模拟对比 - 增强版
ax7 = plt.subplot(3, 4, 7)
def monte_carlo_draws(num_trials=10000):
    freq = np.zeros(RED_RANGE+1)
    for _ in range(num_trials):
        draw = random.sample(range(1, RED_RANGE+1), NUM_RED)
        for d in draw:
            freq[d] += 1
    return freq

sim_freq = monte_carlo_draws()
ax7.plot(range(1, RED_RANGE+1), sim_freq[1:], 'o-', color=COLORS['blue_primary'], 
         linewidth=3, markersize=6, label="Monte Carlo模拟", alpha=0.8, markerfacecolor='white', markeredgewidth=2)
ax7.plot(range(1, RED_RANGE+1), red_freq, 's-', color=COLORS['red_primary'], 
         linewidth=3, markersize=6, label="实际数据", alpha=0.8, markerfacecolor='white', markeredgewidth=2)
ax7.fill_between(range(1, RED_RANGE+1), sim_freq[1:], alpha=0.2, color=COLORS['blue_primary'])
ax7.fill_between(range(1, RED_RANGE+1), red_freq, alpha=0.2, color=COLORS['red_primary'])
ax7.set_title('🎯 模拟 vs 实际对比', fontsize=14, fontweight='bold', pad=20, color=COLORS['text'])
ax7.set_xlabel('号码', fontsize=12, color=COLORS['text'])
ax7.set_ylabel('出现次数', fontsize=12, color=COLORS['text'])
ax7.legend(fontsize=10)
ax7.grid(True, alpha=0.3, color=COLORS['grid'])
ax7.set_facecolor('#FAFAFA')

# 8. 热力图 - 号码共现频率
ax8 = plt.subplot(3, 4, 8)
# 创建号码共现矩阵
co_occurrence = np.zeros((RED_RANGE, RED_RANGE))
for _, row in df.iterrows():
    red_nums = [int(row[f"red{i}"]) for i in range(1, NUM_RED+1)]
    for i in range(len(red_nums)):
        for j in range(i+1, len(red_nums)):
            co_occurrence[red_nums[i]-1][red_nums[j]-1] += 1
            co_occurrence[red_nums[j]-1][red_nums[i]-1] += 1

# 只显示部分数据以提高可读性
subset_size = 20
co_subset = co_occurrence[:subset_size, :subset_size]
im = ax8.imshow(co_subset, cmap='YlOrRd', aspect='auto', interpolation='bilinear')
ax8.set_title('🔥 号码共现热力图 (1-20)', fontsize=14, fontweight='bold', pad=20, color=COLORS['text'])
ax8.set_xlabel('号码', fontsize=12, color=COLORS['text'])
ax8.set_ylabel('号码', fontsize=12, color=COLORS['text'])
ax8.set_xticks(range(0, subset_size, 3))
ax8.set_yticks(range(0, subset_size, 3))
ax8.set_xticklabels(range(1, subset_size+1, 3))
ax8.set_yticklabels(range(1, subset_size+1, 3))
cbar = plt.colorbar(im, ax=ax8, shrink=0.8)
cbar.set_label('共现次数', rotation=270, labelpad=15)

# 9. 号码趋势分析
ax9 = plt.subplot(3, 4, 9)
# 计算最近10期的号码出现趋势
recent_periods = 10
if len(df) >= recent_periods:
    recent_data = df.tail(recent_periods)
    recent_reds = recent_data[[f"red{i}" for i in range(1, NUM_RED+1)]].values.flatten()
    recent_counts = Counter(recent_reds)
    recent_freq = [recent_counts.get(i, 0) for i in range(1, RED_RANGE+1)]
    
    # 计算趋势（最近10期 vs 历史平均）
    historical_avg = [red_counts.get(i, 0) / len(df) * recent_periods for i in range(1, RED_RANGE+1)]
    trend = np.array(recent_freq) - np.array(historical_avg)
    
    colors_trend = ['red' if t > 0 else 'blue' for t in trend]
    bars = ax9.bar(range(1, RED_RANGE+1), trend, color=colors_trend, alpha=0.7, edgecolor='white', linewidth=1)
    ax9.axhline(0, color='black', linestyle='-', linewidth=2)
    ax9.set_title(f'📈 最近{recent_periods}期号码趋势', fontsize=14, fontweight='bold', pad=20, color=COLORS['text'])
    ax9.set_xlabel('号码', fontsize=12, color=COLORS['text'])
    ax9.set_ylabel('趋势值', fontsize=12, color=COLORS['text'])
    ax9.grid(True, alpha=0.3, color=COLORS['grid'])
    ax9.set_facecolor('#FAFAFA')

# 10. 连号分析
ax10 = plt.subplot(3, 4, 10)
consecutive_counts = []
for _, row in df.iterrows():
    red_nums = sorted([row[f"red{i}"] for i in range(1, NUM_RED+1)])
    consecutive = 0
    max_consecutive = 0
    for i in range(1, len(red_nums)):
        if red_nums[i] == red_nums[i-1] + 1:
            consecutive += 1
        else:
            max_consecutive = max(max_consecutive, consecutive)
            consecutive = 0
    max_consecutive = max(max_consecutive, consecutive)
    consecutive_counts.append(max_consecutive)

consecutive_dist = Counter(consecutive_counts)
labels = [f'{i}连号' for i in range(max(consecutive_counts)+1)]
values = [consecutive_dist.get(i, 0) for i in range(max(consecutive_counts)+1)]
colors_bar = plt.cm.viridis(np.linspace(0, 1, len(values)))
bars = ax10.bar(labels, values, color=colors_bar, alpha=0.8, edgecolor='white', linewidth=1.5)
ax10.set_title('🔗 连号出现分析', fontsize=14, fontweight='bold', pad=20, color=COLORS['text'])
ax10.set_xlabel('连号类型', fontsize=12, color=COLORS['text'])
ax10.set_ylabel('出现次数', fontsize=12, color=COLORS['text'])
ax10.grid(True, alpha=0.3, color=COLORS['grid'])
ax10.set_facecolor('#FAFAFA')
# 添加数值标签
for bar, value in zip(bars, values):
    if value > 0:
        ax10.text(bar.get_x() + bar.get_width()/2., value + 0.5, f'{value}', 
                 ha='center', va='bottom', fontsize=10, fontweight='bold')

# 11. 区间分析
ax11 = plt.subplot(3, 4, 11)
# 将33个号码分为3个区间
zone1_counts = []  # 1-11
zone2_counts = []  # 12-22
zone3_counts = []  # 23-33

for _, row in df.iterrows():
    red_nums = [row[f"red{i}"] for i in range(1, NUM_RED+1)]
    zone1 = sum(1 for x in red_nums if 1 <= x <= 11)
    zone2 = sum(1 for x in red_nums if 12 <= x <= 22)
    zone3 = sum(1 for x in red_nums if 23 <= x <= 33)
    zone1_counts.append(zone1)
    zone2_counts.append(zone2)
    zone3_counts.append(zone3)

zone_data = [zone1_counts, zone2_counts, zone3_counts]
zone_labels = ['区间1(1-11)', '区间2(12-22)', '区间3(23-33)']
zone_colors = [COLORS['red_primary'], COLORS['blue_primary'], COLORS['accent']]

box_plot = ax11.boxplot(zone_data, labels=zone_labels, patch_artist=True, 
                        boxprops=dict(alpha=0.7), medianprops=dict(color='black', linewidth=2))
for patch, color in zip(box_plot['boxes'], zone_colors):
    patch.set_facecolor(color)
    
ax11.set_title('📦 区间分布箱线图', fontsize=14, fontweight='bold', pad=20, color=COLORS['text'])
ax11.set_ylabel('号码个数', fontsize=12, color=COLORS['text'])
ax11.grid(True, alpha=0.3, color=COLORS['grid'])
ax11.set_facecolor('#FAFAFA')

# 12. 统计摘要 - 增强版
ax12 = plt.subplot(3, 4, 12)
ax12.axis('off')

# 计算详细统计信息
red_max = max(red_freq)
red_min = min(red_freq)
red_max_nums = [i+1 for i, x in enumerate(red_freq) if x == red_max]
red_min_nums = [i+1 for i, x in enumerate(red_freq) if x == red_min]

blue_max = max(blue_freq)
blue_min = min(blue_freq)
blue_max_nums = [i+1 for i, x in enumerate(blue_freq) if x == blue_max]
blue_min_nums = [i+1 for i, x in enumerate(blue_freq) if x == blue_min]

# 计算变异系数
red_cv = np.std(red_freq) / np.mean(red_freq)
blue_cv = np.std(blue_freq) / np.mean(blue_freq)

stats_text = f"""
📊 详细统计摘要

🔴 红球统计:
   最热号码: {red_max_nums} ({red_max}次)
   最冷号码: {red_min_nums} ({red_min}次)
   标准差: {np.std(red_freq):.2f}
   变异系数: {red_cv:.3f}
   
🔵 蓝球统计:
   最热号码: {blue_max_nums} ({blue_max}次)
   最冷号码: {blue_min_nums} ({blue_min}次)
   标准差: {np.std(blue_freq):.2f}
   变异系数: {blue_cv:.3f}

📈 和值统计:
   均值: {sums.mean():.1f}
   标准差: {sums.std():.1f}
   范围: {sums.min()}-{sums.max()}
   
📏 跨度统计:
   均值: {spans.mean():.1f}
   标准差: {spans.std():.1f}
   范围: {spans.min()}-{spans.max()}
   
🎯 随机性评估:
   红球p值: {chi_red.pvalue:.4f}
   蓝球p值: {chi_blue.pvalue:.4f}
   
💡 建议: 理性购彩
"""

ax12.text(0.05, 0.95, stats_text, transform=ax12.transAxes, fontsize=11,
         verticalalignment='top', 
         bbox=dict(boxstyle='round,pad=1', facecolor=COLORS['background'], alpha=0.9, edgecolor=COLORS['text']))

plt.tight_layout()
plt.subplots_adjust(top=0.94, hspace=0.3, wspace=0.3)
plt.show()

# === 独立性检验（游程检验）===
def runs_test(sequence):
    median = np.median(sequence)
    signs = ["+" if x >= median else "-" for x in sequence]
    runs = 1
    for i in range(1, len(signs)):
        if signs[i] != signs[i-1]:
            runs += 1
    n1 = signs.count("+")
    n2 = signs.count("-")
    if n1 == 0 or n2 == 0:
        return runs, 0, 0
    expected_runs = 1 + 2*n1*n2/(n1+n2)
    var_runs = (2*n1*n2*(2*n1*n2-n1-n2))/(((n1+n2)**2)*(n1+n2-1))
    if var_runs <= 0:
        return runs, expected_runs, 0
    z = (runs - expected_runs)/np.sqrt(var_runs)
    return runs, expected_runs, z

runs, expected, z = runs_test(sums)
print("\n🔄 游程检验结果:")
print(f"   实际游程数: {runs}")
print(f"   期望游程数: {expected:.2f}")
print(f"   Z统计量: {z:.2f}")
if abs(z) <= 1.96:
    print("   ✅ 数据序列基本独立 (95%置信度)")
else:
    print("   ⚠️  数据序列可能存在依赖性")

# === 正态性检验 ===
stat_sums, p_sums = normaltest(sums)
stat_spans, p_spans = normaltest(spans)

print("\n📊 正态性检验:")
print(f"   和值分布: 统计量={stat_sums:.2f}, p值={p_sums:.4f}")
if p_sums > 0.05:
    print("   ✅ 和值分布接近正态分布")
else:
    print("   ⚠️  和值分布偏离正态分布")
    
print(f"   跨度分布: 统计量={stat_spans:.2f}, p值={p_spans:.4f}")
if p_spans > 0.05:
    print("   ✅ 跨度分布接近正态分布")
else:
    print("   ⚠️  跨度分布偏离正态分布")

print("\n" + "="*60)
print("🎯 Enhanced分析完成! 优化图表已生成")
print("💡 建议: 彩票具有随机性，请理性购买")
print("🔍 本分析仅供学习研究，不构成投注建议")
print("="*60)