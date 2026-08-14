---
name: docx-document-generation
description: Generate and manipulate Microsoft Word (.docx) documents programmatically with proper structure, styling, and RTL support for Persian content.
version: 2.0.0
author: Hermes Agent
tags: [docx, word, document-generation, rtl, persian, xml-manipulation, template]
---

# DOCX Document Generation Skill

## Overview
Generate and modify Microsoft Word (.docx) documents programmatically. Covers proper docx structure, RTL/Persian support, template-based generation, and the critical pitfalls that cause Word to reject generated files.

## The ONLY Reliable Workflow
**Never build docx XML from scratch. Always start from a valid template.**

### Primary Path: python-docx API
When python-docx is available and doesn't time out:
```python
from docx import Document
doc = Document("valid_template.docx")
# Clear existing content
for p in list(doc.paragraphs):
    p._element.getparent().remove(p._element)
# Add content using template's existing styles
doc.add_heading("عنوان", level=1)
doc.add_paragraph("متن", style='List Bullet')
doc.save("output.docx")
```

### Fallback Path: Template XML Replacement (when python-docx times out or is unavailable)
On low-RAM VPSs (~1GB), python-docx installation frequently times out. Use this instead:

```bash
# 1. Get a valid template (user's existing docx)
cp user_template.docx template.docx

# 2. Extract ALL files
mkdir tmpl && cd tmpl
unzip -o ../template.docx

# 3. Replace ONLY document.xml with new content
cp new_content.xml word/document.xml
# Optionally update word/styles.xml for styling changes

# 4. Re-zip preserving ALL files
cd tmpl && zip -r ../output.docx .
```

### Essential Files That MUST Be Preserved
Every valid docx requires these (copy them from the template, never create from scratch):

| File | Purpose | Critical |
|------|---------|----------|
| `[Content_Types].xml` | MIME types for all parts | ✅ |
| `_rels/.rels` | Package relationships | ✅ |
| `word/_rels/document.xml.rels` | Document relationships | ✅ |
| `word/document.xml` | Document content (replace this) | ✅ |
| `word/styles.xml` | Paragraph/character styles | ✅ |
| `word/theme/theme1.xml` | Theme colors and fonts | ✅ |
| `word/settings.xml` | Document settings | ✅ |
| `word/fontTable.xml` | Font definitions | ✅ |
| `word/numbering.xml` | Bullet/numbering lists | ✅ |
| `word/webSettings.xml` | Web-related settings | ✅ |
| `docProps/core.xml` | Document metadata | ✅ |
| `docProps/app.xml` | Application metadata | ✅ |
| `docProps/custom.xml` | Custom properties (if present) | ✅ |

## Node.js `docx` Library (npm)

### Installation
```bash
npm install docx
```

### Multi-File Report Structure (Large Documents)

For reports with 6+ sections, use a modular structure to keep code manageable:

```
docx_render.js          # Shared helpers: h1(), h2(), p(), bulletList(), dataTable(), codeBlock(), referencesBlock(), pageBreak(), color constants
build_report_part1.js   # Title page, TOC, intro section (exports: titlePage, tocSection, introSection)
build_report_part2.js   # Section 1 (exports: section1)
...
main.js                 # Requires all parts, assembles Document, packs to buffer
```

**docx_render.js structure:**
```js
const { Paragraph, TextRun, AlignmentType, ExternalHyperlink, PageBreak, ShadingType, WidthType, Table, TableRow, TableCell, BorderStyle, UnderlineType } = require("docx");

const NAVY = "1F4E79";
const AMBER = "BF5700";
const MUTED = "666666";
const TEXT = "222222";
const HEAD = "Calibri";
const BODY = "Calibri";

// Helper functions
function h1(text) { return new Paragraph({ heading: HeadingLevel.HEADING_1, ... }) }
function h2(text) { ... }
function p(text, opts) { ... }
function bulletList(items) { ... }
function pageBreak() { ... }
function referencesBlock(title, items) { ... }
function dataTable(headers, rows, widths) { ... }
function codeBlock(lines) { ... }

module.exports = { h1, h2, p, bulletList, pageBreak, referencesBlock, dataTable, codeBlock, NAVY, AMBER, MUTED, TEXT, HEAD, BODY };
```

### RTL Support for Persian/Arabic in Node.js `docx`

**RTL BODY PARAGRAPH — works with `bidirectional: true` + `AlignmentType.RIGHT` + `rightToLeft: true` on TextRuns:**
```js
new Paragraph({
  bidirectional: true,
  alignment: AlignmentType.RIGHT,
  spacing: { after: 160, line: 300 },
  children: [
    new TextRun({ text: "متن فارسی", font: "Tahoma", size: 22, rightToLeft: true }),
  ],
});
```

**RTL HEADING — CRITICAL PITFALL: heading number floats LEFT when alignment not set**
```js
// ✅ CORRECT — heading number stays right
new Paragraph({
  heading: "Heading1",           // Use STRING, not HeadingLevel constant
  bidirectional: true,
  alignment: AlignmentType.RIGHT,
  spacing: { before: 360, after: 200 },
  children: [
    new TextRun({ text: "عنوان فارسی", bold: true, color: "1F4E79", font: "Calibri", size: 32, rightToLeft: true }),
  ],
});

// ❌ WRONG — heading number floats to LEFT
new Paragraph({
  heading: "Heading1",
  // Missing: bidirectional: true
  // Missing: alignment: AlignmentType.RIGHT
  children: [ /* ... */ ],
});
```

**RULE: ALWAYS set ALL THREE for RTL headings:**
1. `heading: "Heading1"` (string literal, not `HeadingLevel.HEADING_1` constant — both work but be consistent)
2. `bidirectional: true`
3. `alignment: AlignmentType.RIGHT`
4. Plus `rightToLeft: true` on every `TextRun` inside the heading

### Page Count Discrepancy Between English and Persian
When generating bilingual reports (EN + FA), the Persian version will have fewer pages even with the same paragraph count because:
- Persian script is more compact (connected Arabic script takes less horizontal space)
- Persian doesn't capitalise headings (no extra upstroke space)
- To get the same page count, the Persian version needs ~15-20% more content or larger font sizes

**Verification:** Compare raw XML character counts between the two .docx files:
```python
import zipfile
en = zipfile.ZipFile('en.docx').read('word/document.xml')
fa = zipfile.ZipFile('fa.docx').read('word/document.xml')
print(f"Ratio: {len(fa)/len(en):.2f}x")
```

### RTL Bullet Lists
```js
function faBullet(items) {
  return items.map(it => new Paragraph({
    bidirectional: true,
    alignment: AlignmentType.RIGHT,
    spacing: { after: 80 },
    bullet: { level: 0 },
    children: [
      new TextRun({ text: it.lead || "", font: BODY, size: 22, bold: true, color: NAVY, rightToLeft: true }),
      new TextRun({ text: it.text || "", font: BODY, size: 22, rightToLeft: true }),
    ],
  }));
}
```

### Required XML attributes (for template-based XML approach)
Apply `w:bidi w:val="true"` on ALL of these:
- **Paragraph properties** (`w:pPr`)
- **Run properties** (`w:rPr`) 
- **Section properties** (`w:sectPr`)

### Paragraph structure (RTL, Persian text)
```xml
<w:p>
  <w:pPr>
    <w:pStyle w:val="Normal"/>
    <w:rPr><w:bidi w:val="true"/></w:rPr>
  </w:pPr>
  <w:r>
    <w:rPr>
      <w:bidi w:val="true"/>
      <w:sz w:val="22"/>
    </w:rPr>
    <w:t xml:space="preserve">متن فارسی</w:t>
  </w:r>
</w:p>
```

### Bullet Points (numbered list)
```xml
<w:p>
  <w:pPr>
    <w:numPr><w:ilvl w:val="0"/><w:numId w:val="1"/></w:numPr>
    <w:rPr><w:bidi w:val="true"/></w:rPr>
  </w:pPr>
  <w:r>
    <w:rPr><w:bidi w:val="true"/><w:b/><w:sz w:val="21"/></w:rPr>
    <w:t xml:space="preserve">عنوان:</w:t>
  </w:r>
  <w:r>
    <w:rPr><w:bidi w:val="true"/><w:sz w:val="21"/></w:rPr>
    <w:t xml:space="preserve">توضیحات</w:t>
  </w:r>
</w:p>
```

## Font Handling — CRITICAL Rule
**Never hardcode fonts in styles.xml that prevent users from changing fonts later.**

User's exact words: "فونت رو بیخیال شو شاید اون داره خراب میکنه فقط حواست باشه طوری تنظیم نکنی که بعدا نتونم دستی خودم فونت رو عوض کنم"

Best practice cascade:
1. **Let the template's existing theme fonts stand** — Word's Design tab can override these globally
2. If a specific font is needed (e.g., B Nazanin for Persian): update `word/theme/theme1.xml`'s `a:majorFont`/`a:minorFont` `a:latin`/`a:ea` typeface attributes — this lets the user override via Design tab
3. **Last resort**: specify font in `word/styles.xml` with direct `w:ascii`/`w:hAnsi`/`w:eastAsia` attributes — but this makes per-paragraph override harder

## The #1 Golden Rule — Never Call `ET.tostring()` for Document Body

**This single mistake caused an entire session of failed docx generation.** Word silently rejects files where the internal document.xml was serialized via `xml.etree.ElementTree.tostring()`. The output looks valid (opens fine in editors) but Word shows "Word experienced an error trying to open".

**Why:** `ET.tostring()` rewrites namespace prefixes. What was `w:p` becomes `ns0:p` or `ns1:p`. The archive structure and Content_Types still point to the correct namespace URL, but Word's parser sees unreferenced prefix aliases and bails.

**The ONLY correct approach for building document.xml content:** use Python string f-strings, `.format()`, or simple string concatenation to construct the `<w:body>...</w:body>` portion. Copy the `w:document` opening tag *verbatim* from a working template (including all xmlns attributes), then only interpolate the body content.

## DOCX Generation Workflow (Correct Order)

1. **Get a KNOWN-WORKING template** (user's own file or a freshly-saved Word doc)
2. **Extract all files** via `unzip -o template.docx -d tmpdir/`
3. **Build the body XML as a Python string** (f-strings, NOT ElementTree)
4. **Replace only `tmpdir/word/document.xml`** with the new content
5. **Re-zip preserving ALL files** via `zip -r output.docx .`
6. **Verify with the script below**
7. **Send to user for testing in Word**

**If Word reports an error:** Do NOT try more XML patches. Revert to step 1 (fresh template copy), make simpler content. After 3 attempts, offer PDF.

## Verification Checklist

Before delivering any .docx, verify with this script:

```python
import zipfile, re
z = zipfile.ZipFile('output.docx')
essential = ['[Content_Types].xml', '_rels/.rels', 'word/document.xml', 
             'word/styles.xml', 'word/theme/theme1.xml', 'word/settings.xml', 
             'word/fontTable.xml', 'word/numbering.xml']
missing = [f for f in essential if f not in z.namelist()]
if missing:
    print(f"❌ MISSING: {missing}")
else:
    print("✅ All essential files present")
doc = z.read('word/document.xml').decode('utf-8')
texts = re.findall(r'<w:t[^>]*>[^<]+</w:t>', doc)
print(f"Text fragments: {len(texts)}")
# Check for ElementTree namespace corruption
if 'ns0:' in doc or 'ns1:' in doc or 'ns2:' in doc:
    print("❌ CORRUPTED: ElementTree-style namespace prefixes detected")
else:
    print("✅ No namespace corruption")
```

## Style & Font Guidelines for Persian DOCX

### The Font Rule (hard lesson from real session)

**Never hardcode fonts in a way that prevents the user from changing them later.** User's exact words: *"فونت رو بیخیال شو شاید اون داره خراب میکنه فقط حواست باشه طوری تنظیم نکنی که بعدا نتونم دستی خودم فونت رو عوض کنم"*

Best practice (ordered by preference):
1. **Let the template's existing theme fonts stand** — Word's Design > Fonts can override globally
2. **Update theme1.xml** for a custom default font (user can still change via Design tab):
   ```xml
   <a:majorFont>
     <a:latin typeface="B Nazanin"/>
     <a:ea typeface="B Nazanin"/>
   </a:majorFont>
   ```
3. **Direct w:rFonts in styles.xml** only as last resort (makes per-paragraph override harder)
4. **Never specify font at the run level** (`w:rPr > w:rFonts`) unless absolutely necessary for a specific inline style

### Color Palette (from working session)
User's project uses a warm neutral / earthy palette. Suggested heading colors:
- Title: `#345A8A` (dark blue)
- Heading1: `#365F91` (deep blue)
- Heading2/3: `#4F81BD` (medium blue)
- Body: default template black
- Bullet title leads: `#4F81BD` + bold

## RTL Support for Persian/Arabic

Required XML attributes — apply `w:bidi w:val="true"` on ALL of these:
- **Paragraph properties** (`w:pPr`)
- **Run properties** (`w:rPr`)
- **Section properties** (`w:sectPr`)

## Essential Files That MUST Be Preserved

| File | Purpose | Critical |
|------|---------|----------|
| `[Content_Types].xml` | MIME types for all parts | ✅ |
| `_rels/.rels` | Package relationships | ✅ |
| `word/_rels/document.xml.rels` | Document relationships | ✅ |
| `word/document.xml` | Document content (replace this) | ✅ |
| `word/styles.xml` | Paragraph/character styles | ✅ |
| `word/theme/theme1.xml` | Theme colors and fonts | ✅ |
| `word/settings.xml` | Document settings | ✅ |
| `word/fontTable.xml` | Font definitions | ✅ |
| `word/numbering.xml` | Bullet/numbering lists | ✅ |
| `word/webSettings.xml` | Web-rel settings | ✅ |
| `docProps/core.xml` | Document metadata | ✅ |
| `docProps/app.xml` | Application properties | ✅ |
| `docProps/custom.xml` | Custom properties | ✅ |

## Paragraph & Bullet XML Structures

### Paragraph (RTL, Persian)
```xml
<w:p>
  <w:pPr>
    <w:pStyle w:val="Normal"/>
    <w:rPr><w:bidi w:val="true"/></w:rPr>
  </w:pPr>
  <w:r>
    <w:rPr><w:bidi w:val="true"/><w:sz w:val="22"/></w:rPr>
    <w:t xml:space="preserve">متن فارسی</w:t>
  </w:r>
</w:p>
```

### Bullet with bold lead
```xml
<w:p>
  <w:pPr>
    <w:numPr><w:ilvl w:val="0"/><w:numId w:val="1"/></w:numPr>
    <w:rPr><w:bidi w:val="true"/></w:rPr>
  </w:pPr>
  <w:r>
    <w:rPr><w:bidi w:val="true"/><w:b/><w:color w:val="4F81BD"/><w:sz w:val="21"/></w:rPr>
    <w:t xml:space="preserve">عنوان:</w:t>
  </w:r>
  <w:r>
    <w:rPr><w:bidi w:val="true"/><w:sz w:val="21"/></w:rPr>
    <w:t xml:space="preserve">توضیحات</w:t>
  </w:r>
</w:p>
```

## Pitfalls & Anti-Patterns

| Pitfall | Why It Breaks | Solution |
|---------|--------------|----------|
| Building docx XML from scratch | Missing required internal parts; Word shows "error trying to open" | Always use a valid template |
| `ET.tostring()` for document.xml (`#1 WORST OFFENDER`) | Rewrites namespace prefixes (w: → ns0:, ns1:...). Word rejects silently | Build body with Python string f-strings; copy the `w:document` opening tag verbatim from template |
| Hardcoding fonts in styles.xml | User can't change font in Word's Design tab | Use theme fonts; update theme1.xml instead |
| Missing w:bidi on runs/paragraphs | RTL text renders left-to-right | Apply w:bidi on every w:pPr, w:rPr, w:sectPr |
| python-docx on ~1GB VPS | Times out during install | Use template-based XML replacement fallback |
| Attempting 4th docx fix without changing approach | Each attempt wastes time and frustrates user | After 3 attempts: STOP. Offer PDF instead. |
| Hand-crafting XML via `w:rFonts w:cs="B Nazanin"` only | Only sets complex-script font; may not apply theme fonts correctly | Set font in theme1.xml `a:majorFont` > `a:latin` typeface |

## Critical DOCX Failure Recovery

When user reports "Word experienced an error trying to open the file":

1. **Stop** adding more XML modifications. The problem is structural namespace corruption.
2. **Revert** to a KNOWN-GOOD file (user's original working docx or fresh copy).
3. **Only replace word/document.xml** with clean string-fmt content. Nothing else.
4. **Never use `ET.tostring()`**. Use Python string formatting.
5. **After 3 failed attempts: STOP and offer PDF.** The user would rather have a working PDF than keep trying docx. His own words: "هرکدوم کار میکنه یه چی میخوام کار کنه"
6. **PDF fallback**: `pandoc input.docx -o output.pdf --pdf-engine=xelatex -V mainfont=Amiri -V lang=fa`
7. **Cache-busting**: Add `?v=N` to download URLs to bypass browser/CDN cache
8. **Verify PDF page count** before delivery: `pdftoppm -png -r 150 file.pdf page && ls -la page*.png`

## Cover Page Format Requirement

**Page 1 MUST contain ONLY the exercise name** (e.g., "تمرین سوم" + "سوال دوم"). All actual content starts on Page 2. Implementation:

```python
# Cover page content
doc_parts.append(p('Title', 'تمرین سوم', bold=True, color='1F4E79', sz='48', center=True))
doc_parts.append(p('Subtitle', 'سوال دوم', bold=True, color='1F4E79', sz='36', center=True))
# ... university info ...
doc_parts.append('<w:p><w:pPr><w:pageBreakBefore/></w:pPr></w:p>')  # PAGE BREAK
# Then all content starts
```

## PDF Fallback & Verification

1. **3-strikes rule**: After 3 failed DOCX attempts → STOP, generate PDF
2. **Cache busting**: Append `?v=N` to download URLs
3. **Verify PDF pages**: `pdftoppm -png -r 150 file.pdf page && ls -la page*.png` - count pages
4. **Cache busting in URLs**: Add `?v=N` parameter

## Critical DOCX Failure Recovery

When user reports "Word experienced an error trying to open the file":

1. Stop adding more XML modifications. The problem is structural.
2. Revert to a KNOWN-GOOD file (user original working docx or fresh copy).
3. Only replace word/document.xml with clean content, nothing else.
4. Never use xml.etree.ElementTree.tostring() for document.xml. Use Python string formatting.
5. After 2-3 failed attempts: STOP and offer PDF. User would rather have working PDF than broken DOCX.
6. PDF fallback: pandoc input.docx -o output.pdf --pdf-engine=xelatex -V mainfont=Amiri -V lang=fa
