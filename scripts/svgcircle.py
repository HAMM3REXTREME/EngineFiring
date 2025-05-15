import math

def generate_svg_dial(
    total_steps=60,
    longer_tick_line_every=5,
    text_label_every=10,
    radius=100,
    tick_length=5,
    longer_tick_length=10,
    label_radius=80,
    center_x=150,
    center_y=150,
):
    svg_elements = []
    svg_elements.append(f'<svg xmlns="http://www.w3.org/2000/svg" width="{center_x*2}" height="{center_y*2}">')

    for step in range(total_steps):
        angle = 2 * math.pi * step / total_steps
        sin_a = math.sin(angle)
        cos_a = math.cos(angle)

        is_long = (step % longer_tick_line_every == 0)
        tick_len = longer_tick_length if is_long else tick_length

        x1 = center_x + (radius - tick_len) * sin_a
        y1 = center_y - (radius - tick_len) * cos_a
        x2 = center_x + radius * sin_a
        y2 = center_y - radius * cos_a

        svg_elements.append(f'<line x1="{x1:.2f}" y1="{y1:.2f}" x2="{x2:.2f}" y2="{y2:.2f}" stroke="black"/>')

        if step % text_label_every == 0:
            label_x = center_x + label_radius * sin_a
            label_y = center_y - label_radius * cos_a
            label = str(step)
            svg_elements.append(
                f'<text x="{label_x:.2f}" y="{label_y:.2f}" text-anchor="middle" dominant-baseline="middle" font-size="10">{label}</text>'
            )

    svg_elements.append("</svg>")
    return "\n".join(svg_elements)


# Example usage
with open("dial.svg", "w") as f:
    f.write(generate_svg_dial(
        total_steps=100,
        longer_tick_line_every=5,
        text_label_every=10
    ))

