import numpy as np
import matplotlib.pyplot as plt
import csv
import pandas as pd

data_frame = pd.read_csv('data.csv')
axises = data_frame.to_numpy(dtype = int)
print(data_frame)

plt.style.use('fivethirtyeight')
fig = plt.figure(figsize = [20,15])
plt.plot(axises[:,1], color="magenta")
plt.title("Prueba")
plt.xlabel("Tiempo")
plt.ylabel("LED1")
plt.show()

