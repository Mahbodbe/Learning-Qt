# Node.js `docx` RTL Tips (from session 2026-07-27)

## Helper module pattern (docx_render.js)

Create a single helper file that exports all formatting functions, then import from section files:

```js
module.exports = { h1, h2, p, bulletList, pageBreak, referencesBlock, dataTable, codeBlock, NAVY, AMBER, MUTED, TEXT, HEAD, BODY, PageBreak };
```

Import in each section file:
```js
const R = require("./docx_render.js");
const rtl = false;
const section1 = [ R.h1("Title", rtl), ... ];
module.exports = { section1 };
```

## RTL heading pitfall recap

Heading numbers float LEFT when ANY of these three is missing:

1. `heading: "Heading1"` (string)
2. `bidirectional: true`
3. `alignment: AlignmentType.RIGHT`

**Fix: always set all three + `rightToLeft: true` on each TextRun inside heading.**

## Code block with RTL context

`codeBlock()` can be plain (no bidirectional) — code should stay LTR:

```js
function codeBlock(lines) {
  return new Table({
    width: { size: 9072, type: WidthType.DXA },
    rows: [new TableRow({ children: [new TableCell({
      shading: { type: ShadingType.CLEAR, fill: "F4F4F4" },
      children: lines.map(l => new Paragraph({
        children: [new TextRun({ text: l.length ? l : " ", font: "Consolas", size: 17, color: "1a1a1a" })],
      })),
    })]})],
  });
}
```

## Page count balancing

If FA version has fewer pages than EN version:
- Increase font size (24 vs 22)
- Add more content (fuller translation)
- Add blank paragraphs between sections
