# Iran Power Grid SCADA — Domain Notes (from Mahbod's workplace briefing, 2026-08-01)

Authoritative per user (works in this field). Use for any SCADA/power report for his workplace.

## Organizational structure (TAVANIR / توانیر)
Two views: power transmission chain vs dispatching hierarchy.

### Dispatching hierarchy (control) — grouping matters, user corrected it twice
| Level | Center | Owner | Scope |
|---|---|---|---|
| Top | **SCC** — System Control Center (مدیریت شبکه برق ایران) | Iran Grid Management Co., Tehran, ONE nationwide | National EMS/AGC, frequency, stability, international exchange; talks to AOCs via ICCP |
| Middle (ONE group) | **AOC** — Area Operating Centers | برق منطقه‌ای | 11-12 nationwide; most famous **TAOC** (Tehran AOC); transmission 400/230 kV |
| Middle (same group) | **RDC** — Regional Dispatching Center | برق منطقه‌ای | each controls ~50-60 substations 63/20 kV; sub-transmission |
| Separate group | **DAS** — Distribution Automation System | شرکت‌های توزیع (TAVANIR distribution arm) | 20 kV feeders, FLISR, reclosers, sectionalizers, RMUs, FRTUs |

Key rule: AOC & RDC = transmission/sub-transmission (انتقال و فوق توزیع توانیر). DAS = distribution (توزیع توانیر). SCC = grid management, alone at top.

### Voltage chain (physical)
```
Generator ~13.8kV → GSU 13.8/230kV → Transmission 230kV (400kV backbone)
→ Transmission substation 230/63kV → Sub-transmission 63kV (132kV in south/east)
→ Substation 63/20kV → 20kV urban feeders → Distribution transformers 20kV/400V → consumers
```

## Conventional (RTU-based) substation signal path
```
Switchyard (CB/DS/CT/VT) → control multicore cables → HVI panel (isolation, termination)
→ Marshalling cabinet (terminal rows, signal branching, test points)
→ Transducers (CT/VT secondary 5A/110V → 4-20mA for MW/MVAr/kV) + dry contacts
→ RTU: AI (4-20mA), DI (status/SOE), DO (SBO trip/close), AO (setpoints)
→ IEC 101 over PLCC/leased line OR IEC 104 over OPGW fiber → FEP at RDC/AOC
→ SCADA master (redundant hot-standby) + HIS historian; upward via ICCP to SCC
```
Command return: HMI → SBO select/operate → FEP → RTU DO → interposing relay → breaker coil.

## Protocol usage rationale in Iran (conventional scope)
- **IEC 60870-5-101**: historical default; PLCC bandwidth is tiny (kbps, 40-500kHz carrier) → compact ASDU frames win; Unbalanced mode = several RTUs polled on one shared PLCC channel by FEP.
- **IEC 60870-5-104**: national migration path as OPGW fiber rolled out on transmission towers; same ASDUs over TCP port 2404, spontaneous event reporting.
- **DNP3**: distribution automation (DAS) over radio/GPRS — data classes 0-3, event buffering survives link outages.
- **Modbus RTU**: meters and auxiliary equipment inside substations, RS-485.
- **ICCP (IEC 60870-6 TASE.2)**: center-to-center only (AOC/RDC ↔ SCC), Bilateral Tables.
- **Explicitly OUT of scope for "conventional" reports**: IEC 61850 (MMS/GOOSE/SV), SAS, merging units, process bus — user said don't include unless asked.

## Delivered artifacts (reference filenames on VPS /var/www/resin-media/)
- iran-scada-conventional.docx — combined conventional report
- iran-scada-architecture.png — 5-band dispatching hierarchy diagram
- substation-signal-flow.png — 6-stage telemetry path + 3-stage SBO return loop
- iran-voltage-chain.png — physical chain mapped to dispatch centers
- hv-scada-comprehensive.docx — earlier generic HV report (source /tmp/hv-scada-report.md; user wants its intro/FEP/HIS/ICCP/marshalling text reused verbatim)
