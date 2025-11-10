import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

# ---- 1. Define your map ----
x = np.array([1000, 2000, 3000, 4000])  # RPM
y = np.array([0.2, 0.5, 0.8])           # Load
z = np.array([
    [14.7, 13.5, 12.8, 12.5],
    [14.5, 13.3, 12.6, 12.3],
    [14.2, 13.0, 12.3, 12.0]
], dtype=float)

X, Y = np.meshgrid(x, y)

# ---- 2. Create 3D plot ----
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

# Plot the surface
surf = ax.plot_surface(X, Y, z, cmap='viridis', edgecolor='k', alpha=0.8)

# Add contour projections on XY plane
ax.contour(X, Y, z, zdir='z', offset=z.min(), cmap='viridis')

# Colorbar
fig.colorbar(surf, ax=ax, shrink=0.5, aspect=5, label='AFR')

# Labels
ax.set_xlabel('RPM')
ax.set_ylabel('Load')
ax.set_zlabel('AFR')
ax.set_title('3D Map Visualization')

plt.show()
