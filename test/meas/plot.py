import sys
import matplotlib.pyplot as plt
import pandas as pd

filepath = sys.argv[1]

df = pd.read_csv(filepath)

y_snd = df.iloc[:, 0]
y_rcv = df.iloc[:, 1]

print(f'SND (millisecond) :: min: {y_snd.min()}, max: {y_snd.max()}, mean: {y_snd.mean()}, median: {y_snd.median()}')
print(f'RCV (millisecond) :: min: {y_rcv.min()}, max: {y_rcv.max()}, mean: {y_rcv.mean()}, median: {y_rcv.median()}')

plt.title('Process data exchange with SOEM')

row_count = len(df)
x = range(1, row_count + 1)

plt.plot(x, y_snd, label='SND', color='b')
plt.xlabel('Iteration')
plt.ylabel('SND', color='b')
plt.tick_params(axis='y', labelcolor='b')
plt.legend(loc='upper left')


plt.twinx()
plt.plot(x, y_rcv, label='RCV', color='r')
plt.ylabel('RCV', color='r')
plt.tick_params(axis='y', labelcolor='r')
plt.legend(loc='upper right')

plt.show()
