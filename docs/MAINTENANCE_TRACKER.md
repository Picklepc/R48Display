# Maintenance Tracker & Hour Meter

R48Display tracks multiple categories of operational hours and supports a
user-configurable maintenance reminder system. This replaces the single
`runtimeSeconds` counter with a four-tier hour model and adds a persistent
reminder list with confirmation workflow.

Task references map to `TASKS.md` milestone M3.

---

## Hour Meter Design

### Why Four Tiers

A single "powered-on" counter conflates very different operating conditions.
For most use cases, users want to track:

- **Total hours** — how long the device has been running overall (useful for
  general maintenance schedules tied to calendar time on the machine).
- **Standby hours** — powered on but no significant current draw (key-on
  while parked, system idle, battery float). Lowest wear on most components.
- **Active hours** — current above the activity threshold, including time that
  also crosses the work threshold. Moderate-to-high component wear.
- **Working hours** — current above the work threshold. High load (mowing,
  climbing hills, peak inverter load, surge draw). Highest wear and always a
  subset of active hours.

This maps naturally onto the four activity states from `TASKS.md` M2.

### Counter Definitions

| Counter | NVS Key | Increments when |
|---|---|---|
| `hoursTotal` | `hrs_total` | Device is powered on (always) |
| `hoursStandby` | `hrs_standby` | \|current\| < `activityAmps` AND not charging |
| `hoursActive` | `hrs_active` | discharge >= `activityAmps` |
| `hoursWorking` | `hrs_working` | discharge >= `workAmps` |

Note: charging time is counted in `hoursTotal` but not in any sub-category
counter. Working time is also active time, so the invariant is
`hoursTotal >= hoursActive >= hoursWorking`. Idle/non-active time is the
remainder after active time.

Counters are stored as `float` (hours) in NVS. They are updated every 60 s by
computing elapsed seconds since the last tick and converting to fractional hours
before adding. This keeps NVS write frequency low while maintaining sub-minute
precision over long runtimes.

### Hour Counter Persistence

On clean boot, NVS keys are read. If a key is missing (first boot), it is
initialized to 0.0. If the device loses power mid-tick, at most 60 s of data
is lost. This is acceptable for hour-meter purposes.

An explicit "reset hours" action is available in web settings for each counter
individually, with a confirmation prompt to prevent accidental reset.

---

## BMS Degradation Metrics

### What Can Be Tracked

Some BMS protocols expose degradation data directly. Others do not, requiring
local tracking from observed telemetry. The table below describes the target
metric set.

| Metric | Source | Description |
|---|---|---|
| `cycleCount` | BMS (when available) | Full charge/discharge cycles logged by BMS |
| `capacityFadePct` | Computed | (designAh - currentFullAh) / designAh × 100 |
| `maxCellSpreadEver` | Local NVS | Highest cell imbalance (mV) ever observed |
| `minCellVoltageEver` | Local NVS | Lowest single-cell voltage (mV) ever observed |
| `maxTempEver` | Local NVS | Highest temperature (°C) ever observed |
| `lowVoltageEvents` | Local NVS | Count of times any cell fell below the floor |
| `highCurrentEvents` | Local NVS | Count of times discharge exceeded workAmps for ≥ 5 s |

### Protocol Availability (to be expanded in BMS_PROFILES.md)

| Protocol | cycleCount | capacityFade | BMS temps | cell voltages |
|---|---|---|---|---|
| humsienk_watt | yes (WATT field) | yes (if design Ah known) | yes | yes |
| jbd_xiaoxiang | yes | yes | yes | yes |
| Daly (planned) | yes | yes | yes | yes |
| JK (planned) | yes | yes | yes | yes |
| DC House (research) | TBD | TBD | TBD | TBD |
| Homsienk (research) | TBD | TBD | TBD | TBD |

When `cycleCount` is available from the BMS, use the BMS value. Do not attempt
to count cycles locally if the BMS tracks them, as the BMS has more accurate
information about partial cycles.

### Local Event Tracking

`lowVoltageEvents` and `highCurrentEvents` are tracked in NVS as `uint32_t`.
They increment during normal BMS polling. Neither counter can be reset by the
user (they are permanent lifetime records, not maintenance items).

Low-voltage floor: default to (nominalPackVoltage / cellCount × 0.9) or a
user-configured value in settings. A cell falling below this value during a
discharge represents a meaningful stress event for most LiFePO4 and NMC
chemistries.

High-current event: requires the current to exceed `workAmps` for at least 5
consecutive seconds (across polling intervals) to avoid counting brief surges.

### Capacity Fade Display

```
capacityFadePct = (designAh - reportedFullAh) / designAh × 100
```

Where `reportedFullAh` is the BMS's "remaining Ah at 100 % SOC" or the
actual measured capacity at last full charge (protocol-dependent).

Display in the web Battery Health card as a horizontal progress bar:

- 0–10 %: green (healthy)
- 10–20 %: yellow (monitor)
- 20–30 %: orange (plan replacement)
- > 30 %: red (replace soon)

---

## Maintenance Reminder System

### Data Model

Each maintenance item is a small struct:

```cpp
struct MaintenanceItem {
  uint8_t  id;             // 1–20
  char     name[41];       // user label, null terminated
  uint8_t  type;           // see MaintenanceType enum
  float    interval;       // hours, days, or cycles (per type)
  uint32_t lastResetTs;    // Unix timestamp of last confirmation (0 = never confirmed)
  float    lastResetVal;   // hours/cycles at last confirmation
  char     notes[81];      // optional user notes, null terminated
};

enum MaintenanceType : uint8_t {
  MAINT_HOURS_TOTAL   = 0,
  MAINT_HOURS_STANDBY = 1,
  MAINT_HOURS_ACTIVE  = 2,
  MAINT_HOURS_WORKING = 3,
  MAINT_DAYS          = 4,
  MAINT_CYCLES        = 5
};
```

Total storage: up to 20 items x ~140 bytes = ~2.8 KB. Store serialized as JSON
in NVS. Current firmware uses the `maint` key for the item list and `mh_<id>`
keys for per-item completion history. Each item keeps the 10 most recent
completion records with timestamp, reference value, and optional notes.

The 0.3.0 partition table expands default NVS to 256 KB so maintenance items and
history do not crowd out Wi-Fi, BMS, MQTT, hour-meter, and degradation settings.
The new NVS partition requires a full USB/fresh flash when upgrading from 0.2.x;
OTA alone cannot install the new partition table.

### Progress Calculation

On every call to `/api/maintenance`, compute progress for each item:

```
currentVal = currentHoursTotal  (if type HOURS_TOTAL)
           | currentHoursActive (if type HOURS_ACTIVE)
           | currentHoursWorking(if type HOURS_WORKING)
           | daysSinceEpoch     (if type DAYS)
           | cycleCount         (if type CYCLES)

elapsed    = currentVal - lastResetVal  (or currentVal if lastResetTs == 0)
remaining  = interval - elapsed
pct        = elapsed / interval × 100  (clamped to 0–100 for display, allowed > 100)
overdue    = elapsed >= interval
```

For `MAINT_DAYS`, `currentVal` is computed from Unix timestamp:
`(now - lastResetTs) / 86400.0`.

### API Endpoints

**GET `/api/maintenance`**

Returns JSON array. Each element:
```json
{
  "id": 1,
  "name": "Check blades / cutting edges",
  "type": "hours_working",
  "interval": 25.0,
  "lastResetTs": 1750000000,
  "lastResetVal": 142.5,
  "notes": "Sharpen or replace",
  "elapsed": 18.3,
  "remaining": 6.7,
  "pct": 73,
  "overdue": false
}
```

**POST `/api/maintenance`**

Creates or updates a maintenance item. Body (JSON):
```json
{
  "id": 1,          // omit for new item; server assigns next available ID
  "name": "...",
  "type": "hours_working",
  "interval": 25.0,
  "notes": "..."
}
```

Returns the saved item including computed progress.

**POST `/api/maintenance/confirm`**

Marks the item as done. Records current timestamp and current reference value
as the new baseline. Also appends one completion history record for the item.
Returns updated item with `elapsed: 0` and `pct: 0`.

Optional body:
```json
{
  "id": 1,
  "notes": "Replaced blades, installed OEM set"
}
```
If provided, `notes` is stored in the item's history entry.

**GET `/api/maintenance/history?id=<id>`**

Returns the 10 most recent completion records for one item.

**POST `/api/maintenance/history/update`**

Updates one history entry by `id` and `original_ts`. Body:

```json
{
  "id": 1,
  "original_ts": 1782867600,
  "ts": 1782871200,
  "notes": "Corrected completion date and notes"
}
```

Returns `{"ok": true}`. Duplicate timestamps for the same item are rejected so
future edit/delete operations can still target one row unambiguously.

**POST `/api/maintenance/history/delete`**

Removes a single history entry by `id` and `ts`. Returns `{"ok": true}`.

**GET `/api/maintenance/export`**

Downloads maintenance items and history as CSV.

**POST `/api/maintenance/delete`**

Removes item and its history. Returns `{"ok": true}`.

### Preset Templates

On first boot (when the `maint` NVS key is absent), populate with three
default templates so the user has a starting point. Templates use generic
language appropriate for all usage categories:

| Name | Type | Interval | Notes |
|---|---|---|---|
| Inspect cutting edges / wear items | hours_working | 25 h | Sharpen, balance, or replace as needed |
| Inspect tires / wheels / bearings | hours_active | 100 h | Check pressure, wear, and play |
| Annual inspection | days | 365 | Full service check |

The user can rename, edit, delete, or add to these. No factory reset is required
to clear them — DELETE is sufficient.

---

## Web UI — Maintenance Page

The maintenance tracker lives at `/maintenance` (new web route). The page
starts with two hour-meter bars, followed by machine/project notes, then the
maintenance item cards. Progress uses the same polling mechanism as the
dashboard.

### Hour Meter Bars

The top card shows two bars:

- **Total displayed hours**: installation baseline plus tracked device hours.
- **Working / Active / Total**: working time, active non-working time, and the
  remaining total/non-active time on one bar. Values are capped visually if a
  stored counter is out of order, and a warning is shown.

### Machine / Project Notes

The second card on `/maintenance` stores machine and project reference data in
NVS under the `machine` key. It is intended for Ryobi mower stats and equivalent
project records:

- Manufacturer
- Model number
- Serial / VIN
- Machine manufacture date
- Gauge install date
- Battery / pack model
- Battery install date
- Up to 12 user-defined label/value fields
- Freeform notes for part numbers, installation notes, wiring notes, service
  references, or other details

APIs:

- **GET `/api/machine-info`** returns the saved record.
- **POST `/api/machine-info`** saves form fields and a JSON `fields_json` array
  for custom fields.

### Layout

**Header:**
```
Maintenance Tracker                          [+ Add Item]
```

**Per-item card:**
```
┌────────────────────────────────────────────────────────┐
│  Check blades / cutting edges          [Edit] [Delete] │
│  Working hours — every 25 h                            │
│  ████████████░░░░  18.3 h / 25 h  (73%)               │
│  6.7 h remaining                                       │
│  Notes: Sharpen or replace                             │
│                                          [✓ Mark Done] │
└────────────────────────────────────────────────────────┘
```

Color coding:
- < 75 % elapsed: progress bar in theme primary color
- 75–100 % elapsed: progress bar in warning color (yellow/orange)
- Overdue (> 100 %): progress bar in bad color (red), card border highlighted

**Add/Edit modal:**
- Name (text input, required)
- Type (select: Total Hours / Standby Hours / Active Hours / Working Hours /
  Days / Cycles)
- Interval (number input with unit label matching type)
- Notes (textarea, optional)
- Save / Cancel buttons

**Mark Done modal:**
- "Confirm: [item name] — mark as done?"
- Optional note field ("What was done?")
- Confirm / Cancel

### LCD Integration

On the Status page (page 3), show one of:
- `Maintenance: OK` — no items are overdue
- `Maintenance: 2 due` — N items are overdue (N shown in warning color)

Do not show individual item names on the LCD — the display is too small and
the web UI is the right place for that detail.

---

## Hour Counter & Maintenance Data in API Status

Add to `/api/status` JSON:

```json
{
  "hours": {
    "total": 248.5,
    "standby": 62.1,
    "active": 141.3,
    "working": 45.1
  },
  "degradation": {
    "cycle_count": 47,
    "capacity_fade_pct": 4.2,
    "max_cell_spread_mv": 38,
    "min_cell_voltage_mv": 3102,
    "max_temp_c": 41.5,
    "low_voltage_events": 2,
    "high_current_events": 18
  },
  "maintenance": {
    "overdue_count": 1,
    "items": [ /* same format as /api/maintenance GET */ ]
  }
}
```

The `items` array in the status endpoint allows the web dashboard to show the
overdue badge without a separate API call.
