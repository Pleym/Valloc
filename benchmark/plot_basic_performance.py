import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np

# Configuration du style
plt.style.use('default')
sns.set_style("whitegrid")
plt.rcParams['figure.figsize'] = [15, 10]
plt.rcParams['font.size'] = 12
plt.rcParams['axes.labelsize'] = 12
plt.rcParams['axes.titlesize'] = 14

# Lecture des données
df = pd.read_csv('basic_performance_results.csv')

# Création de la figure avec deux sous-graphiques
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(15, 12))

# Couleurs pour les allocateurs
colors = {'valloc': '#2E86C1', 'malloc': '#28B463'}

# 1. Graphique des temps d'allocation
allocation_data = df[df['operation'] == 'allocation']
for allocator in ['valloc', 'malloc']:
    data = allocation_data[allocation_data['allocator'] == allocator]
    ax1.plot(data['size'], data['time'] * 1000, 
             label=allocator, marker='o', 
             color=colors[allocator], linewidth=2)

ax1.set_xscale('log')
ax1.set_yscale('log')
ax1.set_xlabel('Taille du bloc (bytes)')
ax1.set_ylabel('Temps d\'allocation (ms)')
ax1.set_title('Temps d\'Allocation par Taille de Bloc')
ax1.grid(True, which="both", ls="-", alpha=0.2)
ax1.legend()

# 2. Graphique des temps de libération
free_data = df[df['operation'] == 'free']
for allocator in ['valloc', 'malloc']:
    data = free_data[free_data['allocator'] == allocator]
    ax2.plot(data['size'], data['time'] * 1000, 
             label=allocator, marker='o', 
             color=colors[allocator], linewidth=2)

ax2.set_xscale('log')
ax2.set_yscale('log')
ax2.set_xlabel('Taille du bloc (bytes)')
ax2.set_ylabel('Temps de libération (ms)')
ax2.set_title('Temps de Libération par Taille de Bloc')
ax2.grid(True, which="both", ls="-", alpha=0.2)
ax2.legend()

# Ajustement de la mise en page
plt.tight_layout()

# Sauvegarde du graphique
plt.savefig('basic_performance_comparison.png', dpi=300, bbox_inches='tight')
plt.close()
