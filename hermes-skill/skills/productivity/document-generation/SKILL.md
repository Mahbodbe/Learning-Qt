---
name: document-generation
category: productivity
description: Generate Word (.docx), PDF, and other office documents programmatically with Python. RTL/Persian support, tables, images, formatting.
triggers:
  - user asks to create a report, document, or file in DOCX/Word/PDF format
  - user needs a structured document with Persian/Arabic text (RTL)
  - user needs bulk document generation from data
---

# Document Generation (python-docx)

## CRITICAL: pip timeout death (this VPS)
- `pip install python-docx` ALWAYS times out on this VPS (>60s network). Do NOT wait for it.
- The `resin-web` venv at `/root/resin-web/venv/bin/python` does NOT have python-docx installed (it failed to install in prior sessions too).
- **DO NOT waste time installing python-docx.** It always fails here. If pip starts in background, wait max 30s then kill and switch to the clone approach.
- **After 2+ failed attempts at opening a generated docx, IMMEDIATELY offer the user PDF instead.** The user's threshold is very low for this — repeated failures cause frustration escalation from "هنوزم درست نشده" to "ناموسا؟".

## Working approaches for DOCX generation (ordered by reliability)

### Approach 1 (MOST RELIABLE — used successfully): Clone document.xml into valid template
This is the ONLY method that has worked reliably for Persian/RTF docx output on this VPS:
1. **Get a known-working template**: Use `/var/www/resin-media/instr-bands-6-11-unified.docx` or the user's v2 file from their document cache at `/root/.hermes/cache/documents/doc_3824aeed554e_instr-bands-6-10-v2.docx`
2. **Extract the template**: `mkdir -p /tmp/build && cd /tmp/build && unzip -o /var/www/resin-media/instr-bands-6-11-unified.docx 2>/dev/null`
3. **Build new document.xml**: Write clean XML string with ALL required namespace declarations on the `<w:document>` element:
   ```xml
   <w:document xmlns:mc="http://schemas.openxmlformats.org/markup-compatibility/2006"
               xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"
               xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
               mc:Ignorable="w14">
   ```
4. **Replace document.xml**: `cat new_document.xml > /tmp/build/word/document.xml`
5. **Fix font references**: Run `sed` on `word/theme/theme1.xml` to replace Calibri/Cambria with B Nazanin:
   ```bash
   sed -i 's|<a:latin typeface="Calibri"|<a:latin typeface="B Nazanin"|g' word/theme/theme1.xml
   sed -i 's|<a:latin typeface="Cambria"|<a:latin typeface="B Nazanin"|g' word/theme/theme1.xml
   sed -i 's|<a:ea typeface=""|<a:ea typeface="B Nazanin"|g' word/theme/theme1.xml
   ```
6. **Fix styles.xml font references**: Replace theme-based font refs with direct B Nazanin:
   ```bash
   sed -i 's|w:asciiTheme="minorHAnsi" w:eastAsiaTheme="minorEastAsia" w:hAnsiTheme="minorHAnsi" w:cstheme="minorBidi"|w:ascii="B Nazanin" w:hAnsi="B Nazanin" w:eastAsia="B Nazanin" w:cs="B Nazanin"|g' word/styles.xml
   sed -i 's|w:asciiTheme="majorHAnsi" w:eastAsiaTheme="majorEastAsia" w:hAnsiTheme="majorHAnsi" w:cstheme="majorBidi"|w:ascii="B Nazanin" w:hAnsi="B Nazanin" w:eastAsia="B Nazanin" w:cs="B Nazanin"|g' word/styles.xml
   ```
   **CRITICAL**: Both `styles.xml` AND `theme1.xml` must have B Nazanin references, or Word will use fallback fonts. This is the #1 cause of "B Nazanin not showing" on user's machine.
7. **Re-zip cleanly**: `cd /tmp/build && zip -r /tmp/output.docx . 2>/dev/null`
8. **Verify** with python3 zipfile that ALL essential files exist:
   ```python
   ['[Content_Types].xml', '_rels/.rels', 'word/document.xml', 'word/styles.xml',
    'word/theme/theme1.xml', 'word/settings.xml', 'word/fontTable.xml', 'word/numbering.xml']
   ```
9. **Test**: `cp /tmp/output.docx /var/www/resin-media/output.docx`
10. **Also build PDF** as fallback: `pandoc output.docx -o output.pdf --pdf-engine=xelatex -V mainfont="Amiri" -V lang=fa`

### Approach 2: python-docx via resin-web venv (if pip works)
- Activate the resin-web venv: `cd /root/resin-web && source venv/bin/activate`
- Try installing: `pip install python-docx -q`
- If it takes >30s, kill it — it WILL timeout (happens ~70% of the time on this VPS)
- Activate the resin-web venv: `cd /root/resin-web && source venv/bin/activate`
- Try installing: `pip install python-docx`
- If it takes >30s, kill it — it WILL timeout (happens ~70% of the time on this VPS)
- If fast enough to install, use standard python-docx API (see RTL section below)

### Approach 3: Pandoc + xelatex PDF (fallback, works every time)
- **Pandoc + xelatex**: `pandoc input.docx -o output.pdf --pdf-engine=xelatex -V mainfont="Amiri" -V lang=fa`
- **Prerequisites**: `apt install texlive-xetex texlive-fonts-recommended texlive-lang-arabic`
- **Font limitation**: B Nazanin is NOT installed on this VPS. Use Amiri (installed) as fallback.
- **Known issue**: Unicode arrows (→) fail in Amiri. Result is readable but has glyph warnings.
- **When to use**: After 2 failed docx attempts or when the user says "هرکدوم کار میکنه" (whatever works).

### Approach 4: Raw XML zip (DO NOT USE — confirmed broken)
Building the ENTIRE docx as a raw XML zip from scratch with `zipfile.ZipFile` + `io.BytesIO`:
- **ALWAYS fails** with "Word experienced an error trying to open the file."
- The structure can never exactly match a real Word template without a reference file.
- Missing metadata, wrong namespace prefixes, broken relationships — any of these cause Word rejection.
- **Never use this approach.** Always start from a working template file.

## What DOES NOT WORK (been there, failed that)
- **`python-docx` from pip**: Always times out. Never got it installed this session.
- **Raw XML with custom namespaces**: Word rejects files with `'Word experienced an error trying to open the file.'` even when all essential XML files are present.
- **Copying document.xml from one docx to another's template**: Only works if the template's styles (Title, Subtitle, Heading1, Compact, etc.) are EXACTLY the styles referenced in document.xml. Document.xml from different versions will have style name mismatches.
- **Building with ET (ElementTree)**: Namespace prefix mangling causes XML corruption.

## When to give up and offer PDF
If 3+ attempts at DOCX generation fail or the user expresses frustration, STOP trying and offer a PDF instead. User's own words: "هرکدوم کار میکنه یه چی میخوام کار کنه" (whatever works, I just want something that works). PDF always opens reliably.

## Raw DOCX construction (zero-dependency fallback)
When python-docx is not available and pip is too slow, create a `.docx` as a ZIP file containing:
```
[Content_Types].xml
_rels/.rels
word/_rels/document.xml.rels
word/document.xml           # the actual content
word/styles.xml              # fonts, heading colors, RTL
word/numbering.xml           # bullet/numbered lists
word/fontTable.xml           # font declarations
word/settings.xml
word/webSettings.xml
word/theme/theme1.xml        # color theme
docProps/core.xml
docProps/app.xml
```

Each is a small XML file. Python stdlib only: `zipfile.ZipFile`, `io.BytesIO`.

### Key RTL setup in document.xml
```xml
<w:sectPr>
  <w:bidi w:val="true"/>
  <w:pgSz w:w="11906" w:h="16838"/>
</w:sectPr>
```

### Key RTL in styles.xml Normal style
```xml
<w:style w:type="paragraph" w:default="1" w:styleId="Normal">
  <w:rPr>
    <w:bidi w:val="true"/>
    <w:rFonts w:ascii="B Nazanin" w:hAnsi="B Nazanin" w:eastAsia="B Nazanin" w:cs="B Nazanin"/>
    <w:sz w:val="22"/>
  </w:rPr>
</w:style>
```

### Bullet list with numbering.xml
Define an abstractNum with bullet format and a num pointing to it. Then in document.xml:
```xml
<w:p>
  <w:pPr>
    <w:numPr><w:ilvl w:val="0"/><w:numId w:val="1"/></w:numPr>
  </w:pPr>
  <w:r><w:t>text</w:t></w:r>
</w:p>
```

## Design colors for warm Persian reports (for مهبد)
When asked for "beautiful/family-style" colors (not cold blue corporate) or "خشن‌خشن" (detailed/bold), use these warm sienna/amber or deep navy/charcoal tones:

| Style | Elements | Hex | Description |
|---|---|---|---|
| **Warm Earth** | Title | `#5D3A1A` | Deep dark brown |
| | Headings (H1/H2) | `#A0522D` | Sienna/warm sienna |
| | Body text | `#3D2B1F` | Dark espresso/charcoal-brown |
| | Accent/Bullets | `#CD853F` | Peru/warm amber |
| **Royal Navy** | Title | `#002060` | Very dark royal navy |
| | Headings (H1/H2) | `#1F497D` | Deep classic navy |
| | Body text | `#000000` | Pure black |
| | Accent/Bullets | `#4F81BD` | Steel blue |

Theme name suggestion: `Hermes Warm` or `Hermes Navy` — set in theme1.xml for coordinated accent colors.

## The "Zip File is Corrupt / Word Cannot Open" Debugging Path
When Word reports "Word experienced an error trying to open the file" after raw XML zip construction:
1. **Never copy document.xml between random docx files** — namespace, stylesheet, and relationship differences (like missing `rId` targets in `document.xml.rels` or numbering IDs in `numbering.xml`) will brick the file.
2. **The "Clean Clone" Method (SAFEST)**:
   - Extract a working DOCX: `unzip base.docx -d /tmp/base`
   - Modify ONLY the target elements inside `/tmp/base/word/document.xml` using `re.sub` or clean XML parsing, leaving namespaces intact.
   - Re-zip: `cd /tmp/base && zip -r /tmp/fixed.docx .`
   - This preserves all metadata (`[Content_Types].xml`, `_rels`, `settings.xml`) intact.
3. **If B Nazanin is not showing on locally-opened files**:
   - Word requires explicit font definition. `w:rFonts w:asciiTheme="..."` delegates to the theme font. To override directly, use `w:rFonts w:ascii="B Nazanin" w:hAnsi="B Nazanin" w:eastAsia="B Nazanin" w:cs="B Nazanin"`.
   - Update both `styles.xml` (Normal style `w:rPr` default) AND the runs in `document.xml`.
   - Also update `word/theme/theme1.xml` (replace `<a:latin typeface="Calibri"/>` and `<a:ea typeface=""/>` with B Nazanin).

## Related references
- `references/thesis-report-pattern.md` — Complete template for Iranian university-style exercise reports (cover page + technical sections + failure causes). Used for Dr. Afshar's instrumentation assignments.

## Key Learning: Markdown → pandoc → PDF (Most Reliable Path for Persian RTL)

**After extensive trial-and-error, the single most reliable path for Persian/RTL PDF generation is Markdown → pandoc → xelatex with Amiri font.** This succeeded immediately where all docx approaches consistently failed with font/substitution errors or "Word experienced an error."

### Recommended workflow (Persian/RTL reports):
1. **Write content in Markdown** (supports RTL, Persian, tables, code blocks)
2. **Convert via pandoc with xelatex:**
   ```bash
   pandoc input.md -o output.pdf --pdf-engine=xelatex \
       -V mainfont='Amiri' -V lang=fa -V dir=rtl
   ```
3. **Font:** Use `Amiri` (pre-installed) instead of B Nazanin (not installed)
4. **RTL support:** Add `-V dir=rtl` for proper RTL layout

### When to use docx vs PDF:
- **Markdown → PDF**: Faster, reliable, no layout issues. Default choice for "give me a report."
- **Clone-template DOCX**: Only when user explicitly needs editable .docx. Use template clone approach with dual font fix.

## RTL / Persian Word Documents

Use `python-docx` library. Key setup for Persian/Arabic (RTL) text:

```python
from docx import Document
from docx.shared import Pt, RGBColor, Inches
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.enum.table import WD_TABLE_ALIGNMENT
from docx.oxml.ns import qn
from docx.oxml import OxmlElement

doc = Document()
```

### RTL section setup

```python
sect = doc.sections[0]
sectPr = sect._sectPr
bidi = OxmlElement('w:bidi')
sectPr.append(bidi)
```

### Default font (fallback)

```python
style = doc.styles['Normal']
style.font.name = 'Times New Roman'
style.font.size = Pt(12)
rpr = style.element.get_or_add_rPr()
rfonts = rpr.get_or_add_rFonts()
rfonts.set(qn('w:eastAsia'), 'B Nazanin')
rfonts.set(qn('w:ascii'), 'Times New Roman')
rfonts.set(qn('w:hAnsi'), 'Times New Roman')
```

### RTL paragraph helper

```python
def rtl_para(text='', size=12, bold=False, align='right'):
    p = doc.add_paragraph()
    p.alignment = WD_ALIGN_PARAGRAPH.RIGHT if align == 'right' else WD_ALIGN_PARAGRAPH.CENTER
    pPr = p._p.get_or_add_pPr()
    pPr.set(qn('w:bidi'), 'true')
    r = p.add_run(text)
    r.font.size = Pt(size)
    r.font.bold = bold
    r.font.name = 'Times New Roman'
    r._element.get_or_add_rPr().get_or_add_rFonts().set(qn('w:eastAsia'), 'B Nazanin')
    return p
```

### RTL table helper

```python
def make_table(headers, rows):
    t = doc.add_table(rows=1, cols=len(headers))
    t.style = 'Table Grid'
    t.alignment = WD_TABLE_ALIGNMENT.RIGHT
    hdr = t.rows[0].cells
    for i, h in enumerate(headers):
        p = hdr[i].paragraphs[0]
        p.alignment = WD_ALIGN_PARAGRAPH.RIGHT
        p._p.get_or_add_pPr().set(qn('w:bidi'), 'true')
        run = p.add_run(h)
        run.font.bold = True
        run.font.name = 'Times New Roman'
        run._element.get_or_add_rPr().get_or_add_rFonts().set(qn('w:eastAsia'), 'B Nazanin')
    for row in rows:
        cells = t.add_row().cells
        for i, val in enumerate(row):
            p = cells[i].paragraphs[0]
            p.alignment = WD_ALIGN_PARAGRAPH.RIGHT
            p._p.get_or_add_pPr().set(qn('w:bidi'), 'true')
            run = p.add_run(str(val))
            run.font.name = 'Times New Roman'
            run._element.get_or_add_rPr().get_or_add_rFonts().set(qn('w:eastAsia'), 'B Nazanin')
    return t
```

## Node.js `docx` approach (when python-docx is unavailable)

When python-docx cannot be installed (pip times out), the Node.js `docx` npm package is a reliable alternative. Install: `npm install docx`. Use with `node` CLI.

### RTL heading fix — CRITICAL for Persian/Arabic

When creating RTL headings with right-to-left text, **BOTH** must be set on the Paragraph:
```javascript
new Paragraph({
  heading: "Heading1",
  bidirectional: true,                    // ← REQUIRED for RTL
  alignment: AlignmentType.RIGHT,        // ← REQUIRED to keep heading number on right
  children: [new TextRun({ text, rightToLeft: true, font: "Tahoma" })],
})
```
**Without `AlignmentType.RIGHT`**, the heading number (e.g. "۱.") will float to the left edge even though `bidirectional: true` is set. This is the #1 cause of "heading number stuck on left" in Persian docx reports.

For regular body paragraphs (RTL):
```javascript
new Paragraph({
  bidirectional: true,
  alignment: AlignmentType.RIGHT,
  spacing: { after: 160, line: 300 },
  children: [new TextRun({ text, font: "Tahoma", size: 22, rightToLeft: true })],
})
```

### Page count parity for bilingual reports

When producing equivalent English and Persian versions of the same report:
- Persian text is ~85-90% the character count of English for equivalent content (Farsi is denser)
- The rendered page count will typically differ by 1-3 pages unless the longer version is padded
- **To match English page count**, the Persian version needs ~110-120% as many paragraphs or include extra detail content in each section
- Use `char count` comparison via `document.xml` size as a rough parity check

### Charts/Canvas in dashboard reports

When embedding Chart.js canvases in a dashboard HTML:
- **Always set explicit height**: `<canvas style="height:200px!important;width:100%!important;"></canvas>`
- Without explicit height, Chart.js tries to compute it from CSS, which can result in infinitely scrolling charts
- Empty telemetry: when data array is empty, Chart.js still works — pass `labels: ['']` and `data: [0]` to avoid null rendering

### Promise.all failure in dashboard API calls

When a dashboard fetches from multiple API endpoints using `Promise.all`:
- If ANY single fetch rejects (e.g. empty array returns 404/500, or network blip), **ALL** stats remain unupdated (showing `-` placeholder)
- **Fix**: Replace `Promise.all` with per-endpoint `safeFetch` pattern:
```javascript
async function safeFetch(url, def) {
  try { const r = await fetch(url); if (!r.ok) return def; return await r.json(); }
  catch(e) { return def; }
}
const statsRes = await safeFetch('/api/stats', {free:0,occupied:0,...});
```

## Common operations

| Goal | Code |
|---|---|
| Heading | `p = doc.add_paragraph(); r = p.add_run(text); r.font.bold = True; r.font.size = Pt(16)` |
| Bullet point | `p = doc.add_paragraph(style='List Bullet')` + RTL setup |
| Add image | `doc.add_picture(path, width=Inches(5))` |
| Page break | `doc.add_page_break()` |
| Save | `doc.save('/path/to/output.docx')` |

## Reading existing DOCX

```python
import zipfile, re
with zipfile.ZipFile(path) as z:
    xml = z.read('word/document.xml').decode('utf-8')
text = re.sub(r'<[^>]+>', ' ', xml)
text = re.sub(r'\\s+', ' ', text).strip()
```

## Cloning a template's styles (reuse formatting from an existing file)
Extract the `styles.xml` from the user's reference DOCX and repack it with your new `document.xml`:
```python
# Copy styles from template to output:
z_src = zipfile.ZipFile(template_path)
styles_xml = z_src.read('word/styles.xml')
# ... write to new zip with your own document.xml
```
This preserves exact font, color, and heading styling without python-docx.

## Pitfalls

- **B Nazanin not installed**: Falls back to default font. Install via `apt install fonts-bnazanin` or similar.
- **python-docx 1.2.0+**: Older versions lack some API features (e.g. section RTL).
- **Google Fonts blocked in Iran**: Don't rely on CDN fonts for web-exported docs.
- **set_rtl on element**: Use `element.set(qn('w:bidi'), 'true')` — the `'true'` string, not boolean.
- **Table cell scaling**: Cells with long Persian text may not wrap correctly without explicit widths.
- **pip install timeout**: On this VPS, `pip install python-docx` ALWAYS times out (>60s/hard timeout). NEVER attempt pip install of python-docx. Wait 0s—skip pip immediately and use the clone-template approach. If pip started in background, kill after 30s.
- **Theme colors in theme1.xml**: When defining a custom color scheme, `document.xml` and `styles.xml` both need to reference the theme colors correctly, or Word falls back to defaults. Test by opening in Word/Online after building.

### CRITICAL: Dual font fix — B Nazanin in BOTH theme1.xml AND styles.xml
This is the **single most common DOCX failure mode** and was the root cause across multiple build attempts this session. The user reported "B Nazanin not applied" multiple times because only one of the two font files was updated.

**The fix must be applied to BOTH files after EVERY `unzip` operation:**

```bash
# 1. Fix theme1.xml — provides the font glyph source
sed -i 's|<a:latin typeface="Calibri"|<a:latin typeface="B Nazanin"|g' word/theme/theme1.xml
sed -i 's|<a:latin typeface="Cambria"|<a:latin typeface="B Nazanin"|g' word/theme/theme1.xml
sed -i 's|<a:ea typeface=""/>|<a:ea typeface="B Nazanin"/>|g' word/theme/theme1.xml

# 2. Fix styles.xml — assigns the font to paragraph/run defaults
sed -i 's|w:asciiTheme="minorHAnsi" w:eastAsiaTheme="minorEastAsia" w:hAnsiTheme="minorHAnsi" w:cstheme="minorBidi"|w:ascii="B Nazanin" w:hAnsi="B Nazanin" w:eastAsia="B Nazanin" w:cs="B Nazanin"|g' word/styles.xml
sed -i 's|w:asciiTheme="majorHAnsi" w:eastAsiaTheme="majorEastAsia" w:hAnsiTheme="majorHAnsi" w:cstheme="majorBidi"|w:ascii="B Nazanin" w:hAnsi="B Nazanin" w:eastAsia="B Nazanin" w:cs="B Nazanin"|g' word/styles.xml
```

**Why both are strictly required**: Word applies fonts in a two-layer system:
1. `theme1.xml` defines the font glyph repository (what the OS actually renders)
2. `styles.xml` assigns specific fonts (by name, matching the theme's keys) to paragraph/run defaults

If `theme1.xml` maps the theme font "minorHAnsi" to Calibri but `styles.xml` says "B Nazanin", Word has a font mismatch and falls back to its default. If `styles.xml` delegates via `w:asciiTheme` but `theme1.xml` doesn't define B Nazanin, Word substitutes. **Both files must consistently reference B Nazanin.**

### CRITICAL: Pip install is DEAD on this VPS
`pip install python-docx` has failed every single attempt (~10+ across multiple sessions) with network timeout. The resin-web venv at `/root/resin-web/venv/` does NOT have python-docx. **NEVER rely on python-docx for this user's DOCX generation.** Always use the template-clone approach (Approach 1).

### CRITICAL: Give up after 2 failed attempts
The user's patience for DOCX generation failures is extremely low. After 2 failed attempts (or at the first sign of user frustration like "هنوزم درست نشده" or "ناموسا؟"), immediately offer a PDF as alternative. User's explicit instruction: "هرکدوم کار میکنه یه چی میخوام کار کنه" (whatever works, I just want something that works).

### CRITICAL: Always build PDF alongside DOCX
Every docx build should also produce a PDF fallback:
```bash
pandoc output.docx -o output.pdf --pdf-engine=xelatex -V mainfont="Amiri" -V lang=fa
```
This way, if the DOCX is broken (which happens ~50% of the time with the clone approach), the user can still use the PDF.

### CRITICAL: Never build a docx from raw XML strings
Building with `zipfile.ZipFile(io.BytesIO(), ...)` and constructing every XML file from scratch ALWAYS fails. Word always reports "Word experienced an error trying to open the file." The structure can never precisely match a real Word template. Always start from a known-working .docx template.

### What DOES NOT WORK (tested, failed)
- **Raw XML in `BytesIO` zip** — Word rejects every time.
- **`python-docx` via pip** — network timeout every time on this VPS.
- **Copying document.xml from one docx into another's template dir** — style name mismatches (one doc uses Heading1, the other uses Compact; both reference style IDs that don't exist in the target styles.xml) produce corrupt output. Only works if both files come from the same template.

### What WORKS (the clone approach)
1. Extract a VALID template (use `/var/www/resin-media/instr-bands-6-11-unified.docx` or user's cached v2 file)
2. Write a clean `document.xml` string with correct namespaces
3. Replace template's document.xml with yours
4. Fix font refs in theme1.xml AND styles.xml (both MUST have B Nazanin)
5. Re-zip
6. NEVER skip writing the PDF fallback
