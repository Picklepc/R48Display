#include "WebPages.h"

namespace R48Web {

String dashboardBody() {
  return F(
      "<section class='toolbar'>"
      "<button data-display='0'>Dashboard</button>"
      "<button data-display='1'>Battery</button>"
      "<button data-display='2'>Clock</button>"
      "<button data-display='3'>Status</button>"
      "</section>"
      "<section class='dashboard-panel'>"
      "<div class='gauge-card dashboard-gauge'>"
      "<div class='gauge' id='soc-gauge' style='--pct:0'><b id='dash-soc'>--</b></div>"
      "<div class='subline' id='dash-mode'>Waiting for BMS</div>"
      "</div>"
      "<div class='dash-stats'>"
      "<div class='metric'><div class='label'>Pack Voltage</div><div class='value' id='dash-voltage'>--</div></div>"
      "<div class='metric'><div class='label'>Load</div><div class='value' id='dash-load'>--</div></div>"
      "<div class='metric'><div class='label'>Charge</div><div class='value' id='dash-charge'>--</div></div>"
      "<div class='metric'><div class='label'>ETA</div><div class='value' id='dash-runtime'>--</div></div>"
      "</div>"
      "</section>"
      "<section class='dash-strip'>"
      "<div class='metric'><div class='label'>Total Hours</div><div class='value' id='dash-hours'>--</div></div>"
      "<div class='metric'><div class='label'>Active Hours</div><div class='value' id='dash-active-hours'>--</div></div>"
      "<div class='metric'><div class='label'>Working Hours</div><div class='value' id='dash-working-hours'>--</div></div>"
      "<div class='metric'><div class='label'>Health</div><div class='value' id='dash-health'>--</div></div>"
      "<div class='metric'><div class='label'>Maintenance</div><div class='value sm' id='dash-maintenance'>Not configured</div></div>"
      "<div class='metric'><div class='label'>BMS Link</div><div class='value sm' id='dash-link'>--</div></div>"
      "</section>"
      "<section class='card'>"
      "<h2>Battery Health</h2>"
      "<div class='dash-strip'>"
      "<div class='metric'><div class='label'>Capacity</div><div class='value sm' id='bh-capacity'>--</div></div>"
      "<div class='metric'><div class='label'>Cycles</div><div class='value' id='bh-cycles'>--</div></div>"
      "<div class='metric'><div class='label'>Worst Spread</div><div class='value' id='bh-spread'>--</div></div>"
      "<div class='metric'><div class='label'>Min Cell</div><div class='value' id='bh-mincell'>--</div></div>"
      "<div class='metric'><div class='label'>Max Temp</div><div class='value' id='bh-temp'>--</div></div>"
      "<div class='metric'><div class='label'>LV Events</div><div class='value' id='bh-lv'>--</div></div>"
      "<div class='metric'><div class='label'>HC Events</div><div class='value' id='bh-hc'>--</div></div>"
      "</div>"
      "</section>"
      // System status card
      "<section class='card'>"
      "<h2>System</h2>"
      "<div class='dash-strip'>"
      "<div class='metric'><div class='label'>Firmware</div><div class='value sm' id='sys-fw'>--</div></div>"
      "<div class='metric'><div class='label'>Uptime</div><div class='value sm' id='sys-uptime'>--</div></div>"
      "<div class='metric'><div class='label'>Hostname</div><div class='value sm' id='sys-host'>--</div></div>"
      "<div class='metric'><div class='label'>IP Address</div><div class='value sm' id='sys-ip'>--</div></div>"
      "<div class='metric'><div class='label'>Wi-Fi</div><div class='value sm' id='sys-wifi'>--</div></div>"
      "<div class='metric'><div class='label'>NTP Clock</div><div class='value sm' id='sys-ntp'>--</div></div>"
      "</div>"
      "</section>"
      // Device power + MQTT status card
      "<section class='card'>"
      "<h2>Device</h2>"
      "<div class='dash-strip'>"
      "<div class='metric'><div class='label'>Board Battery</div><div class='value sm' id='sys-bat'>--</div></div>"
      "<div class='metric'><div class='label'>Power Source</div><div class='value sm' id='sys-pwr'>--</div></div>"
      "<div class='metric'><div class='label'>Power Save</div><div class='value sm' id='sys-save'>--</div></div>"
      "<div class='metric'><div class='label'>Mic RMS</div><div class='value sm' id='sys-mic-rms'>--</div></div>"
      "<div class='metric'><div class='label'>Heap Free</div><div class='value sm' id='sys-heap'>--</div></div>"
      "<div class='metric'><div class='label'>MQTT</div><div class='value sm' id='sys-mqtt'>--</div></div>"
      "<div class='metric'><div class='label'>BLE Policy</div><div class='value sm' id='sys-ble-pol'>--</div></div>"
      "</div>"
      "</section>");
}

String batteryBody() {
  return F(
      // Pack overview
      "<section class='bms-pack'>"
      "<div class='gauge-card'>"
      "<div class='gauge' id='bat-soc-gauge' style='--pct:0'><b id='bat-soc'>--</b></div>"
      "</div>"
      "<div class='pack-cols'>"
      "<div class='metric'><div class='label'>Voltage</div><div class='value' id='bat-pack'>--</div></div>"
      "<div class='metric'><div class='label'>Current</div><div class='value' id='bat-current'>--</div></div>"
      "<div class='metric'><div class='label'>Power</div><div class='value' id='bat-power'>--</div></div>"
      "<div class='metric'><div class='label'>State</div><div class='value sm' id='bat-state'>--</div></div>"
      "</div>"
      "</section>"
      // Cell voltages
      "<section class='card'>"
      "<div class='section-head'>"
      "<h2>Cells</h2>"
      "<span id='bat-cell-count' style='color:var(--muted);font-size:13px'>--</span>"
      "<span id='bat-spread' style='margin-left:auto;font-size:12px;padding:2px 7px;border-radius:4px;background:var(--panel2);color:var(--muted)'>--</span>"
      "<span id='bat-balance' style='font-size:12px;padding:2px 7px;border-radius:4px;background:var(--panel2);color:var(--muted)'>--</span>"
      "</div>"
      "<div class='cells' id='cells'></div>"
      "</section>"
      // Temperatures
      "<section class='card'>"
      "<h2 style='margin-bottom:10px'>Temperature</h2>"
      "<div class='grid' id='bat-temps'><div class='metric'><div class='label'>--</div><div class='value sm'>--</div></div></div>"
      "</section>"
      // Capacity & health
      "<section class='card'>"
      "<h2 style='margin-bottom:10px'>Capacity</h2>"
      "<div style='background:var(--panel2);border-radius:6px;height:10px;margin-bottom:12px;overflow:hidden'>"
      "<div id='bat-cap-fill' style='height:100%;width:0%;border-radius:6px;background:linear-gradient(90deg,var(--primary2),var(--primary));transition:width .4s'></div>"
      "</div>"
      "<div class='grid'>"
      "<div class='metric'><div class='label'>Remaining</div><div class='value' id='bat-remaining'>--</div></div>"
      "<div class='metric'><div class='label'>Full Charge</div><div class='value' id='bat-total'>--</div></div>"
      "<div class='metric'><div class='label'>Design</div><div class='value' id='bat-design'>--</div></div>"
      "<div class='metric'><div class='label'>Cycles</div><div class='value' id='bat-cycles'>--</div></div>"
      "<div class='metric'><div class='label'>Average Cell</div><div class='value' id='bat-avg-cell'>--</div></div>"
      "<div class='metric'><div class='label'>Health Est.</div><div class='value' id='bat-health'>--</div></div>"
      "</div>"
      "</section>"
      // BMS diagnostics
      "<section class='card'>"
      "<h2 style='margin-bottom:10px'>BMS Connection</h2>"
      "<div class='grid'>"
      "<div class='metric'><div class='label'>Protocol</div><div class='value sm' id='diag-protocol'>--</div></div>"
      "<div class='metric'><div class='label'>Address</div><div class='value sm' id='diag-address'>--</div></div>"
      "<div class='metric'><div class='label'>Signal</div><div class='value sm' id='diag-rssi'>--</div></div>"
      "<div class='metric'><div class='label'>Frames</div><div class='value sm' id='diag-frames'>--</div></div>"
      "<div class='metric'><div class='label'>Errors</div><div class='value sm' id='diag-errors'>--</div></div>"
      "<div class='metric'><div class='label'>Last Data</div><div class='value sm' id='diag-last-rx'>--</div></div>"
      "</div>"
      "<div id='diag-error-row' style='display:none;margin-top:8px'>"
      "<div class='label'>Last Error</div><div class='value sm' id='diag-last-error' style='color:var(--bad)'>--</div>"
      "</div>"
      "<details style='margin-top:10px'>"
      "<summary style='color:var(--muted);font-size:12px;cursor:pointer'>Last frame hex</summary>"
      "<code id='diag-frame-hex' style='display:block;margin-top:6px;font-size:11px;word-break:break-all;color:var(--muted)'>--</code>"
      "</details>"
      "</section>");
}

String settingsBody() {
  return F(
      "<section class='grid'>"
      "<form class='card wide' id='settings-form'>"
      "<div class='section-head'><h2>Settings</h2><button class='primary'>Save</button></div>"

      // Wi-Fi
      "<div class='settings-group'>"
      "<h3>Wi-Fi</h3>"
      "<div class='form-grid'>"
      "<label>SSID<input name='wifi_ssid' autocomplete='off'></label>"
      "<label>Password<input name='wifi_password' type='password' autocomplete='new-password' placeholder='leave blank to keep current'></label>"
      "</div>"
      "<div class='actions' style='margin-top:10px'>"
      "<button type='button' id='wifi-scan'>Scan Networks</button>"
      "<button type='button' id='wifi-forget'>Forget Wi-Fi</button>"
      "<button type='button' id='setup-ap'>Start Setup AP</button>"
      "</div>"
      "<div id='wifi-results' class='list'></div>"
      "</div>"

      // System — identity fields directly after WiFi
      "<div class='settings-group'>"
      "<h3>System</h3>"
      "<div class='form-grid'>"
      "<label>Display Title<input name='title' autocomplete='off' placeholder='Ryobi Mower (leave blank to use hostname)'></label>"
      "<label>Hostname (network / mDNS)<input name='hostname' autocomplete='off' placeholder='ryobi'></label>"
      "<label>OTA Password<input name='ota_password' type='password' autocomplete='new-password'></label>"
      "</div>"
      "</div>"

      // Time & NTP — after System, before BMS (clock is network-adjacent)
      "<div class='settings-group'>"
      "<h3>Time &amp; NTP</h3>"
      "<div class='form-grid'>"
      "<label>Timezone (POSIX)<input name='timezone' autocomplete='off' placeholder='PST8PDT,M3.2.0,M11.1.0'></label>"
      "<label>NTP Server<input name='ntp_server' autocomplete='off' placeholder='pool.ntp.org'></label>"
      "<label class='check'><input name='ntp_enabled' type='checkbox'> Enable NTP time sync</label>"
      "<label>Clock Format<select name='time_format'>"
      "<option value='12h'>12-hour (AM/PM)</option>"
      "<option value='24h'>24-hour (military)</option>"
      "</select></label>"
      "</div>"
      "</div>"

      // Battery / BMS
      "<div class='settings-group'>"
      "<h3>Battery Monitor (BMS)</h3>"
      "<div class='form-grid'>"
      "<label>Protocol<select name='bms_protocol' id='bms-protocol'></select></label>"
      "<label>BMS Name or Address<input name='bms_name' autocomplete='off' placeholder='blank = auto-select first compatible'></label>"
      "<label>Nominal Pack Ah<input name='nominal_pack_ah' type='number' min='1' max='1000' step='0.1'></label>"
      "<label>Typical Load Amps<input name='typical_mow_amps' type='number' min='1' max='600' step='0.1'></label>"
      "<label class='check'><input name='discharge_current_negative' type='checkbox'> Negative current means discharge</label>"
      "</div>"
      "<div class='actions' style='margin-top:10px'>"
      "<button type='button' id='ble-scan'>Scan for BMS</button>"
      "<button type='button' id='bms-read'>Read Now</button>"
      "<button type='button' id='bms-reconnect'>Reconnect</button>"
      "</div>"
      "<div id='ble-results'></div>"
      "</div>"

      // Appearance
      "<div class='settings-group'>"
      "<h3>Appearance</h3>"
      "<div class='form-grid'>"
      "<label>Usage Category<select name='usage_category' id='usage-category'></select></label>"
      "<label>Color Scheme<select name='theme_id' id='theme-select'></select><span class='hint' id='theme-description'></span></label>"
      "<label>Pack / Vehicle Label<input name='mower_model' autocomplete='off' placeholder='e.g. 48V Ryobi Mower Pack'></label>"
      "<label>Page Subtitle<input name='subtitle' autocomplete='off' placeholder='Fresh electrons, suspiciously organized.'></label>"
      "<label>Temperature Unit<select name='temp_unit'>"
      "<option value='F'>Fahrenheit (&deg;F)</option>"
      "<option value='C'>Celsius (&deg;C)</option>"
      "</select></label>"
      "<label>Brightness<input name='brightness' type='range' min='20' max='255'></label>"
      "<label>Display Rotation<select name='display_rotation'>"
      "<option value='0'>0&deg; (default)</option>"
      "<option value='90'>90&deg;</option>"
      "<option value='180'>180&deg;</option>"
      "<option value='270'>270&deg;</option>"
      "</select></label>"
      "<label class='check'><input name='display_enabled' type='checkbox'> Display enabled</label>"
      "</div>"
      "</div>"

      // Activity & Thresholds
      "<div class='settings-group'>"
      "<h3>Activity &amp; Thresholds</h3>"
      "<p class='hint' style='margin-bottom:10px'>Each amp threshold is paired with the label shown when that state is active.</p>"
      "<div class='threshold-grid'>"
      "<label>Charging above (A)<input name='charge_min_amps' type='number' min='0.1' max='200' step='0.1'></label>"
      "<label>Charging label<input name='label_charging' autocomplete='off' placeholder='e.g. Recharging'></label>"
      "<label>Standby (below active)<input name='standby_hint' value='Uses active threshold' readonly disabled></label>"
      "<label>Standby label<input name='label_standby' autocomplete='off' placeholder='e.g. Standby'></label>"
      "<label>Active above (A)<input name='mower_run_amps' type='number' min='1' max='300' step='0.1'></label>"
      "<label>Active label<input name='label_active' autocomplete='off' placeholder='e.g. Driving'></label>"
      "<label>Working / surge above (A)<input name='mowing_detect_amps' type='number' min='1' max='600' step='0.1'></label>"
      "<label>Working label<input name='label_working' autocomplete='off' placeholder='e.g. Mowing'></label>"
      "</div>"
      "<div class='form-grid' style='margin-top:10px'>"
      "<label class='check'><input name='activity_detection' type='checkbox'> Activity mode detection</label>"
      "<label class='check'><input name='work_detection' type='checkbox'> Work / surge detection</label>"
      "<label class='check'><input name='feature_mic' type='checkbox'> Audio validation — when enabled, Working state requires both amps AND mic noise above threshold</label>"
      "<label>Mic Threshold (RMS)<input name='mic_run_threshold' type='number' min='100' max='12000' step='50'><span class='hint'>Watch the RMS value in the web dashboard Audio Assist line while mowing to find your threshold</span></label>"
      "</div>"
      "</div>"

      // Hour Meter — after Activity since thresholds drive what gets counted
      "<div class='settings-group'>"
      "<h3>Hour Meter</h3>"
      "<p class='hint' style='margin-bottom:10px'>"
      "<b>Total Hours</b> = Baseline + Counted. "
      "<b>Active Hours</b> counts time above the Active amps threshold (driving, idling under load). "
      "<b>Working Hours</b> counts time above the Working/surge threshold (blades on, high-draw tasks). "
      "All counters increment only while BMS data is live. Edit to correct or reset; saving immediately persists."
      "</p>"
      "<div class='form-grid'>"
      "<label>Original Hours At Install"
      "<span class='hint'>Odometer reading when you fitted this display. Added to counted hours for the total.</span>"
      "<input name='hours_baseline' type='number' min='0' max='99999' step='any'></label>"
      "<label>Total Displayed Hours"
      "<span class='hint'>Baseline + counted. Edit to override the displayed total.</span>"
      "<input name='hours_total' type='number' min='0' max='99999' step='any'></label>"
      "<label>Counted Hours"
      "<span class='hint'>Accumulated by this device since first boot. Read-only.</span>"
      "<input name='hours_counted' readonly></label>"
      "<label>Active Hours"
      "<span class='hint'>Time spent above the Active amps threshold. Edit to reset (set 0) or correct.</span>"
      "<input name='hours_active' type='number' min='0' max='99999' step='any'></label>"
      "<label>Working Hours"
      "<span class='hint'>Time spent above the Working/surge threshold. Edit to reset or correct.</span>"
      "<input name='hours_working' type='number' min='0' max='99999' step='any'></label>"
      "</div>"
      "</div>"

      // Power Management
      "<div class='settings-group'>"
      "<h3>Power Management</h3>"
      "<div class='form-grid'>"
      "<label>LCD Timeout (seconds)<input name='lcd_timeout_sec' type='number' min='0' max='3600' step='5'></label>"
      "<label>Idle BLE Wake Hours<input name='idle_ble_wake_hours' type='number' min='0.25' max='24' step='0.25'></label>"
      "<label>Low Voltage Floor (V/cell)<input name='low_voltage_floor_v' type='number' min='2.0' max='3.8' step='0.01'></label>"
      "<label>Board Battery Low Threshold (%)"
      "<span class='hint'>Force low power mode and pause hour counting when the onboard LiPo drops below this level. On startup from a depleted state, hours won't count until the battery rises above this threshold.</span>"
      "<input name='board_battery_low_pct' type='number' min='5' max='80' step='5'></label>"
      "<label class='check'><input name='power_save_enabled' type='checkbox'> Enable Power Save Mode"
      "<span class='note'>Reduces screen timeout, BLE polling speed, and CPU frequency.</span></label>"
      "</div>"
      "</div>"

      // MQTT / Home Assistant
      "<div class='settings-group'>"
      "<h3>MQTT / Home Assistant</h3>"
      "<div class='form-grid'>"
      "<label class='check'><input name='mqtt_enabled' type='checkbox' id='mqttEnCb' onchange='toggleMqtt(this.checked)'> Enable MQTT</label>"
      "<label id='mqttHostRow'>Broker Host<input name='mqtt_host' autocomplete='off' placeholder='192.168.1.100'></label>"
      "<label id='mqttPortRow'>Broker Port<input name='mqtt_port' type='number' min='1' max='65535' value='1883'></label>"
      "<label id='mqttUserRow'>Username (optional)<input name='mqtt_user' autocomplete='off'></label>"
      "<label id='mqttPassRow'>Password<input name='mqtt_password' type='password' autocomplete='new-password' placeholder='leave blank to keep current'></label>"
      "<label id='mqttPfxRow'>Topic Prefix<input name='mqtt_topic_prefix' autocomplete='off' placeholder='r48display/hostname'></label>"
      "</div>"
      "<div id='mqttActions' style='display:none;margin-top:.6rem;gap:.5rem;display:none;flex-wrap:wrap'>"
      "<button type='button' onclick='mqttPublishNow()'>Publish Now</button>"
      "<button type='button' onclick='mqttRediscover()'>Send HA Discovery</button>"
      "<button type='button' onclick='mqttTest()'>Test Connection</button>"
      "<span id='mqttStatusLine' style='align-self:center;font-size:.85em;color:var(--muted)'></span>"
      "</div>"
      "</div>"

      "</form>"

      // OTA card (separate form, multipart)
      "<div class='card wide'>"
      "<h2 style='margin-bottom:12px'>Firmware Update</h2>"
      "<form method='POST' action='/update' enctype='multipart/form-data'>"
      "<div class='form-grid' style='margin-bottom:10px'>"
      "<label>Firmware .bin<input type='file' name='firmware' accept='.bin'></label>"
      "</div>"
      "<button class='primary'>Upload &amp; Reboot</button>"
      "</form>"
      "<p class='hint' style='margin-top:10px'>Embedded OTA updater. Select the compiled .bin and click Upload. The device reboots automatically after a successful flash.</p>"
      "</div>"

      "</section>");
}

String maintenanceBody() {
  return F(
      "<section class='card wide'>"
      "<div class='section-head'><h2>Maintenance</h2>"
      "<button class='primary' onclick='openMaintForm(null)'>+ Add Item</button></div>"
      "<div id='maint-list'><div class='empty'>Loading…</div></div>"
      "</section>"
      "<div id='maint-modal' style='display:none;position:fixed;inset:0;background:rgba(0,0,0,.6);z-index:100;display:none;align-items:center;justify-content:center'>"
      "<div class='card' style='min-width:280px;max-width:420px;width:92%;padding:20px'>"
      "<h3 id='maint-modal-title'>Add Item</h3>"
      "<div class='form-grid'>"
      "<label>Name<input id='mf-name' maxlength='40' autocomplete='off'></label>"
      "<label>Type<select id='mf-type'>"
      "<option value='HOURS_ACTIVE'>Active Hours</option>"
      "<option value='HOURS_WORKING'>Working Hours</option>"
      "<option value='HOURS_TOTAL'>Total Hours</option>"
      "<option value='DAYS'>Days</option>"
      "</select></label>"
      "<label id='mf-interval-label'>Interval (h)<input id='mf-interval' type='number' min='0.1' step='any'></label>"
      "<label>Notes<input id='mf-notes' maxlength='80' autocomplete='off'></label>"
      "</div>"
      "<input type='hidden' id='mf-id' value='0'>"
      "<div style='display:flex;gap:8px;margin-top:14px'>"
      "<button class='primary' onclick='saveMaintItem()'>Save</button>"
      "<button onclick='closeMaintModal()'>Cancel</button>"
      "</div>"
      "</div>"
      "</div>");
}

String updateBody() {
  return F(
      "<section class='card'>"
      "<h2>Firmware Update</h2>"
      "<form method='POST' action='/update' enctype='multipart/form-data'>"
      "<label>Firmware .bin<input type='file' name='firmware' accept='.bin'></label>"
      "<button class='primary'>Upload and Reboot</button>"
      "</form>"
      "<p class='note'>This updater is embedded in firmware and does not require an SD card.</p>"
      "</section>");
}

String appScript() {
  return F(R"JS(
const $ = (id) => document.getElementById(id);
const qsa = (sel) => Array.from(document.querySelectorAll(sel));
let themeOptions = [];
let usageCategories = [];

function get(obj, path, fallback = '--') {
  return path.split('.').reduce((o, key) => (o && o[key] !== undefined) ? o[key] : undefined, obj) ?? fallback;
}

function text(id, value) {
  const el = $(id);
  if (el) el.textContent = value ?? '--';
}

function fmtNum(value, digits = 1, suffix = '') {
  const num = Number(value);
  if (!Number.isFinite(num)) return '--';
  return num.toFixed(digits) + suffix;
}

function postForm(url, data) {
  return fetch(url, {
    method: 'POST',
    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
    body: new URLSearchParams(data)
  });
}

function chooseWifi(ssid) {
  const input = document.querySelector('[name=wifi_ssid]');
  if (input) input.value = ssid;
}

function chooseBle(name, address, profile) {
  const input = document.querySelector('[name=bms_name]');
  if (input) input.value = name || address || '';
  const select = document.querySelector('[name=bms_protocol]');
  if (select && profile) select.value = profile;
}

function selectedUsage() {
  const id = $('usage-category')?.value;
  return usageCategories.find((u) => u.id === id);
}

function updateThemeDescription() {
  const selected = $('theme-select')?.value;
  const theme = themeOptions.find((t) => t.id === selected);
  const description = $('theme-description');
  if (description) description.textContent = theme ? `${theme.family}: ${theme.description}` : '';
}

function applyCategoryTheme() {
  const usage = selectedUsage();
  const themeSelect = $('theme-select');
  if (usage && themeSelect && usage.default_theme) themeSelect.value = usage.default_theme;
  updateThemeDescription();
}

function refreshThemeCss() {
  const link = document.querySelector('link[href^="/theme.css"]');
  if (link) link.href = `/theme.css?v=${Date.now()}`;
}

function renderCells(cells) {
  const host = $('cells');
  if (!host) return;
  host.innerHTML = '';
  if (!cells || !cells.length) {
    host.innerHTML = '<div class="empty">No cell data yet</div>';
    return;
  }
  const nums = cells.map(Number).filter(Number.isFinite);
  const minV = Math.min(...nums);
  const maxV = Math.max(...nums);
  const spreadMv = (maxV - minV) * 1000;
  cells.forEach((cell, i) => {
    const v = Number(cell);
    const pct = Number.isFinite(v) ? ((v - 2.80) / (3.65 - 2.80)) * 100 : 0;
    const isMin = spreadMv >= 20 && v === minV && minV > 0;
    const isMax = spreadMv >= 20 && v === maxV && maxV > 0;
    const border = isMin ? 'border-color:var(--warn)' : isMax ? 'border-color:var(--primary)' : '';
    const div = document.createElement('div');
    div.className = 'cell';
    div.style.cssText = border;
    div.innerHTML = `<span>${i + 1}</span><b>${Number.isFinite(v) ? v.toFixed(3) : '--'} V</b><i style="--w:${Math.min(100, Math.max(4, pct))}%"></i>`;
    host.appendChild(div);
  });
}

function renderTemps(temps, unit) {
  const host = $('bat-temps');
  if (!host) return;
  const labels = ['MOS', 'Board', 'T1', 'T2', 'T3', 'T4', 'T5', 'T6', 'T7', 'T8'];
  if (!temps || !temps.length) {
    host.innerHTML = '<div class="metric"><div class="label">Temperature</div><div class="value sm">--</div></div>';
    return;
  }
  host.innerHTML = temps.map((t, i) => {
    const v = Number(t);
    const disp = Number.isFinite(v) ? `${v.toFixed(1)} ${unit || '°C'}` : '--';
    return `<div class="metric"><div class="label">${labels[i] || `T${i}`}</div><div class="value sm">${disp}</div></div>`;
  }).join('');
}

function renderDetails(data) {
  const body = $('details');
  if (!body) return;
  const activeLabel = get(data, 'usage.active_label', 'activity');
  const workLabel = get(data, 'usage.work_label', 'work');
  const rows = [
    ['Host', get(data, 'hostname')],
    ['Usage', `${get(data, 'usage.label')} / ${get(data, 'vehicle.label')}`],
    ['Theme', `${get(data, 'theme.label')} (${get(data, 'theme.family')})`],
    ['Status', `${get(data, 'vehicle.activity_label')} (${get(data, 'vehicle.activity_state')})`],
    ['SSID', get(data, 'wifi.ssid')],
    ['RSSI', `${get(data, 'wifi.rssi')} dBm`],
    ['Mode', get(data, 'wifi.mode')],
    ['BMS target', get(data, 'bms.target_name')],
    ['BMS protocol', get(data, 'bms.protocol')],
    ['BMS address', get(data, 'bms.address')],
    ['Last error', get(data, 'bms.last_error', '')],
    ['Charge threshold', `${get(data, 'vehicle.charge_min_amps')} A`],
    [`${activeLabel} threshold`, `${get(data, 'vehicle.run_threshold_a', get(data, 'mower.run_threshold_a'))} A`],
    [`${workLabel} threshold`, `${get(data, 'vehicle.work_threshold_a', get(data, 'mower.work_threshold_a'))} A`],
    ['Activity detection', get(data, 'usage.activity_detection')],
    ['Work detection', get(data, 'usage.work_detection')],
    ['Audio assist', `${get(data, 'mic.status')} rms ${get(data, 'mic.rms')} threshold ${get(data, 'mic.threshold')}`],
    [`${workLabel} likely`, get(data, 'vehicle.work_likely', get(data, 'mower.blades_likely_on'))],
    ['Hours: total', `${get(data, 'hours.total')} h`],
    ['Hours: baseline', `${get(data, 'hours.baseline')} h`],
    ['Hours: counted', `${get(data, 'hours.counted')} h`],
    ['Hours: active', `${get(data, 'hours.active')} h`],
    ['Hours: working', `${get(data, 'hours.working')} h`],
    ['Power source', `${get(data, 'screen_battery.power_source')} / save:${get(data, 'screen_battery.power_save_enabled')}`],
    ['BLE policy', `${get(data, 'bms.policy')} (${get(data, 'bms.soc_rate_pct_per_hour')}%/h)`],
    ['NTP', get(data, 'clock.ntp_configured') ? `sync via ${get(data, 'clock.ntp_server')}` : 'disabled'],
    ['Touch', get(data, 'hardware.touch_ready')],
    ['Rotation', `${get(data, 'display.rotation_degrees')} degrees`],
    ['Display page', get(data, 'display.page_name')],
    ['Clock', get(data, 'clock.local_time')]
  ];
  body.innerHTML = rows.map(([k, v]) => `<tr><th>${k}</th><td>${v === '' ? '--' : v}</td></tr>`).join('');
}

function render(data) {
  const soc = Number(get(data, 'bms.soc', 0));
  text('dash-soc', get(data, 'bms.last_analog_age') === 'never' ? '--' : `${soc}%`);
  const gauge = $('soc-gauge');
  if (gauge) gauge.style.setProperty('--pct', Number.isFinite(soc) ? soc : 0);
  text('dash-mode', get(data, 'vehicle.mode', get(data, 'mower.mode')));
  text('dash-voltage', fmtNum(get(data, 'bms.pack_voltage'), 1, ' V'));
  text('dash-load', `${fmtNum(get(data, 'vehicle.discharge_a', get(data, 'mower.discharge_a')), 1, ' A')} / ${fmtNum(get(data, 'vehicle.discharge_w', get(data, 'mower.discharge_w')), 0, ' W')}`);
  text('dash-charge', `${fmtNum(get(data, 'vehicle.charge_a', get(data, 'mower.charge_a')), 1, ' A')} / ${fmtNum(get(data, 'vehicle.charge_w', get(data, 'mower.charge_w')), 0, ' W')}`);
  const chargeA = Number(get(data, 'vehicle.charge_a', 0));
  text('dash-runtime', chargeA > 0.2
    ? fmtNum(get(data, 'vehicle.charge_estimate_hours', get(data, 'mower.charge_estimate_hours')), 1, ' h to full')
    : fmtNum(get(data, 'vehicle.runtime_estimate_hours', get(data, 'mower.runtime_estimate_hours')), 1, ' h'));
  text('dash-hours', fmtNum(get(data, 'hours.total'), 1, ' h'));
  text('dash-active-hours', fmtNum(get(data, 'hours.active'), 1, ' h'));
  text('dash-working-hours', fmtNum(get(data, 'hours.working'), 1, ' h'));
  text('dash-health', fmtNum(get(data, 'bms.health_percent'), 1, '%'));
  text('dash-link', `${get(data, 'bms.link')} ${get(data, 'bms.rssi')} dBm`);

  // Battery Health card
  const dg = get(data, 'degradation', {});
  const fadePct = parseFloat(dg.capacity_fade_pct || 0);
  const totalAhDash = parseFloat(get(data, 'bms.remaining_ah') || 0) * 100 / Math.max(1, parseFloat(get(data, 'bms.soc') || 1));
  text('bh-capacity', dg.capacity_fade_pct !== undefined ? `${(100 - fadePct).toFixed(1)}% (${fadePct.toFixed(1)}% fade)` : '--');
  text('bh-cycles', dg.cycle_count !== undefined ? String(dg.cycle_count) : '--');
  text('bh-spread', dg.max_cell_spread_mv !== undefined ? fmtNum(dg.max_cell_spread_mv, 1, ' mV') : '--');
  const minCv = parseFloat(dg.min_cell_voltage_v || 0);
  text('bh-mincell', minCv > 0.1 ? `${minCv.toFixed(3)} V` : '--');
  const maxTc = parseFloat(dg.max_temp_c || 0);
  const bTempUnit = get(data, 'bms.temp_unit', 'C');
  const maxTDisplay = maxTc > 0.1
    ? (bTempUnit === 'F' ? `${(maxTc * 9 / 5 + 32).toFixed(1)} °F` : `${maxTc.toFixed(1)} °C`)
    : '--';
  text('bh-temp', maxTDisplay);
  text('bh-lv', dg.low_voltage_events !== undefined ? String(dg.low_voltage_events) : '--');
  text('bh-hc', dg.high_current_events !== undefined ? String(dg.high_current_events) : '--');

  // Maintenance strip summary (fire-and-forget, non-blocking)
  if ($('dash-maintenance')) {
    fetch('/api/maintenance', {cache: 'no-store'}).then(r => r.json()).then(items => {
      const overdue = items.filter(i => i.overdue).length;
      const total = items.length;
      text('dash-maintenance', total === 0 ? 'None configured' : overdue > 0 ? `${overdue} item${overdue > 1 ? 's' : ''} due` : `${total} item${total > 1 ? 's' : ''}, all OK`);
    }).catch(() => {});
  }

  // Battery monitor page
  const soc2 = Number(get(data, 'bms.soc', 0));
  const batGauge = $('bat-soc-gauge');
  if (batGauge) batGauge.style.setProperty('--pct', Number.isFinite(soc2) ? soc2 : 0);
  text('bat-soc', get(data, 'bms.last_analog_age') === 'never' ? '--' : `${soc2}%`);
  text('bat-pack', fmtNum(get(data, 'bms.pack_voltage'), 2, ' V'));
  const rawA = Number(get(data, 'bms.raw_current_a', 0));
  const disA = Number(get(data, 'vehicle.discharge_a', 0));
  const chgA = Number(get(data, 'vehicle.charge_a', 0));
  text('bat-current', Number.isFinite(rawA) ? rawA.toFixed(2) + ' A' : '--');
  const disW = Number(get(data, 'vehicle.discharge_w', 0));
  const chgW = Number(get(data, 'vehicle.charge_w', 0));
  const pw = (disW > 0 ? disW : -chgW);
  text('bat-power', Number.isFinite(pw) ? pw.toFixed(0) + ' W' : '--');
  text('bat-state', get(data, 'vehicle.activity_label', get(data, 'vehicle.mode')));
  text('bat-spread', fmtNum(get(data, 'bms.delta_cell_mv'), 0, ' mV'));
  const balState = Number(get(data, 'bms.balance_state', 0));
  text('bat-balance', balState === 1 ? 'Balancing ▲' : balState === 2 ? 'Balancing ▼' : balState ? 'Balancing' : 'Balance off');
  text('bat-cell-count', `${get(data, 'bms.cell_count', 0)} cells`);
  renderCells(get(data, 'bms.cells', []));
  renderTemps(get(data, 'bms.temps', []), get(data, 'bms.temp_unit'));
  const remAh = Number(get(data, 'bms.remaining_ah', 0));
  const totAh = Number(get(data, 'bms.total_ah', 0));
  const capPct = totAh > 0 ? Math.min(100, (remAh / totAh) * 100) : 0;
  const capFill = $('bat-cap-fill');
  if (capFill) capFill.style.width = `${capPct}%`;
  text('bat-remaining', fmtNum(remAh, 1, ' Ah'));
  text('bat-total', fmtNum(totAh, 1, ' Ah'));
  text('bat-design', fmtNum(get(data, 'bms.design_ah'), 1, ' Ah'));
  text('bat-cycles', get(data, 'bms.cycles', '--'));
  text('bat-avg-cell', fmtNum(get(data, 'bms.avg_cell_v'), 3, ' V'));
  text('bat-health', fmtNum(get(data, 'bms.health_percent'), 1, '%'));
  text('diag-protocol', get(data, 'bms.protocol_label', get(data, 'bms.protocol')));
  text('diag-address', get(data, 'bms.address'));
  text('diag-rssi', `${get(data, 'bms.rssi')} dBm`);
  text('diag-frames', `${get(data, 'bms.frames')} rx / ${get(data, 'bms.errors')} err`);
  text('diag-errors', get(data, 'bms.errors'));
  text('diag-last-rx', get(data, 'bms.last_rx_age', get(data, 'bms.last_analog_age')));
  const lastErr = get(data, 'bms.last_error', '');
  const errRow = $('diag-error-row');
  if (errRow) errRow.style.display = lastErr && lastErr !== '--' ? '' : 'none';
  text('diag-last-error', lastErr);
  text('diag-frame-hex', get(data, 'bms.frame_hex', '--'));

  // System & Device status cards (dashboard footer)
  text('sys-fw', `${get(data, 'firmware')} — ${get(data, 'project')}`);
  text('sys-uptime', get(data, 'uptime'));
  text('sys-host', (() => { const h = get(data, 'hostname'); const m = get(data, 'wifi.mdns', ''); return m ? `${h} (${m})` : h; })());
  text('sys-ip', get(data, 'wifi.ip') || get(data, 'wifi.ap_ip') || '--');
  text('sys-wifi', `${get(data, 'wifi.ssid') || get(data, 'wifi.mode')} ${get(data, 'wifi.rssi')} dBm`);
  text('sys-ntp', get(data, 'clock.ntp_configured') ? `✓ ${get(data, 'clock.ntp_server')}` : 'disabled');
  text('sys-bat', get(data, 'screen_battery.label') || '--');
  text('sys-pwr', get(data, 'screen_battery.power_source') || '--');
  text('sys-save', get(data, 'screen_battery.power_save_enabled') ? 'on' : 'off');
  { const rms = get(data, 'mic.rms'); const en = get(data, 'mic.enabled'); text('sys-mic-rms', en ? String(rms) : 'disabled'); }
  text('sys-heap', (() => { const h = Number(get(data, 'hardware.free_heap', 0)); return h > 0 ? `${Math.round(h / 1024)} KB` : '--'; })());
  text('sys-mqtt', get(data, 'mqtt.status') || (get(data, 'mqtt.enabled') ? 'enabled' : 'disabled'));
  text('sys-ble-pol', `${get(data, 'bms.policy') || '--'}`);
  renderDetails(data);
}

async function refresh() {
  try {
    const res = await fetch('/api/status', {cache: 'no-store'});
    render(await res.json());
  } catch (err) {
    console.warn(err);
  }
}

async function loadProfiles() {
  const select = $('bms-protocol');
  if (!select) return;
  const res = await fetch('/api/bms/profiles');
  const data = await res.json();
  select.innerHTML = data.profiles.map(p => `<option value="${p.id}">${p.label}</option>`).join('');
}

async function loadThemes() {
  const select = $('theme-select');
  if (!select) return;
  const res = await fetch('/api/themes');
  const data = await res.json();
  themeOptions = data.themes || [];
  select.innerHTML = themeOptions.map(t => `<option value="${t.id}">${t.label} (${t.family})</option>`).join('');
  updateThemeDescription();
}

async function loadUsageCategories() {
  const select = $('usage-category');
  if (!select) return;
  const res = await fetch('/api/usage-categories');
  const data = await res.json();
  usageCategories = data.categories || [];
  select.innerHTML = usageCategories.map(u => `<option value="${u.id}">${u.label}</option>`).join('');
}

async function loadSettings() {
  const form = $('settings-form');
  if (!form) return;
  await Promise.all([loadProfiles(), loadThemes(), loadUsageCategories()]);
  const res = await fetch('/api/settings', {cache: 'no-store'});
  const data = await res.json();
  qsa('#settings-form [name]').forEach((el) => {
    const key = el.name;
    if (el.type === 'checkbox') el.checked = !!data[key];
    else if (data[key] !== undefined) el.value = data[key];
  });
  updateThemeDescription();
}

async function scanWifi() {
  const host = $('wifi-results');
  if (!host) return;
  host.innerHTML = '<div class="empty">Scanning...</div>';
  const data = await (await fetch('/api/wifi/scan')).json();
  host.innerHTML = data.networks.map(n => `<button type="button" onclick="chooseWifi('${String(n.ssid).replaceAll("'", "\\'")}')">${n.ssid || '(hidden)'} <span>${n.rssi} dBm</span></button>`).join('') || '<div class="empty">No networks found</div>';
}

async function scanBle() {
  const host = $('ble-results');
  if (!host) return;
  host.innerHTML = '<div class="empty">Scanning…</div>';
  const data = await (await fetch('/api/ble/scan')).json();
  if (!data.devices || !data.devices.length) {
    host.innerHTML = '<div class="empty">No BLE devices found</div>';
    return;
  }
  host.innerHTML = data.devices.map(d => {
    const name = String(d.name || '').replaceAll("'", "\\'");
    const addr = String(d.address || '').replaceAll("'", "\\'");
    const profiles = Array.isArray(d.compatible_profiles) && d.compatible_profiles.length
      ? d.compatible_profiles
      : d.recommended_profile ? [d.recommended_profile] : [];
    const protoHtml = profiles.length
      ? profiles.map(pid => {
          const safe = String(pid).replaceAll("'", "\\'");
          const plabel = d.recommended_profile === pid
            ? `${pid.replace(/_/g, ' ')} ★`
            : pid.replace(/_/g, ' ');
          return `<button class="ble-proto" type="button" onclick="chooseBle('${name}','${addr}','${safe}')">${plabel}</button>`;
        }).join('')
      : '<span class="hint">no compatible protocol found</span>';
    return `<div class="ble-device"><div class="ble-name">${d.name || d.address}<span>${d.rssi} dBm</span></div><div class="ble-protos">${protoHtml}</div></div>`;
  }).join('');
}

function toggleMqtt(on) {
  const rows = ['mqttHostRow','mqttPortRow','mqttUserRow','mqttPassRow','mqttPfxRow'];
  rows.forEach(id => { const el = document.getElementById(id); if (el) el.style.display = on ? '' : 'none'; });
  const acts = document.getElementById('mqttActions');
  if (acts) acts.style.display = on ? 'flex' : 'none';
}

async function mqttPublishNow() {
  const r = await fetch('/api/mqtt/publish-now', {method:'POST'});
  const j = await r.json();
  const el = document.getElementById('mqttStatusLine');
  if (el) el.textContent = j.ok ? 'Published.' : (j.error || 'Failed');
}

async function mqttRediscover() {
  const r = await fetch('/api/mqtt/rediscover', {method:'POST'});
  const j = await r.json();
  const el = document.getElementById('mqttStatusLine');
  if (el) el.textContent = j.ok ? 'Discovery sent.' : (j.error || 'Failed');
}

async function mqttTest() {
  const r = await fetch('/api/mqtt/test');
  const j = await r.json();
  const el = document.getElementById('mqttStatusLine');
  if (el) el.textContent = j.status + (j.broker ? ' → ' + j.broker : '');
}

function wireActions() {
  qsa('[data-display]').forEach(btn => btn.addEventListener('click', () => postForm('/api/display/page', {page: btn.dataset.display}).then(refresh)));
  $('wifi-scan')?.addEventListener('click', scanWifi);
  $('ble-scan')?.addEventListener('click', scanBle);
  $('bms-read')?.addEventListener('click', () => postForm('/api/bms/read-now', {}).then(refresh));
  $('bms-reconnect')?.addEventListener('click', () => postForm('/api/bms/reconnect', {}).then(refresh));
  $('wifi-forget')?.addEventListener('click', () => postForm('/api/wifi/forget', {}).then(() => setTimeout(refresh, 800)));
  $('setup-ap')?.addEventListener('click', () => postForm('/api/provisioning/start', {}).then(() => alert('Setup AP is starting. Connect to the R48Display AP shown on the display.')));
  $('usage-category')?.addEventListener('change', applyCategoryTheme);
  $('theme-select')?.addEventListener('change', e => {
    updateThemeDescription();
    postForm('/api/settings', {theme_id: e.target.value}).then(() => refreshThemeCss());
  });
  let brightnessTimer = null;
  document.querySelector('[name=brightness]')?.addEventListener('input', e => {
    clearTimeout(brightnessTimer);
    brightnessTimer = setTimeout(() => postForm('/api/settings', {brightness: e.target.value}), 180);
  });
  document.querySelector('[name=display_rotation]')?.addEventListener('change', e => {
    postForm('/api/settings', {display_rotation: e.target.value});
  });
  $('settings-form')?.addEventListener('submit', async (ev) => {
    ev.preventDefault();
    const form = ev.currentTarget;
    const data = Object.fromEntries(new FormData(form).entries());
    data.discharge_current_negative = form.elements.discharge_current_negative.checked ? '1' : '0';
    data.display_enabled = form.elements.display_enabled.checked ? '1' : '0';
    data.activity_detection = form.elements.activity_detection.checked ? '1' : '0';
    data.work_detection = form.elements.work_detection.checked ? '1' : '0';
    data.feature_mic = form.elements.feature_mic.checked ? '1' : '0';
    data.ntp_enabled = form.elements.ntp_enabled.checked ? '1' : '0';
    data.power_save_enabled = form.elements.power_save_enabled.checked ? '1' : '0';
    data.mqtt_enabled = form.elements.mqtt_enabled.checked ? '1' : '0';
    delete data.standby_hint;
    delete data.hours_counted;
    const saved = await postForm('/api/settings', data);
    if (!saved.ok) {
      const detail = await saved.text();
      alert(detail || `Settings save failed (${saved.status})`);
      return;
    }
    refreshThemeCss();
    await loadSettings();
    await refresh();
  });
}

wireActions();
loadSettings().then(() => {
  const mqttCb = document.getElementById('mqttEnCb');
  if (mqttCb) toggleMqtt(mqttCb.checked);
});
refresh();
setInterval(refresh, 2500);

// ── Maintenance tracker ───────────────────────────────────────────────────
const MAINT_TYPE_LABELS = {HOURS_ACTIVE:'Active Hours',HOURS_WORKING:'Working Hours',HOURS_TOTAL:'Total Hours',DAYS:'Days'};
const MAINT_TYPE_UNIT   = {HOURS_ACTIVE:'h',HOURS_WORKING:'h',HOURS_TOTAL:'h',DAYS:'d'};

async function loadMaintenance() {
  const host = $('maint-list');
  if (!host) return;
  const items = await (await fetch('/api/maintenance',{cache:'no-store'})).json().catch(()=>[]);
  if (!items.length) { host.innerHTML="<div class='empty'>No maintenance items — add one above.</div>"; return; }
  host.innerHTML = items.map(it => {
    const pct = Math.min(100, parseFloat(it.pct)||0);
    const barColor = pct >= 100 ? 'var(--bad)' : pct >= 75 ? 'var(--warn)' : 'var(--primary)';
    const elapsed = parseFloat(it.elapsed)||0;
    const remaining = parseFloat(it.remaining)||0;
    const unit = MAINT_TYPE_UNIT[it.type]||'h';
    const typeLabel = MAINT_TYPE_LABELS[it.type]||it.type;
    const overdueTag = it.overdue ? "<span style='color:var(--bad);font-weight:700'> OVERDUE</span>" : '';
    return `<div class='card' style='margin-bottom:10px'>
      <div class='section-head' style='margin-bottom:8px'>
        <span style='font-weight:600'>${it.name}${overdueTag}</span>
        <span style='color:var(--muted);font-size:12px'>${typeLabel}</span>
      </div>
      <div style='background:var(--line);border-radius:4px;height:8px;margin-bottom:8px'>
        <div style='background:${barColor};border-radius:4px;height:8px;width:${pct}%;transition:width .4s'></div>
      </div>
      <div style='display:flex;justify-content:space-between;font-size:13px;color:var(--muted);margin-bottom:10px'>
        <span>${elapsed.toFixed(1)}${unit} elapsed</span>
        <span>${it.overdue ? 'overdue' : remaining.toFixed(1)+unit+' left'} / ${parseFloat(it.interval).toFixed(0)}${unit}</span>
      </div>
      ${it.notes ? `<div style='font-size:12px;color:var(--muted);margin-bottom:8px'>${it.notes}</div>` : ''}
      <div style='display:flex;gap:8px'>
        <button onclick='confirmMaint(${it.id})' class='primary'>Mark Done</button>
        <button onclick='openMaintForm(${JSON.stringify(it)})'>Edit</button>
        <button onclick='deleteMaint(${it.id})' style='color:var(--bad)'>Delete</button>
      </div>
    </div>`;
  }).join('');
}

function openMaintForm(item) {
  $('maint-modal-title').textContent = item ? 'Edit Item' : 'Add Item';
  $('mf-id').value = item ? item.id : 0;
  $('mf-name').value = item ? item.name : '';
  $('mf-type').value = item ? item.type : 'HOURS_ACTIVE';
  $('mf-interval').value = item ? parseFloat(item.interval).toFixed(1) : '100';
  $('mf-notes').value = item ? (item.notes||'') : '';
  updateMaintIntervalLabel();
  const modal = $('maint-modal');
  modal.style.display='flex';
}

function closeMaintModal() { $('maint-modal').style.display='none'; }

function updateMaintIntervalLabel() {
  const t = $('mf-type')?.value||'HOURS_ACTIVE';
  const lbl = $('mf-interval-label');
  if (lbl) lbl.firstChild.textContent = 'Interval (' + (MAINT_TYPE_UNIT[t]||'h') + ')';
}

$('mf-type')?.addEventListener('change', updateMaintIntervalLabel);

async function saveMaintItem() {
  const data = {
    id: $('mf-id').value,
    name: $('mf-name').value.trim(),
    type: $('mf-type').value,
    interval: $('mf-interval').value,
    notes: $('mf-notes').value.trim()
  };
  if (!data.name) { alert('Name is required'); return; }
  if (!parseFloat(data.interval) > 0) { alert('Interval must be > 0'); return; }
  const res = await postForm('/api/maintenance', data);
  if (!res.ok) { alert('Failed to save item'); return; }
  closeMaintModal();
  loadMaintenance();
}

async function confirmMaint(id) {
  if (!confirm('Mark this item as done? This resets the counter from now.')) return;
  await postForm('/api/maintenance/confirm', {id});
  loadMaintenance();
}

async function deleteMaint(id) {
  if (!confirm('Delete this maintenance item?')) return;
  await postForm('/api/maintenance/delete', {id});
  loadMaintenance();
}

if ($('maint-list')) loadMaintenance();
)JS");
}

}  // namespace R48Web
