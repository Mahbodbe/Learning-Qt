---
title: Warm Family Color Palette
description: Warm, family-friendly color palette for docx documents
---

# Warm Family Color Palette

## Primary Colors (Heading Hierarchy)

| Role | Hex | RGB | Usage |
|------|-----|-----|-------|
| **Title / Heading 1** | `#365F91` | 54, 95, 145 | Main title, Heading 1 |
| **Heading 2** | `#4F81BD` | 79, 129, 189 | Heading 2 |
| **Heading 3** | `#365F91` | 54, 95, 145 | Heading 3 (same as H1 but smaller) |
| **Heading 4** | `#4F81BD` | 79, 129, 189 | Heading 4 (same as H2 but smaller) |
| **Subtitle / Muted** | `#8B7355` | 139, 115, 85 | Subtitles, muted text |

## Warm Accent Colors (Body Text)

| Name | Hex | Usage |
|------|-----|-------|
| Saddle Brown | `#8B4513` | Bullet titles, bold leads |
| Peru | `#CD853F` | Sub-bullets, secondary highlights |
| Chocolate | `#D2691E` | Accent text |
| Dark Goldenrod | `#B8860B` | Warning/important text |
| Rosy Brown | `#8B7355` | Muted text, footers |

## Background Fills (Tables)

| Purpose | Hex | Theme Color |
|---------|-----|-------------|
| Header row | `4F81BD` | accent1 |
| Alt rows (odd) | `DBE5F1` | accent1 tint 19% |
| Even rows | `FFFFFF` | background1 |

## Theme Color Mapping (theme1.xml)

```xml
<a:clrScheme name="Warm Family">
  <a:dk1><a:srgbClr val="3D2B1F"/></a:dk1>      <!-- Dark text -->
  <a:lt1><a:srgbClr val="FFFAF0"/></a:lt1>      <!-- Light background -->
  <a:dk2><a:srgbClr val="5D3A1A"/></a:dk2>      <!-- Dark accent -->
  <a:lt2><a:srgbClr val="F5DEB3"/></a:lt2>      <!-- Light accent -->
  <a:accent1><a:srgbClr val="8B4513"/></a:accent1>  <!-- Primary accent -->
  <a:accent2><a:srgbClr val="A0522D"/></a:accent2>  <!-- Secondary -->
  <a:accent3><a:srgbClr val="CD853F"/></a:accent3>  <!-- Tertiary -->
  <a:accent4><a:srgbClr val="D2691E"/></a:accent4>  <!-- Quaternary -->
  <a:accent5><a:srgbClr val="B8860B"/></a:accent5>  <!-- Quinary -->
  <a:accent6><a:srgbClr val="8B7355"/></a:accent6>  <!-- Senary -->
  <a:hlink><a:srgbClr val="A0522D"/></a:hlink>        <!-- Hyperlink -->
  <a:folHlink><a:srgbClr val="8B4513"/></a:folHlink>  <!-- Followed hyperlink -->
</a:clrScheme>
```

## Font Scheme (theme1.xml)

```xml
<a:fontScheme name="Persian Warm">
  <a:majorFont>
    <a:latin typeface="B Nazanin"/>
    <a:ea typeface="B Nazanin"/>
    <a:cs typeface="B Nazanin"/>
  </a:majorFont>
  <a:minorFont>
    <a:latin typeface="B Nazanin"/>
    <a:ea typeface="B Nazanin"/>
    <a:cs typeface="B Nazanin"/>
  </a:minorFont>
</a:fontScheme>
```

## Usage in Styles

```xml
<!-- Heading 1 -->
<w:rPr>
  <w:rFonts w:asciiTheme="majorHAnsi" w:eastAsiaTheme="majorEastAsia" w:hAnsiTheme="majorHAnsi" w:cstheme="majorBidi"/>
  <w:b/>
  <w:color w:val="365F91" w:themeColor="accent1" w:themeShade="BF"/>
  <w:sz w:val="28"/>
</w:rPr>

<!-- Heading 2 -->
<w:rPr>
  <w:rFonts w:asciiTheme="majorHAnsi" w:eastAsiaTheme="majorEastAsia" w:hAnsiTheme="majorHAnsi" w:cstheme="majorBidi"/>
  <w:b/>
  <w:color w:val="4F81BD" w:themeColor="accent1"/>
  <w:sz w:val="26"/>
</w:rPr>

<!-- Bullet lead text -->
<w:rPr>
  <w:rFonts w:ascii="B Nazanin" w:hAnsi="B Nazanin" w:eastAsia="B Nazanin"/>
  <w:b/>
  <w:color w:val="A0522D"/>
  <w:sz w:val="21"/>
</w:rPr>
```

## CSS/HTML Equivalent (for reference)

```css
:root {
  --title-blue: #365F91;
  --heading-blue: #4F81BD;
  --subtitle-brown: #8B7355;
  --bullet-brown: #8B4513;
  --accent-amber: #CD853F;
  --warm-gold: #B8860B;
  --muted-brown: #8B7355;
}
```

## Notes

- **Blue tones** for structural headings (trust, technical)
- **Brown/gold tones** for content/body (warmth, approachability)
- **Never use pure black** (`#000000`) for text - use `#3D2B1F` (dark brown)
- **Never use pure white** for backgrounds - use `#FFFAF0` (warm off-white)
- **Bullet colors** should match heading hierarchy colors