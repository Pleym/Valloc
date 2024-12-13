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
basic_df = pd.read_csv('../test/basic_performance_results.csv')
segmented_df = pd.read_csv('../benchmark_results.csv')

# Filtrer les données d'allocation
basic_alloc = basic_df[basic_df['operation'] == 'allocation']
segmented_alloc = segmented_df[segmented_df['test_type'] == 'allocation']

# Création de la figure
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(16, 16))

# Couleurs pour les différents allocateurs
colors = {
    'valloc (basic)': '#2E86C1',
    'malloc (basic)': '#28B463',
    'valloc (segmented)': '#E74C3C',
    'malloc (segmented)': '#8E44AD'
}

# 1. Comparaison des temps d'allocation
for allocator in ['valloc', 'malloc']:
    # Données de base
    basic_data = basic_alloc[basic_alloc['allocator'] == allocator]
    ax1.plot(basic_data['size'], basic_data['time'] * 1000,
             label=f'{allocator} (basic)',
             color=colors[f'{allocator} (basic)'],
             marker='o', linewidth=2)
    
    # Données avec segmentation
    seg_data = segmented_alloc[segmented_alloc['allocator'] == allocator]
    ax1.plot(seg_data['size'], seg_data['time'] * 1000,
             label=f'{allocator} (segmented)',
             color=colors[f'{allocator} (segmented)'],
             marker='s', linewidth=2)

ax1.set_xscale('log')
ax1.set_yscale('log')
ax1.set_xlabel('Taille du bloc (bytes)')
ax1.set_ylabel('Temps d\'allocation (ms)')
ax1.set_title('Comparaison des Temps d\'Allocation\n(Basic vs Segmented)')
ax1.grid(True, which="both", ls="-", alpha=0.2)
ax1.legend()

# 2. Ratio des performances (valloc/malloc)
ax2.axhline(y=1, color='gray', linestyle='--', label='Performance égale')

# Calcul et tracé des ratios pour la version basique
basic_valloc = basic_alloc[basic_alloc['allocator'] == 'valloc'].set_index('size')['time']
basic_malloc = basic_alloc[basic_alloc['allocator'] == 'malloc'].set_index('size')['time']
basic_ratio = basic_valloc / basic_malloc

# Calcul et tracé des ratios pour la version segmentée
seg_valloc = segmented_alloc[segmented_alloc['allocator'] == 'valloc'].set_index('size')['time']
seg_malloc = segmented_alloc[segmented_alloc['allocator'] == 'malloc'].set_index('size')['time']
seg_ratio = seg_valloc / seg_malloc

# Tracer les ratios
ax2.plot(basic_ratio.index, basic_ratio.values,
         label='valloc/malloc (basic)',
         color='#2E86C1',
         marker='o', linewidth=2)
ax2.plot(seg_ratio.index, seg_ratio.values,
         label='valloc/malloc (segmented)',
         color='#E74C3C',
         marker='s', linewidth=2)

ax2.set_xscale('log')
ax2.set_xlabel('Taille du bloc (bytes)')
ax2.set_ylabel('Ratio de temps (valloc/malloc)')
ax2.set_title('Ratio des Performances\n(valloc vs malloc)')
ax2.grid(True, which="both", ls="-", alpha=0.2)
ax2.legend()

# Ajustement de la mise en page
plt.tight_layout()

# Sauvegarde du graphique
plt.savefig('allocator_comparison.png', dpi=300, bbox_inches='tight')
plt.close()
