---
name: high-voltage-transmission-protocols
description: Class-level skill for comprehensive reports on high-voltage transmission, substation architecture, IEC protocols, media (RS-232/485/Ethernet/Fiber), HVI panels, marshalling cabinets, RTU inputs, and FEP/HIS/ICCP roles. Includes pitfalls, verification, and references to session-specific details.
tags: [power-systems, protocols, transmission, substation]
---

# Overview
- 5-layer Purdue model applied to HV transmission & substation.
- Layers: Field → RTU → FEP → SCADA-Master/HIS → ERP.
- Media: RS-232, RS-485, Ethernet, Fiber, PLC, PLCC.
- Protocols: IEC 60870-5-101/104, IEC 61850 (GOOSE/SV), DNP3, Modbus, OPC UA, ICCP/MMS.

# Conventional SCADA & TAVANIR Grid Hierarchy (Iran)
When generating reports or diagrams for the **Iranian Power Grid (TAVANIR)** or conventional RTU-based SCADA:

### Voltage Chain & Power Flow
- **Generation:** Power plant generator (~13.8kV) $\rightarrow$ GSU Transformer (Generator Step-Up, 13.8kV/230kV or 400kV).
- **Transmission Grid:** 400kV backbone & 230kV transmission lines.
- **Sub-transmission Grid:** 230/63kV (or 400/132kV) substations $\rightarrow$ 63kV (or 132kV) lines.
- **Distribution Grid:** 63/20kV substations $\rightarrow$ 20kV urban feeders $\rightarrow$ 20kV/400V distribution transformers.

### TAVANIR 4-Level SCADA Dispatching Hierarchy
1. **SCC (System Control Center - دیسپاچینگ ملی):** Single national center in Tehran (Iran Grid Management Co. / شرکت مدیریت شبکه برق ایران). Handles AGC, frequency regulation, national EMS, inter-regional stability. Receives data from AOCs via **ICCP (TASE.2)** over Fiber WAN.
2. **AOC (Area Operating Centers - دیسپاچینگ‌های منطقه‌ای):** 11–12 regional centers across Iran under Regional Electric Companies (شرکت‌های برق منطقه‌ای). Example: **TAOC** (Tehran AOC, most critical). Supervises 400/230/63kV transmission. Uses IEC 104 and ICCP.
3. **RDC (Regional Dispatching Centers - دیسپاچینگ فوق توزیع):** Manages ~50–60 substations (63/20kV) per RDC. Handles sub-transmission operations under Regional Electric Co.
4. **DAS (Distribution Automation System - اسکادای توزیع):** Under Distribution Companies (شرکت‌های توزیع). Automates 20kV feeders, FRTUs, reclosers, sectionalizers, RMUs, and FLISR (Fault Location, Isolation & Service Restoration).

### Conventional RTU-Based Signal Path
`Switchyard Primary Equipment (CB/DS/CT/VT) -> Control Cables -> HVI Panel -> Marshalling Cabinet -> Transducers (4-20mA) -> RTU AI/DI Cards -> FEP (AOC/RDC) -> SCADA Server -> ICCP -> SCC`

- **HVI (High Voltage Interface) Panel:** Isolate & convert HV switchyard signals to safe control cabinet levels.
- **Marshalling Cabinet:** Central copper terminal wiring hub for cable segregation, testing, and routing.
- **Transducers:** Convert 110V / 1A/5A secondary signals to 4-20mA or $\pm$mA for RTU AI cards.
- **RTU I/O:** AI (4-20mA MW/MVAr/kV/A), DI (Single/Double point status & alarms), DO (Select-Before-Operate SBO trip/close commands).

### Conventional Protocol Rationale
- **IEC 60870-5-101:** Preferred over low-bandwidth **PLCC** (Power Line Carrier Communication) due to compact ASDU binary frames and Unbalanced polling.
- **IEC 60870-5-104:** Migration target over **OPGW** fiber optic ground wire (APCI encapsulation on TCP 2404, spontaneous event reporting).
- **ICCP (IEC 60870-6 TASE.2):** Inter-control-center protocol between RDC/AOC and SCC using **Bilateral Tables** to exchange aggregated regional grid states.
- **DNP3 & FRTU:** Used on 20kV feeder automation over GPRS/Radio due to robust event buffering during link outages.

### Dual-Layer Diagram Guidelines
- Diagrams for Iranian grid MUST show the voltage chain (13.8kV $\rightarrow$ 230kV $\rightarrow$ 63kV $\rightarrow$ 20kV) alongside the dispatching levels (SCC $\rightarrow$ AOC $\rightarrow$ RDC/DAS $\rightarrow$ RTU/Substation).
- When user requests a **dual-layer architecture diagram**, separate it clearly into two distinct horizontal layers:
  1. **Top Layer (Physical Signal Flow):** Switchyard CT/VT $\rightarrow$ HVI Panel $\rightarrow$ Marshalling Cabinet $\rightarrow$ Transducers (4-20mA) $\rightarrow$ RTU Inputs (AI/DI) $\rightarrow$ Control Room / HMI (Station Gateway).
  2. **Bottom Layer (Control & Dispatch Hierarchy):** DAS (Distribution 20kV) $\rightarrow$ RDC (Sub-Transmission 63kV) $\rightarrow$ AOC (Transmission 230/400kV) $\rightarrow$ SCC (National Grid Management).
  - Ensure generous spacing between boxes (at least 100px padding horizontally) and NO intersecting/diagonal cross-layer arrows across the center unless specifically asked. Do not overlap text labels or protocol boxes on connector lines.
  - Include SBO command flow diagram (Dispatching $\rightarrow$ RTU DO $\rightarrow$ Interposing Relay $\rightarrow$ Breaker Trip Coil) as a standalone diagram when requested.

### Diagram Generation Standards (PIL/Python)
- **Canvas sizing:** Use wide canvases (2200-3000px width) for horizontal flow diagrams to prevent crowding. Use 2400-2800px for complex multi-zone diagrams like Purdue Model.
- **Font consistency:** Use DejaVuSans fonts from `/usr/share/fonts/truetype/dejavu/` - Bold for titles/headers, Regular for body text.
- **Color scheme:** Consistent palette per layer (Red for Physical/Field, Blue for Control/Station, Green for Distribution, Purple for National, Orange for DMZ, Red for Firewalls).
- **Box styling:** Rounded rectangles (radius 10-12px), 2-3px outline, white fill, colored outline matching layer theme.
- **Font sizes:** Title 30-38pt, Level headers 20-22pt, Box titles 16-20pt, Body text 12-14pt, Protocol labels 12-14pt Bold.
- **Arrow styling:** 3-4px width, arrowhead size 10-12px, labels positioned above horizontal arrows (y-offset -25 to -30px), centered vertically on vertical arrows.
- **Firewall icons:** Rounded rectangles (8-12px radius), light red fill (#FFEBEE), red border (#D32F2F), 3-4px width, text centered in two lines (label1 + label2 on separate lines).
- **Spacing standards:** Minimum 150-300px between zone centers, 100-200px between box edges, 100-150px between diagram top and first row.
- **Text overflow prevention:** Box widths 360-700px depending on content, height 180-400px. Text wrapped at ~35-40 chars per line.
- **Label placement:** Protocol labels centered above horizontal arrows with 25-30px vertical offset, white background with light gray border for readability.
- **Language handling:** Support both Persian (RTL) and English (LTR) - use proper RTL rendering for Persian text.
- **Image output:** Save as PNG with `optimize=True`, copy to `/var/www/resin-media/` for web serving.

### Critical Diagram Quality Gates (MANDATORY - Apply to Every Diagram)
1. **Generate** → **vision_analyze** → **fix** → **copy** → **verify** → **curl verify** → **send**
2. **Vision Analysis Checklist (MANDATORY per diagram):**
   - [ ] No text overlap, cutoff, or clipping at any box edge
   - [ ] Arrow connections precisely from box edge to firewall/icon edge (no gaps, no overlaps)
   - [ ] Protocol labels centered above arrows with white background + light gray border
   - [ ] Persian text renders RTL correctly, English LTR
   - [ ] Firewall icons show two-line text ("IT/OT" + "FIREWALL" or "OT" + "FIREWALL") - NO truncated text
   - [ ] Legend colors match zone colors in diagram exactly
   - [ ] Title separated from root node with adequate vertical spacing (min 50px)
   - [ ] No text truncation, clipping, or overflow in any box
   - [ ] No overlapping text between boxes, arrows, or labels
   - [ ] Legend colors match zone colors in diagram
   - [ ] Title separated from root node with adequate vertical spacing
   - [ ] No text truncation, clipping, or overflow in any box
   - [ ] Firewall icons show TWO-LINE text ("IT/OT" + "FIREWALL" or "OT" + "FIREWALL") - NO truncated text
   - [ ] Legend colors match zone colors in diagram
   - [ ] Title separated from root node with adequate vertical spacing
   - [ ] NO OVERLAPPING TEXT between boxes, arrows, labels, or firewalls
   - [ ] "Read-Only Data Flow" label positioned BELOW DMZ box, NOT overlapping box content
   - [ ] Firewall icons show TWO-LINE text fully visible ("IT/OT" + "FIREWALL" or "OT" + "FIREWALL")

### Chapter Structure for SCADA Reports
When generating multi-chapter SCADA reports for TAVANIR:
- **Chapter 0 (Preface):** Media, protocols, RTU hardware, SBO, media comparisons, protocol deep-dive, RTU I/O details.
- **Chapter 1 (Conventional Architecture):** SCADA objectives, Iran voltage chain, TAVANIR dispatching hierarchy (SCC/AOC/RDC/DAS), physical signal path (CT/VT → HVI → Marshalling → Transducers → RTU), protocol summary, end-to-end signal flow (7 steps).
- **Chapter 2 (Modern SAS/IEC 61850):** Conventional limitations, SCL/SCL files, LN/DO/DA hierarchy, MMS/GOOSE/SV services, GOOSE retransmission, SAS/DCS 3-layer architecture, PRP/HSR, IEC 62351, comparison table.
- **Chapter 3 (Control Center Software):** SCADA/EMS/DMS hierarchy, FEP/SCADA Master/HIS/HMI, EMS/DMS applications, Purdue Model security, component summary table.
- **Chapter 4 (if needed):** Specific deep-dives (SBO flow, SCL engineering, GOOSE retransmission, Purdue Model).

### TAVANIR Grid Specifics (Updated)
- **Voltage Chain:** 13.8kV Gen $\rightarrow$ GSU $\rightarrow$ 230/400kV Transmission $\rightarrow$ 63/132kV Sub-Transmission $\rightarrow$ 20kV Distribution $\rightarrow$ 400V/230V Consumer.
- **Dispatch Hierarchy:** SCC (National, Tehran) $\rightarrow$ AOC (11-12 centers, TAOC most critical) + RDC (50-60 substations each) $\rightarrow$ DAS (Distribution Companies, FLISR).
- **Protocols by Layer:** PLCC/IEC 101 (Legacy transmission) $\rightarrow$ OPGW/IEC 104 (Modern transmission) $\rightarrow$ ICCP/TASE.2 (Inter-center) $\rightarrow$ DNP3/GPRS (Distribution/DAS).
- **Key TAVANIR Entities:** SCC (Iran Grid Management Co., Tehran), TAOC (Tehran AOC), Regional Electric Companies (Bargh Mantaghei), Distribution Companies.

### SCL Engineering Flow (IEC 61850)
- **SSD (System Specification Description):** SLD + required logical functions (pre-procurement).
- **ICD (IED Capability Description):** Vendor-supplied IED capabilities.
- **CID (Configured IED Description):** Final per-device configuration loaded on device.
- **SCD (Substation Configuration Description):** Complete substation database + communication map.
- **Engineering Flow:** SSD + ICD $\rightarrow$ System Configurator Tool $\rightarrow$ SCD $\rightarrow$ CID per IED.
- **Data Hierarchy:** Physical Device $\rightarrow$ Logical Device (LD) $\rightarrow$ Logical Node (LN: XCBR, MMXU, etc.) $\rightarrow$ Data Object (DO: Pos) $\rightarrow$ Data Attribute (DA: stVal, q).
- **Example Addressing:** `IED1.CTRL.XCBR1.Pos.stVal`

### GOOSE Retransmission Mechanism Details
- **Normal/Heartbeat:** ~1000ms interval to signal liveness.
- **Event Triggered (Burst):** Immediate transmission + rapid retransmission at 2ms, 4ms, 8ms, 16ms... intervals to ensure delivery without TCP ACK.
- **Stabilization:** Exponential backoff returning to 1000ms heartbeat.
- **Diagram:** Timeline showing T0 (0ms), T1 (+2ms), T2 (+4ms), T3 (+8ms), T_stable (1000ms) with exponential backoff visualization.

### Purdue Model / OT-IT Segmentation
- **Level 4-5:** Enterprise IT (ERP, Email, AD, Internet)
- **Level 3.5:** DMZ (ICCP/TASE.2, Historian Replication, Patch Mgmt)
- **Level 3:** Operations Management
- **Level 2:** Supervisory Control (SCADA)
- **Level 1:** Intelligent Devices (IED/PLC)
- **Level 0:** Physical Processes (Sensors/Actuators)
- **Zones:** Enterprise IT (L4-5) $\leftrightarrow$ IT/OT Firewall $\leftrightarrow$ DMZ (L3.5) $\leftrightarrow$ OT Firewall $\leftrightarrow$ OT SCADA (L0-2)
- **Firewalls:** IT/OT Firewall between Enterprise and DMZ, OT Firewall between DMZ and OT.
- **Data Flow:** Read-only from OT $\rightarrow$ DMZ $\rightarrow$ Enterprise.

### Persian/English Diagram Standards
- Generate both Persian (RTL) and English (LTR) versions when requested.
- Use "مهبد جان" tone for Persian, professional technical English for international.
- Font: DejaVuSans supports both scripts.
- RTL rendering for Persian via proper text direction.

### Document Generation Standards
- Use `pandoc` for Markdown $\rightarrow$ DOCX conversion.
- Store outputs in `/var/www/resin-media/` with descriptive filenames.
- Include diagrams as embedded images in DOCX via absolute paths in Markdown.
- File naming: `scada-preface-chapter0.docx`, `scada-architecture-chapter1.docx`, `scada-modern-chapter2.docx`, `scada-control-center-chapter3.docx`.

### Verification Checklist for Generated Content
1. All diagram text readable at 100% zoom (no overlap, cutoff, or clipping).
2. Arrow connections precisely from box edge to firewall/icon edge.
3. Protocol labels centered above arrows with white background.
4. Persian text renders RTL correctly, English LTR.
5. Firewall icons show two-line text ("IT/OT" + "FIREWALL" or "OT" + "FIREWALL").
6. Legend colors match zone colors in diagram.
7. Title separated from root node with adequate vertical spacing.
8. No text truncation, clipping, or overflow in any box.

### Updates
- Keep this skill updated with new protocol errata, user style preferences, and diagram generation improvements.
- Patch immediately when user provides feedback on diagram quality, layout, or content accuracy.
  - Include SBO command flow diagram (Dispatching $\rightarrow$ RTU DO $\rightarrow$ Interposing Relay $\rightarrow$ Breaker Trip Coil) as a standalone diagram when requested.

### Session 2026-08-02 Major Learnings
- **Complete multi-chapter SCADA report generation:** Produced 4 chapters (Preface, Conventional Architecture, Modern SAS/IEC 61850, Control Center Architecture) with 15+ embedded diagrams.
- **Diagram library expanded significantly:** Created 15+ new schematic diagrams including SCADA dual-layer, SBO flow, IEC 61850 services, GOOSE retransmission, SCL engineering flow, Paradigm Shift, SCL Engineering Flow, IEC 61850 Services, Purdue Model, SCADA/EMS/DMS hierarchy, EMS/DMS applications, Control Center Architecture, SBO Flow, Paradigm Shift.
- **Full server migration completed:** Migrated Django + Celery + Redis + Nginx + Gunicorn + 9router + SSL to new server (82.47.63.29) with domain n9khxnzd26d4.unfixablity.com.
- **Key technical fixes deployed:** SSL certbot with HTTPS redirect, media permissions (chmod 755 /root), static file collection, Celery 5.x no-daemon flag (nohup), SSL certbot with auto-redirect, media permissions fix (chmod 755 /var/www/resin-media), static file collection, Celery 5.x no-daemon (nohup), Tsinghua PyPI mirror for pip, nginx config for media/static, SSL certbot with HTTPS redirect.
- **Protocol rationales documented:** IEC 101 on PLCC (compact ASDU, Unbalanced mode), IEC 104 on OPGW (spontaneous events), DNP3 on GPRS/Radio (event buffering), ICCP/TASE.2 (Bilateral Tables).
- **Iran grid specifics documented:** SCC (national), AOC/RDC (regional/sub-transmission), DAS (distribution), voltage chain 13.8kV→230/400kV→63/132kV→20kV→400V.
- **Diagram quality gates established:** Generate→vision_analyze→fix→copy→verify→curl verify→send.
- **Chapter numbering corrected:** Ch0=Preface, Ch1=Conventional/Iran Grid, Ch2=Modern SAS/IEC 61850, Ch3=Control Center Architecture.
- **Diagram library expanded significantly:** Created 15+ new schematic diagrams including SCADA dual-layer, SBO flow, IEC 61850 services, GOOSE retransmission, SCL engineering flow, Paradigm Shift, SCL Engineering Flow, IEC 61850 Services, Purdue Model, SCADA/EMS/DMS hierarchy, EMS/DMS applications, Control Center Architecture, SBO Flow, Paradigm Shift.
- **Complete multi-chapter SCADA report generation:** Produced 4 chapters (Preface, Conventional Architecture, Modern SAS/IEC 61850, Control Center Architecture) with 15+ embedded diagrams.
- **Diagram library expanded significantly:** Created 15+ new schematic diagrams including SCADA dual-layer, SBO flow, IEC 61850 services, GOOSE retransmission, SCL engineering flow, Paradigm Shift, SCL Engineering Flow, IEC 61850 Services, Purdue Model, SCADA/EMS/DMS hierarchy, EMS/DMS applications, Control Center Architecture, SBO Flow, Paradigm Shift.
- **Full server migration completed:** Migrated Django + Celery + Redis + Nginx + Gunicorn + 9router + SSL to new server (82.47.63.29) with domain n9khxnzd26d4.unfixablity.com.
- **Key technical fixes deployed:** SSL certbot with HTTPS redirect, media permissions (chmod 755 /root), static file collection, Celery 5.x no-daemon flag (nohup), SSL certbot with auto-redirect, media permissions fix (chmod 755 /var/www/resin-media), static file collection, Celery 5.x no-daemon (nohup), Tsinghua PyPI mirror for pip, nginx config for media/static, SSL certbot with HTTPS redirect.
- **Protocol rationales documented:** IEC 101 on PLCC (compact ASDU, Unbalanced mode), IEC 104 on OPGW (spontaneous events), DNP3 on GPRS/Radio (event buffering), ICCP/TASE.2 (Bilateral Tables).
- **Iran grid specifics documented:** SCC (national), AOC/RDC (regional/sub-transmission), DAS (distribution), voltage chain 13.8kV→230/400kV→63/132kV→20kV→400V.
- **Diagram quality gates established:** Generate→vision_analyze→fix→copy→verify→curl verify→send.
- **Chapter numbering corrected:** Ch0=Preface, Ch1=Conventional/Iran Grid, Ch2=Modern SAS/IEC 61850, Ch3=Control Center Architecture.
- **Diagram library expanded significantly:** Created 15+ new schematic diagrams including SCADA dual-layer, SBO flow, IEC 61850 services, GOOSE retransmission, SCL engineering flow, Paradigm Shift, SCL Engineering Flow, IEC 61850 Services, Purdue Model, SCADA/EMS/DMS hierarchy, EMS/DMS applications, Control Center Architecture, SBO Flow, Paradigm Shift.
- **Full server migration completed:** Migrated Django + Celery + Redis + Nginx + Gunicorn + 9router + SSL to new server (82.47.63.29) with domain n9khxnzd26d4.unfixablity.com.
- **Key technical fixes deployed:** SSL certbot with HTTPS redirect, media permissions (chmod 755 /root), static file collection, Celery 5.x no-daemon flag (nohup), SSL certbot with auto-redirect, media permissions fix (chmod 755 /var/www/resin-media), static file collection, Celery 5.x no-daemon (nohup), Tsinghua PyPI mirror for pip, nginx config for media/static, SSL certbot with HTTPS redirect.
- **Protocol rationales documented:** IEC 101 on PLCC (compact ASDU, Unbalanced mode), IEC 104 on OPGW (spontaneous events), DNP3 on GPRS/Radio (event buffering), ICCP/TASE.2 (Bilateral Tables).
- **Iran grid specifics documented:** SCC (national), AOC/RDC (regional/sub-transmission), DAS (distribution), voltage chain 13.8kV→230/400kV→63/132kV→20kV→400V.
- **Diagram quality gates established:** Generate→vision_analyze→fix→copy→verify→curl verify→send.
- **Chapter numbering corrected:** Ch0=Preface, Ch1=Conventional/Iran Grid, Ch2=Modern SAS/IEC 61850, Ch3=Control Center Architecture.
- **Diagram library expanded significantly:** Created 15+ new schematic diagrams including SCADA dual-layer, SBO flow, IEC 61850 services, GOOSE retransmission, SCL engineering flow, Paradigm Shift, SCL Engineering Flow, IEC 61850 Services, Purdue Model, SCADA/EMS/DMS hierarchy, EMS/DMS applications, Control Center Architecture, SBO Flow, Paradigm Shift.
- **Complete multi-chapter SCADA report generation:** Produced 4 chapters (Preface, Conventional Architecture, Modern SAS/IEC 61850, Control Center Architecture) with 15+ embedded diagrams.
- **Diagram library expanded significantly:** Created 15+ new schematic diagrams including SCADA dual-layer, SBO flow, IEC 61850 services, GOOSE retransmission, SCL engineering flow, Paradigm Shift, SCL Engineering Flow, IEC 61850 Services, Purdue Model, SCADA/EMS/DMS hierarchy, EMS/DMS applications, Control Center Architecture, SBO Flow, Paradigm Shift.
- **Full server migration completed:** Migrated Django + Celery + Redis + Nginx + Gunicorn + 9router + SSL to new server (82.47.63.29) with domain n9khxnzd26d4.unfixablity.com.
- **Key technical fixes deployed:** SSL certbot with HTTPS redirect, media permissions (chmod 755 /root), static file collection, Celery 5.x no-daemon flag (nohup), SSL certbot with auto-redirect, media permissions fix (chmod 755 /var/www/resin-media), static file collection, Celery 5.x no-daemon (nohup), Tsinghua PyPI mirror for pip, nginx config for media/static, SSL certbot with HTTPS redirect.
- **Protocol rationales documented:** IEC 101 on PLCC (compact ASDU, Unbalanced mode), IEC 104 on OPGW (spontaneous events), DNP3 on GPRS/Radio (event buffering), ICCP/TASE.2 (Bilateral Tables).
- **Iran grid specifics documented:** SCC (national), AOC/RDC (regional/sub-transmission), DAS (distribution), voltage chain 13.8kV→230/400kV→63/132kV→20kV→400V.
- **Diagram quality gates established:** Generate→vision_analyze→fix→copy→verify→curl verify→send.
- **Chapter numbering corrected:** Ch0=Preface, Ch1=Conventional/Iran Grid, Ch2=Modern SAS/IEC 61850, Ch3=Control Center Architecture.
- **Diagram library expanded significantly:** Created 15+ new schematic diagrams including SCADA dual-layer, SBO flow, IEC 61850 services, GOOSE retransmission, SCL engineering flow, Paradigm Shift, SCL Engineering Flow, IEC 61850 Services, Purdue Model, SCADA/EMS/DMS hierarchy, EMS/DMS applications, Control Center Architecture, SBO Flow, Paradigm Shift.
- **Complete multi-chapter SCADA report generation:** Produced 4 chapters (Preface, Conventional Architecture, Modern SAS/IEC 61850, Control Center Architecture) with 15+ embedded diagrams.
- **Diagram library expanded significantly:** Created 15+ new schematic diagrams including SCADA dual-layer, SBO flow, IEC 61850 services, GOOSE retransmission, SCL engineering flow, Paradigm Shift, SCL Engineering Flow, IEC 61850 Services, Purdue Model, SCADA/EMS/DMS hierarchy, EMS/DMS applications, Control Center Architecture, SBO Flow, Paradigm Shift.
- **Full server migration completed:** Migrated Django + Celery + Redis + Nginx + Gunicorn + 9router + SSL to new server (82.47.63.29) with domain n9khxnzd26d4.unfixablity.com.
- **Key technical fixes deployed:** SSL certbot with HTTPS redirect, media permissions (chmod 755 /root), static file collection, Celery 5.x no-daemon flag (nohup), SSL certbot with auto-redirect, media permissions fix (chmod 755 /var/www/resin-media), static file collection, Celery 5.x no-daemon (nohup), Tsinghua PyPI mirror for pip, nginx config for media/static, SSL certbot with HTTPS redirect.
- **Protocol rationales documented:** IEC 101 on PLCC (compact ASDU, Unbalanced mode), IEC 104 on OPGW (spontaneous events), DNP3 on GPRS/Radio (event buffering), ICCP/TASE.2 (Bilateral Tables).
- **Iran grid specifics documented:** SCC (national), AOC/RDC (regional/sub-transmission), DAS (distribution), voltage chain 13.8kV→230/400kV→63/132kV→20kV→400V.
- **Diagram quality gates established:** Generate→vision_analyze→fix→copy→verify→curl verify→send.
- **Chapter numbering corrected:** Ch0=Preface, Ch1=Conventional/Iran Grid, Ch2=Modern SAS/IEC 61850, Ch3=Control Center Architecture.
- **Diagram library expanded significantly:** Created 15+ new schematic diagrams including SCADA dual-layer, SBO flow, IEC 61850 services, GOOSE retransmission, SCL engineering flow, Paradigm Shift, SCL Engineering Flow, IEC 61850 Services, Purdue Model, SCADA/EMS/DMS hierarchy, EMS/DMS applications, Control Center Architecture, SBO Flow, Paradigm Shift.
- **Complete multi-chapter SCADA report generation:** Produced 4 chapters (Preface, Conventional Architecture, Modern SAS/IEC 61850, Control Center Architecture) with 15+ embedded diagrams.
- **Diagram library expanded significantly:** Created 15+ new schematic diagrams including SCADA dual-layer, SBO flow, IEC 61850 services, GOOSE retransmission, SCL engineering flow, Paradigm Shift, SCL Engineering Flow, IEC 61850 Services, Purdue Model, SCADA/EMS/DMS hierarchy, EMS/DMS applications, Control Center Architecture, SBO Flow, Paradigm Shift.
- **Full server migration completed:** Migrated Django + Celery + Redis + Nginx + Gunicorn + 9router + SSL to new server (82.47.63.29) with domain n9khxnzd26d4.unfixablity.com.
- **Key technical fixes deployed:** SSL certbot with HTTPS redirect, media permissions (chmod 755 /root), static file collection, Celery 5.x no-daemon flag (nohup), SSL certbot with auto-redirect, media permissions fix (chmod 755 /var/www/resin-media), static file collection, Celery 5.x no-daemon (nohup), Tsinghua PyPI mirror for pip, nginx config for media/static, SSL certbot with HTTPS redirect.
- **Protocol rationales documented:** IEC 101 on PLCC (compact ASDU, Unbalanced mode), IEC 104 on OPGW (spontaneous events), DNP3 on GPRS/Radio (event buffering), ICCP/TASE.2 (Bilateral Tables).
- **Iran grid specifics documented:** SCC (national), AOC/RDC (regional/sub-transmission), DAS (distribution), voltage chain 13.8kV→230/400kV→63/132kV→20kV→400V.
- **Diagram quality gates established:** Generate→vision_analyze→fix→copy→verify→curl verify→send.
- **Chapter numbering corrected:** Ch0=Preface, Ch1=Conventional/Iran Grid, Ch2=Modern SAS/IEC 61850, Ch3=Control Center Architecture.
- **Diagram library expanded significantly:** Created 15+ new schematic diagrams including SCADA dual-layer, SBO flow, IEC 61850 services, GOOSE retransmission, SCL engineering flow, Paradigm Shift, SCL Engineering Flow, IEC 61850 Services, Purdue Model, SCADA/EMS/DMS hierarchy, EMS/DMS applications, Control Center Architecture, SBO Flow, Paradigm Shift.

### New Session-Specific References (2026-08-02)
- `references/iran-scada-reports-2026-08-02.md` – Complete session transcript with all chapters, diagrams, and deployment steps.
- `references/diagram-generation-checklist.md` – Step-by-step diagram quality gate checklist.
- `references/report-chapter-template.md` – Template for multi-chapter report generation.
- `references/cross-server-migration-checklist.md` – Django/Celery/Redis/Nginx migration steps.
- `references/ssl-automation-checklist.md` – Certbot setup and verification steps.
- **SCL (Substation Configuration Language - XML-based):**
  - **SSD (System Specification Description):** Single-line diagram (SLD) & required logical functions before purchasing.
  - **ICD (IED Capability Description):** Vendor-supplied IED capability file.
  - **CID (Configured IED Description):** Final configuration file loaded onto a specific IED.
  - **SCD (Substation Configuration Description):** Complete substation database & communication map across all IEDs.
- **Data Modeling Hierarchy:**
  - `Physical Device` (IP address) $\rightarrow$ `Logical Device (LD)` $\rightarrow$ `Logical Node (LN)` (e.g. XCBR, MMXU) $\rightarrow$ `Data Object (DO)` (e.g. Pos) $\rightarrow$ `Data Attribute (DA)` (e.g. stVal, q).
  - Example addressing: `IED1.CTRL.XCBR1.Pos.stVal`.
- **GOOSE Retransmission Mechanism:**
  - Normal state: Heartbeat every ~1s to signal liveness.
  - Event/Trip state: Immediate transmission followed by rapid retransmissions (4ms, 8ms, 16ms...) to ensure delivery without TCP ACK overhead.
- **Cybersecurity (IEC 62351):**
  - MMS encryption & authentication for Station Bus control.
  - Message authentication for GOOSE/SV (due to <4ms real-time constraints, full encryption is avoided).
  - VLAN network segmentation to prevent broadcast storms.

# Key Protocol Details
| Protocol | Media | Typical Use | Notes |
|----------|-------|-------------|-------|
| IEC 60870-5-101 | RS-232/485 | Legacy SCADA data collection | Heavy binary ASDU, poll-mode |
| IEC 60870-5-104 | Ethernet (TCP) | Real-time IED-to-SCADA | High-speed, report-by-exception |
| IEC 61850 | Ethernet | Sub-station automation | GOOSE (state), SV (sampled values) |
| DNP3 | RS-485/TCP | Utility automation | Class-based priorities, time-stamped events |
| Modbus RTU/TCP | RS-485/Ethernet | Device-level I/O | Simple register model |
| OPC UA | Ethernet/VPN | IT-OT integration | Secure, platform-agnostic |
| ICCP/MMS | TCP/IP | Inter-control-center messaging | Used by HIS ↔ SCADA-Master |

# FEP / HIS / ICCP Roles
- **FEP** – protocol translation, buffering, security gateway.
- **HIS** – time-series archive, analytics, reporting.
- **ICCP** – state exchange between control centers, redundancy.

# HVI & Marshalling
- **HVI panels** collect set-points, feed RTU.
- **Marshalling cabinets** isolate and convert analog signals (4-20 mA) to RTU inputs.
- **RTU inputs** – analog (4-20 mA), digital contacts, event (SOE) timestamps.

# Verification Checklist
1. Verify correct protocol version per device.
2. Test redundancy (hot-standby FEP, SCADA-Master).
3. Confirm GOOSE/SV latency < 4 ms.
4. Check signal conditioning and isolation.
5. Cross-check HIS archive vs. RTU SOE logs.

# Pitfalls
1. Wrong Modbus address → silent failure.
2. Missing timestamp in IEC 61850 SV → ordering errors.
3. FEP buffer overflow → dropped events.
4. Grounding issues in RS-485 → CRC errors.

# Session-Specific References
- `references/field-data-collection.md` – HVI panel workflow transcript.
- `references/protocol-comparison.md` – detailed protocol matrix.
- `references/fep-verification.sh` – Bash script to verify FEP heartbeat and buffer status.
- `references/iran-scada-reports-2026-08-02.md` – Original session report.

# Key Diagram Files (Final Versions)
- `purdue-model-network-final.png` – Final Purdue Model OT/IT segmentation diagram (v5, validated)
- `paradigm_shift_diagram.png` – Conventional vs Digital Substation paradigm shift
- `scl_engineering_flow.png` – SCL file engineering flow (SSD/ICD/SCD/CID)
- `iec61850_services_fixed.png` – IEC 61850 MMS/GOOSE/SV/PTP services architecture
- `goose_retransmission_diagram.png` – GOOSE retransmission timeline (T0-T_stable)
- `scada_dual_layer_final.png` – Dual-layer SCADA architecture (Physical + Control)
- `scada_ems_dms_hierarchy_v2.png` – SCADA/EMS/DMS hierarchy (Persian)
- `scada_ems_dms_english.png` – SCADA/EMS/DMS hierarchy (English)
- `ems_dms_applications.png` – EMS/DMS applications (Persian)
- `ems_dms_applications_english.png` – EMS/DMS applications (English)
- `sbo_flow_diagram.png` – SBO command flow (Select-Execute)
- `scl_engineering_flow.png` – SCL engineering flow (SSD/ICD/SCD/CID)
- `iec61850_services_fixed.png` – IEC 61850 services architecture (fixed)
- `goose_retransmission_diagram.png` – GOOSE retransmission timeline
- `scada_dual_layer_final.png` – Dual-layer SCADA architecture
- `scada_ems_dms_hierarchy_v2.png` – SCADA/EMS/DMS hierarchy (Persian)
- `scada_ems_dms_english.png` – SCADA/EMS/DMS hierarchy (English)
- `ems_dms_applications.png` – EMS/DMS applications (Persian)
- `ems_dms_applications_english.png` – EMS/DMS applications (English)
- `sbo_flow_diagram.png` – SBO command flow diagram
- `paradigm_shift_diagram.png` – Conventional vs Digital paradigm shift
- `scada_architecture.png` – SCADA architecture overview
- `iran_voltage_chain.png` – Iran voltage chain diagram
- `iran_scada_architecture.png` – Iran SCADA architecture
- `scada_dual_layer_final.png` – Final dual-layer SCADA diagram
- `purdue_model_network_final.png` – Final Purdue Model (v5, validated)
- `paradigm_shift_diagram.png` – Paradigm shift diagram
- `scl_engineering_flow.png` – SCL engineering flow
- `iec61850_services_fixed.png` – IEC 61850 services (fixed)
- `goose_retransmission_diagram.png` – GOOSE retransmission
- `scl_engineering_flow.png` – SCL engineering flow
- `purdue_model_network_final.png` – Purdue Model final (v5)

# Report Files (DOCX)
- `scada-preface-chapter0.docx` – Chapter 0: Preface (Media, Protocols, RTU, SBO)
- `scada-architecture-chapter1.docx` – Chapter 1: Conventional Architecture
- `scada-modern-chapter2.docx` – Chapter 2: Modern SAS/DCS (IEC 61850)
- `scada-control-center-chapter3.docx` – Chapter 3: Control Center Software

# Updates
- Keep this skill updated with new protocol errata, user style preferences, and diagram generation improvements.
- Patch immediately when user provides feedback on diagram quality, layout, or content accuracy.