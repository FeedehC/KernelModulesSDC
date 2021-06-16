import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import pandas as pd


fig = plt.figure(figsize = [30,20])
plt.style.use('fivethirtyeight')

def init_func(i):
    data_frame = pd.read_csv('data.csv')
    axises = data_frame.to_numpy(dtype = int)
    x = axises[:,0]
    y = axises[:,1]
    plt.subplot(1, 1, 1)    
    plt.plot(y, color="purple")
    plt.title("Alarma")
    plt.xlabel("Tiempo")
    plt.ylabel("LED1")

anim = FuncAnimation(plt.gcf(), init_func, interval=500)
plt.show()
