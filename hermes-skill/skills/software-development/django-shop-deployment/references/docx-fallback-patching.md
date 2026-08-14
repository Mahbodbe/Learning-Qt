# DOCX Fallback Patching (no python-docx)

When `pip install python-docx` is unavailable / timeout on a slow VPS, edit OOXML directly via `unzip` + `sed` + `zip`.

## Pattern

```bash
cp template.docx patched.docx
cd /tmp
unzip -o template.docx -d workdir/         # unpack
sed -i 's|w:ascii="Times New Roman" w:hAnsi="Times New Roman" w:eastAsia="B Nazanin"|w:ascii="B Nazanin" w:hAnsi="B Nazanin" w:eastAsia="B Nazanin"|g' workdir/word/styles.xml
cd workdir
zip -r ../patched.docx .                    # re-pack
cp ../patched.docx /var/www/media/out.docx
```

## Key files to patch

| File in zip | What it controls |
|---|---|
| `word/styles.xml` | Font definitions, heading colours, table styles |
| `word/document.xml` | Content and per-run formatting overrides |

## Font change target

The `Normal` style often has a mix of fonts (ascii=Times, eastAsia=B Nazanin). Change all three to `B Nazanin` for uniform Persian output.

## Colour change (blue headings)

Heading colour is in `w:color w:val="4F81BD"` inside `<w:style w:type="paragraph" w:styleId="Heading1">` in `styles.xml`. Replace the hex value.

## Pitfalls

- `zip -r ../patched.docx .` **must** be run from inside `workdir/` so the archive has the right structure. Running from outside creates a nested `workdir/` prefix → Word rejects the file as corrupted.
- Always include all extracted files (`[Content_Types].xml`, `_rels/`, `word/`, `docProps/`). Missing one → file won't open.
- `sed` with unicode text needs `LC_ALL=C` or works fine as-is in bash.
- Verify after patching: `unzip -l patched.docx` should show same structure as original.
