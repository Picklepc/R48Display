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
      "<div class='metric'><div class='label' id='dash-active-hours-label'>Active Hours</div><div class='value' id='dash-active-hours'>--</div></div>"
      "<div class='metric'><div class='label' id='dash-working-hours-label'>Working Hours</div><div class='value' id='dash-working-hours'>--</div></div>"
      "<div class='metric'><div class='label'>Health</div><div class='value' id='dash-health'>--</div></div>"
      "<div class='metric'><div class='label'>Maintenance</div><div class='value sm' id='dash-maintenance'>Not configured</div></div>"
      "<div class='metric'><div class='label'>BMS Link</div><div class='value sm' id='dash-link'>--</div></div>"
      "<div class='metric' id='dash-pay-wrap' style='display:none'><div class='label'>Pay Pending</div><div class='value sm' id='dash-pay'>--</div></div>"
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
      "<div class='metric'><div class='label'>Batt Backup</div><div class='value sm' id='sys-save'>--</div></div>"
      "<div class='metric'><div class='label'>Mic RMS</div><div class='value sm' id='sys-mic-rms'>--</div></div>"
      "<div class='metric'><div class='label'>Heap Free</div><div class='value sm' id='sys-heap'>--</div></div>"
      "<div class='metric'><div class='label'>MQTT</div><div class='value sm' id='sys-mqtt'>--</div></div>"
      "<div class='metric'><div class='label'>BLE Policy</div><div class='value sm' id='sys-ble-pol'>--</div></div>"
      "</div>"
      "</section>"
      // Weather card
      "<section class='card'>"
      "<h2>Weather</h2>"
      "<div id='wx-current' style='display:flex;align-items:baseline;gap:14px;margin-bottom:14px'>"
      "<span id='wx-temp' style='font-size:36px;font-weight:700;color:var(--primary)'>--</span>"
      "<span id='wx-cond' style='font-size:20px;color:var(--text)'>--</span>"
      "<span id='wx-loc' style='font-size:12px;color:var(--muted);margin-left:auto;align-self:flex-end'>--</span>"
      "</div>"
      "<div id='wx-forecast' style='display:flex;gap:6px;flex-wrap:wrap'></div>"
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

PGM_P settingsBody() {
  return PSTR(
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

      // AP Mode — password and LCD advertising
      "<div class='settings-group'>"
      "<h3>AP Mode</h3>"
      "<div class='form-grid'>"
      "<label>AP Password"
      "<span class='hint'>Password for the setup AP shown on the LCD. Must be 8+ characters. Leave blank to keep current.</span>"
      "<input name='ap_password' type='password' autocomplete='new-password' placeholder='leave blank to keep current'></label>"
      "</div>"
      "<label class='check'>"
      "<input name='advertise_ap_credentials' id='advertiseApCb' type='checkbox'>"
      "Show AP credentials on LCD status screen"
      "</label>"
      "<p class='note'>When enabled, the SSID and password cycle on the bottom status line while the device is in AP mode. Disable if the display is in a public area.</p>"
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
      "<label>Weather Zip Code<input name='zip_code' maxlength='5' pattern='[0-9]{5}' inputmode='numeric' autocomplete='off' placeholder='e.g. 90210'><span class='hint'>US zip code — fetches current weather and 7-day forecast</span></label>"
      "<label>Brightness<input name='brightness' type='range' min='20' max='255'></label>"
      "<label>Display Rotation<select name='display_rotation'>"
      "<option value='0'>0&deg; (default)</option>"
      "<option value='90'>90&deg;</option>"
      "<option value='180'>180&deg;</option>"
      "<option value='270'>270&deg;</option>"
      "</select></label>"
      "<label class='check'><input name='display_enabled' type='checkbox'> Display enabled</label>"
      "<label class='check'><input name='anim_enabled' id='animEnabledCb' type='checkbox' onchange='toggleAnimOptions(this.checked)'> Background animations enabled</label>"
      "<div id='anim-options' style='display:none;margin-top:4px'>"
      "<label>Animation Style<select name='anim_type' id='anim-type-select'>"
      "<option value='0'>None</option>"
      "<option value='1'>Stars</option>"
      "<option value='2'>Embers</option>"
      "<option value='3'>Lightning</option>"
      "<option value='4'>Ripple</option>"
      "<option value='5'>Speed Lines</option>"
      "<option value='6'>Rave</option>"
      "<option value='7'>Pixel Rain</option>"
      "<option value='8'>Geometry</option>"
      "<option value='9'>Flag + Fireworks</option>"
      "<option value='10'>Bats</option>"
      "<option value='11'>Snow</option>"
      "<option value='12'>Leaves</option>"
      "<option value='13'>Hearts</option>"
      "<option value='14'>Grass</option>"
      "<option value='15'>Fireworks</option>"
      "</select></label>"
      "</div>"
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
      "<b>Active Hours</b> counts all time above the Active amps threshold, including Working time. "
      "<b>Working Hours</b> counts the high-draw subset above the Working/surge threshold. "
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
      "<span class='hint'>Time spent above the Active amps threshold, including Working time. Edit to reset or correct.</span>"
      "<input name='hours_active' type='number' min='0' max='99999' step='any'></label>"
      "<label>Working Hours"
      "<span class='hint'>High-draw subset of Active Hours. Edit to reset or correct.</span>"
      "<input name='hours_working' type='number' min='0' max='99999' step='any'></label>"
      "</div>"
      "<label class='check' id='track-hday-wrap'><input name='track_daily_activity' type='checkbox' id='trackHdayCb'> Track Daily Activity History"
      "<span class='note'>Records daily hours to the activity heatmap on the Maintenance page. Requires a Wi-Fi network to be configured (uses NTP for day detection). Enabled by default once an SSID is saved.</span></label>"
      "<label class='check'><input name='track_pay' type='checkbox' id='trackPayCb'> Enable Pay Records"
      "<span class='note'>Shows the Pay Records section on the Maintenance page. Track payees, hourly rates, and working hours per period. Working hours are sourced from the activity history when available.</span></label>"
      "</div>"

      // Power Management
      "<div class='settings-group'>"
      "<h3>Power Management</h3>"
      "<div class='form-grid'>"
      "<label class='check'><input name='power_save_enabled' type='checkbox' id='battEnCb' onchange='toggleBattOptions(this.checked)'> Enable Internal Battery"
      "<span class='note'>When enabled, the onboard LiPo provides backup power when USB is removed and activates the power-saving options below. When disabled, the device shuts down when USB power is removed and the options below have no effect.</span></label>"
      "</div>"
      "<div id='batt-options'>"
      "<div class='form-grid'>"
      "<label>LCD Timeout (seconds)<input name='lcd_timeout_sec' type='number' min='0' max='3600' step='5'></label>"
      "<label>Idle BLE Wake Hours<input name='idle_ble_wake_hours' type='number' min='0.25' max='24' step='0.25'></label>"
      "<label>Low Voltage Floor (V/cell)<input name='low_voltage_floor_v' type='number' min='2.0' max='3.8' step='0.01'></label>"
      "<label>Low Battery Threshold (%)"
      "<span class='hint'>When the onboard LiPo drops below this level, low power mode activates and hour counting pauses. On startup from a depleted state, hours don't count until the battery rises above this threshold.</span>"
      "<input name='board_battery_low_pct' type='number' min='5' max='80' step='5'></label>"
      "</div>"
      "<div class='form-grid' style='margin-top:0.5rem'>"
      "<div><button type='button' id='battOffBtn' class='btn-danger' onclick='batteryOff()'>Cut Battery Power</button>"
      "<span class='hint' style='margin-top:0.25rem;display:block'>Immediately saves all data and toggles the battery off via the PMU key pin. Use this to shut the device down when USB power is removed. The device will restart when USB is reconnected.</span>"
      "</div>"
      "</div>"
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
      "<div id='fwupd'>"
      "<div style='display:flex;justify-content:space-between;align-items:center;gap:10px;flex-wrap:wrap'>"
      "<div>"
      "<div>Installed: <b id='fwupd-current'>&mdash;</b></div>"
      "<div id='fwupd-latest' class='hint' style='margin-top:2px'></div>"
      "</div>"
      "<div style='display:flex;gap:8px'>"
      "<button type='button' id='fwupd-check' onclick='checkFirmwareUpdate()'>Check for updates</button>"
      "<button type='button' id='fwupd-install' class='primary' style='display:none' onclick='applyFirmwareUpdate()'>Install</button>"
      "</div></div>"
      "<div id='fwupd-status' class='hint' style='margin-top:8px'></div>"
      "<div id='fwupd-progress-wrap' style='display:none;background:var(--line);border-radius:4px;height:8px;margin-top:8px'>"
      "<div id='fwupd-progress' style='background:var(--primary);border-radius:4px;height:8px;width:0%;transition:width .3s'></div>"
      "</div>"
      "<div id='fwupd-pick' style='display:none;margin-top:12px'>"
      "<label>Install a specific version<span class='hint'>Any published release &mdash; use this to reinstall or roll back to an earlier version.</span>"
      "<div style='display:flex;gap:8px;margin-top:4px'>"
      "<select id='fwupd-version' style='flex:1'></select>"
      "<button type='button' onclick='installSelectedVersion()'>Install</button>"
      "</div></label></div>"
      "<label class='check' style='margin-top:12px'><input name='auto_update_check' type='checkbox' id='autoUpdCb'> Automatically check for updates</label>"
      "</div>"
      "<details style='margin-top:14px'>"
      "<summary style='cursor:pointer;color:var(--muted);font-size:14px'>Manual upload (advanced)</summary>"
      "<form method='POST' action='/update' enctype='multipart/form-data' style='margin-top:10px'>"
      "<div class='form-grid' style='margin-bottom:10px'>"
      "<label>Firmware image<span class='hint'>Upload <b>firmware.bin</b> (the app image) from a release &mdash; NOT firmware-merged.bin, which is for USB flashing only.</span><input type='file' name='firmware' accept='.bin'></label>"
      "</div>"
      "<button class='primary'>Upload &amp; Reboot</button>"
      "</form>"
      "<p class='hint' style='margin-top:10px'>The device reboots automatically after a successful flash. If it rejects the file, you likely picked firmware-merged.bin &mdash; use the version picker above instead.</p>"
      "</details>"
      "</div>"

      "</section>");
}

PGM_P maintenanceBody() {
  return PSTR(
      // Activity heatmap
      "<section class='card wide' id='heatmap-card' style='display:none'>"
      "<div class='section-head'>"
      "<h2>Activity History</h2>"
      "<div style='display:flex;gap:6px;align-items:center'>"
      "<button id='hm-prev' onclick='hmNav(-1)' style='padding:4px 10px;line-height:1'>&#8592;</button>"
      "<span id='hm-year' style='min-width:36px;text-align:center;font-weight:600;font-size:15px'></span>"
      "<button id='hm-next' onclick='hmNav(1)' style='padding:4px 10px;line-height:1'>&#8594;</button>"
      "<a id='hm-dl' href='/api/heatmap/export.csv' download style='margin-left:6px;border:1px solid var(--line);background:var(--panel2);color:var(--text);border-radius:7px;padding:6px 10px;text-decoration:none;font:inherit;cursor:pointer;font-size:13px'>Export CSV</a>"
      "</div></div>"
      "<div style='overflow-x:auto;padding:6px 0 4px'><div id='hm-svg'></div></div>"
      "<div id='hm-legend' style='display:flex;gap:10px;align-items:center;margin-top:6px;font-size:12px;color:var(--muted)'>"
      "<span>Less</span>"
      "<span id='hm-swatches'></span>"
      "<span>More</span>"
      "</div>"
      "</section>"

      // Floating tooltip (shared)
      "<div id='hm-tip' style='display:none;position:fixed;z-index:300;background:var(--panel2);border:1px solid var(--line);border-radius:8px;padding:10px 14px;font-size:13px;pointer-events:none;box-shadow:0 4px 18px rgba(0,0,0,.45);max-width:260px'></div>"

      // Hours graph
      "<section class='card wide'>"
      "<h2>Hour Meter</h2>"
      "<div id='hours-bars' style='display:grid;gap:10px;margin:10px 0 8px'></div>"
      "<div id='hours-legend' style='display:flex;gap:16px;flex-wrap:wrap;font-size:13px;color:var(--muted)'>Loading\xe2\x80\xa6</div>"
      "</section>"

      // Machine / project notes
      "<section class='card wide'>"
      "<div class='section-head'><h2>Machine / Project Notes</h2>"
      "<button class='primary' form='machine-form'>Save Notes</button></div>"
      "<form id='machine-form'>"
      "<div class='form-grid'>"
      "<label>Manufacturer<input name='machine_make' maxlength='40' autocomplete='off' placeholder='Ryobi'></label>"
      "<label>Model Number<input name='machine_model' maxlength='40' autocomplete='off' placeholder='RY48ZTR100 / RM480e / custom'></label>"
      "<label>Serial / VIN<input name='machine_serial' maxlength='50' autocomplete='off'></label>"
      "<label>Machine Manufacture Date<input name='machine_mfg_date' type='date'></label>"
      "<label>Gauge Install Date<input name='gauge_install_date' type='date'></label>"
      "<label>Battery / Pack Model<input name='battery_model' maxlength='50' autocomplete='off'></label>"
      "<label>Battery Install Date<input name='battery_install_date' type='date'></label>"
      "</div>"
      "<div class='settings-group'>"
      "<div class='section-head' style='margin-bottom:8px'><h3>Custom Fields</h3><button type='button' onclick='addMachineField()'>+ Field</button></div>"
      "<div id='machine-fields' class='list'></div>"
      "</div>"
      "<div class='settings-group'>"
      "<h3>Notes</h3>"
      "<label>Installation notes, part numbers, service references, wiring notes"
      "<textarea name='machine_notes' rows='5' maxlength='800' placeholder='Part numbers, conversion details, wiring notes, purchase dates, torque specs, etc.'></textarea></label>"
      "</div>"
      "<p id='machine-save-status' class='note' style='margin-top:10px'></p>"
      "</form>"
      "</section>"

      // Maintenance list
      "<section class='card wide'>"
      "<div class='section-head'>"
      "<h2>Maintenance</h2>"
      "<div style='display:flex;gap:8px'>"
      "<a href='/api/maintenance/export' style='border:1px solid var(--line);background:var(--panel2);color:var(--text);border-radius:7px;padding:8px 11px;text-decoration:none;font:inherit;cursor:pointer;font-size:13px'>Export CSV</a>"
      "<button class='primary' onclick='openMaintForm(null)'>+ Add Item</button>"
      "</div></div>"
      "<div id='maint-list'><div class='empty'>Loading\xe2\x80\xa6</div></div>"
      "</section>"

      // Pay Records
      "<section class='card wide' id='pay-section' style='display:none'>"
      "<div class='section-head'>"
      "<h2>Pay Records</h2>"
      "<div style='display:flex;gap:8px'>"
      "<a href='/api/pay/export.csv' download style='border:1px solid var(--line);background:var(--panel2);color:var(--text);border-radius:7px;padding:8px 11px;text-decoration:none;font:inherit;cursor:pointer;font-size:13px'>Export CSV</a>"
      "<button class='primary' onclick='openPayForm(null)'>+ Add Payee</button>"
      "</div></div>"
      "<div id='pay-list'><div class='empty'>Loading\xe2\x80\xa6</div></div>"
      "</section>"

      // Pay add / edit modal
      "<div id='pay-modal' style='display:none;position:fixed;inset:0;background:rgba(0,0,0,.6);z-index:100;align-items:center;justify-content:center'>"
      "<div class='card' style='min-width:280px;max-width:420px;width:92%;padding:20px'>"
      "<h3 id='pay-modal-title'>Add Payee</h3>"
      "<div class='form-grid'>"
      "<label>Payee Name<input id='pf-payee' maxlength='40' autocomplete='off' placeholder='Alex'></label>"
      "<label>Hourly Rate<input id='pf-rate' type='number' min='0' step='0.01' placeholder='12.00'></label>"
      "<label>Currency / Label<input id='pf-label' maxlength='8' autocomplete='off' placeholder='$'></label>"
      "<label>Period Start<span class='hint'>Date &amp; time this pay period began. Edit to back-date a forgotten period.</span><input id='pf-start' type='datetime-local'></label>"
      "<label>Notes (optional)<input id='pf-notes' maxlength='80' autocomplete='off'></label>"
      "</div>"
      "<details style='margin-top:10px'>"
      "<summary id='pf-start-work-summary' style='cursor:pointer;color:var(--muted);font-size:13px'>Or start at working-hours into a day</summary>"
      "<p class='hint' id='pf-start-work-hint' style='margin:8px 0'>Back-date the start to a point inside a day's working time &mdash; e.g. start after the first 1.5 hours so an earlier session goes to someone else. Overrides the date &amp; time above.</p>"
      "<div style='display:flex;gap:8px;align-items:flex-end'>"
      "<label style='flex:1'>Day<input id='pf-start-day' type='date' onchange='payStartDayInfo()'></label>"
      "<label style='width:120px'>Hours into day<input id='pf-start-hours' type='number' min='0' step='0.1' placeholder='0'></label>"
      "</div>"
      "<div id='pf-start-dayinfo' class='hint' style='margin-top:6px'></div>"
      "</details>"
      "<input type='hidden' id='pf-id' value='0'>"
      "<div style='display:flex;gap:8px;margin-top:14px'>"
      "<button class='primary' onclick='submitPayForm()'>Save</button>"
      "<button onclick='closePayForm()'>Cancel</button>"
      "</div>"
      "</div>"
      "</div>"

      // Mark Paid modal
      "<div id='pay-confirm-modal' style='display:none;position:fixed;inset:0;background:rgba(0,0,0,.6);z-index:100;align-items:center;justify-content:center'>"
      "<div class='card' style='min-width:280px;max-width:420px;width:92%;padding:20px'>"
      "<h3>Mark Paid \xe2\x80\x94 <span id='pc-payee'></span></h3>"
      "<div id='pay-confirm-summary' style='background:var(--panel2);border-radius:8px;padding:12px;margin-bottom:14px;font-size:14px;line-height:1.7'></div>"
      "<div class='form-grid'>"
      "<label>Close at<span class='hint' id='pc-close-hint'>Leave blank to close now (all hours so far). Set to N working-hours of today to split the day &mdash; the next period picks up where this ends.</span>"
      "<input id='pc-close' type='number' min='0' step='0.1'></label>"
      "<label>Amount paid<span class='hint'>Leave blank to use the calculated amount. Enter a different value for bonuses or adjustments.</span>"
      "<input id='pc-amount' type='number' min='0' step='0.01' placeholder='Leave blank to use calculated'></label>"
      "<label>Notes<input id='pc-notes' maxlength='80' autocomplete='off' placeholder='e.g. paid in cash'></label>"
      "</div>"
      "<input type='hidden' id='pc-id' value='0'>"
      "<div style='display:flex;gap:8px;margin-top:14px'>"
      "<button class='primary' onclick='confirmPay()'>Confirm Payment</button>"
      "<button onclick='closePayConfirm()'>Cancel</button>"
      "</div>"
      "</div>"
      "</div>"

      // Payment history overlay / drawer
      "<div id='pay-hist-overlay' style='display:none;position:fixed;inset:0;background:rgba(0,0,0,.6);z-index:100' onclick='closePayHist()'>"
      "<div style='position:absolute;bottom:0;left:50%;transform:translateX(-50%);width:100%;max-width:600px;max-height:72vh;overflow-y:auto;background:var(--panel);border-radius:16px 16px 0 0;padding:20px' onclick='event.stopPropagation()'>"
      "<div class='section-head' style='margin-bottom:14px'>"
      "<h3 id='pay-hist-title'>Payment History</h3>"
      "<button onclick='closePayHist()' style='font-size:20px;line-height:1;padding:0 8px'>&times;</button>"
      "</div>"
      "<div id='pay-hist-list'></div>"
      "</div>"
      "</div>"

      // Add / edit modal
      "<div id='maint-modal' style='display:none;position:fixed;inset:0;background:rgba(0,0,0,.6);z-index:100;align-items:center;justify-content:center'>"
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
      "<label>Item notes<input id='mf-notes' maxlength='80' autocomplete='off'></label>"
      "</div>"
      "<input type='hidden' id='mf-id' value='0'>"
      "<div style='display:flex;gap:8px;margin-top:14px'>"
      "<button class='primary' onclick='saveMaintItem()'>Save</button>"
      "<button onclick='closeMaintModal()'>Cancel</button>"
      "</div></div></div>"

      // Mark-done modal (with completion notes)
      "<div id='maint-confirm-modal' style='display:none;position:fixed;inset:0;background:rgba(0,0,0,.6);z-index:100;align-items:center;justify-content:center'>"
      "<div class='card' style='min-width:280px;max-width:420px;width:92%;padding:20px'>"
      "<h3 id='mc-name' style='margin-bottom:8px'>Mark Done</h3>"
      "<p class='note' style='margin-bottom:14px'>Resets the counter and saves a completion record. Each item stores the 10 most recent completions.</p>"
      "<label style='display:grid;gap:6px;color:var(--muted);font-size:13px'>Completion notes (optional)"
      "<textarea id='mc-notes' rows='3' maxlength='200' style='width:100%;background:var(--panel2);color:var(--text);border:1px solid var(--line);border-radius:7px;padding:9px;font:inherit;resize:vertical'></textarea>"
      "</label>"
      "<input type='hidden' id='mc-id' value='0'>"
      "<div style='display:flex;gap:8px;margin-top:14px'>"
      "<button class='primary' onclick='submitConfirmMaint()'>Confirm Done</button>"
      "<button onclick='closeConfirmModal()'>Cancel</button>"
      "</div></div></div>"

      // Edit history modal
      "<div id='maint-history-modal' style='display:none;position:fixed;inset:0;background:rgba(0,0,0,.6);z-index:100;align-items:center;justify-content:center'>"
      "<div class='card' style='min-width:280px;max-width:420px;width:92%;padding:20px'>"
      "<h3 style='margin-bottom:8px'>Edit History Entry</h3>"
      "<div class='form-grid'>"
      "<label>Completed At<input id='mh-date' type='datetime-local'></label>"
      "<label>Completion notes"
      "<textarea id='mh-notes' rows='4' maxlength='200' style='width:100%;background:var(--panel2);color:var(--text);border:1px solid var(--line);border-radius:7px;padding:9px;font:inherit;resize:vertical'></textarea>"
      "</label>"
      "</div>"
      "<input type='hidden' id='mh-id' value='0'>"
      "<input type='hidden' id='mh-original-ts' value='0'>"
      "<div style='display:flex;gap:8px;margin-top:14px'>"
      "<button class='primary' onclick='submitHistoryEdit()'>Save</button>"
      "<button onclick='closeHistoryModal()'>Cancel</button>"
      "</div></div></div>");
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

PGM_P appScript() {
  return PSTR(R"JS(
const $ = (id) => document.getElementById(id);
const qsa = (sel) => Array.from(document.querySelectorAll(sel));
let themeOptions = [];
let usageCategories = [];
let _loadedHoursBaseline = 0;
let machineFields = [];
let maintHistoryEntries = {};
let _maintSummaryLast = 0;
let _maintSummaryBusy = false;

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

function escAttr(value) {
  return String(value ?? '').replaceAll('&','&amp;').replaceAll('"','&quot;').replaceAll("'",'&#39;').replaceAll('<','&lt;').replaceAll('>','&gt;');
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
function updateAnimForTheme(themeId) {
  const sel = document.getElementById('anim-type-select');
  if (!sel) return;
  const t = themeOptions.find(t => t.id === themeId);
  if (t != null && t.anim_type != null) sel.value = String(t.anim_type);
}

function applyCategoryTheme() {
  const usage = selectedUsage();
  const themeSelect = $('theme-select');
  if (usage && themeSelect && usage.default_theme) themeSelect.value = usage.default_theme;
  updateThemeDescription();
  updateAnimForTheme(themeSelect?.value);
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
  const activeLabel = get(data, 'vehicle.active_label', get(data, 'usage.active_label', 'activity'));
  const workLabel = get(data, 'vehicle.work_label', get(data, 'usage.work_label', 'work'));
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
    ['Power source', `${get(data, 'screen_battery.power_source')} / batt:${get(data, 'screen_battery.power_save_enabled') ? 'on' : 'off'}`],
    ['BLE policy', `${get(data, 'bms.policy')} (${get(data, 'bms.soc_rate_pct_per_hour')}%/h)`],
    ['NTP', get(data, 'clock.ntp_configured') ? `sync via ${get(data, 'clock.ntp_server')}` : 'disabled'],
    ['Touch', get(data, 'hardware.touch_ready')],
    ['Rotation', `${get(data, 'display.rotation_degrees')} degrees`],
    ['Display page', get(data, 'display.page_name')],
    ['Clock', get(data, 'clock.local_time')]
  ];
  body.innerHTML = rows.map(([k, v]) => `<tr><th>${escAttr(k)}</th><td>${escAttr(v === '' ? '--' : v)}</td></tr>`).join('');
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
  text('dash-active-hours-label', `${get(data, 'vehicle.active_label', 'Active')} Hours`);
  text('dash-working-hours-label', `${get(data, 'vehicle.work_label', 'Working')} Hours`);
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
  if ($('dash-maintenance') && !_maintSummaryBusy && Date.now() - _maintSummaryLast > 30000) {
    _maintSummaryBusy = true;
    fetch('/api/maintenance', {cache: 'no-store'}).then(r => r.json()).then(items => {
      const overdue = items.filter(i => i.overdue).length;
      const total = items.length;
      text('dash-maintenance', total === 0 ? 'None configured' : overdue > 0 ? `${overdue} item${overdue > 1 ? 's' : ''} due` : `${total} item${total > 1 ? 's' : ''}, all OK`);
      _maintSummaryLast = Date.now();
    }).catch(() => {}).finally(() => { _maintSummaryBusy = false; });
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
}

// Recurring poll: compact live telemetry only (theme/usage/mower omitted).
async function refresh() {
  try {
    const res = await fetch('/api/live', {cache: 'no-store'});
    render(await res.json());
  } catch (err) {
    console.warn(err);
  }
}

// Once per page load: full status + the static details/config table.
async function refreshFull() {
  try {
    const res = await fetch('/api/status', {cache: 'no-store'});
    const data = await res.json();
    render(data);
    renderDetails(data);
  } catch (err) {
    console.warn(err);
  }
}

async function refreshWeather() {
  if (!$('wx-forecast')) return;
  try {
    const data = await (await fetch('/api/weather', {cache:'no-store'})).json();
    if (data.valid) {
      text('wx-temp', data.temp || '--');
      text('wx-cond', data.condition || '--');
      text('wx-loc', data.zip ? 'ZIP ' + data.zip : '');
      const fc = $('wx-forecast');
      if (fc && data.forecast) {
        fc.innerHTML = data.forecast.map(d =>
          `<div style='background:var(--panel2);border-radius:7px;padding:10px 12px;min-width:62px;text-align:center;flex:1'>`+
          `<div style='font-size:11px;color:var(--muted);margin-bottom:4px'>${escAttr(d.day)}</div>`+
          `<div style='font-size:11px;color:var(--text);margin-bottom:6px;line-height:1.2'>${escAttr(d.cond)}</div>`+
          `<div style='font-size:15px;font-weight:600;color:var(--primary)'>${escAttr(d.hi)}</div>`+
          `<div style='font-size:12px;color:var(--muted)'>${escAttr(d.lo)}</div>`+
          `</div>`
        ).join('');
      }
    } else {
      text('wx-temp', '--');
      text('wx-cond', data.zip ? 'Waiting…' : 'Set zip in Settings');
      text('wx-loc', data.zip ? 'ZIP ' + data.zip : '');
      const fc = $('wx-forecast');
      if (fc) fc.innerHTML = '';
    }
  } catch(e) {}
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
  select.innerHTML = themeOptions.map(t => `<option value="${t.id}">${t.label}</option>`).join('');
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
  await loadProfiles();
  await loadThemes();
  await loadUsageCategories();
  const res = await fetch('/api/settings', {cache: 'no-store'});
  const data = await res.json();
  qsa('#settings-form [name]').forEach((el) => {
    const key = el.name;
    if (el.type === 'checkbox') el.checked = !!data[key];
    else if (data[key] !== undefined) el.value = data[key];
  });
  _loadedHoursBaseline = parseFloat(data.hours_baseline || '0');
  const battCb = document.getElementById('battEnCb');
  if (battCb) toggleBattOptions(battCb.checked);
  const animCb = document.getElementById('animEnabledCb');
  if (animCb) toggleAnimOptions(animCb.checked);
  updateThemeDescription();
  // Grey out Track Daily Activity when no STA SSID is configured
  const ssidEl = document.querySelector('[name="wifi_ssid"]');
  const trackCb = document.getElementById('trackHdayCb');
  const trackWrap = document.getElementById('track-hday-wrap');
  function syncTrackHday() {
    const has = ssidEl && ssidEl.value.trim().length > 0;
    if (trackCb) trackCb.disabled = !has;
    if (trackWrap) trackWrap.style.opacity = has ? '1' : '0.42';
  }
  if (ssidEl) ssidEl.addEventListener('input', syncTrackHday);
  syncTrackHday();
}

async function scanWifi() {
  const host = $('wifi-results');
  if (!host) return;
  host.innerHTML = '<div class="empty">Scanning...</div>';
  const data = await (await fetch('/api/wifi/scan')).json();
  host.innerHTML = data.networks.map(n => `<button type="button" data-ssid="${escAttr(n.ssid||'')}" onclick="chooseWifi(this.dataset.ssid)">${escAttr(n.ssid || '(hidden)')} <span>${n.rssi} dBm</span></button>`).join('') || '<div class="empty">No networks found</div>';
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
    const safeName = escAttr(d.name || '');
    const safeAddr = escAttr(d.address || '');
    const profiles = Array.isArray(d.compatible_profiles) && d.compatible_profiles.length
      ? d.compatible_profiles
      : d.recommended_profile ? [d.recommended_profile] : [];
    const protoHtml = profiles.length
      ? profiles.map(pid => {
          const plabel = d.recommended_profile === pid
            ? `${pid.replace(/_/g, ' ')} ★`
            : pid.replace(/_/g, ' ');
          return `<button class="ble-proto" type="button" data-name="${safeName}" data-addr="${safeAddr}" data-proto="${escAttr(pid)}" onclick="chooseBle(this.dataset.name,this.dataset.addr,this.dataset.proto)">${escAttr(plabel)}</button>`;
        }).join('')
      : '<span class="hint">no compatible protocol found</span>';
    return `<div class="ble-device"><div class="ble-name">${escAttr(d.name || d.address)}<span>${d.rssi} dBm</span></div><div class="ble-protos">${protoHtml}</div></div>`;
  }).join('');
}

function toggleBattOptions(on) {
  const el = document.getElementById('batt-options');
  if (el) el.style.display = on ? '' : 'none';
}
function toggleAnimOptions(on) {
  const el = document.getElementById('anim-options');
  if (el) el.style.display = on ? '' : 'none';
}
async function batteryOff() {
  const btn = document.getElementById('battOffBtn');
  if (!confirm('This will save all data and immediately cut battery power. The device will turn off if not on USB. Continue?')) return;
  if (btn) { btn.disabled = true; btn.textContent = 'Cutting power…'; }
  try {
    await fetch('/api/battery/off', {method:'POST'});
  } catch(e) {}
  if (btn) btn.textContent = 'Done — device may be off';
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
    updateAnimForTheme(e.target.value);
    const animSel = document.getElementById('anim-type-select');
    const payload = {theme_id: e.target.value};
    if (animSel) payload.anim_type = animSel.value;
    postForm('/api/settings', payload).then(() => refreshThemeCss());
  });
  document.getElementById('animEnabledCb')?.addEventListener('change', e => {
    postForm('/api/settings', {anim_enabled: e.target.checked ? '1' : '0'});
  });
  document.getElementById('anim-type-select')?.addEventListener('change', e => {
    postForm('/api/settings', {anim_type: e.target.value});
  });
  document.getElementById('autoUpdCb')?.addEventListener('change', e => {
    postForm('/api/settings', {auto_update_check: e.target.checked ? '1' : '0'});
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
    data.advertise_ap_credentials = form.elements.advertise_ap_credentials.checked ? '1' : '0';
    data.anim_enabled = form.elements.anim_enabled.checked ? '1' : '0';
    data.anim_type = form.elements.anim_type ? form.elements.anim_type.value : '0';
    data.track_daily_activity = form.elements.track_daily_activity.checked ? '1' : '0';
    data.track_pay = form.elements.track_pay.checked ? '1' : '0';
    delete data.standby_hint;
    delete data.hours_counted;
    // Only send hours_baseline if the user actually changed it; otherwise let
    // hours_total take effect if the user edited that field instead.
    if (Math.abs(parseFloat(data.hours_baseline || '0') - _loadedHoursBaseline) < 0.005) {
      delete data.hours_baseline;
    }
    const saved = await postForm('/api/settings', data);
    if (!saved.ok) {
      const detail = await saved.text();
      alert(detail || `Settings save failed (${saved.status})`);
      return;
    }
    refreshThemeCss();
    await loadSettings();
    await refreshFull();
  });
}

wireActions();
loadSettings().then(() => {
  const mqttCb = document.getElementById('mqttEnCb');
  if (mqttCb) toggleMqtt(mqttCb.checked);
  const battCb = document.getElementById('battEnCb');
  if (battCb) toggleBattOptions(battCb.checked);
  const animCb = document.getElementById('animEnabledCb');
  if (animCb) toggleAnimOptions(animCb.checked);
});
if ($('fwupd-current')) loadUpdateStatus();
// ── Pay stats (dashboard) ─────────────────────────────────────────────────────
async function loadPayStats() {
  const wrap = $('dash-pay-wrap');
  const el   = $('dash-pay');
  if (!wrap || !el) return;
  const resp = await fetch('/api/pay',{cache:'no-store'}).then(r=>r.ok?r.json():null).catch(()=>null);
  if (!resp || !resp.track_pay || !resp.records?.length) {
    wrap.style.display = 'none'; return;
  }
  let total = 0;
  resp.records.forEach(pr => { total += parseFloat(pr.earned)||0; });
  wrap.style.display = '';
  el.textContent = resp.records.length === 1
    ? `${resp.records[0].label||'$'}${(parseFloat(resp.records[0].earned)||0).toFixed(2)}`
    : `${resp.records[0].label||'$'}${total.toFixed(2)}`;
}

// ── Firmware self-update ──────────────────────────────────────────────────────
let _fwupdPoll = null;

function renderUpdateStatus(s) {
  const cur = $('fwupd-current'); if (cur) cur.textContent = 'v' + s.current;
  const auto = $('autoUpdCb'); if (auto) auto.checked = !!s.auto;
  const latest = $('fwupd-latest');
  const status = $('fwupd-status');
  const install = $('fwupd-install');
  const check = $('fwupd-check');
  const pwrap = $('fwupd-progress-wrap');
  const pbar = $('fwupd-progress');
  const busy = !!s.busy;
  if (check) check.disabled = busy;
  if (s.phase === 'downloading') {
    if (pwrap) pwrap.style.display = 'block';
    if (pbar) pbar.style.width = (s.progress || 0) + '%';
  } else if (pwrap) { pwrap.style.display = 'none'; }
  if (latest) {
    if (s.checked && s.latest) latest.textContent = s.available ? `Update available: v${s.latest}` : `Up to date (latest v${s.latest})`;
    else latest.textContent = '';
  }
  if (install) {
    install.style.display = (s.available && !busy) ? '' : 'none';
    if (s.available) install.textContent = `Install v${s.latest}`;
  }
  // Version picker lists v0.3.6+ only (older builds predate the self-updater).
  const selTags = (Array.isArray(s.tags) ? s.tags : []).filter(t => {
    const p = String(t).replace(/^v/,'').split('.').map(n => parseInt(n) || 0);
    return (p[0]*10000 + p[1]*100 + (p[2]||0)) >= 306;
  });
  const pick = $('fwupd-pick'), sel = $('fwupd-version');
  if (pick && sel && selTags.length) {
    pick.style.display = '';
    if (sel.dataset.filled !== String(selTags.length)) {
      sel.innerHTML = selTags.map(t => { const v = String(t).replace(/^v/,'');
        return `<option value="${escAttr(v)}">${escAttr(t)}${v===s.current?' (installed)':''}</option>`; }).join('');
      sel.dataset.filled = String(selTags.length);
    }
  }
  if (status) {
    if (s.phase === 'checking') status.textContent = 'Checking GitHub…';
    else if (s.phase === 'downloading') status.textContent = `Installing… ${s.progress || 0}% — do not power off. The device reboots when done.`;
    else if (s.phase === 'error') status.textContent = s.error || 'Update error';
    else status.textContent = '';
  }
}

async function loadUpdateStatus() {
  const s = await fetch('/api/update/status', {cache:'no-store'}).then(r => r.ok ? r.json() : null).catch(() => null);
  if (s) renderUpdateStatus(s);
}

function stopUpdatePolling() { if (_fwupdPoll) { clearInterval(_fwupdPoll); _fwupdPoll = null; } }
function startUpdatePolling() {
  stopUpdatePolling();
  _fwupdPoll = setInterval(async () => {
    const s = await fetch('/api/update/status', {cache:'no-store'}).then(r => r.ok ? r.json() : null).catch(() => null);
    if (!s) return;  // device may be rebooting mid-update
    renderUpdateStatus(s);
    if (!s.busy && s.phase !== 'checking' && s.phase !== 'downloading') stopUpdatePolling();
  }, 1500);
}

async function checkFirmwareUpdate() {
  const status = $('fwupd-status'); if (status) status.textContent = 'Checking GitHub…';
  const res = await fetch('/api/update/check', {method:'POST'}).catch(() => null);
  if (!res || !res.ok) { if (status) status.textContent = 'Could not start check (device offline?)'; return; }
  startUpdatePolling();
}

async function applyFirmwareUpdate() {
  if (!confirm('Download and install the latest firmware now? The device will reboot and be offline for 1–2 minutes.')) return;
  const status = $('fwupd-status'); if (status) status.textContent = 'Starting update…';
  const install = $('fwupd-install'); if (install) install.style.display = 'none';
  const res = await fetch('/api/update/apply', {method:'POST'}).catch(() => null);
  if (!res || !res.ok) { if (status) status.textContent = 'Could not start update.'; loadUpdateStatus(); return; }
  startUpdatePolling();
  watchForReboot();
}

async function installSelectedVersion() {
  const sel = $('fwupd-version');
  const v = sel && sel.value;
  if (!v) return;
  if (!confirm(`Install v${v} now? The device will reboot and be offline for 1–2 minutes.`)) return;
  const status = $('fwupd-status'); if (status) status.textContent = `Starting install of v${v}…`;
  const res = await fetch('/api/update/apply', {
    method:'POST', headers:{'Content-Type':'application/json'}, body:JSON.stringify({version:v})
  }).catch(()=>null);
  if (!res || !res.ok) { if (status) status.textContent = 'Could not start install.'; loadUpdateStatus(); return; }
  startUpdatePolling();
  watchForReboot();
}

function watchForReboot() {
  const startedVer = $('fwupd-current')?.textContent;
  let sawOffline = false;
  const timer = setInterval(async () => {
    const st = await fetch('/api/status', {cache:'no-store'}).then(r => r.ok ? r.json() : null).catch(() => null);
    if (!st) { sawOffline = true; return; }  // offline window during reboot
    if (sawOffline && st.firmware && ('v' + st.firmware) !== startedVer) {
      clearInterval(timer);
      stopUpdatePolling();
      const status = $('fwupd-status');
      if (status) status.textContent = `Updated to v${st.firmware}. Reloading…`;
      setTimeout(() => location.reload(), 1500);
    }
  }, 3000);
  setTimeout(() => clearInterval(timer), 240000);  // give up after 4 min
}

if ($('dash-soc') || $('bat-soc-gauge') || $('details')) {
  refreshFull();               // full status once (incl. the static details table)
  setInterval(refresh, 5000);  // then compact live telemetry only
}
if ($('wx-forecast')) {
  refreshWeather();
  setInterval(refreshWeather, 5 * 60 * 1000);
}

// ── Pay records ───────────────────────────────────────────────────────────────
let _payRecords = [];
let _payWorkLabel = 'Working';

function _fmtDate(ts) {
  if (!ts) return '&mdash;';
  const d = new Date(ts * 1000);
  return d.toLocaleDateString([], {month:'numeric',day:'numeric',year:'2-digit'}) + ' ' +
         d.toLocaleTimeString([], {hour:'numeric',minute:'2-digit'});
}
function _fmtAmt(lbl, amt) {
  return `${escAttr(lbl)}${parseFloat(amt).toFixed(2)}`;
}
function _ymd(dt) {
  return `${dt.getFullYear()}-${String(dt.getMonth()+1).padStart(2,'0')}-${String(dt.getDate()).padStart(2,'0')}`;
}
function _ymdhm(dt) {
  return `${_ymd(dt)}T${String(dt.getHours()).padStart(2,'0')}:${String(dt.getMinutes()).padStart(2,'0')}`;
}
function _payWorkHoursLabel() {
  return `${_payWorkLabel || 'Working'} Hours`;
}
function _payWorkPhrase() {
  return String(_payWorkLabel || 'working').toLowerCase();
}
function syncPayWorkLabels() {
  const phrase = _payWorkPhrase();
  const summary = $('pf-start-work-summary');
  const hint = $('pf-start-work-hint');
  const closeHint = $('pc-close-hint');
  if (summary) summary.textContent = `Or start at ${phrase} hours into a day`;
  if (hint) hint.textContent = `Back-date the start to a point inside a day's ${phrase} time - e.g. start after the first 1.5 hours so an earlier session goes to someone else. Overrides the date and time above.`;
  if (closeHint) closeHint.textContent = `Leave blank to close now (all hours so far). Set to N ${phrase} hours of today to split the day - the next period picks up where this ends.`;
}

async function loadPay() {
  const section = $('pay-section');
  const host    = $('pay-list');
  if (!host) return;
  const resp = await fetch('/api/pay',{cache:'no-store'}).then(r=>r.ok?r.json():null).catch(()=>null);
  _payWorkLabel = String(resp?.work_label || 'Working').trim() || 'Working';
  syncPayWorkLabels();
  if (!resp || !resp.track_pay) {
    if (section) section.style.display = 'none';
    return;
  }
  if (section) section.style.display = '';
  _payRecords = resp.records || [];
  if (!_payRecords.length) {
    host.innerHTML = "<div class='empty'>No payees configured &mdash; add one above.</div>";
    return;
  }
  host.innerHTML = _payRecords.map(pr => {
    const workH    = parseFloat(pr.work_h) || 0;
    const earned   = parseFloat(pr.earned) || 0;
    const rate     = parseFloat(pr.rate)   || 0;
    const lbl      = pr.label || '$';
    const startTs  = parseInt(pr.period_start) || 0;
    const earnStr  = _fmtAmt(lbl, earned);
    const workStr  = `${workH.toFixed(1)}\xa0h`;
    const sinceStr = startTs ? _fmtDate(startTs) : 'Not set';
    return `<div class='card' style='margin-bottom:10px'>
      <div class='section-head' style='margin-bottom:8px'>
        <div>
          <span style='font-weight:700;font-size:16px'>${escAttr(pr.payee)}</span>
          <span style='color:var(--muted);font-size:13px;margin-left:10px'>${rate > 0 ? escAttr(lbl) + rate.toFixed(2) + '/h' : 'Rate not set'}</span>
        </div>
        <div style='display:flex;gap:6px'>
          <button onclick='openPayHist(${pr.id})' style='font-size:13px'>History</button>
          <button onclick='openPayForm(${pr.id})' style='font-size:13px'>Edit</button>
          <button onclick='deletePayRecord(${pr.id})' style='color:var(--bad);font-size:13px'>Delete</button>
        </div>
      </div>
      <div class='dash-strip' style='margin:8px 0'>
        <div class='metric'><div class='label'>Period Start</div><div class='value sm'>${sinceStr}</div></div>
        <div class='metric'><div class='label'>${escAttr(_payWorkHoursLabel())}</div><div class='value'>${workStr}</div></div>
        <div class='metric'><div class='label'>Earned</div><div class='value'>${earnStr}</div></div>
      </div>
      ${pr.notes ? `<p style='font-size:12px;color:var(--muted);margin:6px 0 8px'>${escAttr(pr.notes)}</p>` : ''}
      <button class='primary' style='margin-top:4px' onclick='openPayConfirm(${pr.id})'>Mark Paid</button>
    </div>`;
  }).join('');
}

function openPayForm(id) {
  const pr = id ? _payRecords.find(p => p.id == id) : null;
  $('pf-id').value    = pr ? pr.id : '0';
  $('pf-payee').value = pr ? pr.payee : '';
  $('pf-rate').value  = pr ? parseFloat(pr.rate).toFixed(2) : '';
  $('pf-label').value = pr ? pr.label : '$';
  $('pf-notes').value = pr ? pr.notes : '';
  const ts = pr ? parseInt(pr.period_start) : 0;
  $('pf-start').value = _ymdhm(ts ? new Date(ts * 1000) : new Date());
  $('pf-start-day').value = '';
  $('pf-start-hours').value = '';
  $('pf-start-dayinfo').textContent = '';
  $('pay-modal-title').textContent = pr ? `Edit — ${pr.payee}` : 'Add Payee';
  $('pay-modal').style.display = 'flex';
}
function closePayForm() { $('pay-modal').style.display = 'none'; }

async function payStartDayInfo() {
  const day = $('pf-start-day').value;
  const info = $('pf-start-dayinfo');
  if (!day) { info.textContent = ''; return; }
  const d = await fetch(`/api/pay/day?date=${day}`,{cache:'no-store'}).then(r=>r.ok?r.json():null).catch(()=>null);
  info.textContent = d ? `That day logged ${parseFloat(d.total_h).toFixed(1)} ${_payWorkPhrase()} hours. Enter 0 for the start of the day.`
                       : 'No data for that day.';
  if ($('pf-start-hours').value === '') $('pf-start-hours').value = '0';
}

async function submitPayForm() {
  const startVal = $('pf-start').value;
  const startTs  = startVal ? Math.floor(new Date(startVal).getTime() / 1000) : 0;
  const body = {
    id:      parseInt($('pf-id').value) || 0,
    payee:   $('pf-payee').value.trim(),
    rate:    parseFloat($('pf-rate').value) || 0,
    label:   $('pf-label').value.trim() || '$',
    notes:   $('pf-notes').value.trim(),
  };
  // Correction tool: "N working-hours into day D" overrides the date/time start.
  const startDay = $('pf-start-day').value;
  if (startDay) {
    body.start_day   = startDay;
    body.start_hours = parseFloat($('pf-start-hours').value) || 0;
  } else {
    body.period_start = startTs;
  }
  if (!body.payee) { alert('Payee name is required'); return; }
  const res = await fetch('/api/pay', {
    method:'POST', headers:{'Content-Type':'application/json'}, body:JSON.stringify(body)
  }).catch(()=>null);
  if (!res?.ok) { alert('Failed to save payee'); return; }
  closePayForm();
  loadPay();
}

async function deletePayRecord(id) {
  const pr = _payRecords.find(p => p.id == id);
  if (!confirm(`Delete ${pr?.payee || 'this payee'} and all payment history? This cannot be undone.`)) return;
  await fetch(`/api/pay?id=${id}`, {method:'DELETE'});
  loadPay();
}

async function openPayConfirm(id) {
  const pr = _payRecords.find(p => p.id == id);
  if (!pr) return;
  $('pc-id').value          = pr.id;
  $('pc-payee').textContent = pr.payee;
  $('pc-amount').value = '';
  $('pc-notes').value  = '';
  $('pc-close').value  = '';
  $('pc-close').placeholder = 'now (all hours so far)';
  const lbl    = pr.label || '$';
  const workH  = parseFloat(pr.work_h) || 0;
  const earned = parseFloat(pr.earned) || 0;
  const since  = parseInt(pr.period_start) ? _fmtDate(parseInt(pr.period_start)) : '&mdash;';
  $('pay-confirm-summary').innerHTML =
    `<div><b>Period:</b> ${since} &rarr; today</div>` +
    `<div><b>${escAttr(_payWorkHoursLabel())}:</b> ${workH.toFixed(1)}\xa0h</div>` +
    `<div><b>Calculated:</b> ${_fmtAmt(lbl, earned)}</div>`;
  $('pay-confirm-modal').style.display = 'flex';
  // Show today's work total so the split field has a reference maximum.
  const today = _ymd(new Date());
  const d = await fetch(`/api/pay/day?date=${today}`,{cache:'no-store'}).then(r=>r.ok?r.json():null).catch(()=>null);
  if (d) $('pc-close').placeholder = `now — today so far: ${parseFloat(d.total_h).toFixed(1)} h`;
}
function closePayConfirm() { $('pay-confirm-modal').style.display = 'none'; }

async function confirmPay() {
  const id     = parseInt($('pc-id').value);
  const amtRaw = $('pc-amount').value.trim();
  const closeRaw = $('pc-close').value.trim();
  const body   = { id, notes: $('pc-notes').value.trim() };
  if (amtRaw !== '') body.amount = parseFloat(amtRaw);
  if (closeRaw !== '') body.close_hours = parseFloat(closeRaw);
  const res = await fetch('/api/pay/confirm', {
    method:'POST', headers:{'Content-Type':'application/json'}, body:JSON.stringify(body)
  }).catch(()=>null);
  if (!res?.ok) {
    const e = await res?.json().catch(()=>null);
    alert(e?.error === 'no_time'
      ? 'Clock not synced — payment not recorded.'
      : 'Payment confirmation failed.');
    return;
  }
  const d = await res.json();
  closePayConfirm();
  alert(`Recorded: ${parseFloat(d.work_h).toFixed(1)}\xa0h ${_payWorkPhrase()} - period reset.`);
  loadPay();
  loadPayStats();
}

async function openPayHist(id) {
  const pr = _payRecords.find(p => p.id == id);
  if (!pr) return;
  $('pay-hist-title').textContent = `${pr.payee} — Payment History`;
  $('pay-hist-overlay').style.display = 'flex';
  const host = $('pay-hist-list');
  host.innerHTML = "<div class='empty'>Loading&hellip;</div>";
  const hist = await fetch(`/api/pay/history?id=${id}`,{cache:'no-store'}).then(r=>r.ok?r.json():null).catch(()=>null);
  if (!hist?.length) {
    host.innerHTML = "<div class='empty'>No payments recorded yet.</div>";
    return;
  }
  const lbl = pr.label || '$';
  host.innerHTML = [...hist].reverse().map(e => {
    const from = _fmtDate(e.from_ts);
    const to   = _fmtDate(e.to_ts);
    return `<div style='display:flex;justify-content:space-between;align-items:flex-start;padding:10px 0;border-bottom:1px solid var(--line)'>
      <div>
        <div style='font-size:13px;color:var(--muted)'>${from} &rarr; ${to}</div>
        ${e.notes ? `<div style='font-size:12px;color:var(--muted);margin-top:2px'>${escAttr(e.notes)}</div>` : ''}
      </div>
      <div style='text-align:right;flex-shrink:0;margin-left:14px'>
        <div style='font-weight:700'>${_fmtAmt(lbl, e.amount)}</div>
        <div style='font-size:12px;color:var(--muted)'>${parseFloat(e.work_h).toFixed(1)}\xa0h</div>
      </div>
    </div>`;
  }).join('');
}
function closePayHist() { $('pay-hist-overlay').style.display = 'none'; }

// ── Maintenance tracker ───────────────────────────────────────────────────
const MAINT_TYPE_LABELS = {HOURS_ACTIVE:'Active Hours',HOURS_WORKING:'Working Hours',HOURS_TOTAL:'Total Hours',DAYS:'Days'};
const MAINT_TYPE_UNIT   = {HOURS_ACTIVE:'h',HOURS_WORKING:'h',HOURS_TOTAL:'h',DAYS:'d'};
let _maintItems = [];

function renderMachineFields(fields = machineFields) {
  const host = $('machine-fields');
  if (!host) return;
  machineFields = Array.isArray(fields) ? fields : [];
  if (!machineFields.length) {
    host.innerHTML = "<div class='empty'>No custom fields yet. Add part numbers, tire sizes, controller info, charger model, or other project details.</div>";
    return;
  }
  host.innerHTML = machineFields.map((f, i) => `
    <div class='machine-field' style='display:grid;grid-template-columns:minmax(120px,.9fr) minmax(160px,1.4fr) auto;gap:8px;align-items:end'>
      <label>Label<input class='mf-label' maxlength='32' value="${escAttr(f.label)}" autocomplete='off'></label>
      <label>Value<input class='mf-value' maxlength='80' value="${escAttr(f.value)}" autocomplete='off'></label>
      <button type='button' onclick='removeMachineField(${i})' style='color:var(--bad)'>Remove</button>
    </div>
  `).join('');
}

function addMachineField(label = '', value = '') {
  if (machineFields.length >= 12) { alert('Maximum 12 custom fields'); return; }
  machineFields.push({label, value});
  renderMachineFields();
}

function removeMachineField(index) {
  machineFields.splice(index, 1);
  renderMachineFields();
}

function collectMachineFields() {
  return qsa('#machine-fields .machine-field').map(row => ({
    label: row.querySelector('.mf-label')?.value.trim() || '',
    value: row.querySelector('.mf-value')?.value.trim() || '',
  })).filter(f => f.label || f.value).slice(0, 12);
}

async function loadMachineInfo() {
  const form = $('machine-form');
  if (!form) return;
  const data = await fetch('/api/machine-info',{cache:'no-store'}).then(r=>r.json()).catch(()=>null);
  if (!data) return;
  const map = {
    machine_make: data.make,
    machine_model: data.model_number,
    machine_serial: data.serial_number,
    machine_mfg_date: data.manufacture_date,
    gauge_install_date: data.gauge_install_date,
    battery_model: data.battery_model,
    battery_install_date: data.battery_install_date,
    machine_notes: data.notes,
  };
  Object.entries(map).forEach(([name, value]) => {
    const el = form.elements[name];
    if (el) el.value = value || '';
  });
  renderMachineFields(data.fields || []);
}

async function saveMachineInfo(ev) {
  ev.preventDefault();
  const form = ev.currentTarget;
  const status = $('machine-save-status');
  const payload = Object.fromEntries(new FormData(form).entries());
  payload.fields_json = JSON.stringify(collectMachineFields());
  const res = await postForm('/api/machine-info', payload);
  if (!res.ok) {
    const detail = await res.text();
    if (status) status.textContent = detail || 'Save failed';
    return;
  }
  const saved = await res.json().catch(()=>null);
  renderMachineFields(saved?.fields || collectMachineFields());
  if (status) status.textContent = 'Saved.';
}

async function loadMaintHours() {
  const data = await fetch('/api/hours',{cache:'no-store'}).then(r=>r.json()).catch(()=>null);
  const h = data?.hours || {};
  const baseline = parseFloat(h.baseline)||0;
  const counted = parseFloat(h.counted)||0;
  const total = parseFloat(h.total)||Math.max(0, baseline + counted);
  const activeRaw = parseFloat(h.active)||0;
  const workingRaw = parseFloat(h.working)||0;
  const activeLabel = get(data, 'vehicle.active_label', 'Active');
  const workLabel = get(data, 'vehicle.work_label', 'Working');
  const active = Math.min(Math.max(activeRaw, 0), Math.max(total, 0));
  const working = Math.min(Math.max(workingRaw, 0), Math.max(active, 0));
  const bars = $('hours-bars'), legend = $('hours-legend');
  if (!bars || !legend) return;
  if (total < 0.01) {
    bars.innerHTML = '';
    legend.innerHTML = '<span>No hours recorded yet</span>';
    return;
  }
  const fmt = v => `${(Number.isFinite(v) ? v : 0).toFixed(1)}h`;
  const pct = (v, max) => max > 0 ? Math.max(0, Math.min(100, v / max * 100)) : 0;
  const segment = (s, max) => s.val > 0.005
    ? `<div title='${escAttr(s.label)}: ${fmt(s.val)}' style='width:${pct(s.val,max).toFixed(1)}%;background:${s.color};height:100%;transition:width .4s'></div>`
    : '';
  const row = (title, note, max, segs) => `
    <div>
      <div style='display:flex;justify-content:space-between;gap:8px;font-size:13px;margin-bottom:4px'>
        <span>${escAttr(title)}</span><span style='color:var(--muted)'>${escAttr(note)}</span>
      </div>
      <div style='display:flex;height:18px;border-radius:6px;overflow:hidden;background:var(--line)'>
        ${segs.map(s=>segment(s,max)).join('')}
      </div>
    </div>`;
  bars.innerHTML = [
    row('Total displayed hours', `${fmt(total)} = ${fmt(baseline)} install + ${fmt(counted)} tracked`, total, [
      {label:'Original install', val:baseline, color:'var(--muted)'},
      {label:'Tracked since install', val:Math.max(0, total - baseline), color:'var(--primary)'},
    ]),
    row(`${workLabel} / ${activeLabel} / Total`, `${fmt(workingRaw)} ${workLabel.toLowerCase()} / ${fmt(activeRaw)} ${activeLabel.toLowerCase()} / ${fmt(total)} total`, total, [
      {label:workLabel, val:working, color:'var(--bad)'},
      {label:`${activeLabel}, not ${workLabel.toLowerCase()}`, val:Math.max(0, active - working), color:'var(--warn)'},
      {label:'Total, not active', val:Math.max(0, total - active), color:'var(--line)'},
    ]),
  ].join('');
  const warnings = [];
  if (activeRaw > total + 0.01) warnings.push('Active exceeds total; graph capped.');
  if (workingRaw > activeRaw + 0.01) warnings.push('Working exceeds active; graph capped.');
  legend.innerHTML = [
    {label:'Total',val:total,color:'var(--text)'},
    {label:'Install',val:baseline,color:'var(--muted)'},
    {label:'Tracked',val:counted,color:'var(--primary)'},
    {label:activeLabel,val:activeRaw,color:'var(--warn)'},
    {label:workLabel,val:workingRaw,color:'var(--bad)'},
  ].map(s =>
    `<span style='display:inline-flex;align-items:center;gap:4px'><span style='width:10px;height:10px;border-radius:2px;background:${s.color};flex-shrink:0'></span><b>${fmt(s.val)}</b>&nbsp;${escAttr(s.label)}</span>`
  ).join('') + (warnings.length ? `<span style='color:var(--bad)'>${warnings.join(' ')}</span>` : '');
}

async function loadMaintenance() {
  const host = $('maint-list');
  if (!host) return;
  const items = await fetch('/api/maintenance',{cache:'no-store'}).then(r=>r.json()).catch(()=>[]);
  _maintItems = items;
  if (!items.length) { host.innerHTML="<div class='empty'>No maintenance items — add one above.</div>"; return; }
  host.innerHTML = items.map(it => {
    const pct = Math.min(100, parseFloat(it.pct)||0);
    const barColor = pct >= 100 ? 'var(--bad)' : pct >= 75 ? 'var(--warn)' : 'var(--primary)';
    const elapsed = parseFloat(it.elapsed)||0;
    const remaining = parseFloat(it.remaining)||0;
    const unit = MAINT_TYPE_UNIT[it.type]||'h';
    const typeLabel = MAINT_TYPE_LABELS[it.type]||it.type;
    const notStarted = it.type === 'DAYS' && parseInt(it.last_reset_ts) === 0;
    const overdueTag = it.overdue ? "<span style='color:var(--bad);font-weight:700'> OVERDUE</span>" : '';
    return `<div class='card' style='margin-bottom:10px'>
      <div class='section-head' style='margin-bottom:8px'>
        <span style='font-weight:600'>${escAttr(it.name)}${overdueTag}</span>
        <span style='color:var(--muted);font-size:12px'>${typeLabel}</span>
      </div>
      <div style='background:var(--line);border-radius:4px;height:8px;margin-bottom:8px'>
        <div style='background:${barColor};border-radius:4px;height:8px;width:${pct}%;transition:width .4s'></div>
      </div>
      <div style='display:flex;justify-content:space-between;font-size:13px;color:var(--muted);margin-bottom:10px'>
        <span>${notStarted ? 'Not started' : elapsed.toFixed(1)+unit+' elapsed'}</span>
        <span>${notStarted ? 'Every '+parseFloat(it.interval).toFixed(0)+unit : it.overdue ? 'overdue' : remaining.toFixed(1)+unit+' left / '+parseFloat(it.interval).toFixed(0)+unit}</span>
      </div>
      ${it.notes ? `<div style='font-size:12px;color:var(--muted);margin-bottom:8px'>${escAttr(it.notes)}</div>` : ''}
      <div style='display:flex;gap:8px;flex-wrap:wrap'>
        <button onclick='openConfirmModal(${it.id})' class='primary'>Mark Done</button>
        <button onclick='openMaintForm(${it.id})'>Edit</button>
        <button onclick='toggleHistory(${it.id},this)'>&#9658; History</button>
        <button onclick='deleteMaint(${it.id})' style='color:var(--bad)'>Delete</button>
      </div>
      <div id='hist-${it.id}' style='display:none;margin-top:10px'></div>
    </div>`;
  }).join('');
}

function openConfirmModal(id) {
  const it = _maintItems.find(x => x.id == id);
  $('mc-id').value = id;
  $('mc-name').textContent = 'Mark Done — ' + (it ? it.name : '');
  $('mc-notes').value = '';
  $('maint-confirm-modal').style.display = 'flex';
  $('mc-notes').focus();
}
function closeConfirmModal() { $('maint-confirm-modal').style.display = 'none'; }

async function submitConfirmMaint() {
  const id = $('mc-id').value;
  const notes = $('mc-notes').value.trim();
  const res = await postForm('/api/maintenance/confirm', {id, notes});
  if (!res.ok) { alert('Failed to record completion'); return; }
  closeConfirmModal();
  loadMaintenance();
}

async function toggleHistory(id, btn) {
  const panel = $('hist-' + id);
  if (!panel) return;
  if (panel.style.display !== 'none') {
    panel.style.display = 'none';
    btn.innerHTML = '&#9658; History';
    return;
  }
  btn.innerHTML = '&#9660; History';
  panel.style.display = 'block';
  panel.innerHTML = "<div class='empty' style='font-size:13px'>Loading…</div>";
  await loadItemHistory(id, panel);
}

async function loadItemHistory(id, panel) {
  const data = await fetch('/api/maintenance/history?id='+id,{cache:'no-store'}).then(r=>r.json()).catch(()=>null);
  if (!data || !data.entries || !data.entries.length) {
    panel.innerHTML = "<div class='empty' style='font-size:13px'>No history yet — use Mark Done to record a completion.</div>";
    return;
  }
  maintHistoryEntries[id] = data.entries;
  const unit = MAINT_TYPE_UNIT[data.type]||'h';
  const sorted = [...data.entries].sort((a,b) => (parseInt(b.ts)||0) - (parseInt(a.ts)||0));
  panel.innerHTML = "<div style='border-top:1px solid var(--line);padding-top:8px'>" +
    sorted.map(e => {
      const date = e.ts > 0 ? new Date(e.ts*1000).toLocaleString() : 'Unknown date';
      return `<div style='display:flex;justify-content:space-between;align-items:flex-start;gap:8px;padding:6px 0;border-bottom:1px solid var(--line);font-size:13px'>
        <div>
          <div>${date}</div>
          ${e.notes ? `<div style='color:var(--muted);margin-top:2px'>${escAttr(e.notes)}</div>` : ''}
        </div>
        <div style='display:flex;align-items:center;gap:8px;flex-shrink:0'>
          <span style='color:var(--muted);font-size:12px'>${parseFloat(e.val).toFixed(1)}${unit}</span>
          <button onclick='openHistoryEdit(${id},${e.ts})' style='padding:4px 8px;font-size:12px'>Edit</button>
          <button onclick='deleteHistoryEntry(${id},${e.ts})' style='color:var(--bad);padding:4px 8px;font-size:12px'>✕</button>
        </div>
      </div>`;
    }).join('') + "</div>";
}

function datetimeLocalFromTs(ts) {
  const d = new Date((parseInt(ts)||0) * 1000);
  if (!Number.isFinite(d.getTime())) return '';
  const pad = n => String(n).padStart(2, '0');
  return `${d.getFullYear()}-${pad(d.getMonth()+1)}-${pad(d.getDate())}T${pad(d.getHours())}:${pad(d.getMinutes())}`;
}

function epochFromDatetimeLocal(value) {
  const ms = new Date(value).getTime();
  return Number.isFinite(ms) ? Math.floor(ms / 1000) : 0;
}

function openHistoryEdit(itemId, ts) {
  const entry = (maintHistoryEntries[itemId] || []).find(e => parseInt(e.ts) === parseInt(ts));
  if (!entry) { alert('History entry not found'); return; }
  $('mh-id').value = itemId;
  $('mh-original-ts').value = ts;
  $('mh-date').value = datetimeLocalFromTs(ts);
  $('mh-notes').value = entry.notes || '';
  $('maint-history-modal').style.display = 'flex';
  $('mh-date').focus();
}

function closeHistoryModal() { $('maint-history-modal').style.display = 'none'; }

async function submitHistoryEdit() {
  const itemId = $('mh-id').value;
  const ts = epochFromDatetimeLocal($('mh-date').value);
  if (!ts) { alert('Completion date is required'); return; }
  const res = await postForm('/api/maintenance/history/update', {
    id: itemId,
    original_ts: $('mh-original-ts').value,
    ts,
    notes: $('mh-notes').value.trim()
  });
  if (!res.ok) {
    const detail = await res.text();
    alert(detail || 'Failed to update history entry');
    return;
  }
  closeHistoryModal();
  const panel = $('hist-' + itemId);
  if (panel) await loadItemHistory(itemId, panel);
}

async function deleteHistoryEntry(itemId, ts) {
  if (!confirm('Remove this history entry?')) return;
  await postForm('/api/maintenance/history/delete', {id:itemId, ts});
  const panel = $('hist-' + itemId);
  if (panel) await loadItemHistory(itemId, panel);
}

function openMaintForm(idOrNull) {
  const item = typeof idOrNull === 'number'
    ? _maintItems.find(x => x.id == idOrNull) : null;
  $('maint-modal-title').textContent = item ? 'Edit Item' : 'Add Item';
  $('mf-id').value = item ? item.id : 0;
  $('mf-name').value = item ? item.name : '';
  $('mf-type').value = item ? item.type : 'HOURS_ACTIVE';
  $('mf-interval').value = item ? parseFloat(item.interval).toFixed(1) : '100';
  $('mf-notes').value = item ? (item.notes||'') : '';
  updateMaintIntervalLabel();
  $('maint-modal').style.display = 'flex';
}
function closeMaintModal() { $('maint-modal').style.display = 'none'; }

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
  if (!(parseFloat(data.interval) > 0)) { alert('Interval must be > 0'); return; }
  const res = await postForm('/api/maintenance', data);
  if (!res.ok) { alert('Failed to save item'); return; }
  closeMaintModal();
  loadMaintenance();
}

async function deleteMaint(id) {
  if (!confirm('Delete this item and all its history?')) return;
  await postForm('/api/maintenance/delete', {id});
  loadMaintenance();
}

// ── Activity heatmap ──────────────────────────────────────────────────────────
let _hmData = null;       // full API response
let _hmYear  = 0;
let _hmMaint = {};        // { "YYYY-MM-DD": ["Item A", ...] }
const HM_CELL = 11, HM_GAP = 2, HM_PITCH = 13;
const HM_MONTHS = ['Jan','Feb','Mar','Apr','May','Jun','Jul','Aug','Sep','Oct','Nov','Dec'];
const HM_DAYS   = ['Sun','Mon','Tue','Wed','Thu','Fri','Sat'];

async function initHeatmap() {
  const card = $('heatmap-card');
  if (!card) return;
  const raw = await fetch('/api/heatmap', {cache:'no-store'}).then(r => r.ok ? r.json() : null).catch(() => null);
  if (!raw) return;
  _hmData = raw;
  _hmYear = raw.cur_year;
  _hmMaint = {};
  if (Array.isArray(raw.maintenance)) {
    raw.maintenance.forEach(m => {
      if (!Array.isArray(m) || m.length < 2) return;
      const day = String(m[0] || '');
      const name = String(m[1] || '');
      if (!day || !name) return;
      (_hmMaint[day] = _hmMaint[day] || []).push(name);
    });
  } else {
    _hmMaint = raw.maintenance || {};
  }
  if (!raw.maintenance) {
  // Build maint-by-date index from all history
  const items = _maintItems.length ? _maintItems
    : await fetch('/api/maintenance', {cache:'no-store'}).then(r => r.json()).catch(() => []);
  for (const it of items) {  // one at a time — don't flood the single-threaded server
    const h = await fetch('/api/maintenance/history?id='+it.id, {cache:'no-store'}).then(r => r.json()).catch(() => null);
    if (!h || !h.entries) continue;
    for (const e of h.entries) {
      if (!e.ts) continue;
      const key = _ymd(new Date(e.ts * 1000));
      (_hmMaint[key] = _hmMaint[key] || []).push(it.name);
    }
  }
  }
  card.style.display = '';
  renderHeatmap();
}

function hmNav(dir) {
  if (!_hmData) return;
  const years = Object.keys(_hmData.years).map(Number).sort((a,b)=>a-b);
  const idx = years.indexOf(_hmYear) + dir;
  if (idx < 0 || idx >= years.length) return;
  _hmYear = years[idx];
  renderHeatmap();
}

function renderHeatmap() {
  const svg = $('hm-svg');
  if (!svg || !_hmData) return;
  const year  = _hmYear;
  const ydata = (_hmData.years[String(year)] || []);
  const isLeap = (year % 4 === 0 && year % 100 !== 0) || year % 400 === 0;
  const totalDays = isLeap ? 366 : 365;
  const jan1Dow = new Date(year, 0, 1).getDay();
  const numCols = Math.ceil((totalDays + jan1Dow) / 7);
  const LEFT = 26, TOP = 18;
  const W = LEFT + numCols * HM_PITCH + 2;
  const H = TOP + 7 * HM_PITCH + 4;
  const aLbl = escAttr(_hmData.active_label || 'Active');
  const wLbl = escAttr(_hmData.work_label  || 'Working');
  // Color levels: 0=empty, 1=standby, 2=active, 3=working-light, 4=working-heavy
  const OPAS = ['0.28','0.50','0.74','1.0'];  // levels 1-4 (level 0 uses --line)
  let out = `<svg xmlns="http://www.w3.org/2000/svg" width="${W}" height="${H}" style="display:block">`;
  // Day-of-week labels (Mon, Wed, Fri)
  for (let r = 0; r < 7; r++) {
    if (r % 2 === 1) out += `<text x="${LEFT-3}" y="${TOP+r*HM_PITCH+HM_CELL-1}" text-anchor="end" font-size="9" fill="var(--muted)">${HM_DAYS[r]}</text>`;
  }
  // Month labels and cells
  let lastMonth = -1;
  for (let d = 0; d < totalDays; d++) {
    const col = Math.floor((d + jan1Dow) / 7);
    const row = (d + jan1Dow) % 7;
    const x = LEFT + col * HM_PITCH;
    const y = TOP  + row * HM_PITCH;
    const dt = new Date(year, 0, d + 1);
    const mo = dt.getMonth();
    if (mo !== lastMonth) { lastMonth = mo; out += `<text x="${x}" y="${TOP-4}" font-size="9" fill="var(--muted)">${HM_MONTHS[mo]}</text>`; }
    const sta = (ydata[d*4+0]||0)/10, act = (ydata[d*4+1]||0)/10, wrk = (ydata[d*4+2]||0)/10;
    const lvl = wrk>=3?4 : wrk>0?3 : act>0?2 : sta>0?1 : 0;
    const ds = _ymd(dt);
    const cellFill = lvl===0 ? `fill="var(--line)" fill-opacity="1"` : `fill="var(--primary)" fill-opacity="${OPAS[lvl-1]}"`;
    out += `<rect class="hmc" data-d="${ds}" data-s="${sta.toFixed(1)}" data-a="${act.toFixed(1)}" data-w="${wrk.toFixed(1)}" data-al="${aLbl}" data-wl="${wLbl}" x="${x}" y="${y}" width="${HM_CELL}" height="${HM_CELL}" rx="2" ${cellFill}/>`;
  }
  out += '</svg>';
  svg.innerHTML = out;
  // Year nav state
  const $y = $('hm-year'); if ($y) $y.textContent = year;
  const years = Object.keys(_hmData.years).map(Number).sort((a,b)=>a-b);
  const idx = years.indexOf(year);
  const prev = $('hm-prev'); if (prev) prev.disabled = idx <= 0;
  const next = $('hm-next'); if (next) next.disabled = idx >= years.length - 1;
  // Legend swatches
  const sw = $('hm-swatches');
  if (sw) sw.innerHTML =
    `<span style="display:inline-block;width:${HM_CELL}px;height:${HM_CELL}px;border-radius:2px;background:var(--line);vertical-align:middle"></span>` +
    [0.28,0.50,0.74,1.0].map(o=>`<span style="display:inline-block;width:${HM_CELL}px;height:${HM_CELL}px;border-radius:2px;background:var(--primary);opacity:${o};vertical-align:middle"></span>`).join('');
  // Hover
  svg.querySelectorAll('.hmc').forEach(r => {
    r.addEventListener('mouseenter', hmTipShow);
    r.addEventListener('mouseleave', hmTipHide);
  });
}

function hmTipShow(e) {
  const r = e.target, ds = r.dataset.d;
  const sta = parseFloat(r.dataset.s), act = parseFloat(r.dataset.a), wrk = parseFloat(r.dataset.w);
  const aLbl = escAttr(r.dataset.al || 'Active'), wLbl = escAttr(r.dataset.wl || 'Working');
  const dt = new Date(ds + 'T12:00:00');
  const maints = (_hmMaint[ds] || []);
  let html = `<div style="font-weight:600;margin-bottom:6px">${HM_DAYS[dt.getDay()]}, ${HM_MONTHS[dt.getMonth()]} ${dt.getDate()}, ${dt.getFullYear()}</div>`;
  html += `<table style="border-collapse:collapse;font-size:12px;width:100%">`;
  html += `<tr><td style="color:var(--muted);padding:1px 10px 1px 0">Standby</td><td>${sta.toFixed(1)} h</td></tr>`;
  html += `<tr><td style="color:var(--muted);padding:1px 10px 1px 0">${aLbl}</td><td>${(act-wrk).toFixed(1)} h</td></tr>`;
  html += `<tr><td style="color:var(--muted);padding:1px 10px 1px 0">${wLbl}</td><td>${wrk.toFixed(1)} h</td></tr>`;
  if (maints.length) html += `<tr><td colspan="2" style="padding-top:6px;color:var(--primary);font-size:11px">${maints.map(escAttr).join(', ')}</td></tr>`;
  html += '</table>';
  const tip = $('hm-tip');
  tip.innerHTML = html; tip.style.display = 'block';
  const cr = e.target.getBoundingClientRect(), tr = tip.getBoundingClientRect();
  let lx = cr.left + window.scrollX + HM_CELL/2 - tr.width/2;
  let ly = cr.top  + window.scrollY - tr.height - 8;
  if (ly < window.scrollY + 4) ly = cr.bottom + window.scrollY + 8;
  if (lx < 4) lx = 4;
  if (lx + tr.width > window.innerWidth - 4) lx = window.innerWidth - tr.width - 4;
  tip.style.left = lx+'px'; tip.style.top = ly+'px';
}
function hmTipHide() { const t=$('hm-tip'); if(t) t.style.display='none'; }

if ($('maint-list')) {
  $('machine-form')?.addEventListener('submit', saveMachineInfo);
  // Sequential, not parallel: a burst of concurrent fetches can exhaust the
  // synchronous ESP32 web server's connections and wedge it until power-cycle.
  (async () => {
    await loadMaintenance();
    await loadPay();
    await loadMachineInfo();
    await loadMaintHours();
    await initHeatmap();
  })();
}
if ($('dash-pay-wrap')) loadPayStats();
)JS");
}

}  // namespace R48Web
