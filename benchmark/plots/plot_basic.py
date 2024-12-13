import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np

# Configuration du style
plt.style.use('default')
sns.set_style("whitegrid")
plt.rcParams['figure.figsize'] = [16, 12]
plt.rcParams['font.size'] = 14
plt.rcParams['axes.labelsize'] = 14
plt.rcParams['axes.titlesize'] = 16
plt.rcParams['legend.fontsize'] = 12

# Lecture des données
basic_df = pd.read_csv('/Users/dorian/CHPS/AISE/valloc/test/basic_performance_results.csv')

# Calcul du temps moyen pour les données de base
basic_avg = basic_df.pivot_table(
    index=['allocator', 'size'],
    columns='operation',
    values='time'
).reset_index()
basic_avg['avg_time'] = (basic_avg['allocation'] + basic_avg['free']) / 2

# Création de la figure
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(16, 16))

# Couleurs pour les différents allocateurs
colors = {
    'valloc': '#2E86C1',
    'malloc': '#28B463'
}

# 1. Comparaison des temps moyens
for allocator in ['valloc', 'malloc']:
    data = basic_avg[basic_avg['allocator'] == allocator]
    ax1.plot(data['size'], data['avg_time'] * 1000,
             label=allocator,
             color=colors[allocator],
             marker='o', linewidth=2)

ax1.set_xscale('log')
ax1.set_yscale('log')
ax1.set_xlabel('Taille du bloc (bytes)')
ax1.set_ylabel('Temps moyen d\'exécution (ms)')
ax1.set_title('Comparaison des Temps Moyens d\'Exécution\n(Allocation + Libération) / 2')
ax1.grid(True, which="both", ls="-", alpha=0.2)
ax1.legend()

# 2. Ratio des performances moyennes (valloc/malloc)
ax2.axhline(y=1, color='gray', linestyle='--', label='Performance égale')

# Calcul et tracé des ratios
valloc_data = basic_avg[basic_avg['allocator'] == 'valloc'].set_index('size')['avg_time']
malloc_data = basic_avg[basic_avg['allocator'] == 'malloc'].set_index('size')['avg_time']
ratio = valloc_data / malloc_data

# Tracer les ratios
ax2.plot(ratio.index, ratio.values,
         label='valloc/malloc',
         color='#2E86C1',
         marker='o', linewidth=2)

ax2.set_xscale('log')
ax2.set_xlabel('Taille du bloc (bytes)')
ax2.set_ylabel('Ratio des temps moyens (valloc/malloc)')
ax2.set_title('Ratio des Performances Moyennes')
ax2.grid(True, which="both", ls="-", alpha=0.2)
ax2.legend()

# Ajout d'une note sur la formule
fig.text(0.02, 0.02, 'Note: Temps moyen = (Temps allocation + Temps libération) / 2',
         fontsize=10, style='italic')

# Ajustement de la mise en page
plt.tight_layout()

# Sauvegarde du graphique
plt.savefig('average_time_comparison.png', dpi=300, bbox_inches='tight')
plt.close()
