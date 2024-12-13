import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns

# Configuration du style
plt.style.use('default')
sns.set_style("whitegrid")
plt.rcParams['figure.figsize'] = [16, 10]
plt.rcParams['font.size'] = 14
plt.rcParams['axes.labelsize'] = 14
plt.rcParams['axes.titlesize'] = 16
plt.rcParams['legend.fontsize'] = 12
plt.rcParams['xtick.labelsize'] = 12
plt.rcParams['ytick.labelsize'] = 12

# Lecture des données
df = pd.read_csv('multithread_comparison_results.csv')

# Palette de couleurs professionnelle
colors = {'valloc': '#2E86C1', 'malloc': '#28B463', 'calloc': '#E74C3C'}
markers = {'valloc': 'o', 'malloc': 's', 'calloc': '^'}

# Création de la figure
plt.figure(figsize=(16, 10))

# Calcul des moyennes pour chaque combinaison allocateur/taille
grouped = df.groupby(['allocator', 'size'])['time'].mean().reset_index()

# Tracé des lignes pour chaque allocateur
for allocator in grouped['allocator'].unique():
    data = grouped[grouped['allocator'] == allocator]
    plt.plot(data['size'] / 1024, data['time'] * 1000,
            label=f'{allocator}',
            color=colors[allocator],
            marker=markers[allocator],
            linewidth=3,
            markersize=10)

# Configuration des axes
plt.xscale('log')
plt.yscale('log')
plt.grid(True, which="both", ls="-", alpha=0.3)
plt.xlabel('Taille des blocs (KB)', fontsize=14, labelpad=10)
plt.ylabel('Temps moyen d\'exécution (ms)', fontsize=14, labelpad=10)
plt.title('Performance des Allocateurs en Fonction\nde la Taille des Blocs', 
          fontsize=16, pad=20)

# Ajout d'une légende améliorée
plt.legend(title='Allocateur', fontsize=12, title_fontsize=14, 
          frameon=True, fancybox=True, shadow=True)

# Ajout des annotations pour les points extrêmes
for allocator in grouped['allocator'].unique():
    data = grouped[grouped['allocator'] == allocator]
    
    # Annotation du point maximum
    max_time = data['time'].max()
    max_size = data.loc[data['time'] == max_time, 'size'].iloc[0]
    plt.annotate(f'{max_time*1000:.1f}ms',
                xy=(max_size/1024, max_time*1000),
                xytext=(10, 10), 
                textcoords='offset points',
                fontsize=10,
                bbox=dict(facecolor='white', edgecolor='none', alpha=0.7))
    
    # Annotation du point minimum
    min_time = data['time'].min()
    min_size = data.loc[data['time'] == min_time, 'size'].iloc[0]
    plt.annotate(f'{min_time*1000:.1f}ms',
                xy=(min_size/1024, min_time*1000),
                xytext=(10, -15),
                textcoords='offset points',
                fontsize=10,
                bbox=dict(facecolor='white', edgecolor='none', alpha=0.7))

# Ajustement de la mise en page
plt.tight_layout()

# Sauvegarde du graphique
plt.savefig('thread_size_results.png', dpi=300, bbox_inches='tight')
plt.close()
