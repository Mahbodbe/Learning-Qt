---
title: Persian RTL Formatting Guide for DOCX
description: Best practices for RTL Persian/Arabic content in DOCX
---

# Persian/Arabic RTL Formatting in DOCX

## Essential RTL Attributes

### Paragraph Level (w:pPr)
```xml
<w:pPr>
  <w:bidi w:val="true"/>     <!-- RTL paragraph direction -->
  <w:jc w:val="center"/>     <!-- Center alignment -->
</w:pPr>
```

### Run Level (w:rPr)
```xml
<w:rPr>
  <w:bidi w:val="true"/>     <!-- RTL run direction -->
  <w:rFonts w:ascii="B Nazanin" w:hAnsi="B Nazanin" w:eastAsia="B Nazanin" w:cs="B Nazanin"/>
  <w:sz w:val="22"/>         <!-- font size in half-points (11pt = 22) -->
  <w:color w:val="8B4513"/>  <!-- optional color -->
</w:rPr>
```

### Section Level
```xml
<w:sectPr>
  <w:bidi w:val="true"/>     <!-- RTL section direction -->
</w:sectPr>
```

## Persian Font Stack

| Font | Best For | Notes |
|------|----------|-------|
| B Nazanin | Body text, headings | Classic Persian font |
| IranSans | Modern UI, body | Clean, modern |
| Vazirmatn | Code, UI | Developer-friendly |
| IranNastaliq | Calligraphy | Artistic only |

**Font specification in styles.xml:**
```xml
<w:rPr>
  <w:rFonts w:ascii="B Nazanin" w:hAnsi="B Nazanin" w:eastAsia="B Nazanin" w:cs="B Nazanin"/>
</w:rPr>
```

**Or use theme fonts (user-changeable):**
```xml
<w:rFonts w:asciiTheme="majorHAnsi" w:eastAsiaTheme="majorEastAsia" w:hAnsiTheme="majorHAnsi" w:cstheme="majorBidi"/>
```

## Color Palette (Warm Family Theme)

| Color Name | Hex | Usage |
|------------|-----|-------|
| Dark Brown | `5D3A1F` | Main headings, Title |
| Medium Brown | `8B4513` | Heading 1 |
| Saddle Brown | `A0522D` | Heading 2, bullet leads |
| Peru | `CD853F` | Heading 3 |
| Chocolate | `D2691E` | Accent 4 |
| Dark Goldenrod | `B8860B` | Accent 5 |
| Rosy Brown | `8B7355` | Accent 6, subtitles |
| Sandy Brown | `F4A460` | Light accents |

## List Numbering (Bullets)

```xml
<!-- In numbering.xml -->
<w:abstractNum w:abstractNumId="0">
  <w:lvl w:ilvl="0">
    <w:numFmt w:val="bullet"/>
    <w:lvlText w:val="●"/>
    <w:rPr>
      <w:rFonts w:ascii="B Nazanin" w:hAnsi="B Nazanin" w:eastAsia="B Nazanin"/>
      <w:color w:val="A0522D"/>
      <w:sz w:val="22"/>
    </w:rPr>
  </w:lvl>
</w:abstractNum>
<w:num w:numId="1">
  <w:abstractNumId w:val="0"/>
</w:num>
```

Usage in document.xml:
```xml
<w:p>
  <w:pPr>
    <w:numPr><w:ilvl w:val="0"/><w:numId w:val="1"/></w:numPr>
  </w:pPr>
  <w:r><w:rPr><w:bidi w:val="true"/><w:rFonts w:ascii="B Nazanin" w:hAnsi="B Nazanin" w:eastAsia="B Nazanin"/><w:b/><w:color w:val="A0522D"/><w:sz w:val="21"/></w:rPr>
  <w:t>● عنوان</w:t>
  </w:r>
  <w:r><w:rPr><w:bidi w:val="true"/><w:rFonts w:ascii="B Nazanin" w:hAnsi="B Nazanin" w:eastAsia="B Nazanin"/><w:sz w:val="21"/></w:rPr>
  <w:t xml:space="preserve">متن توضیح</w:t></w:r>
</w:p>
```

## Table Styling

```xml
<w:tbl>
  <w:tblPr>
    <w:tblStyle w:val="TableGrid"/>
    <w:tblW w:w="100%" w:type="pct"/>
  </w:tblPr>
  <w:tr>
    <w:tc><w:tcPr><w:shd w:fill="4F81BD"/></w:tcPr>
      <w:p><w:pPr><w:jc w:val="center"/><w:rPr><w:b/><w:color w:val="FFFFFF"/></w:rPr></w:pPr>
      <w:r><w:t>سرستون</w:t></w:r></w:p></w:tc>
  </w:tr>
</w:tbl>
```

## Font Size Mapping

| Points | w:val (half-points) |
|--------|---------------------|
| 8pt | 16 |
| 9pt | 18 |
| 10pt | 20 |
| 11pt | 22 |
| 12pt | 24 |
| 14pt | 28 |
| 16pt | 32 |
| 18pt | 36 |
| 20pt | 40 |
| 24pt | 48 |
| 28pt | 56 |
| 32pt | 64 |
| 36pt | 72 |
| 40pt | 80 |

## Common Mistakes to Avoid

| Mistake | Fix |
|---------|-----|
| Missing `w:bidi` on runs | Add `w:bidi w:val="true"` to every `<w:rPr>` |
| Using `w:jc="right"` for RTL | Use `w:jc="right"` for RTL text alignment |
| Hardcoding font names | Use theme fonts or allow user override |
| Forgetting `w:bidi` on section | Add `<w:bidi w:val="true"/>` to `<w:sectPr>` |
| Missing `xml:space="preserve"` | Add to `<w:t>` for leading/trailing spaces |

## Quick Validation Checklist

- [ ] All `<w:rPr>` have `<w:bidi w:val="true"/>`
- [ ] Section has `<w:sectPr><w:bidi w:val="true"/></w:sectPr>`
- [ ] Fonts specified as B Nazanin (or theme fonts)
- [ ] RTL paragraphs use `w:jc w:val="right"` or `center`
- [ ] Bullet lists reference `numbering.xml` (numId="1")
- [ ] `numbering.xml` present in package
- [ ] Colors use hex values or theme colors
- [ ] Persian digits (۱۲۳۴۵۶۷۸۹۰) used in content