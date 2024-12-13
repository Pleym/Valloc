import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# Lecture des données
df = pd.read_csv('benchmark_thread_cache.csv')

# Configuration du style
plt.style.use('seaborn')
sns.set_palette("husl")

# Création du boxplot
plt.figure(figsize=(10, 6))
sns.boxplot(x='use_cache', y='execution_time', data=df)

# Personnalisation du graphique
plt.title('Performance Comparison: With vs Without Thread Cache')
plt.xlabel('Thread Cache Enabled')
plt.ylabel('Execution Time (seconds)')
plt.xticks([0, 1], ['Without Cache', 'With Cache'])

# Ajout des points individuels
sns.swarmplot(x='use_cache', y='execution_time', data=df, color='.25', size=8, alpha=0.5)

# Sauvegarde du graphique
plt.savefig('benchmark/plots/thread_cache_performance.png')
plt.close()

# Calcul des statistiques
stats = df.groupby('use_cache')['execution_time'].agg(['mean', 'std', 'min', 'max'])
print("\nPerformance Statistics:")
print("\nWithout Cache (use_cache=0):")
print(stats.loc[0])
print("\nWith Cache (use_cache=1):")
print(stats.loc[1])

# Calcul de l'amélioration
improvement = ((stats.loc[0]['mean'] - stats.loc[1]['mean']) / stats.loc[0]['mean']) * 100
print(f"\nPerformance Improvement: {improvement:.2f}%")
