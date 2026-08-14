# PIL Architecture Diagram Recipe (proven 2026-08-01, SCADA diagrams)

## When
Need a professional layered architecture schematic (Purdue-model style bands, boxes, labeled arrows) on a VPS where matplotlib/graphviz are NOT installed. PIL (Pillow) IS available in the hermes venv.

## Recipe
1. Canvas: `Image.new("RGB", (1500, ~2100), "#FAFBFC")`, DejaVu fonts from `/usr/share/fonts/truetype/dejavu/DejaVuSans[-Bold].ttf` (always installed).
2. Horizontal color bands per level: pastel fill + darker outline per tier
   (purple #EDE7F6/#5E4B8B, blue #E3F2FD/#1E5C8F, green #E8F5E9/#2E7D4F, yellow #FFF8E1/#9A7B1E, red #FFEBEE/#A94442).
3. Helper functions: `rbox()` rounded_rectangle, `box(cx,cy,w,h,title,subs)` centered title + sub-lines, `arrow()` with bidirectional heads + white-chip label (draw label rect over the line so text stays readable).
4. **Band label placement:** anchor band names at the RIGHT edge (`anchor="ra"`, x=W-60) — left-anchored labels collide with left-column boxes.
5. **VERIFY WITH VISION after every render** — `vision_analyze` the PNG asking specifically about "overlapping text, boxes outside bands, arrow labels readable". First render almost always has 1-2 collisions (title/body overlap in a box, boxes overflowing band bottom edge, band header hit by arrow label). Iterate until clean. Typical fixes:
   - box title 2 lines → increase box h and start body text lower
   - boxes overflow band → recompute band height = box_h + margins, keep box cy = band_y + band_h/2 + header_offset
   - arrow label collides with band header → shorten arrow or move label midpoint
6. Publish: copy PNG to /var/www/resin-media/ + chmod 644; embed in report markdown as `![caption](/tmp/name.png)` BEFORE pandoc so it lands inside the docx.

## Pitfalls
- matplotlib not installed on this VPS and pip times out — do NOT try to install; PIL is enough.
- Don't add stray imports when re-running an edited script (one crash came from a leftover `import matplotlib`).
- Persian text in PIL/DejaVu renders unshaped — keep diagram text ENGLISH; the surrounding report carries the Persian.
