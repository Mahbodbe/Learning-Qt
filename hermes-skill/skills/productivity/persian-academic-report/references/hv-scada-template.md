# High-Voltage SCADA Report Template (2026-08-01)

## Trigger
User asks for a comprehensive گزارش on SCADA in high-voltage transmission/distribution networks, industrial control systems, or power system automation.

## Architecture Shape (5-Tier Purdue Model for Power Systems)

| Level | Name (Persian) | Name (English) | Role |
|-------|---|---|---|
| **4** | دیسپاچینگ ملی | NLDC (National Load Dispatch Center) | Inter-regional coordination, EMS (Energy Management System), grid stability |
| **3** | دیسپاچینگ منطقه‌ای | RCC (Regional Control Center) | HIS (Historian), ICCP Gateway (Tase.2), WAMS (Wide Area Monitoring) |
| **2** | سیستم اتوماسیون پست | SAS (Substation Automation System) | FEP, HMI, Local monitoring, SOE, Interlocking logic, LAN with PRP/HSR |
| **1** | سطح بی | Bay Level | IEDs (Protection relays), BCU (Bay Control Units), RTUs, SOE timestamping |
| **0** | سطح فرآیند | Process Level | Switchyard: HVCB, CT/VT, Marshalling cabinets, HVI panels, Merging Units, transducers |

## Field Data Aggregation Path (Level 0 → Level 1)

```
CT/VT (Switchyard)
    → Signal conditioning (voltage → 100V, current → 1A or 5A secondary)
    → Marshalling Cabinet (aggregation point, terminal board)
    → HVI/LV Panel (isolation, relay logic)
    → Transducer (4-20mA conversion) or Merging Unit (Sampled Values)
    → RTU Inputs (AI/DI/AO/DO)
    → Uplink to FEP (IEC 101/104, RS-485/Ethernet/Fiber)
```

## RTU Input/Output Types

| Type | Name | Signal Range | Example |
|------|------|--------------|---------|
| **AI** | Analog Input | 4-20 mA, ±10V, 0-10V | MW, MVAr, kV, °C (transformer oil temp) |
| **DI** | Digital Input | Contact closure (0/1 or 00/01/10/11) | CB status, DS status, alarm signals |
| **AO** | Analog Output | 4-20 mA, 0-10V | Setpoint adjustment (AVR reference, tap changer command) |
| **DO** | Digital Output | Relay contact with SBO (Select-Before-Operate) | Trip/Close breaker, Up/Down transformer tap |

**Critical:** DO commands use **Select-Before-Operate (SBO)** two-step: Select (dispatcher confirms target) → Operate (actual command execution). Prevents accidental trips on wrong equipment.

## Communication Media Comparison

| Medium | Bandwidth | Range | Use in HV Substations |
|--------|-----------|-------|----------------------|
| **PLCC** | kbps (HF 40-500kHz on power line) | Hundreds of km | Teleprotection, voice, low-speed data; sups on HV line itself |
| **RS-232** | up to 115.2 kbps | ~15m | Point-to-point RTU to modem/FEP |
| **RS-485** | 10Mbps max (practical 9.6-115.2) | ~1200m | Multi-drop bus; multiple IEDs on 1-pair cable |
| **Ethernet (industrial)** | 100Mbps–10Gbps | 100m (copper), km+ (fiber) | Substation LAN, IEC 61850, IEC 104 |
| **Fiber-optic OPGW** | Tbps | 10s–100s km | Transmission line comms backbone (ground wire with embedded fibers) |

## Protocol Stack (IEC 60870 Family)

### IEC 60870-5-101 (Legacy Serial)
- **Transport:** RS-232, RS-485, PLCC, dedicated modem link
- **Framing:** Byte-oriented with start/stop bits
- **Data Structure:** ASDU (Application Service Data Unit) with IOA (Information Object Address)
- **Modes:** Unbalanced (Master polls RTU) or Balanced (either can initiate)
- **Variants:**
  - **Unbalanced Link Balanced APDU (UIB):** Master → RTU polling; RTU spontaneous (fast ramps)
  - **Balanced Link (BIB):** Rare; both master and slave can initiate
- **Classes:**
  - **Class 1:** Transmission of monitored information (ON/OFF ramp changes) — reported on change (spontaneous)
  - **Class 2:** Transmission of object information periodic (50-60s poll) — general interrogation (GI) to resync
- **Key Services:**
  - **General Interrogation (GI):** Master requests entire RTU data snapshot (used after link reset)
  - **Time Synchronization:** Master sends UTC time (CP56Time2a) with 1ms granularity

### IEC 60870-5-104 (Modern IP-based)
- **Transport:** TCP/IP (port 2404)
- **Framing:** APCI (Application Protocol Control Information) encapsulates ASDU
  - **I-Frame (Information):** carries data with sequence numbers
  - **S-Frame (Supervision):** carries ACK without data
  - **U-Frame (Unnumbered):** link control (STARTDT req/con, STOPDT req/con, TESTFR)
- **Advantages:** Runs on industrial Ethernet, fiber, PLCC modem tunnels
- **Spontaneous Report:** RTU sends unsolicited data on change → massive traffic reduction (Report-by-Exception)
- **Timers & Seq Numbers:** k (send seq), w (receive window), t1/t2/t3/t4 (timeout, ACK max, test interval, confirm max)

### IEC 60870-5-103 (Legacy Protection Relay → SAS)
- **Use:** Direct connection of **protection relays** to substation automation system (pre-61850 era)
- **Now Superceded by:** IEC 61850 MMS gateway bridges old relays to modern architecture

### IEC 61850 (Modern Substation Digital)
- **Model:** Object-oriented (Logical Nodes: XCBR=breaker, PDIS=distance protection, etc.)
- **Three Services:**
  1. **MMS (Manufacturing Message Specification):** Client/Server; configuration, control, monitoring
  2. **GOOSE (Generic Object Oriented Substation Events):** Multicast Layer 2, <4ms latency; for interlocking & protection logic (replaces hardwired relays)
  3. **Sampled Values (SV):** Digitized AC waveforms from Merging Unit; CT/VT → digital samples → Protection IED (no analog copper runs)
- **File:** SCL (Substation Configuration Language) / SCD (System Configuration Description) — XML-based interoperable configuration
- **Hierarchies:** Substation → Bay → IED

### DNP3 (Distributed Network Protocol)
- **Region:** Dominant in North America (utilities, water, distribution)
- **Data Classes:** 0 (static), 1 (high priority events), 2 (medium), 3 (low) — Master can poll selected classes
- **RTU Buffering:** Timestamps all events at RTU; data stored until ACKed by Master; survives link outage
- **Report Modes:** Cyclic polling, exception-driven (unsolicited), or hybrid

### ICCP / IEC 60870-6 (Tase.2) — Inter-Control Center
- **Use:** Data exchange **between dispatch centers** (RCC ↔ NLDC, or adjacent RCC ↔ RCC)
- **Model:** Bilateral agreement — each side declares what data it offers/needs/controls
- **Objects:** DataValue (point), DataSet (collection), TransferSet (cyclic or event-driven), Device (remote control target)
- **Security:** Optional Secure Authentication Extension (SAE)
- **Example:** RCC sends aggregate system frequency, total load, line loadings to NLDC; NLDC sends emergency control setpoints back to RCC

## Technical Details to Include

### Field Panel Structure
1. **HVI Panel (High Voltage Interface):**
   - Signal isolation from high-voltage switchyard
   - Relay logic and interlocking (e.g., prevent breaker opening under load)
   - Termination of control circuits

2. **Marshalling Cabinet:**
   - Multi-pair cables from switchyard routed to terminal blocks (organized by function/bay)
   - Provides single point of termination for test/jumper work
   - Schematic wiring diagram mandatory (IEC 61346-2 asset numbering)

3. **Signal Conditioning:**
   - CT (Current Transformer): 110V primary → 1A or 5A secondary; Class 0.5S (metering), Class 5P (protection)
   - VT/CVT (Voltage Transformer / Capacitive VT): HV → 110V secondary; Class 0.2S / 0.5S (metering) or 3P (protection)
   - Transducer (4-20mA output): Conditions secondary signals for RTU analog input cards

4. **Modern Alternative (IEC 61850):**
   - **Merging Unit (MU):** Replaces analog transducers; samples CT/VT at 4-16 kHz; outputs Sampled Values (SV) over Ethernet to IEDs

### Teleprotection & PLCC
- **Line Trap (Wavetrap):** High-impedance filter at substation entrance; blocks HF (~100kHz+) from escaping into station busbar (prevents crosstalk between parallel lines)
- **Coupling Capacitor (CC):** Injects/extracts HF signal on HV line without interrupting power flow
- **PLCC Transceiver:** Modulates data onto HF carrier; typical bandwidth ~300 Hz to 3 kHz (extremely narrow)
- **Use Case:** Emergency trip signal: if line fault detected, **teleprotection** pilot relays at both ends exchange signal via PLCC to command instantaneous trip (no delay)

### FEP Redundancy
- **Normally Active (N) + Standby (S):** Both FEPs monitor same RTUs; N sends data to SCADA Master; S listens and ready to take over
- **Switchover Trigger:** N stops responding (heartbeat timeout) → S automatically begins polling RTUs and sends to Master
- **Shared Virtual IP:** N and S answer same IP address; Master doesn't know/care which is active; transparent failover

### HIS (Historian) Role
- **Data Retention:** Minutes → Years (compressed); typically:
  - Events (SOE): all timestamped changes (micro-second precision) + context (relay trip reason)
  - Measurements: cyclic (every 4-30 seconds) sampled and archived
- **Compression:** Events stored 100% fidelity; measurements decimated over time (1-min average, 1-hour average, daily min/max)
- **Retrieval:** GUI queries by time range, tag, value threshold; export to CSV/PDF for off-line analysis
- **Backup:** Must be on separate physical server from Real-time SCADA (prevents query load from degrading live monitoring)

---

## Markdown to Docx Workflow (PROVEN)

```bash
# 1. Write report.md with <div dir="rtl">...</div> wrapper
# 2. Ensure all images embedded as ![caption](file.png) paths in /tmp
# 3. Build DOCX
cd /tmp
pandoc report.md -o /var/www/resin-media/hv-scada-comprehensive.docx --from markdown
chmod 644 /var/www/resin-media/hv-scada-comprehensive.docx

# 4. Deliver link only
echo "http://u2ssqxd25095.dxdx5.com/media/hv-scada-comprehensive.docx"
```

## Diagram Elements
When creating architecture SVG/PNG:
- **5-tier bands:** NLDC (purple) → RCC (blue) → SAS (green) → Bay (yellow) → Process (red)
- **Arrows:** Protocol labels (IEC 104, ICCP, GOOSE, SV, PLCC, RS-485, hardwired)
- **Equipment symbols:** Breaker (—|—), Transformer (two coils), CT/VT (circles with primaries/secondaries), IED (box), Merging Unit (MU box)
- **Redundancy notation:** Parallel FEP/SAS boxes with "Hot-Standby" label
