# Thesis-style Report Pattern (Iranian University Format)

## Structure (used successfully for Dr. Afshar's instrumentation exercises)

### Page 1: Cover Page (exercise name ONLY)
```
Header: "تمرین شماره" (title, large)
        "شماره دوم" (subtitle, large)

(blank lines)

Body:   "دانشگاه صنعتی امیرکبیر (پلیتکنیک تهران)" (subtitle, center)
        "دانشکده مهندسی برق" (subtitle, center)
        "درس: ابزار دقیق" (subtitle, center)
        "استاد: دکتر احمد افشار" (subtitle, center)
        "اردیبهشت ۱۴۰۵" (subtitle, center)

Names:  "عرفان اشکش — مهبد بمانیچم — مانی محمدی" (normal, center)

Page break
```

### Pages 2+: Structured Technical Content

### Section 1: Introduction (مقدمه)
- Context: Why this sensor/parameter matters in industrial control
- Specific model being analyzed
- Key features of the device

### Section 2: Device Introduction (معرفی)
- What the device measures
- Applications (oil, gas, petrochemical, power plants)
- Operating principle summary

### Section 3: Specifications (مشخصات)
- List technical specs point by point:
  - Output signal: 4-20 mA
  - Protocol: HART (version 5 or 7)
  - Pressure range: vacuum to 700 bar
  - Accuracy: ±0.075% of span
  - Temperature range: -40 to +125°C
  - Protection: IP66/IP67/NEMA 4X
  - Material: SS316L or Hastelloy

### Section 4: Structure & Components (ساختار کلی و اجزای اصلی)
Numbered bullet list (۶ parts):
1. Input pressure section (HP/LP)
2. Measurement sensor (silicon/micromachined)
3. Signal processing circuit (amplify, filter, ADC)
4. Internal microprocessor (calibration, protocol management)
5. Output module (4-20mA, HART, Fieldbus)
6. Communication & configuration (HART Communicator)

### Section 5: Operating Principle (نحوه عملکرد)
Paragraph explaining the measurement chain:
```
Pressure → sensor → signal conditioning → ADC → 
microprocessor (error compensation, calibration) → 
standard output (4-20mA + HART) → DCS/PLC
```

### Section 6: Block Diagram (بلوک دیاگرام عملکرد)
Simple numbered steps:
1. Input pressure → 2. Sensor → 3. Signal conditioning →
4. Digital conversion → 5. Processing (error compensation) →
6. Standard output

### Section 7: Failure Causes (علل خرابی) — RED headings
Use `lead_color='C00000'` (red) for bullet headings in this section:
1. Zero/Span shift (temperature, mechanical creep)
2. Frozen/erratic output (sensor failure, noise)
3. HART communication error (loop resistance <250Ω)
4. Fluid leakage from sensor (O-ring/diaphragm wear)
5. Temperature error (internal temp sensor failure)
6. Output saturation (input exceeds calibration range)

### Section 8: Calibration & Preventive Maintenance (کالیبراسیون و تعمیرات پیشگیرانه)
- Annual calibration check
- Visual inspection (every 6 months)
- HART performance test
- Diaphragm cleaning
- Damping adjustment

## CSS-style formula for building content volume
- 1 section heading = ~0.3 pages visual space
- 1 normal paragraph (4-5 lines) = ~0.15 pages
- 1 bullet item (2 lines lead + 4 lines text) = ~0.12 pages
- 6 bullet items per section = ~0.7 pages
- To reach 3-5 pages: minimum 5-7 sections with 4+ bullet items each

## Font and color palette for Dr. Afshar reports
- Title: `color='1F4E79'`, `sz='40'` (very dark navy)
- Subtitle: `color='2E75B6'`, `sz='28'` (medium navy)
- Headings (H1): `#1F4E79` or `#345A8A`
- Failure cause headings: `#C00000` (red)
- Body text: black, Normal style
- Font: B Nazanin via `w:cs="B Nazanin"` in run properties

## Bulgarian text spacing cheat
Use empty paragraphs between sections: `p('Normal', '', sz='8')`
Page breaks: `'<w:p><w:pPr><w:pageBreakBefore/></w:pPr></w:p>'`
