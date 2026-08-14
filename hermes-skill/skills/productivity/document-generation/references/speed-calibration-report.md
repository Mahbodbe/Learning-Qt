# Speed Calibration Report for Group 5 — Session Reference

## Session Context
- **User**: مهبد (Mahbod) — EE student, Group 5
- **Course**: ابزار دقیق (Dr. Afshar) — Term 1404-1405
- **Parameter**: سرعت (Speed) — rotational & linear
- **Standard**: ISO/IEC 17025 alignment
- **Deadline**: 31 Tir upload, 1 Mordad presentation

## Report Structure (Current Version)
1. مقدمه و تبیین پارامتر سرعت در ابزار دقیق صنعتی
2. انواع حسگرهای سرعت تحت تست (UUT) و ساختار فیزیکی
   - تاکومترهای نوری، انکودرهای نوری/مطلق، سنسورهای اثر هال/القایی
3. تجهیزات کالیبراتور سرعت و استانداردهای آزمایشگاهی
   - استند مرجع چرخان (Rotational Calibration Stand) + انکودر ۲۰ بیتی
   - فرکانس‌متر مرجع OCXO
   - استروبوسکوپ مرجع
   - کالیبراتور خطی/لرزش
4. فرآیند گام‌به‌گام کالیبراسیون
   - کنترل محیطی (دما ۲۳±۲°C، رطوبت <۶۰٪، mesa anti-vibration)
   - تمیزکاری و بازرسی چشمی (شارپ، انحنای شفت)
   - هم‌راستاسازی لیزری (Alignment) — جلوگیری از خطای سینوسی
   - پیش‌گرمایش (Warm-up) ۱۵-۳۰ دقیقه
   - نقاط کالیبراسیون (۵-۱۰ نقطه، دو جهت رفت/برگشت)
5. محاسبات عدم قطعیت (Uncertainty per GUM)
   - نوع A: تکرارپذیری (u_A = s/√n)
   - نوع B: رزولوشن، پایداری کالیبراتور، دمایی، لرزه
   - ضریب پوشش k=2 (سطح اطمینان ۹۵٪)
6. جمع‌بندی و الزامات ردیابی (Traceability per ISO/IEC 17025)

## File Locations
- DOCX: `/var/www/resin-media/speed-calibration-report.docx`
- PDF: `/var/www/resin-media/speed-calibration-report.pdf`
- Template used: `/var/www/resin-media/instr-bands-6-11-unified.docx` (v2 template)

## Font & Styling Decisions
- Font: B Nazanin (explicit in styles.xml AND theme1.xml)
- Colors: Warm sienna/amber (`#A0522D`, `#CD853F`, `#5D3A1A`) — user requested "خشن‌خشن" (detailed/bold)
- RTL: Full RTL with `w:bidi` throughout
- Styles: Title, Subtitle, Heading1, Heading2, Compact, Normal, ListBullet
- Bullet style: Compact with bold lead text in Peru (`#CD853F`)

## Template Strategy (SUCCESSFUL)
Used the "Clean Clone" method:
1. `unzip -o base_v2_template.docx -d /tmp/template` — extract working v2 template
2. Replace `word/document.xml` only (keep ALL other files intact)
3. `zip -r output.docx .` from template directory
4. `sed` fix `word/styles.xml` and `word/theme/theme1.xml` for B Nazanin
5. Re-zip from template directory

## Pitfalls Avoided
- ❌ Raw XML zip from scratch → Word "corrupt file" error
- ❌ Copying document.xml between different template versions → style ID mismatches
- ❌ Missing `numbering.xml` → bullet lists fail
- ❌ Missing `theme1.xml` B Nazanin → font fallback to Calibri/Cambria
- ❌ `isEmptyPosition` inverted logic in World.cpp (fixed)
- ❌ `moveOrganism` inverted condition (fixed)

## Next Session Quick-Start
If needing another calibration report (e.g., Pressure for Group 2):
1. Use `instr-bands-6-11-unified.docx` as template
2. Replace `document.xml` with new content using same styles
3. Keep B Nazanin fixes in styles.xml + theme1.xml
3. Re-zip, verify, deliver