#include "WebPages.h"

namespace R48Web {

String dashboardBody() {
  return F(
      "<section class='toolbar'>"
      "<button data-display='0'>Dashboard</button>"
      "<button data-display='1'>Power</button>"
      "<button data-display='2'>Clock</button>"
      "<button data-display='3'>Status</button>"
      "</section>"
      "<section class='dashboard-grid'>"
      "<div class='gauge-card'>"
      "<div class='gauge' id='soc-gauge' style='--pct:0'><b id='dash-soc'>--</b><span>SOC</span></div>"
      "<div class='subline' id='dash-mode'>Waiting for BMS</div>"
      "</div>"
      "<div class='metric'><div class='label'>Pack Voltage</div><div class='value' id='dash-voltage'>--</div></div>"
      "<div class='metric'><div class='label'>Load</div><div class='value' id='dash-load'>--</div></div>"
      "<div class='metric'><div class='label'>Charge</div><div class='value' id='dash-charge'>--</div></div>"
      "<div class='metric'><div class='label'>Runtime Estimate</div><div class='value' id='dash-runtime'>--</div></div>"
      "<div class='metric'><div class='label'>BMS Link</div><div class='value sm' id='dash-link'>--</div></div>"
      "</section>");
}

String batteryBody() {
  return F(
      "<section class='grid'>"
      "<div class='metric'><div class='label'>SOC</div><div class='value' id='bat-soc'>--</div></div>"
      "<div class='metric'><div class='label'>Pack</div><div class='value' id='bat-pack'>--</div></div>"
      "<div class='metric'><div class='label'>Cell Spread</div><div class='value' id='bat-spread'>--</div></div>"
      "<div class='metric'><div class='label'>Capacity</div><div class='value sm' id='bat-capacity'>--</div></div>"
      "<div class='metric'><div class='label'>Temperature</div><div class='value sm' id='bat-temp'>--</div></div>"
      "<div class='metric'><div class='label'>Cycles</div><div class='value' id='bat-cycles'>--</div></div>"
      "</section>"
      "<section class='card'>"
      "<div class='section-head'><h2>Cells</h2><span id='cell-count'>--</span></div>"
      "<div class='cells' id='cells'></div>"
      "</section>");
}

String settingsBody() {
  return F(
      "<section class='grid'>"
      "<form class='card wide' id='settings-form'>"
      "<div class='section-head'><h2>Settings</h2><button class='primary'>Save</button></div>"
      "<div class='form-grid'>"
      "<label>Hostname<input name='hostname' autocomplete='off'></label>"
      "<label>OTA Password<input name='ota_password' type='password' autocomplete='new-password'></label>"
      "<label>Wi-Fi SSID<input name='wifi_ssid' autocomplete='off'></label>"
      "<label>Wi-Fi Password<input name='wifi_password' type='password' autocomplete='new-password' placeholder='leave blank to keep current'></label>"
      "<label>BMS Protocol<select name='bms_protocol' id='bms-protocol'></select></label>"
      "<label>BMS Name or Address<input name='bms_name' autocomplete='off' placeholder='blank means first compatible device'></label>"
      "<label>Usage Category<select name='usage_category' id='usage-category'></select></label>"
      "<label>Color Scheme<select name='theme_id' id='theme-select'></select><span class='hint' id='theme-description'></span></label>"
      "<label>System Label<input name='mower_model' autocomplete='off' placeholder='cart, mower, trailer, pack name'></label>"
      "<label>Nominal Pack Ah<input name='nominal_pack_ah' type='number' min='1' max='400' step='0.1'></label>"
      "<label>Typical Load Amps<input name='typical_mow_amps' type='number' min='5' max='160' step='0.1'></label>"
      "<label>Activity Detect Amps<input name='mower_run_amps' type='number' min='1' max='80' step='0.1'></label>"
      "<label>Work / Surge Detect Amps<input name='mowing_detect_amps' type='number' min='1' max='200' step='0.1'></label>"
      "<label>Timezone<input name='timezone' autocomplete='off'></label>"
      "<label>Brightness<input name='brightness' type='range' min='20' max='255'></label>"
      "<label>Display Rotation<select name='display_rotation'>"
      "<option value='0'>0 degrees</option>"
      "<option value='90'>90 degrees</option>"
      "<option value='180'>180 degrees</option>"
      "<option value='270'>270 degrees</option>"
      "</select></label>"
      "<label>Audio Assist Threshold<input name='mic_run_threshold' type='number' min='100' max='12000' step='50'></label>"
      "<label class='check'><input name='discharge_current_negative' type='checkbox'> Negative current means discharge</label>"
      "<label class='check'><input name='display_enabled' type='checkbox'> Display enabled</label>"
      "<label class='check'><input name='activity_detection' type='checkbox'> Activity mode detection</label>"
      "<label class='check'><input name='work_detection' type='checkbox'> Work / surge detection</label>"
      "<label class='check'><input name='feature_mic' type='checkbox'> Audio assist for mower work detection</label>"
      "</div>"
      "</form>"
      "<div class='card'>"
      "<div class='section-head'><h2>Wi-Fi</h2><button id='wifi-scan'>Scan</button></div>"
      "<div class='actions'><button id='wifi-forget'>Forget Wi-Fi</button><button id='setup-ap'>Start Setup AP</button></div>"
      "<div id='wifi-results' class='list'></div>"
      "</div>"
      "<div class='card'>"
      "<div class='section-head'><h2>BLE</h2><button id='ble-scan'>Scan</button></div>"
      "<div class='actions'><button id='bms-read'>Read Now</button><button id='bms-reconnect'>Reconnect</button></div>"
      "<div id='ble-results' class='list'></div>"
      "</div>"
      "</section>");
}

String statusBody() {
  return F(
      "<section class='grid'>"
      "<div class='metric'><div class='label'>Firmware</div><div class='value sm' id='status-fw'>--</div></div>"
      "<div class='metric'><div class='label'>Uptime</div><div class='value sm' id='status-uptime'>--</div></div>"
      "<div class='metric'><div class='label'>Wi-Fi</div><div class='value sm' id='status-wifi'>--</div></div>"
      "<div class='metric'><div class='label'>IP</div><div class='value sm' id='status-ip'>--</div></div>"
      "<div class='metric'><div class='label'>BLE</div><div class='value sm' id='status-ble'>--</div></div>"
      "<div class='metric'><div class='label'>Board Power</div><div class='value sm' id='status-board'>--</div></div>"
      "</section>"
      "<section class='card'>"
      "<h2>Details</h2>"
      "<table><tbody id='details'></tbody></table>"
      "</section>");
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
  const min = Math.min(...nums);
  const max = Math.max(...nums);
  cells.forEach((cell, i) => {
    const v = Number(cell);
    const pct = Number.isFinite(v) && max > min ? ((v - min) / (max - min)) * 100 : 50;
    const div = document.createElement('div');
    div.className = 'cell';
    div.innerHTML = `<span>${i + 1}</span><b>${Number.isFinite(v) ? v.toFixed(3) : '--'} V</b><i style="--w:${Math.max(4, pct)}%"></i>`;
    host.appendChild(div);
  });
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
    ['SSID', get(data, 'wifi.ssid')],
    ['RSSI', get(data, 'wifi.rssi')],
    ['Mode', get(data, 'wifi.mode')],
    ['BMS target', get(data, 'bms.target_name')],
    ['BMS protocol', get(data, 'bms.protocol')],
    ['BMS address', get(data, 'bms.address')],
    ['Last error', get(data, 'bms.last_error', '')],
    [`${activeLabel} threshold`, `${get(data, 'vehicle.run_threshold_a', get(data, 'mower.run_threshold_a'))} A`],
    [`${workLabel} threshold`, `${get(data, 'vehicle.work_threshold_a', get(data, 'mower.mowing_threshold_a'))} A`],
    ['Activity detection', get(data, 'usage.activity_detection')],
    ['Work detection', get(data, 'usage.work_detection')],
    ['Audio assist', `${get(data, 'mic.status')} rms ${get(data, 'mic.rms')} threshold ${get(data, 'mic.threshold')}`],
    [`${workLabel} likely`, get(data, 'vehicle.work_likely', get(data, 'mower.blades_likely_on'))],
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
  text('dash-runtime', fmtNum(get(data, 'vehicle.runtime_estimate_hours', get(data, 'mower.runtime_estimate_hours')), 1, ' h'));
  text('dash-link', `${get(data, 'bms.link')} ${get(data, 'bms.rssi')} dBm`);

  text('bat-soc', `${get(data, 'bms.soc')}%`);
  text('bat-pack', fmtNum(get(data, 'bms.pack_voltage'), 2, ' V'));
  text('bat-spread', fmtNum(get(data, 'bms.delta_cell_mv'), 0, ' mV'));
  text('bat-capacity', `${fmtNum(get(data, 'bms.remaining_ah'), 1)} / ${fmtNum(get(data, 'bms.design_ah'), 0)} Ah`);
  text('bat-temp', `${fmtNum(get(data, 'bms.pcb_temp'), 1)} ${get(data, 'bms.temp_unit')}`);
  text('bat-cycles', get(data, 'bms.cycles'));
  text('cell-count', `${get(data, 'bms.cell_count', 0)} cells`);
  renderCells(get(data, 'bms.cells', []));

  text('status-fw', get(data, 'firmware'));
  text('status-uptime', get(data, 'uptime'));
  text('status-wifi', `${get(data, 'wifi.ssid')} ${get(data, 'wifi.rssi')} dBm`);
  text('status-ip', get(data, 'wifi.ip') || get(data, 'wifi.ap_ip'));
  text('status-ble', `${get(data, 'bms.link')} ${get(data, 'bms.status')}`);
  text('status-board', get(data, 'screen_battery.label'));
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
  host.innerHTML = '<div class="empty">Scanning...</div>';
  const data = await (await fetch('/api/ble/scan')).json();
  host.innerHTML = data.devices.map(d => {
    const name = String(d.name || '').replaceAll("'", "\\'");
    const address = String(d.address || '').replaceAll("'", "\\'");
    const profile = String(d.recommended_profile || '').replaceAll("'", "\\'");
    const label = d.recommended_label || (d.compatible ? 'compatible' : 'unknown profile');
    return `<button type="button" onclick="chooseBle('${name}','${address}','${profile}')">${d.name || d.address} <span>${d.rssi} dBm ${label}</span></button>`;
  }).join('') || '<div class="empty">No BLE devices found</div>';
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
  $('theme-select')?.addEventListener('change', updateThemeDescription);
  $('settings-form')?.addEventListener('submit', async (ev) => {
    ev.preventDefault();
    const form = ev.currentTarget;
    const data = Object.fromEntries(new FormData(form).entries());
    data.discharge_current_negative = form.elements.discharge_current_negative.checked ? '1' : '0';
    data.display_enabled = form.elements.display_enabled.checked ? '1' : '0';
    data.activity_detection = form.elements.activity_detection.checked ? '1' : '0';
    data.work_detection = form.elements.work_detection.checked ? '1' : '0';
    data.feature_mic = form.elements.feature_mic.checked ? '1' : '0';
    await postForm('/api/settings', data);
    refreshThemeCss();
    await loadSettings();
    await refresh();
  });
}

wireActions();
loadSettings();
refresh();
setInterval(refresh, 2500);
)JS");
}

}  // namespace R48Web
