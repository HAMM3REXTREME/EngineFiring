import numpy as np

x_start, x_end, x_step = 0.0, 9000.0, 3000.0
y_start, y_end, y_step = 0.0, 200.0, 100.0
expr = "x/190000"

x_values = np.arange(x_start, x_end + 1e-6, x_step)
y_values = np.arange(y_start, y_end + 1e-6, y_step)

print("\\", end="\t")
for x in x_values:
    print(f"{x:.1f}", end="\t")
print()

for y in y_values:
    print(f"{y:.1f}", end="\t")
    for x in x_values:
        z = eval(expr, {"x": x, "y": y})
        print(f"{z:.2f}", end="\t")
    print()
