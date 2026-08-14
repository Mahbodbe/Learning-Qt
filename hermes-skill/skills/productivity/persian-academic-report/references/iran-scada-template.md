# Iran SCADA Report Template (TAVANIR Conventional Systems) - 2026-08-01

## Trigger
User asks for a comprehensive گزارش on SCADA in Iranian power grid (TAVANIR), high-voltage transmission/sub-transmission/distribution, conventional RTU-based systems.

## Iran Power Grid Hierarchy (Dispatching Levels)

| Level | Organization | Role | Protocol |
|-------|--------------|------|----------|
| **4** | SCC (System Control Center) - Iran Grid Management Co. | National EMS/AGC, frequency control, inter-regional coordination | ICCP (TASE.2) over Fiber WAN |
| **3** | AOC (Area Operating Centers) - 11-12 centers (e.g., TAOC Tehran) | Transmission (400/230kV) & Sub-transmission (63kV) supervision | IEC 60870-5-104 / ICCP |
| **3** | RDC (Regional Dispatching Centers) | Controls ~50-60 substations 63/20kV, sub-transmission dispatching | IEC 60870-5-104 / Inter-Dispatch Link |
| **2** | DAS (Distribution Automation System) - TAVANIR Distribution Co. | 20kV feeder automation, FLISR, reclosers, sectionalizers, RMUs | IEC 104 Fiber / DNP3 GPRS-Radio / IEC 101 |
| **1** | Substation RTUs & FRTUs (Field) | Conventional RTU telemetry, local mimic panel control | IEC 101 PLCC/Leased / IEC 104 Fiber / DNP3 Radio |
| **0** | Physical Voltage Chain | Generation → Transmission → Sub-transmission → Distribution | — |

## Voltage Chain (Iran)

```
Power Plant (13.8kV)
    → GSU Transformer (13.8/230kV or 400kV)
    → Transmission Lines (230kV / 400kV backbone)  -- PLCC on conductors
    → Transmission Substation (230/63kV)
    → Sub-transmission Grid (63kV)
    → Sub-transmission Post (63/20kV)
    → Distribution Feeders (20kV urban)
    → Distribution Transformers (20kV/400V)
    → Consumers (400V/230V)
```

## Conventional Substation Signal Flow Path (Level 0 → Level 1 → FEP)

```
[Primary Equipment: CB / DS / ES / CT / VT / Power Transformer]
    │ (Multi-core control cables)
    ▼
[HVI Panel: HV/LV Isolation, Surge Suppression, Interlocking Contactors]
    │ (Internal wiring)
    ▼
[Marshalling Cabinet: Terminal strips, signal branching, test points]
    │ (110V / 5A AC signals)
    ▼
[Transducers & Protection Relays: 4-20mA converters, Dry contacts, MW/MVAr/kV transducers]
    │ (4-20mA / Dry Contact signals)
    ▼
[Conventional RTU: AI cards (4-20mA), DI cards (SOE/Status), DO cards (SBO Trip/Close), CPU (IEC 101/104 Driver)]
    │ (PLCC / Fiber Optic Cable)
    ▼
[FEP: Protocol Translation, Real-time DB, Alarms]
    │ (ICCP / IEC 104)
    ▼
[AOC / RDC Operator HMI]  →  [SCC via ICCP]
```

### Telecontrol Command Path (SBO Return Loop - Right to Left)
```
Dispatching HMI (SBO Select → Operate via ICCP/IEC 104)
    → FEP forwards to RTU
    → RTU verifies SBO → Energizes DO
    → Interposing Relay activates
    → Circuit Breaker Trip/Close Coil (Hardwired)
```

## RTU I/O Types (Conventional)

| Type | Signal | Examples |
|------|--------|----------|
| **AI** | 4-20mA / ±10V / 0-10V | MW, MVAr, kV, A, Transformer oil temp |
| **DI** | Contact closure (Single/Double point) | CB status (Open/Close), DS status, Protection alarms |
| **AO** | 4-20mA / 0-10V | AVR setpoint, Tap changer reference |
| **DO** | Relay contact with SBO | Trip/Close CB, Raise/Lower Tap |

## Communication Media in Iran

| Medium | Bandwidth | Range | TAVANIR Use |
|--------|-----------|-------|-------------|
| **PLCC** | kbps (40-500 kHz carrier) | 100s km on HV line | Teleprotection, voice, low-speed SCADA on 230/400kV lines |
| **RS-232** | up to 115.2 kbps | ~15m | RTU to modem/FEP point-to-point |
| **RS-485** | practical 9.6-115.2 kbps | ~1200m | Multi-drop IED/RTU bus |
| **Ethernet (Industrial)** | 100Mbps-10Gbps | 100m Cu / km+ fiber | Substation LAN, IEC 61850, IEC 104 |
| **Fiber-optic OPGW** | Tbps | 10s-100s km | Transmission backbone (ground wire with embedded fibers) |
| **GPRS / Radio** | narrowband | varies | Distribution FRTU, recloser/RMU links |

## Protocol Stack (Iran Conventional Focus)

### IEC 60870-5-101 (Legacy Serial - Still Dominant)
- **Transport:** RS-232, RS-485, PLCC, leased line modem
- **Why used in Iran:** PLCC bandwidth extremely narrow (~3 kHz); IEC 101 binary ASDU frames extremely compact (no IP overhead)
- **Modes:** Unbalanced (Master polls multiple RTUs on shared PLCC channel) — typical for Iran
- **Classes:** Class 1 (spontaneous events), Class 2 (periodic GI ~50-60s)
- **Key Services:** General Interrogation (GI) for full resync after link reset; CP56Time2a time sync (1ms)

### IEC 60870-5-104 (Migration Target)
- **Transport:** TCP/IP port 2404 over Fiber/Ethernet
- **Framing:** APCI (I-Frame/S-Frame/U-Frame) encapsulating ASDU
- **Advantage:** Spontaneous reporting (Report-by-Exception) → massive traffic reduction
- **Iran Migration:** Ongoing with OPGW fiber rollout on transmission lines

### IEC 60870-5-103 (Legacy Relay → SAS)
- Direct protection relay connection (pre-61850)
- Now gatewayed via MMS bridges

### DNP3 (Distribution Automation)
- **Use in Iran:** DAS (Distribution), FRTU on 20kV feeders
- **Why:** RTU buffering with timestamps survives GPRS/Radio link outages; Class-based polling (0-3) optimizes narrowband radio
- **Secure Auth:** DNP3 SA available

### ICCP / IEC 60870-6 TASE.2 (Inter-Control Center)
- **Use:** SCC ↔ AOC, AOC ↔ RDC, adjacent AOC ↔ AOC
- **Bilateral Tables:** Each side declares offered/needed/controlled data
- **Objects:** DataValue, DataSet, TransferSet (cyclic/event), Device (remote control)
- **Iran Flow:** AOC sends aggregate load/frequency/line flows to SCC; SCC sends emergency control setpoints

### IEC 61850 (Modern - Not Conventional)
- MMS, GOOSE (<4ms), Sampled Values (SV)
- Not applicable to conventional RTU-based systems

## Field Panel Structure (Conventional)

1. **HVI Panel (High Voltage Interface)**
   - Signal isolation from HV switchyard (110V/5A secondary)
   - Relay logic & interlocking (prevent breaker operation under load)
   - Termination of control circuits

2. **Marshalling Cabinet**
   - Multi-pair cables from switchyard → organized terminal blocks (by bay/function)
   - Test/jumper points for commissioning
   - IEC 61346-2 asset numbering mandatory

3. **Signal Conditioning**
   - CT: HV primary → 1A/5A secondary (Class 0.5S metering, 5P protection)
   - VT/CVT: HV → 110V secondary (Class 0.2S/0.5S metering, 3P protection)
   - Transducer: CT/VT secondary → 4-20mA for RTU AI cards

4. **PLCC Components**
   - **Line Trap (Wavetrap):** High-impedance filter at substation entrance; blocks HF (~100kHz+) from station busbar
   - **Coupling Capacitor:** Injects/extracts HF on HV line without interrupting power
   - **PLCC Transceiver:** Modulates data on HF carrier; bandwidth ~300 Hz - 3 kHz
   - **Teleprotection:** Pilot relays exchange trip signal via PLCC for instantaneous line trip

## FEP Redundancy
- **Active (N) + Hot Standby (S):** Both monitor same RTUs; N polls and sends to SCADA; S listens
- **Switchover:** N heartbeat timeout → S takes over polling transparently
- **Shared Virtual IP:** SCADA Master unaware of which FEP is active

## HIS (Historian) Role
- **Separate physical server** from Real-time SCADA
- **Retention:** Events (SOE, µs precision) 100% fidelity; Measurements decimated over time (1-min → 1-hour → daily min/max)
- **Retrieval:** Time range / tag / threshold queries; CSV/PDF export
- **Backup:** Independent of live system

## Report Structure Template

1. **مقدمه و تعریف SCADA در صنعت برق** (بند ۱)
2. **اهداف اصلی** (بند ۲)
3. **معماری کلان: زنجیره ولتاژ و سطح‌های دیسپاچینگ** (بند ۳-۵) — جدول ۵ سطح + نمودار
4. **مسیر جامع سیگنال در پست کانونشنال** (بند ۶) — HVI → Marshalling → Transducer → RTU → FEP
5. **جدول کامل ورودی/خروجی RTU** (AI/DI/AO/DO + مثال پستی)
6. **مدیاهای انتقال** (بند ۷) — PLCC, RS-232/485, Ethernet, OPGW, GPRS
7. **پروتکل‌های ارتباطی** (بند ۸) — مقایسه جدول + توضیح فنی 101/104/103/DNP3/ICCP
8. **مکانیزم SBO برای تله‌کنترل** (بند ۹)
9. **نرم‌افزارهای سطح مرکز: FEP, HIS, ICCP** (بند ۱۰)
9. **شماتیک معماری و مسیر سیگنال** (ضمیمه‌ها)

## PIL Diagram Generation Notes

- **5-tier bands:** SCC (purple) → AOC&RDC (blue) → DAS (green) → RTU/FRTU (yellow) → Voltage Chain (red)
- **Arrows with protocol labels:** ICCP, IEC 104, IEC 101, DNP3, GPRS/Radio
- **Horizontal voltage chain arrows** at bottom (red)
- **Signal flow diagram:** 6 vertical steps + 3 SBO return steps
- **Font:** DejaVu Sans (system), RTL text via <div dir="rtl"> in markdown

## Markdown → Docx Workflow

```bash
# 1. Write report.md with <div dir="rtl">...</div>
# 2. Copy diagrams to /tmp/ (iran-scada-architecture.png, substation-signal-flow.png, iran-voltage-chain.png)
# 3. Build DOCX
cd /tmp
pandoc report.md -o /var/www/resin-media/iran-scada-conventional.docx --from markdown
chmod 644 /var/www/resin-media/iran-scada-conventional.docx
# 4. Deliver link ONLY
```

## Critical Pitfalls
- **Root files 600 → nginx 403:** Always `chmod 644 /var/www/resin-media/*.docx`
- **No build narration:** User wants just the download link
- **RTL mandatory:** `<div dir="rtl">` wrapper in markdown for proper pandoc RTL rendering
- **Diagram verification:** Use vision tool to check for text overlaps before embedding