---
name: academic-pdf-processing
description: "Process academic PDFs: extract text, summarize chapters, generate study notes, create DOCX/HTML deliverables via Pandoc for university projects. Covers PDF text extraction, chapter summarization, study guide creation, and deliverable generation via Pandoc."
version: 1.0.0
author: Hermes Agent
license: MIT
platforms: [linux, macos, windows]
metadata:
  hermes:
    tags: [academic, pdf, research, pandoc, docx, study-notes, summarization]
    related_skills: [django-shop-deployment, research]
---

# Academic PDF Processing & Study Guide Generation

Process academic PDFs (textbooks, research papers, lecture slides) into structured study materials and deliverables.

## When to Use

- User shares academic PDFs (textbooks, papers, lecture notes) and wants structured summaries
- User needs DOCX/HTML deliverables for university projects via Pandoc
- User wants chapter-by-chapter study guides with key concepts, formulas, diagrams described
- User is studying for exams or writing papers and needs organized notes

## Workflow

### 0. Pre-requisites
```bash
apt-get install -y poppler-utils pandoc
# texlive-latex-base NOT needed for DOCX output (only for PDF via LaTeX)
```

### 1. Extract Text from PDFs
```bash
# Full extract
pdftotext "/path/to/file.pdf" /tmp/full.txt

# Line-range extract (chapter-by-chapter batch mode)
pdftotext "/path/to/file.pdf" - | sed -n 'START_LINE,END_LINEp' > /tmp/chunk.txt

# Inline (quick look)
pdftotext "/path/to/file.pdf" - | head -200
```

### 2. Process & Summarize Chapter by Chapter (Batch Mode)

When the user sends 10+ course files, process them in sequence:
1. For each PDF, run `pdftotext "/path.pdf" - | head -N` to get the structure
2. Extract per chapter: **Key Concepts**, **Key Formulas** (in table), **Worked Examples**, **Key Takeaways**
3. Write each chapter as an independent section in `study-guide.md`
4. At the end, combine all sections into one document

**Batch-mode signals:** user says "این فایل پروژه درسه", "اینم خوب بخون", "اینو کامل بخون و یاد بگیر" followed by successive file uploads = batch processing.

### 3. Generate Study Guide (Markdown)
Structure:
```markdown
# [Book/Paper Title] — Study Guide

## Chapter N: [Title]
### Key Concepts
- Concept 1: definition + formula if applicable
- Concept 2: ...

### Key Formulas
| Symbol | Meaning | Formula | Units |
|--------|---------|---------|-------|
| R | Reynolds number | (3160 × Q × Gt × D) / µ | dimensionless |

### Worked Examples
[from the Textbook Gives
- Example 1: problem → solution steps → answer
- Example 2: ...

### Key Takeaways
- Point 1
- Point 2

### Practice Problems from Textbook
- Problem 1: [text] → Answer
- Problem 2: [text] → Answer

### Cross-References
- Related sections in other chapters
- Related external resources
```

### 4. Generate DOCX/HTML Deliverables via Pandoc
```bash
# Install pandoc + texlive if needed
apt-get install -y pandoc texlive-latex-base texlive-latex-extra

# Markdown → DOCX
pandoc study-guide.md -o study-guide.docx --from markdown --to docx

# Markdown → HTML (with custom CSS)
pandoc study-guide.md -o study-guide.html --from markdown --to html5 --css=style.css

# With table of contents
pandoc study-guide.md -o output.docx --toc --toc-depth=2
```

### 4b. Direct DOCX Generation (Fallback when python-docx times out)
**CRITICAL:** On low-RAM VPSs (~1GB), `pip install python-docx` frequently times out (>60s). Use **template-based XML replacement** instead:

```bash
# 1. Get a valid template (user's existing docx)
cp user_template.docx template.docx

# 2. Extract ALL files
mkdir tmpl && cd tmpl
unzip -o ../template.docx

# 3. Replace ONLY document.xml with new content
# Build body content as Python string f-strings (NOT ElementTree.tostring!)
# Copy w:document opening tag VERBATIM from template
cp new_content.xml word/document.xml

# 4. Re-zip preserving ALL files
cd tmpl && zip -r ../output.docx .
```

**Critical DOCX Gotchas:**
- NEVER use `xml.etree.ElementTree.tostring()` for document.xml — rewrites namespace prefixes (w: → ns0:). Word rejects the file.
- Build body with Python string f-strings; copy the w:document opening tag VERBATIM from template.
- NEVER hardcode fonts in styles.xml — let user change fonts via Design tab. Update theme1.xml instead.
- Keep w:bidi on ALL w:pPr, w:rPr, w:sectPr for RTL Persian.
- Missing files break the docx: [Content_Types].xml, _rels/.rels, word/styles.xml, word/theme/theme1.xml, word/fontTable.xml, word/numbering.xml, word/settings.xml, word/numbering.xml, docProps/*, word/fontTable.xml.

**PDF fallback:** If 3 DOCX attempts fail → STOP and offer PDF:
```bash
pandoc input.docx -o output.pdf --pdf-engine=xelatex -V mainfont='Amiri' -V lang=fa
```

### 5. Create Study Package (ZIP)
```bash
zip -r study-package.zip study-guide.md study-guide.docx study-guide.html references/
```

## Key Tools & Commands

| Tool | Purpose | Install |
|------|---------|---------|
| `pdftotext` (poppler-utils) | Extract text from PDF | `apt-get install poppler-utils` |
| `pandoc` | Convert MD ↔ DOCX/HTML/PDF | `apt-get install pandoc texlive-latex-base texlive-latex-extra` |
| `python3 -m fitz` (PyMuPDF) | Alternative PDF extraction | `pip install pymupdf` |
| `pandoc` | MD → DOCX/HTML/PDF | Included with texlive |

## Typical Workflow for a Textbook

```bash
# 1. Extract all text
pdftotext textbook.pdf textbook_full.txt

# 2. Split by chapters (if page numbers known, or search for "Chapter")
# 3. For each chapter, extract 500-1000 lines at a time
pdftotext textbook.pdf - | sed -n '500,1000p'  # lines 500-1000

# 4. For each chapter, create summary markdown
# 4.1 Read 500 lines at a time with offset/limit
# 4.2 Extract: definitions, formulas, worked examples, key takeaways
# 4.3 Write chapter-N.md

# 4. Combine all chapters into study-guide.md
# 5. Generate deliverables
pandoc study-guide.md -o study-guide.docx --toc --toc-depth=2
pandoc study-guide.md -o study-guide.html --css=style.css
```

## Study Guide Template (Markdown)

```markdown
# [Subject] — [Course/Exam Name] Study Guide

## Table of Contents
1. [Chapter 1 Title](#chapter-1)
2. [Chapter 2 Title](#chapter-2)
...

---

## Chapter 1: [Title] {#chapter-1}

### Key Concepts
- **Concept**: Definition + significance
- **Formula**: $E = mc^2$ — meaning of each variable

### Key Formulas
| Symbol | Meaning | Formula | Units |
|--------|---------|---------|-------|
| $Re$ | Reynolds number | $Re = \frac{\rho v D}{\mu}$ | dimensionless |
| $Nu$ | Nusselt number | $Nu = \frac{hD}{k}$ | dimensionless |

### Worked Examples
**Example 1.1**: [Problem statement]
*Solution*: Step 1 → Step 2 → Answer: X

### Key Takeaways
- Point 1
- Point 2

### Practice Problems
1. Problem → Solution
2. Problem → Solution

---

## Chapter 2: [Title] {#chapter-2}
...
```

## Pandoc Commands Quick Reference

| Output | Command |
|--------|---------|
| DOCX with TOC | `pandoc input.md -o output.docx --toc --toc-depth=2` |
| HTML with CSS | `pandoc input.md -o out.html --css=style.css --metadata title="Title"` |
| PDF (via LaTeX) | `pandoc input.md -o out.pdf --pdf-engine=xelatex` |
| EPUB | `pandoc input.md -o out.epub --epub-cover-image=cover.jpg` |

## Tips & Gotchas

| Issue | Fix |
|-------|-----|
| Pandoc can't find fonts | Install fonts: `apt-get install fonts-dejavu fonts-liberation` |
| Arabic/Persian text reversed in PDF | Use `--pdf-engine=xelatex` and add `\usepackage{bidi}` in header |
| Tables too wide in DOCX | Use `--wrap=preserve` or reduce font size |
| Images not showing in DOCX | Use absolute paths or embed with `--self-contained` |
| Persian/Arabic in PDF | Use `--pdf-engine=xelatex -V mainfont="Vazirmatn"` |
| **User sends 10+ files in sequence** | Enter **batch mode**: for each, extract key sections, confirm per-file completion with short ack like `✅ file N: topic read`. Do not summarize everything into one giant response mid-batch; let the user drive the sequence. |
| **User wants to receive the DOCX via Telegram** | Telegram attachment limit is 20MB. If DOCX is larger, serve it via Django's media URL (see `references/docx-delivery-django.md`). |
| **User wants terse confirmation per file, not summaries** | User may say "این فایل رو بخون و تایید کن" — respond with `✅ [filename]: topic, X pages processed.` and stop. Do not expand into a full summary unless asked. |
| **Persian/Arabic PDFs** | Use `pdftotext` with `-enc UTF-8` for proper encoding; `pandoc --pdf-engine=xelatex -V mainfont="Vazirmatn"` for PDF output |
| **DOCX generation from python-docx (new)** | When building structured DOCX reports directly (not via Pandoc), use `python-docx` library with RTL-aware paragraph/table styling: set `run.font.name = 'Times New Roman'`, run font eastAsia to 'B Nazanin', and set paragraph RTLO via `pPr` `bidi` XML attribute. For Persian tables, create `make_table` helper that sets RTL on every cell. Example: see session where `instr-bands-6-11-unified.docx` was built with RTL tables, bullet lists, and multi-style sections. |
| **Large DOCX for university project** | When authoring multi-section university project reports (22-band structure per Instr 04 spec), include: RTL title on first page, per-band subheadings, input/output tables with headers/rows, bullet lists for specifications, and appendix flowchart descriptions. Generate file at `/var/www/resin-media/` and serve via `http://domain/media/filename.docx`. |

## References
- [Pandoc Manual](https://pandoc.org/MANUAL.html)
- [Poppler pdftotext](https://poppler.freedesktop.org/)
- [PyMuPDF](https://pymupdf.readthedocs.io/)
- [DOCX Delivery via Django Media](references/docx-delivery-django.md) — serve DOCX files to users through Django + Nginx instead of Telegram attachments

## When to Update This Skill

- New PDF processing tool discovered (e.g., `marker`, `nougat`, `grobid`)
- New Pandoc feature/version with better PDF/DOCX output
- User requests new output format (Anki cards, Anki decks, Notion import, Obsidian vault)
- New academic workflow requested (Anki deck generation, spaced repetition scheduling)

## References
- [Pandoc Manual](https://pandoc.org/MANUAL.html)
- [Poppler pdftotext](https://poppler.freedesktop.org/)
- [PyMuPDF](https://pymupdf.readthedocs.io/)