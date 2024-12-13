import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np

# Configuration du style
plt.style.use('default')
sns.set_style("whitegrid")
plt.rcParams['figure.figsize'] = [16, 14]
plt.rcParams['font.size'] = 14
plt.rcParams['axes.labelsize'] = 14
plt.rcParams['axes.titlesize'] = 16
plt.rcParams['legend.fontsize'] = 12
plt.rcParams['xtick.labelsize'] = 12
plt.rcParams['ytick.labelsize'] = 12

# Lecture des données
df = pd.read_csv('benchmark_results.csv')

# Création de la figure avec trois sous-graphiques
fig = plt.figure(constrained_layout=True)
gs = fig.add_gridspec(3, 2)

# Palette de couleurs professionnelle
colors = ['#2E86C1', '#28B463', '#E74C3C']
sns.set_palette(colors)

# 1. Graphique des temps d'allocation par taille (échelle log)
ax1 = fig.add_subplot(gs[0, :])
allocation_data = df[df['test_type'] == 'allocation']
sns.lineplot(data=allocation_data, x='size', y='time', hue='allocator', 
             marker='o', ax=ax1, linewidth=3, markersize=8)
ax1.set_xscale('log')
ax1.set_yscale('log')
ax1.set_xlabel('Taille de l\'allocation (bytes)')
ax1.set_ylabel('Temps d\'exécution (secondes)')
ax1.set_title('Comparaison des Performances d\'Allocation\npar Taille de Bloc', pad=20)
ax1.grid(True, which="both", ls="-", alpha=0.3)
ax1.legend(title='Allocateur', bbox_to_anchor=(1.05, 1), loc='upper left', frameon=True)

# 2. Comparaison des ratios de performance
ax2 = fig.add_subplot(gs[1, :])
pivot_data = allocation_data.pivot(index='size', columns='allocator', values='time')
ratio_valloc_malloc = pivot_data['valloc'] / pivot_data['malloc']
ratio_valloc_calloc = pivot_data['valloc'] / pivot_data['calloc']

ax2.plot(pivot_data.index, ratio_valloc_malloc, '-o', color='#2E86C1', linewidth=3, markersize=8, label='valloc/malloc')
ax2.plot(pivot_data.index, ratio_valloc_calloc, '-o', color='#E74C3C', linewidth=3, markersize=8, label='valloc/calloc')
ax2.axhline(y=1, color='#28B463', linestyle='--', linewidth=2, label='Performance égale')
ax2.set_xscale('log')
ax2.set_xlabel('Taille de l\'allocation (bytes)')
ax2.set_ylabel('Ratio de temps')
ax2.set_title('Ratio des Performances\n(valloc vs autres allocateurs)', pad=20)
ax2.grid(True, alpha=0.3)
ax2.legend(bbox_to_anchor=(1.05, 1), loc='upper left', frameon=True)

# 3. Tests de texte (barplot)
ax3 = fig.add_subplot(gs[2, :])
text_data = df[df['test_type'] == 'text']
sns.barplot(data=text_data, x='allocator', y='time', ax=ax3, palette=colors)
ax3.set_xlabel('Allocateur')
ax3.set_ylabel('Temps d\'exécution (secondes)')
ax3.set_title('Performance sur les Tests de Manipulation de Texte', pad=20)
ax3.grid(True, axis='y', alpha=0.3)

# 4. Statistiques détaillées
ax4 = fig.add_subplot(gs[2, 1])
ax4.axis('off')
stats_text = []

# Statistiques pour les allocations
stats_text.append("Statistiques des allocations:")
for size in allocation_data['size'].unique():
    size_data = allocation_data[allocation_data['size'] == size]
    stats_text.append(f"\nTaille {size} bytes:")
    for alloc in ['valloc', 'malloc', 'calloc']:
        time = size_data[size_data['allocator'] == alloc]['time'].iloc[0]
        stats_text.append(f"  {alloc}: {time:.6f}s")

# Statistiques pour les tests de texte
stats_text.append("\n\nStatistiques des tests de texte:")
for alloc in text_data['allocator']:
    time = text_data[text_data['allocator'] == alloc]['time'].iloc[0]
    stats_text.append(f"  {alloc}: {time:.6f}s")

ax4.text(0, 1, '\n'.join(stats_text), va='top', fontfamily='monospace')

# Ajustement de la mise en page
plt.tight_layout()

# Sauvegarde des graphiques
plt.savefig('benchmark_results.png', dpi=300, bbox_inches='tight')
plt.close()

# Création d'un rapport markdown
with open('benchmark_report.md', 'w') as f:
    f.write("# Rapport de Performance des Allocateurs\n\n")
    
    f.write("## 1. Performance des Allocations\n\n")
    f.write("### Petites allocations (16-256 bytes)\n")
    small_alloc = allocation_data[allocation_data['size'] <= 256]
    for alloc in ['valloc', 'malloc', 'calloc']:
        avg_time = small_alloc[small_alloc['allocator'] == alloc]['time'].mean()
        f.write(f"- {alloc}: {avg_time:.6f}s en moyenne\n")
    
    f.write("\n### Allocations moyennes (4KB)\n")
    med_alloc = allocation_data[allocation_data['size'] == 4096]
    for alloc in ['valloc', 'malloc', 'calloc']:
        time = med_alloc[med_alloc['allocator'] == alloc]['time'].iloc[0]
        f.write(f"- {alloc}: {time:.6f}s\n")
    
    f.write("\n### Grandes allocations (1MB)\n")
    large_alloc = allocation_data[allocation_data['size'] == 1048576]
    for alloc in ['valloc', 'malloc', 'calloc']:
        time = large_alloc[large_alloc['allocator'] == alloc]['time'].iloc[0]
        f.write(f"- {alloc}: {time:.6f}s\n")
    
    f.write("\n## 2. Performance des Tests de Texte\n\n")
    for alloc in ['valloc', 'malloc', 'calloc']:
        time = text_data[text_data['allocator'] == alloc]['time'].iloc[0]
        f.write(f"- {alloc}: {time:.6f}s\n")
    
    f.write("\n## 3. Analyse\n\n")
    
    # Calcul des ratios moyens
    avg_ratio_malloc = ratio_valloc_malloc.mean()
    avg_ratio_calloc = ratio_valloc_calloc.mean()
    
    f.write(f"- Ratio moyen valloc/malloc: {avg_ratio_malloc:.2f}\n")
    f.write(f"- Ratio moyen valloc/calloc: {avg_ratio_calloc:.2f}\n")
    
    f.write("\n### Points clés\n")
    f.write("1. Pour les petites allocations:\n")
    f.write("   - malloc et calloc sont plus rapides\n")
    f.write("   - L'overhead de mmap est visible\n\n")
    
    f.write("2. Pour les grandes allocations:\n")
    f.write("   - valloc devient plus efficace\n")
    f.write("   - Particulièrement performant pour les allocations de 1MB\n\n")
    
    f.write("3. Pour les tests de texte:\n")
    f.write("   - Performances similaires entre les allocateurs\n")
    f.write("   - Les différences sont moins marquées avec des opérations réelles\n")

print("Le rapport a été sauvegardé dans benchmark_report.md")
