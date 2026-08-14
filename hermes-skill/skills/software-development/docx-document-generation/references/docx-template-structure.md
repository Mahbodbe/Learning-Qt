---
title: DOCX Template File Structure
description: Complete reference for valid DOCX template file structure
---

# DOCX Template File Structure

A valid .docx file is a ZIP archive containing these essential files:

## Required Files (MUST be present)

| File | Purpose | Source |
|------|---------|--------|
| `[Content_Types].xml` | MIME type mappings for all parts | Template |
| `_rels/.rels` | Package-level relationships | Template |
| `word/document.xml` | Main document content | **Generated** |
| `word/styles.xml` | Paragraph/character styles | Template (or modified) |
| `word/theme/theme1.xml` | Theme colors, fonts, effects | Template |
| `word/settings.xml` | Document settings | Template |
| `word/fontTable.xml` | Font definitions | Template |
| `word/numbering.xml` | Bullet/numbering definitions | Template |
| `word/webSettings.xml` | Web compatibility settings | Template |
| `word/_rels/document.xml.rels` | Document part relationships | Template |
| `docProps/core.xml` | Core properties (author, dates) | Template |
| `docProps/app.xml` | Application properties | Template |
| `docProps/custom.xml` | Custom properties (if present) | Template |

## Optional but Recommended

| File | Purpose |
|------|---------|
| `word/settings.xml` | Document settings (zoom, view, etc.) |
| `word/webSettings.xml` | Web compatibility settings |
| `word/fontTable.xml` | Embedded font definitions |
| `word/footnotes.xml` | Footnotes content |
| `word/endnotes.xml` | Endnotes content |
| `word/footnotes.xml.rels` | Footnotes relationships |
| `word/endnotes.xml.rels` | Endnotes relationships |
| `word/comments.xml` | Comments |
| `word/comments.xml.rels` | Comments relationships |

## Minimal Valid DOCX Structure

```
output.docx
├── [Content_Types].xml
├── _rels/
│   └── .rels
├── docProps/
│   ├── core.xml
│   ├── app.xml
│   └── custom.xml (optional)
└── word/
    ├── document.xml          ← YOUR CONTENT HERE
    ├── styles.xml            ← from template
    ├── numbering.xml         ← from template
    ├── settings.xml          ← from template
    ├── webSettings.xml       ← from template
    ├── fontTable.xml         ← from template
    ├── theme/
    │   └── theme1.xml        ← from template
    ├── _rels/
    │   └── document.xml.rels ← from template
    ├── fontTable.xml
    └── footnotes.xml / endnotes.xml (optional)
```

## Critical Notes

1. **Never omit** `[Content_Types].xml` - Word won't recognize the file
2. **Never omit** `_rels/.rels` - package relationships broken
3. **Never omit** `word/_rels/document.xml.rels` - internal links broken
3. **Keep template's `numbering.xml`** - bullets/numbering won't work without it
4. **Keep template's `theme1.xml`** - colors/fonts defined here
5. **Keep template's `fontTable.xml`** - font definitions
5. **Keep `docProps/core.xml`** - document metadata

## Minimal Valid DOCX Creation Checklist

- [ ] Unzip a valid .docx template
- [ ] Replace ONLY `word/document.xml` with new content
- [ ] Optionally update `word/styles.xml` if styling changes needed
- [ ] Keep ALL other files exactly as-is
- [ ] Re-zip with `zip -r output.docx .`
- [ ] Verify with Word (no repair dialog)