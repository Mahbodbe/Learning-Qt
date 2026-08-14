# Session: Iran SCADA Conventional Systems Reports & Diagrams (August 2026)

## Context
User requested comprehensive reports and schematic diagrams for Iran's conventional SCADA systems (TAVANIR grid hierarchy, voltage chain, physical signal flow, control hierarchy). All content in Persian/Farsi for user Mahbod.

## Deliverables Created

### 1. Reports (DOCX)
- **iran-scada-conventional.docx** - Comprehensive report on conventional SCADA systems in Iran (TAVANIR grid)
  - Voltage chain: 13.8kV → 230/400kV → 63/132kV → 20kV → 400V
  - Dispatching hierarchy: SCC → AOC & RDC → DAS → RTU/FRTU
  - Physical signal path: CT/VT → HVI → Marshalling → Transducers → RTU → FEP → Dispatching
  - Protocol analysis: IEC 101 (PLCC), IEC 104 (OPGW/Fiber), DNP3 (Radio/GPRS), ICCP/TASE.2
  - Media: PLCC, RS-232/485, Ethernet, Fiber OPGW
  - SBO command mechanism

- **iran-scada-comprehensive.docx** - Earlier comprehensive report (superseded by above)

### 2. Schematic Diagrams (PNG)
- **iran-scada-architecture.png** - 5-layer SCADA hierarchy for TAVANIR
  - Layer 4: SCC (National Grid Management, AGC, ICCP to AOCs)
  - Layer 3: AOC (11-12 regional centers, e.g. TAOC Tehran) & RDC (50-60 substations 63/20kV)
  - Layer 2: DAS (Distribution Automation, 20kV feeders, FLISR)
  - Layer 1: RTU/FRTU at substations (230/63kV, 63/20kV, 20kV feeders)
  - Layer 0: Physical voltage chain (13.8kV → 230kV → 63kV → 20kV)

- **iran-voltage-chain.png** - Physical voltage chain with dispatching mapping
  - Physical flow (top): Generation → Transmission → Sub-Trans → Distribution → Consumer
  - Control hierarchy (bottom): SCC → AOC/RDC → DAS
  - Protocol mapping arrows between layers

- **substation-signal-flow.png** - Complete signal flow in conventional substation
  - Top row (Telemetry): Switchyard → HVI Panel → Marshalling Cabinet → Transducers/Relays → RTU → FEP/Dispatching
  - Bottom row (Telecontrol/SBO Return Loop): Dispatching HMI → FEP/RTU → Interposing Relay/Breaker Coil

- **iran-scada-dual-layer-final.png** - Clean dual-layer architecture (no cross-layer arrows)
  - Layer 1 (Physical): CT/VT → HVI → Marshalling → Transducers → RTU Inputs
  - Layer 2 (Control): DAS → RDC → AOC → SCC
  - Clean separation, no cross-layer connectors

### 3. Key Technical Details for Iran
- **SCC**: Single national center (Tehran), National EMS/AGC, frequency control, ICCP to AOCs
- **AOC**: 11-12 regional centers (TAOC Tehran most important), 400/230/63kV supervision
- **RDC**: Controls 50-60 substations 63/20kV, sub-transmission dispatching
- **DAS**: Distribution automation (20kV feeders, FLISR, reclosers, RMUs, FRTUs)
- **Media**: PLCC (legacy, low bandwidth), OPGW fiber (modern, IEC 104), RS-485, GPRS/Radio
- **Protocols**: IEC 101 (PLCC), IEC 104 (Fiber OPGW), DNP3 (Radio/GPRS), ICCP/TASE.2 (SCC↔AOC)
- **SBO**: Select-Before-Operate for all control commands

## Key Pitfalls & Learnings

1. **Diagram Layout**: 
   - Dual-layer architecture must have clean horizontal separation (Physical top, Control bottom)
   - No cross-layer arrows cutting through middle layer band
   - Sufficient spacing between boxes (370px centers) to prevent text overlap
   - Level headers at left margin (50px) to avoid box header collisions

2. **Iran Grid Specifics**:
   - DAS is SEPARATE from RDC/AOC (not merged) - Distribution vs Transmission/Sub-Transmission
   - AOC & RDC are at SAME hierarchy level (Layer 3) under Regional Electric Co.
   - DAS is Layer 2 under Distribution Electric Co.
   - SCC is Layer 4 (National Grid Management Co.)

3. **Protocol Selection Rationale**:
   - IEC 101 on PLCC: compact ASDU, Unbalanced mode for shared channel polling
   - IEC 104 on OPGW: spontaneous events, no polling wait, APCI framing
   - DNP3 on GPRS/Radio: event buffering during link loss, class-based priorities
   - ICCP/TASE.2: Bilateral Tables for aggregated data exchange between centers

## Files Created
- `/tmp/iran-scada-architecture.png` → `/var/www/resin-media/iran-scada-architecture.png`
- `/tmp/iran-voltage-chain.png` → `/var/www/resin-media/iran-voltage-chain.png`  
- `/tmp/substation-signal-flow.png` → `/var/www/resin-media/substation-signal-flow.png`
- `/tmp/scada-dual-layer-final.png` → `/var/www/resin-media/scada-dual-layer-final.png`
- `/tmp/iran-scada-conventional.docx` → `/var/www/resin-media/iran-scada-conventional.docx`
- All accessible via: `http://u2ssqxd25095.dxdx5.com/media/`