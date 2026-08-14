#!/usr/bin/env python3
"""Verify a generated .docx file is valid and well-formed."""
import zipfile, re, sys

def verify_docx(path):
    """Verify a .docx file has all required parts and valid content."""
    try:
        z = zipfile.ZipFile(path)
    except zipfile.BadZipFile:
        print(f"❌ {path}: Not a valid ZIP file")
        return False

    names = z.namelist()

    # Check essential files
    essential = [
        '[Content_Types].xml',
        '_rels/.rels',
        'word/document.xml',
        'word/styles.xml',
        'word/theme/theme1.xml',
        'word/settings.xml',
        'word/fontTable.xml',
        'word/numbering.xml',
    ]

    missing = [f for f in essential if f not in names]
    if missing:
        print(f"❌ Missing essential files: {missing}")
        return False

    print("✅ All essential files present")

    # Check document.xml has content
    doc_xml = z.read('word/document.xml').decode('utf-8')
    text_fragments = re.findall(r'<w:t[^>]*>([^<]+)</w:t>', doc_xml)
    if len(text_fragments) < 10:
        print(f"⚠️  Very little text content ({len(text_fragments)} fragments)")
        return False
    print(f"✅ Document has {len(text_fragments)} text fragments")

    # Check styles
    styles_xml = z.read('word/styles.xml').decode('utf-8')
    if 'B Nazanin' in styles_xml:
        print("✅ B Nazanin font present")
    else:
        print("ℹ️  B Nazanin not in styles.xml (may use theme)")

    # Check heading colors
    if '4F81BD' in styles_xml or '365F91' in styles_xml:
        print("✅ Blue heading colors present")

    # Check RTL
    if 'w:bidi' in doc_xml:
        print("✅ RTL (bidi) markers present")
    else:
        print("⚠️  No RTL markers found")

    # Check numbering
    if 'numbering.xml' in z.namelist():
        print("✅ numbering.xml present (bullets work)")
    else:
        print("⚠️  numbering.xml missing (bullets may not render)")

    print(f"\n✅ {path} is a valid, well-formed DOCX")
    return True

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: python verify-docx.py <path-to-docx>")
        sys.exit(1)
    ok = verify_docx(sys.argv[1])
    sys.exit(0 if ok else 1)