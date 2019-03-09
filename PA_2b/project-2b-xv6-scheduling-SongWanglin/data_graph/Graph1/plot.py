import matplotlib.pyplot as plt;
import numpy as np;

t = np.linspace(0,1000, 11);
# fp = open("data.txt", "r");
ticks1 = [1, 28, 52, 71, 96, 119, 143, 169, 191, 217, 247];
ticks2 = [0, 40, 82,116, 168, 207, 245, 285, 321, 360, 398];
ticks3 = [0, 47, 116, 172, 240, 292, 349, 415, 466, 528, 577];
plt.plot(t, ticks1, 'r--', t, ticks2, 'b--', t, ticks3, 'g--');
plt.xlabel('unit of time');
plt.ylabel('ticks');
plt.title('Ticks for 3 processes with different tickets');
plt.gca().legend(('tickets=2','tickets=4','tickets=6'));
plt.savefig('Graph.png');
