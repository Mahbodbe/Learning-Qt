# CA_slide_rep.txt Pattern — Bilingual (EN+FA) DOCX Reports

## Source
Observed in a report generated for Mani Mohammadi's ARM Interrupts project
(Amirkabir University, Dr. Pourfard). The JavaScript `docx` API template is
in the session transcript file `CA_slide_rep.txt`.

## Key `docx` API Elements

| Element | Implementation |
|---------|---------------|
| Font pair | EN = `Calibri`, FA = `Tahoma` |
| Color scheme | Blue headings `#1F4E79`, gray sub `#404040` |
| RTL | `rightToLeft: true` + `bidirectional: true` on both TextRun and Paragraph |
| Table header | Blue fill `"1F4E79"`, white bold text |
| Alternating rows | `#F2F6FA` (even) / `#FFFFFF` (odd) |
| Code block | Single-cell table, `#F4F4F4` fill, `Consolas` 18pt |
| Links | `ExternalHyperlink` with blue underline `#0563C1` |
| Header | Centered report title |
| Footer | `Page N of " + PageNumber.CURRENT` |
| Cover | University → dept → title (blue 44pt) → prepared-by → instructor → date |
| TOC | `TableOfContents` with `headingStyleRange: "1-2"` |

## Helper Functions

```
h1(en)           → Heading1 with blue underline border
h1fa(fa)         → RTL blue Heading1
h2(en) / h2fa(fa)→ Heading2 with dark gray
pEn(text)        → justified English paragraph
pFa(text)        → RTL justified Farsi paragraph
bulletEn()       → bulleted English item
bulletFa()       → bulleted RTL Farsi item
link(label,url)  → ExternalHyperlink
sourceLine()     → bullet + link
sourcesBlock()   → title + list of sourceLine items
simpleTable()    → blue header + alternating rows
codeBlock(lines) → gray table with Consolas text
pageBreak()      → PageBreak paragraph
```

## Structure Each Section Follows

```
h1("N. Section Title")
h1fa("N. عنوان بخش به فارسی")
pEn("English explanation paragraph...")
pFa("توضیح فارسی...")
h2("N.M Subsection")
... content ...
sourcesBlock("Sources", "منابع", [list of {label, url}])
pageBreak()
```

## EXTI / Interrupt-Specific Content Pattern Used

The worked examples showed:
- CPU/series context (Cortex-M3 vs M4, STM32F1 vs F4)
- Pin → EXTI line mapping
- AFIO vs SYSCFG difference
- NVIC priority setting
- ISR body (check line → clear PR → set flag)
- Nesting/tail-chaining/late-arrival notes between the two examples

This content pattern maps well to any embedded-systems interrupt chapter.
