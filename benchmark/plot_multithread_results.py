import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np

# Configuration du style global
plt.style.use('default')
sns.set_theme()

# Lecture des données
df = pd.read_csv('multithread_results.csv')

# 1. Graphique des temps d'allocation
plt.figure(figsize=(8, 5))
plt.plot(df['block_size'] / 1024, df['allocation_time'] * 1000, 'o-', color='#2ecc71', linewidth=2)
plt.xscale('log')
plt.yscale('log')
plt.grid(True, which="both", ls="-", alpha=0.2)
plt.xlabel('Taille des blocs (KB)')
plt.ylabel('Temps (ms)')
plt.title('Temps d\'allocation par taille de bloc')
plt.tight_layout()
plt.savefig('multithread_allocation_time.png', dpi=300, bbox_inches='tight')
plt.close()

# 2. Graphique des temps de nettoyage
plt.figure(figsize=(8, 5))
plt.plot(df['block_size'] / 1024, df['cleanup_time'] * 1000, 's-', color='#e74c3c', linewidth=2)
plt.xscale('log')
plt.yscale('log')
plt.grid(True, which="both", ls="-", alpha=0.2)
plt.xlabel('Taille des blocs (KB)')
plt.ylabel('Temps (ms)')
plt.title('Temps de nettoyage par taille de bloc')
plt.tight_layout()
plt.savefig('multithread_cleanup_time.png', dpi=300, bbox_inches='tight')
plt.close()

# 3. Graphique comparatif allocation vs nettoyage
plt.figure(figsize=(8, 5))
plt.plot(df['block_size'] / 1024, df['allocation_time'] * 1000, 'o-', label='Allocation', color='#2ecc71', linewidth=2)
plt.plot(df['block_size'] / 1024, df['cleanup_time'] * 1000, 's-', label='Nettoyage', color='#e74c3c', linewidth=2)
plt.xscale('log')
plt.yscale('log')
plt.grid(True, which="both", ls="-", alpha=0.2)
plt.xlabel('Taille des blocs (KB)')
plt.ylabel('Temps (ms)')
plt.title('Comparaison des temps d\'allocation et de nettoyage')
plt.legend()
plt.tight_layout()
plt.savefig('multithread_comparison.png', dpi=300, bbox_inches='tight')
plt.close()

# 4. Graphique du ratio
plt.figure(figsize=(8, 5))
ratio = df['allocation_time'] / df['cleanup_time']
plt.plot(df['block_size'] / 1024, ratio, 'D-', color='#9b59b6', linewidth=2)
plt.xscale('log')
plt.grid(True, which="both", ls="-", alpha=0.2)
plt.xlabel('Taille des blocs (KB)')
plt.ylabel('Ratio Allocation/Nettoyage')
plt.title('Ratio entre temps d\'allocation et temps de nettoyage')

# Ajout d'annotations pour les points extrêmes
max_ratio_idx = ratio.idxmax()
min_ratio_idx = ratio.idxmin()

plt.annotate(f'Max: {ratio[max_ratio_idx]:.1f}x',
            xy=(df['block_size'][max_ratio_idx] / 1024, ratio[max_ratio_idx]),
            xytext=(10, 10), textcoords='offset points')

plt.annotate(f'Min: {ratio[min_ratio_idx]:.1f}x',
            xy=(df['block_size'][min_ratio_idx] / 1024, ratio[min_ratio_idx]),
            xytext=(10, -10), textcoords='offset points')

plt.tight_layout()
plt.savefig('multithread_ratio.png', dpi=300, bbox_inches='tight')
