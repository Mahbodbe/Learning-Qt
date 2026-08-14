# Session Pitfalls & Lessons (2026-07-27)

## Context
Generated ARM Cortex-M Interrupts/EXTI report (English + Persian) and multiple Persian instrumentation reports for Mahbod's university projects.

## Critical Pitfalls Discovered

### 1. Hand-built `word/document.xml` is UNRELIABLE
- **Symptom**: Generated `.docx` files Word refuses to open
- **Fix**: Use `pandoc markdown → docx` or `docx` library directly

### 2. Spread syntax `...` on `R.codeBlock()` fails
- **Error**: `TypeError: Spread syntax requires ...iterable[Symbol.iterator] to be a function`
- **Fix**: Assign to variable first: `const t = R.codeBlock([...]); sections.push(t);`

### 3. Multi-part JS file combining
Multiple report parts (build_report_en_part1.js through part8.js) each have their own `const R = require("./docx_render.js")` and `const rtl = false;` lines.

**When assembling into one file:**
- Remove duplicate `const R = require(...)` lines (keep only the first)
- Remove duplicate `const rtl = false;` lines
- Remove `const { titlePage, section1 } = require(...)` lines
- Remove all `module.exports = { ... };` lines
- Run `node -c file.js` to check syntax before execution

### 4. `node -c file.js` for syntax checking
Always check JS syntax before running: `node -c /tmp/gen_report.js`

### 5. `npm install` / `pip install` on remote VPS times out
- **Workarounds (in priority order)**:
  1. `apt-get install python3-<package>` for system package equivalents
  2. `rsync` pre-built `node_modules` or venv from local machine
  3. `pip install --prefer-binary --timeout 300` with extended timeout

### 6. `chmod 644` is MANDATORY
Root-owned files in `/var/www/resin-media/` default to 600 → nginx 403 Forbidden.

### 7. User hates delivery verbosity
Mahbod: "دمت گرم، Rapport — just give me the link"
**Rule**: When delivering a file, provide ONLY the download URL. Do NOT narrate build steps, installation, or process.

### 8. ASCII box-drawing in xelatex PDFs
Box characters (┌─┐) trigger hundred of "Missing character" warnings. Fine in DOCX.

### 9. Farsi `node docx` headings should be right-aligned
Heading1/Heading2 in Persian reports need `bidirectional: true` + `alignment: AlignmentType.RIGHT` on the Paragraph, and `rightToLeft: true` on the TextRun. Without these, heading numbers appear on the LEFT side of the page.
