import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import csv
import pandas as pd


#print(data_frame)

fig = plt.figure(figsize = [30,20])

data_skip = 10

def init_func():
    #ax.clear()
    data_frame = pd.read_csv('data.csv')
    axises = data_frame.to_numpy(dtype = int)
    x = axises[:,0]
    y = axises[:,1]
    plt.subplot(1, 1, 1)
    plt.style.use('fivethirtyeight')
    plt.plot(y, color="magenta")
    plt.title("Alarma")
    plt.xlabel("Tiempo")
    plt.ylabel("LED1")
    plt.show()

# def update_plot(i):
#     ax.plot(x[i:i+data_skip], y[i:i+data_skip], color='magenta')
#     ax.scatter(x[i], y[i])

anim = FuncAnimation(fig, init_func, interval=500)
#anim = FuncAnimation(fig, update_plot,frames=np.arange(0, len(x), data_skip),init_func=init_func,interval=1000)

#plt.show()

#anim.save('alarma.mp4', dpi=150, fps = 30, writer='ffmpeg')

