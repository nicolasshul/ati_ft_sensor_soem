import matplotlib.pyplot as plt
import csv

# File path
filename = "data.csv"

# Storage
t = []
fx, fy, fz = [], [], []
tx, ty, tz = [], [], []

cols = ["t", "fx", "fy", "fz", "tx", "ty", "tz"]

# Read CSV
with open(filename, 'r') as f:
    reader = csv.DictReader(f)
    for row in reader:
        if any(row[c] is None or row[c] == "" for c in cols):
            print("Skipping incomplete row:", row)
            continue
        t.append(float(row['t']))
        fx.append(float(row['fx']))
        fy.append(float(row['fy']))
        fz.append(float(row['fz']))
        tx.append(float(row['tx']))
        ty.append(float(row['ty']))
        tz.append(float(row['tz']))

# Plotting Force
plt.figure()
plt.plot(t, fx, label='Fx')
plt.plot(t, fy, label='Fy')
plt.plot(t, fz, label='Fz')
plt.xlabel('Time (s)')
plt.ylabel('Force')
plt.title('Force vs Time')
plt.legend()
plt.grid()

# Plotting Torque
plt.figure()
plt.plot(t, tx, label='Tx')
plt.plot(t, ty, label='Ty')
plt.plot(t, tz, label='Tz')
plt.xlabel('Time (s)')
plt.ylabel('Torque')
plt.title('Torque vs Time')
plt.legend()
plt.grid()

plt.show()


