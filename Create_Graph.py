import pandas as pd
import matplotlib.pyplot as plt

# Load the CSV File into a DataFrame
data=pd.read_csv('output.csv')

# Extract data for the graph
days=data['Day']
memory_utilization=data['Memory Utilization (%)']
cpu_utilization=data['CPU Utilization (%)']

# Set the figure size
plt.figure(figsize=(30,15))

# Plot Bar Graph for Memory and CPU Utilization
bar_width=0.4
x_index=range(len(days))

plt.bar(x_index,memory_utilization,width=bar_width,label='Memory Utilization (%)',color='b')
plt.bar([x+bar_width for x in x_index],cpu_utilization,width=bar_width,label='CPU Utilization (%)',color='r')

# Customize the Graph
plt.xlabel('Days')
plt.ylabel('Utilization (%)')
plt.title('Memory and CPU Utilization')
plt.xticks([x+bar_width/2 for x in x_index],labels=days)
plt.legend()

plt.tight_layout()
plt.show()
