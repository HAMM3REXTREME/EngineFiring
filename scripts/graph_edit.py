#!/usr/bin/env python3
"""
XY Graph Editor — Fixed domain/range graph editing

Controls:
 • Left click: add point
 • Right click & drag: move point
 • Middle click: delete nearest point
 • + / – buttons: adjust grid density
 • Export/Import to text format "x y" per line (ignore '#' comments)
"""

import tkinter as tk
from tkinter import filedialog, messagebox, simpledialog
import math

POINT_RADIUS = 5
HIT_RADIUS = 10
PADDING = 40


class XYEditor:
    def __init__(self, root):
        self.root = root
        root.title("XY Graph Editor Tool | Internal Use")

        # Fixed domain/range setup
        self.xmin = self.ask_float("Enter X min:", 0.0)
        self.xmax = self.ask_float("Enter X max:", 1.0)
        self.ymin = self.ask_float("Enter Y min:", 0.0)
        self.ymax = self.ask_float("Enter Y max:", 1.0)
        if self.xmax <= self.xmin or self.ymax <= self.ymin:
            messagebox.showerror("Invalid range", "Invalid domain/range values.")
            root.destroy()
            return

        self.points = []
        self.dragging_idx = None
        self.grid_divisions = 5

        # UI
        ctrl = tk.Frame(root)
        ctrl.pack(side="top", fill="x", padx=6, pady=6)
        tk.Button(ctrl, text="Import", command=self.import_points).pack(side="left")
        tk.Button(ctrl, text="Export", command=self.export_points).pack(side="left")
        tk.Button(ctrl, text="Clear", command=self.clear_all).pack(side="left")
        tk.Button(ctrl, text="–", command=self.decrease_grid).pack(side="left", padx=(10, 0))
        tk.Button(ctrl, text="+", command=self.increase_grid).pack(side="left")
        tk.Label(ctrl, text="  Left=add | Right=drag | Middle=delete").pack(side="left")

        self.canvas = tk.Canvas(root, bg="white")
        self.canvas.pack(fill="both", expand=True)
        self.canvas.bind("<Button-1>", self.on_left_click)
        self.canvas.bind("<Button-2>", self.on_middle_click)
        self.canvas.bind("<Button-3>", self.on_right_press)
        self.canvas.bind("<B3-Motion>", self.on_right_drag)
        self.canvas.bind("<ButtonRelease-3>", self.on_right_release)
        self.canvas.bind("<Motion>", self.on_motion)
        self.canvas.bind("<Configure>", lambda e: self.draw())

        self.status = tk.StringVar(value="Ready")
        tk.Label(root, textvariable=self.status, anchor="w").pack(side="bottom", fill="x")

        self.draw()

    # === Helper ===
    def ask_float(self, prompt, default):
        while True:
            s = simpledialog.askstring("Setup", prompt, initialvalue=str(default))
            if s is None:
                return default
            try:
                return float(s)
            except ValueError:
                messagebox.showerror("Invalid input", "Please enter a number.")

    # === Coordinate mapping ===
    def data_to_canvas(self, x, y):
        w, h = self.get_canvas_size()
        px = PADDING + (x - self.xmin) / (self.xmax - self.xmin) * (w - 2 * PADDING)
        py = PADDING + (1 - (y - self.ymin) / (self.ymax - self.ymin)) * (h - 2 * PADDING)
        return px, py

    def canvas_to_data(self, px, py):
        w, h = self.get_canvas_size()
        x = self.xmin + (px - PADDING) / (w - 2 * PADDING) * (self.xmax - self.xmin)
        y = self.ymin + (1 - (py - PADDING) / (h - 2 * PADDING)) * (self.ymax - self.ymin)
        return x, y

    def get_canvas_size(self):
        return max(self.canvas.winfo_width(), 200), max(self.canvas.winfo_height(), 200)

    # === Mouse actions ===
    def on_left_click(self, event):
        x, y = self.canvas_to_data(event.x, event.y)
        self.points.append((x, y))
        self.draw()

    def on_right_press(self, event):
        if not self.points:
            return
        idx = self.find_nearest_point(event.x, event.y)
        if idx is not None and self.distance_to_canvas_point(idx, event.x, event.y) <= HIT_RADIUS:
            self.dragging_idx = idx

    def on_right_drag(self, event):
        if self.dragging_idx is None:
            return
        x, y = self.canvas_to_data(event.x, event.y)
        x = max(self.xmin, min(self.xmax, x))
        y = max(self.ymin, min(self.ymax, y))
        self.points[self.dragging_idx] = (x, y)
        self.draw()

    def on_right_release(self, event):
        self.dragging_idx = None

    def on_middle_click(self, event):
        if not self.points:
            return
        idx = self.find_nearest_point(event.x, event.y)
        if idx is not None and self.distance_to_canvas_point(idx, event.x, event.y) <= HIT_RADIUS:
            del self.points[idx]
            self.draw()

    def on_motion(self, event):
        x, y = self.canvas_to_data(event.x, event.y)
        self.status.set(f"Mouse: x={x:.4g}, y={y:.4g}  |  Points: {len(self.points)}")

    # === Utility ===
    def distance_to_canvas_point(self, idx, cx, cy):
        px, py = self.data_to_canvas(*self.points[idx])
        return math.hypot(px - cx, py - cy)

    def find_nearest_point(self, cx, cy):
        if not self.points:
            return None
        nearest_idx, nearest_d2 = None, None
        for i, (x, y) in enumerate(self.points):
            px, py = self.data_to_canvas(x, y)
            d2 = (px - cx) ** 2 + (py - cy) ** 2
            if nearest_d2 is None or d2 < nearest_d2:
                nearest_idx, nearest_d2 = i, d2
        return nearest_idx

    # === Grid controls ===
    def increase_grid(self):
        self.grid_divisions = min(20, self.grid_divisions + 1)
        self.draw()

    def decrease_grid(self):
        self.grid_divisions = max(1, self.grid_divisions - 1)
        self.draw()

    # === Drawing ===
    def draw_axes(self):
        w, h = self.get_canvas_size()
        self.canvas.create_rectangle(PADDING, PADDING, w - PADDING, h - PADDING, outline="#ccc")
        for i in range(self.grid_divisions + 1):
            t = i / self.grid_divisions
            # vertical
            px = PADDING + t * (w - 2 * PADDING)
            self.canvas.create_line(px, PADDING, px, h - PADDING, fill="#eee")
            xv = self.xmin + t * (self.xmax - self.xmin)
            self.canvas.create_text(px, h - PADDING + 14, text=f"{xv:.2f}", anchor="n", font=("TkDefaultFont", 8))
            # horizontal
            py = PADDING + t * (h - 2 * PADDING)
            self.canvas.create_line(PADDING, py, w - PADDING, py, fill="#eee")
            yv = self.ymax - t * (self.ymax - self.ymin)
            self.canvas.create_text(PADDING - 6, py, text=f"{yv:.2f}", anchor="e", font=("TkDefaultFont", 8))

    def draw(self):
        self.canvas.delete("all")
        self.draw_axes()
        if not self.points:
            w, h = self.get_canvas_size()
            self.canvas.create_text(w/2, h/2, text="Left-click to add a point", fill="#999", font=("TkDefaultFont", 14))
            return
        pts_sorted = sorted(self.points, key=lambda p: p[0])
        coords = [self.data_to_canvas(x, y) for x, y in pts_sorted]
        for i in range(len(coords) - 1):
            self.canvas.create_line(*coords[i], *coords[i + 1], width=2)
        for (x, y) in coords:
            self.canvas.create_oval(x - POINT_RADIUS, y - POINT_RADIUS, x + POINT_RADIUS, y + POINT_RADIUS,
                                    fill="black", outline="white")

    # === File I/O ===
    def export_points(self):
        if not self.points:
            messagebox.showinfo("Export", "No points to export.")
            return
        path = filedialog.asksaveasfilename(defaultextension=".xy",
                                            filetypes=[("XY files", "*.xy"), ("Text files", "*.txt"), ("All files", "*.*")])
        if not path:
            return
        pts_sorted = sorted(self.points, key=lambda p: p[0])
        try:
            with open(path, "w") as f:
                f.write("# x y (space separated)\n")
                for x, y in pts_sorted:
                    f.write(f"{x:.8f} {y:.8f}\n")
            messagebox.showinfo("Exported", f"Saved {len(pts_sorted)} points to:\n{path}")
        except Exception as e:
            messagebox.showerror("Error", str(e))

    def import_points(self):
        path = filedialog.askopenfilename(filetypes=[("XY files", "*.xy"), ("Text files", "*.txt"), ("All files", "*.*")])
        if not path:
            return
        try:
            new_points = []
            with open(path, "r") as f:
                for line in f:
                    line = line.strip()
                    if not line or line.startswith("#"):
                        continue
                    parts = line.split()
                    if len(parts) != 2:
                        continue
                    try:
                        x, y = float(parts[0]), float(parts[1])
                        new_points.append((x, y))
                    except ValueError:
                        continue
            if not new_points:
                messagebox.showwarning("Import", "No valid points found in file.")
                return
            if self.points and not messagebox.askyesno("Replace?", "Replace existing points with imported ones?"):
                return
            self.points = new_points
            self.draw()
            messagebox.showinfo("Imported", f"Loaded {len(new_points)} points.")
        except Exception as e:
            messagebox.showerror("Error", f"Failed to import: {e}")

    def clear_all(self):
        if messagebox.askyesno("Clear", "Remove all points?"):
            self.points.clear()
            self.draw()


if __name__ == "__main__":
    root = tk.Tk()
    app = XYEditor(root)
    root.mainloop()

