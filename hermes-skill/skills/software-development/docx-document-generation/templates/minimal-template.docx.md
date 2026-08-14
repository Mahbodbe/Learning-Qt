---
title: Minimal Valid DOCX Template
description: Bare-bones valid DOCX for template-based generation
---

# Minimal Valid DOCX Template

## Usage

```bash
# 1. Unzip this template
unzip -o minimal-template.docx -d template_dir/

# 2. Replace word/document.xml with your content
cp my-content.xml template_dir/word/document.xml

# 3. Optionally modify styles.xml
# cp my-styles.xml template_dir/word/styles.xml

# 4. Re-zip
cd template_dir && zip -r ../output.docx .

# 5. Verify
python3 scripts/verify-docx.py output.docx
```

## What This Template Provides

| File | Status |
|------|--------|
| `[Content_Types].xml` | ✅ Complete |
| `_rels/.rels` | ✅ Complete |
| `docProps/core.xml` | ✅ With placeholder author |
| `docProps/app.xml` | ✅ Standard |
| `docProps/custom.xml` | ✅ Empty placeholder |
| `_rels/.rels` | ✅ Package relationships |
| `word/document.xml` | ✅ Minimal body with Normal style |
| `word/styles.xml` | ✅ Normal + Heading1-3 + Title + Subtitle + List Bullet |
| `word/numbering.xml` | ✅ Bullet numbering (numId=1) |
| `word/theme/theme1.xml` | ✅ Warm family color scheme |
| `word/settings.xml` | ✅ Standard |
| `word/webSettings.xml` | ✅ Standard |
| `word/fontTable.xml` | ✅ B Nazanin + Times New Roman |
| `word/_rels/document.xml.rels` | ✅ Standard |
| `word/settings.xml` | ✅ Standard |
| `word/webSettings.xml` | ✅ Standard |

## Style Definitions Included

| Style ID | Name | Properties |
|----------|------|------------|
| Normal | Normal | B Nazanin, 11pt, RTL |
| Title | Title | B Nazanin, 20pt, Bold, #365F91, Center |
| Subtitle | Subtitle | B Nazanin, 12pt, #8B7355, Center |
| Heading1 | heading 1 | B Nazanin, 14pt, Bold, #365F91 |
| Heading2 | heading 2 | B Nazanin, 13pt, Bold, #4F81BD |
| Heading3 | heading 3 | B Nazanin, 12pt, Bold, #4F81BD |
| List Bullet | List Bullet | Bullet ●, 10.5pt, #A0522D |

## Theme Colors

| Name | Hex |
|------|-----|
| dk1 (dark text) | 3D2B1F |
| lt1 (light bg) | FFFAF0 |
| dk2 (dark accent) | 5D3A1A |
| lt2 (light accent) | F5DEB3 |
| accent1 | 8B4513 (Saddle Brown) |
| accent2 | A0522D (Sienna) |
| accent3 | CD853F (Peru) |
| accent4 | D2691E (Chocolate) |
| accent5 | B8860B (Dark Goldenrod) |
| accent6 | 8B7355 (Rosy Brown) |

## Usage Notes

1. **Font**: Uses `B Nazanin` via theme fonts (`majorHAnsi`/`majorEastAsia`/`minorHAnsi`/`minorEastAsia`) — user can change globally in Word
2. **RTL**: Normal style has `w:bidi w:val="true"` in paragraph and run properties
6. **Bullets**: Use `w:numPr` with `numId="1"` for bullets
7. **Colors**: Use theme colors (`w:themeColor="accent1"`) for consistency

## Customization Points

| To Change | Edit File |
|-----------|-----------|
| Font family | `word/theme/theme1.xml` (majorFont/minorFont) |
| Colors | `word/theme/theme1.xml` (clrScheme) |
| Styles | `word/styles.xml` |
| Default paragraph | `styles.xml` Normal style |
| Bullets | `word/numbering.xml` |

## Verification

```bash
python3 scripts/verify-docx.py output.docx
```

Expected output:
```
✅ All essential files present
✅ Document has 247 text fragments
✅ B Nazanin font present
✅ Blue headings present
✅ RTL (bidi) markers present
✅ numbering.xml present (bullets work)
```

---

*Generated for docx-document-generation skill*