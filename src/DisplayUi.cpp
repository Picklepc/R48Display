#include "DisplayUi.h"

#include "BoardConfig.h"

#include <lvgl.h>
#include <math.h>
#include <string.h>

namespace R48DisplayUi {

namespace {

constexpr uint16_t WIDTH = DISPLAY_WIDTH;
constexpr uint16_t HEIGHT = DISPLAY_HEIGHT;
constexpr uint32_t LV_HANDLER_MS = 16;

constexpr uint32_t COL_BG = 0x050605;
constexpr uint32_t COL_PANEL = 0x10170D;
constexpr uint32_t COL_PANEL_2 = 0x17220F;
constexpr uint32_t COL_LINE = 0x2A3524;
constexpr uint32_t COL_TEXT = 0xF6FAF0;
constexpr uint32_t COL_MUTED = 0x9CAA92;
constexpr uint32_t COL_DIM = 0x56634E;
constexpr uint32_t COL_PRIMARY = 0xB7F23B;
constexpr uint32_t COL_PRIMARY_DARK = 0x4F7B13;
constexpr uint32_t COL_ACCENT = 0x40E0D0;
constexpr uint32_t COL_WARN = 0xFFD166;
constexpr uint32_t COL_BAD = 0xFF4D4D;

constexpr int16_t CLOCK_CX = 180;
constexpr int16_t CLOCK_CY = 180;

Arduino_GFX *gfx = nullptr;
bool lvReady = false;
uint8_t activePage = 255;
uint32_t lastTickMs = 0;
uint32_t lastHandlerMs = 0;

lv_disp_draw_buf_t drawBuf;
lv_disp_drv_t dispDrv;
static lv_color_t buf1[WIDTH * 24];

struct UiTheme {
  uint32_t bg = COL_BG;
  uint32_t panel = COL_PANEL;
  uint32_t panel2 = COL_PANEL_2;
  uint32_t line = COL_LINE;
  uint32_t text = COL_TEXT;
  uint32_t muted = COL_MUTED;
  uint32_t primary = COL_PRIMARY;
  uint32_t primaryDark = COL_PRIMARY_DARK;
  uint32_t accent = COL_ACCENT;
  uint32_t warn = COL_WARN;
  uint32_t bad = COL_BAD;
};

UiTheme uiTheme;

struct Widgets {
  lv_obj_t *root = nullptr;
  lv_obj_t *arcA = nullptr;
  lv_obj_t *arcB = nullptr;
  lv_obj_t *barA = nullptr;
  lv_obj_t *barB = nullptr;
  lv_obj_t *metric[12]{};
  lv_obj_t *caption[12]{};
  lv_obj_t *status = nullptr;
  lv_obj_t *statusDot = nullptr;
  lv_obj_t *clockTick[60]{};
  lv_point_t clockTickPts[60][2]{};
  lv_obj_t *hourHand = nullptr;
  lv_obj_t *minuteHand = nullptr;
  lv_obj_t *secondHand = nullptr;
  lv_point_t hourPts[2]{};
  lv_point_t minutePts[2]{};
  lv_point_t secondPts[2]{};
};

Widgets w;

uint32_t themed(uint32_t hex) {
  if (hex == COL_BG) return uiTheme.bg;
  if (hex == COL_PANEL) return uiTheme.panel;
  if (hex == COL_PANEL_2) return uiTheme.panel2;
  if (hex == COL_LINE || hex == COL_DIM) return uiTheme.line;
  if (hex == COL_TEXT) return uiTheme.text;
  if (hex == COL_MUTED) return uiTheme.muted;
  if (hex == COL_PRIMARY) return uiTheme.primary;
  if (hex == COL_PRIMARY_DARK) return uiTheme.primaryDark;
  if (hex == COL_ACCENT) return uiTheme.accent;
  if (hex == COL_WARN) return uiTheme.warn;
  if (hex == COL_BAD) return uiTheme.bad;
  return hex;
}

lv_color_t color(uint32_t hex) {
  return lv_color_hex(themed(hex));
}

void applyTheme(const Snapshot &s) {
  uiTheme.bg = s.themeBg;
  uiTheme.panel = s.themePanel;
  uiTheme.panel2 = s.themePanel2;
  uiTheme.line = s.themeLine;
  uiTheme.text = s.themeText;
  uiTheme.muted = s.themeMuted;
  uiTheme.primary = s.themePrimary;
  uiTheme.primaryDark = s.themePrimaryDark;
  uiTheme.accent = s.themeAccent;
  uiTheme.warn = s.themeWarn;
  uiTheme.bad = s.themeBad;
}

String upper(String text) {
  text.toUpperCase();
  return text;
}

String voltsText(float volts) {
  return volts > 0.1f ? String(volts, 1) + "V" : "--";
}

String ampsText(float amps) {
  return String(amps, amps >= 10.0f ? 0 : 1) + "A";
}

String wattsText(float watts) {
  if (watts >= 1000.0f) return String(watts / 1000.0f, 1) + "kW";
  return String(watts, 0) + "W";
}

String ahText(float ah) {
  return ah > 0.1f ? String(ah, ah >= 100.0f ? 0 : 1) + "Ah" : "--";
}

uint32_t statusColor(const Snapshot &s) {
  if (s.chargeAmps > 0.2f) return COL_PRIMARY;
  if (s.dischargeAmps > 1.0f) return COL_ACCENT;
  if (s.bleLink == "online") return COL_PRIMARY_DARK;
  return COL_DIM;
}

void flushCb(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *colorP) {
  if (!gfx) {
    lv_disp_flush_ready(disp);
    return;
  }
  const uint32_t drawW = area->x2 - area->x1 + 1;
  const uint32_t drawH = area->y2 - area->y1 + 1;
  gfx->draw16bitRGBBitmap(area->x1, area->y1, reinterpret_cast<uint16_t *>(&colorP->full), drawW, drawH);
  lv_disp_flush_ready(disp);
}

void clearHandles() {
  w = Widgets{};
}

void styleRoot(lv_obj_t *obj) {
  lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(obj, color(COL_BG), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_all(obj, 0, LV_PART_MAIN);
}

lv_obj_t *label(lv_obj_t *parent, const char *text, int16_t x, int16_t y, int16_t width,
                const lv_font_t *font, uint32_t fg, lv_text_align_t align = LV_TEXT_ALIGN_LEFT) {
  lv_obj_t *obj = lv_label_create(parent);
  lv_obj_set_pos(obj, x, y);
  lv_obj_set_width(obj, width);
  lv_label_set_long_mode(obj, LV_LABEL_LONG_DOT);
  lv_label_set_text(obj, text);
  lv_obj_set_style_text_font(obj, font, LV_PART_MAIN);
  lv_obj_set_style_text_color(obj, color(fg), LV_PART_MAIN);
  lv_obj_set_style_text_align(obj, align, LV_PART_MAIN);
  return obj;
}

lv_obj_t *panel(lv_obj_t *parent, int16_t x, int16_t y, int16_t width, int16_t height,
                uint32_t bg = COL_PANEL, int16_t radius = 8) {
  lv_obj_t *obj = lv_obj_create(parent);
  lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_pos(obj, x, y);
  lv_obj_set_size(obj, width, height);
  lv_obj_set_style_radius(obj, radius, LV_PART_MAIN);
  lv_obj_set_style_bg_color(obj, color(bg), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_color(obj, color(COL_LINE), LV_PART_MAIN);
  lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN);
  lv_obj_set_style_pad_all(obj, 0, LV_PART_MAIN);
  return obj;
}

lv_obj_t *bar(lv_obj_t *parent, int16_t x, int16_t y, int16_t width, int16_t height,
              uint32_t fill = COL_PRIMARY) {
  lv_obj_t *obj = lv_bar_create(parent);
  lv_obj_set_pos(obj, x, y);
  lv_obj_set_size(obj, width, height);
  lv_bar_set_range(obj, 0, 100);
  lv_obj_set_style_radius(obj, height / 2, LV_PART_MAIN);
  lv_obj_set_style_bg_color(obj, color(COL_LINE), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_radius(obj, height / 2, LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(obj, color(fill), LV_PART_INDICATOR);
  return obj;
}

lv_obj_t *ring(lv_obj_t *parent, int16_t x, int16_t y, int16_t size, int16_t width,
               uint32_t fill = COL_PRIMARY, int16_t start = 135, int16_t end = 45) {
  lv_obj_t *obj = lv_arc_create(parent);
  lv_obj_set_pos(obj, x, y);
  lv_obj_set_size(obj, size, size);
  lv_arc_set_range(obj, 0, 100);
  lv_arc_set_bg_angles(obj, start, end);
  lv_obj_remove_style(obj, nullptr, LV_PART_KNOB);
  lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_style_arc_width(obj, width, LV_PART_MAIN);
  lv_obj_set_style_arc_width(obj, width, LV_PART_INDICATOR);
  lv_obj_set_style_arc_color(obj, color(COL_LINE), LV_PART_MAIN);
  lv_obj_set_style_arc_color(obj, color(fill), LV_PART_INDICATOR);
  return obj;
}

lv_obj_t *line(lv_obj_t *parent, lv_point_t *points, uint8_t count, uint32_t fg, uint8_t width) {
  lv_obj_t *obj = lv_line_create(parent);
  lv_obj_set_pos(obj, 0, 0);
  lv_line_set_points(obj, points, count);
  lv_obj_set_style_line_color(obj, color(fg), 0);
  lv_obj_set_style_line_width(obj, width, 0);
  lv_obj_set_style_line_rounded(obj, true, 0);
  return obj;
}

void metric(uint8_t index, const char *cap, int16_t x, int16_t y, int16_t width,
            const lv_font_t *valueFont = &lv_font_montserrat_20,
            lv_text_align_t align = LV_TEXT_ALIGN_CENTER) {
  if (index >= 12) return;
  w.caption[index] = label(w.root, cap, x, y, width, &lv_font_montserrat_12, COL_MUTED, align);
  w.metric[index] = label(w.root, "--", x, y + 14, width, valueFont, COL_TEXT, align);
}

void setText(lv_obj_t *obj, const char *text) {
  if (!obj) return;
  const char *current = lv_label_get_text(obj);
  const char *next = text ? text : "";
  if (!current || strcmp(current, next) != 0) lv_label_set_text(obj, next);
}

void setText(lv_obj_t *obj, const String &text) {
  setText(obj, text.c_str());
}

void setTextColor(lv_obj_t *obj, uint32_t fg) {
  if (obj) lv_obj_set_style_text_color(obj, color(fg), LV_PART_MAIN);
}

void setBgColor(lv_obj_t *obj, uint32_t bg) {
  if (obj) lv_obj_set_style_bg_color(obj, color(bg), LV_PART_MAIN);
}

void setBar(lv_obj_t *obj, float value) {
  if (!obj) return;
  const int16_t next = static_cast<int16_t>(constrain(value, 0.0f, 100.0f));
  if (lv_bar_get_value(obj) != next) lv_bar_set_value(obj, next, LV_ANIM_ON);
}

void setRing(lv_obj_t *obj, float value) {
  if (!obj) return;
  const int16_t next = static_cast<int16_t>(constrain(value, 0.0f, 100.0f));
  if (lv_arc_get_value(obj) != next) lv_arc_set_value(obj, next);
}

void setRingColor(lv_obj_t *obj, uint32_t fg) {
  if (obj) lv_obj_set_style_arc_color(obj, color(fg), LV_PART_INDICATOR);
}

void buildDashboard() {
  w.arcA = ring(w.root, 17, 17, 326, 14, COL_PRIMARY);
  w.arcB = ring(w.root, 44, 44, 272, 8, COL_ACCENT);

  w.metric[0] = label(w.root, "--", 76, 105, 208, &lv_font_montserrat_48, COL_TEXT, LV_TEXT_ALIGN_CENTER);
  w.metric[1] = label(w.root, "--", 92, 154, 176, &lv_font_montserrat_20, COL_MUTED, LV_TEXT_ALIGN_CENTER);

  w.status = panel(w.root, 102, 185, 156, 34, COL_PANEL_2, 16);
  w.statusDot = lv_obj_create(w.status);
  lv_obj_clear_flag(w.statusDot, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_pos(w.statusDot, 13, 12);
  lv_obj_set_size(w.statusDot, 10, 10);
  lv_obj_set_style_radius(w.statusDot, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_border_width(w.statusDot, 0, 0);
  w.metric[2] = label(w.status, "--", 30, 8, 112, &lv_font_montserrat_16, COL_PRIMARY, LV_TEXT_ALIGN_CENTER);

  // Row 1: ETA centered and wider
  metric(4, "ETA", 96, 242, 168, &lv_font_montserrat_18);
  // Row 2: HOURS left, LOAD right
  metric(5, "HOURS", 58, 287, 88, &lv_font_montserrat_18);
  metric(3, "LOAD", 214, 287, 88, &lv_font_montserrat_18);
  // Row 3: combined power watts, centered
  metric(6, "WATTS", 104, 323, 152, &lv_font_montserrat_16);
}

void buildPower() {
  w.arcA = ring(w.root, 38, 28, 284, 16, COL_ACCENT);
  w.metric[0] = label(w.root, "--", 75, 114, 210, &lv_font_montserrat_40, COL_TEXT, LV_TEXT_ALIGN_CENTER);
  w.metric[1] = label(w.root, "--", 84, 157, 192, &lv_font_montserrat_20, COL_MUTED, LV_TEXT_ALIGN_CENTER);
  metric(2, "CELLS", 14, 221, 88, &lv_font_montserrat_18);
  metric(3, "SPREAD", 136, 221, 88, &lv_font_montserrat_18);
  metric(4, "TEMP", 258, 221, 88, &lv_font_montserrat_18);
  metric(5, "CAPACITY", 88, 279, 184, &lv_font_montserrat_18);
  metric(6, "HOURS", 88, 324, 184, &lv_font_montserrat_14);
}

void buildClock() {
  w.arcA = ring(w.root, 20, 20, 320, 6, COL_PRIMARY, 0, 360);
  lv_arc_set_value(w.arcA, 100);
  w.arcB = ring(w.root, 42, 42, 276, 2, COL_ACCENT, 0, 360);
  lv_arc_set_value(w.arcB, 100);

  for (uint8_t i = 0; i < 60; ++i) {
    const float a = (i * 6.0f - 90.0f) * DEG_TO_RAD;
    const bool major = i % 5 == 0;
    const int16_t outer = major ? 146 : 141;
    const int16_t inner = major ? 128 : 136;
    w.clockTickPts[i][0] = {static_cast<lv_coord_t>(CLOCK_CX + cosf(a) * inner),
                            static_cast<lv_coord_t>(CLOCK_CY + sinf(a) * inner)};
    w.clockTickPts[i][1] = {static_cast<lv_coord_t>(CLOCK_CX + cosf(a) * outer),
                            static_cast<lv_coord_t>(CLOCK_CY + sinf(a) * outer)};
    w.clockTick[i] = line(w.root, w.clockTickPts[i], 2, major ? COL_TEXT : COL_DIM, major ? 3 : 1);
  }

  w.hourPts[0] = {CLOCK_CX, CLOCK_CY};
  w.hourPts[1] = {CLOCK_CX, static_cast<lv_coord_t>(CLOCK_CY - 54)};
  w.minutePts[0] = {CLOCK_CX, CLOCK_CY};
  w.minutePts[1] = {CLOCK_CX, static_cast<lv_coord_t>(CLOCK_CY - 84)};
  w.secondPts[0] = {CLOCK_CX, CLOCK_CY};
  w.secondPts[1] = {CLOCK_CX, static_cast<lv_coord_t>(CLOCK_CY - 105)};
  w.hourHand = line(w.root, w.hourPts, 2, COL_TEXT, 7);
  w.minuteHand = line(w.root, w.minutePts, 2, COL_PRIMARY, 5);
  w.secondHand = line(w.root, w.secondPts, 2, COL_BAD, 2);

  lv_obj_t *hub = lv_obj_create(w.root);
  lv_obj_clear_flag(hub, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_pos(hub, CLOCK_CX - 8, CLOCK_CY - 8);
  lv_obj_set_size(hub, 16, 16);
  lv_obj_set_style_radius(hub, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(hub, color(COL_TEXT), 0);
  lv_obj_set_style_border_width(hub, 0, 0);

  w.metric[0] = label(w.root, "--", 83, 96, 194, &lv_font_montserrat_20, COL_PRIMARY, LV_TEXT_ALIGN_CENTER);
  w.metric[1] = label(w.root, "--", 86, 244, 188, &lv_font_montserrat_16, COL_MUTED, LV_TEXT_ALIGN_CENTER);
  w.metric[2] = label(w.root, "--", 60, 286, 240, &lv_font_montserrat_14, COL_ACCENT, LV_TEXT_ALIGN_CENTER);
}

void buildStatus() {
  metric(0, "WIFI", 35, 28, 290, &lv_font_montserrat_18);
  metric(1, "IP", 35, 74, 290, &lv_font_montserrat_18);
  metric(2, "BLE", 35, 120, 290, &lv_font_montserrat_18);
  metric(3, "BMS", 35, 166, 290, &lv_font_montserrat_14);
  metric(4, "BOARD", 35, 211, 135, &lv_font_montserrat_16);
  metric(5, "PWR", 190, 211, 135, &lv_font_montserrat_16);
  metric(6, "FW", 35, 263, 135, &lv_font_montserrat_16);
  metric(7, "UPTIME", 190, 263, 135, &lv_font_montserrat_16);
  w.metric[8] = label(w.root, "--", 24, 324, 312, &lv_font_montserrat_12, COL_WARN, LV_TEXT_ALIGN_CENTER);
}

void buildPage(uint8_t page) {
  if (!lvReady) return;
  lv_obj_t *screen = lv_scr_act();
  lv_obj_clean(screen);
  clearHandles();
  w.root = screen;
  styleRoot(w.root);
  switch (page % PAGE_COUNT) {
    case 0: buildDashboard(); break;
    case 1: buildPower(); break;
    case 2: buildClock(); break;
    default: buildStatus(); break;
  }
  activePage = page % PAGE_COUNT;
}

String clockDateText(const String &localTime) {
  if (localTime.length() < 10 || localTime == "--") return "NTP";
  return localTime.substring(0, 10);
}

String clockTimeText(const Snapshot &s) {
  const String &localTime = s.localTime;
  if (localTime.length() < 16 || localTime == "--") return "--:--";
  if (s.use24h) return localTime.substring(11, 16);
  int hour = localTime.substring(11, 13).toInt();
  const int minute = localTime.substring(14, 16).toInt();
  const bool pm = hour >= 12;
  hour %= 12;
  if (hour == 0) hour = 12;
  char buf[12];
  snprintf(buf, sizeof(buf), "%d:%02d %s", hour, minute, pm ? "PM" : "AM");
  return String(buf);
}

void updateClock(const Snapshot &s) {
  const String localTime = s.localTime;
  if (localTime.length() >= 19 && localTime != "--") {
    const int hour = localTime.substring(11, 13).toInt();
    const int minute = localTime.substring(14, 16).toInt();
    const int second = localTime.substring(17, 19).toInt();
    const float hourA = (((hour % 12) + minute / 60.0f) * 30.0f - 90.0f) * DEG_TO_RAD;
    const float minA = ((minute + second / 60.0f) * 6.0f - 90.0f) * DEG_TO_RAD;
    const float secA = (second * 6.0f - 90.0f) * DEG_TO_RAD;
    w.hourPts[0] = {CLOCK_CX, CLOCK_CY};
    w.hourPts[1] = {static_cast<lv_coord_t>(CLOCK_CX + cosf(hourA) * 58),
                    static_cast<lv_coord_t>(CLOCK_CY + sinf(hourA) * 58)};
    w.minutePts[0] = {CLOCK_CX, CLOCK_CY};
    w.minutePts[1] = {static_cast<lv_coord_t>(CLOCK_CX + cosf(minA) * 88),
                      static_cast<lv_coord_t>(CLOCK_CY + sinf(minA) * 88)};
    w.secondPts[0] = {CLOCK_CX, CLOCK_CY};
    w.secondPts[1] = {static_cast<lv_coord_t>(CLOCK_CX + cosf(secA) * 105),
                      static_cast<lv_coord_t>(CLOCK_CY + sinf(secA) * 105)};
    lv_line_set_points(w.hourHand, w.hourPts, 2);
    lv_line_set_points(w.minuteHand, w.minutePts, 2);
    lv_line_set_points(w.secondHand, w.secondPts, 2);
  }
  setText(w.metric[0], clockTimeText(s));
  setText(w.metric[1], clockDateText(localTime));
  setText(w.metric[2], upper(s.mode.length() ? s.mode : "standby"));
}

void updateDashboard(const Snapshot &s) {
  const bool charging = s.chargeAmps > 0.2f;
  const float powerPercent = charging ? constrain(s.chargeAmps * 4.0f, 0.0f, 100.0f) : s.loadPercent;
  const uint32_t powerColor = charging ? COL_PRIMARY : COL_ACCENT;
  setRing(w.arcA, s.socValid ? s.soc : 0);
  setRingColor(w.arcA, s.socValid ? COL_PRIMARY : COL_DIM);
  setRing(w.arcB, powerPercent);
  setRingColor(w.arcB, powerColor);
  setText(w.metric[0], s.socValid ? String(s.soc) + "%" : "--");
  setText(w.metric[1], voltsText(s.packVoltage));
  setText(w.metric[2], upper(s.mode.length() ? s.mode : "standby"));
  setTextColor(w.metric[2], statusColor(s));
  setBgColor(w.statusDot, statusColor(s));
  setText(w.metric[3], charging ? ampsText(s.chargeAmps) + " in" : String(s.loadPercent, 0) + "%");
  setText(w.metric[4], charging ? s.chargeEta : s.runEta);
  setText(w.metric[5], s.runtimeHours);
  const float activeWatts = charging ? s.chargeWatts : s.dischargeWatts;
  setText(w.metric[6], activeWatts > 1.0f ? wattsText(activeWatts) : "--");
  setTextColor(w.metric[6], charging ? COL_ACCENT : COL_TEXT);
}

void updatePower(const Snapshot &s) {
  const float health = s.healthPercent > 0.0f ? s.healthPercent : 0.0f;
  setRing(w.arcA, health);
  setRingColor(w.arcA, health >= 80.0f ? COL_PRIMARY : health >= 60.0f ? COL_WARN : COL_BAD);
  setText(w.metric[0], health > 0.0f ? String(health, 0) + "%" : "--");
  setText(w.metric[1], "battery health");
  setText(w.metric[2], s.cellCount ? String(s.cellCount) : "--");
  setText(w.metric[3], s.deltaCellMv > 0.0f ? String(s.deltaCellMv, 0) + "mV" : "--");
  setText(w.metric[4], s.temp);
  setText(w.metric[5], ahText(s.remainingAh) + " / " + ahText(s.totalAh));
  setText(w.metric[6], s.hoursStr);
}

void updateStatus(const Snapshot &s) {
  setText(w.metric[0], s.wifiConnected ? s.ssid + " " + String(s.wifiRssi) + "dBm" : s.provisioning ? s.ssid : "offline");
  setText(w.metric[1], s.ip.length() ? s.ip : "--");
  setText(w.metric[2], s.bleLink + " " + String(s.bmsRssi) + "dBm");
  setText(w.metric[3], s.bmsTarget.length() ? s.bmsTarget : s.bmsAddress);
  setText(w.metric[4], s.screenBattery);
  setText(w.metric[5], s.powerSource);
  setText(w.metric[6], s.firmware);
  setText(w.metric[7], s.uptime);
  if (s.maintOverdue > 0) {
    String m = String(s.maintOverdue) + " maintenance item";
    if (s.maintOverdue > 1) m += "s";
    m += " due";
    setText(w.metric[8], m);
  } else if (s.mqttStatus.length() && s.mqttStatus != "MQTT connected") {
    setText(w.metric[8], s.mqttStatus);
  } else {
    setText(w.metric[8], s.lastError.length() ? s.lastError : s.bmsStatus);
  }
}

void updateRows(const Snapshot &s) {
  switch (activePage) {
    case 0: updateDashboard(s); break;
    case 1: updatePower(s); break;
    case 2: updateClock(s); break;
    default: updateStatus(s); break;
  }
}

}  // namespace

void begin(Arduino_GFX *display) {
  gfx = display;
  if (lvReady) return;
  lv_init();
  lv_disp_draw_buf_init(&drawBuf, buf1, nullptr, WIDTH * 24);
  lv_disp_drv_init(&dispDrv);
  dispDrv.hor_res = WIDTH;
  dispDrv.ver_res = HEIGHT;
  dispDrv.draw_buf = &drawBuf;
  dispDrv.flush_cb = flushCb;
  lv_disp_drv_register(&dispDrv);
  lvReady = true;
  activePage = 255;
  lastTickMs = millis();
}

void tick(bool sleeping) {
  if (!lvReady) return;
  const uint32_t now = millis();
  const uint32_t delta = now - lastTickMs;
  if (delta > 0) {
    lv_tick_inc(delta);
    lastTickMs = now;
  }
  const uint32_t interval = sleeping ? 500u : LV_HANDLER_MS;
  if (now - lastHandlerMs >= interval) {
    lv_timer_handler();
    lastHandlerMs = now;
  }
}

void draw(const Snapshot &s) {
  if (!lvReady) return;
  applyTheme(s);
  const uint8_t page = s.page % PAGE_COUNT;
  if (s.fullRedraw || activePage != page) buildPage(page);
  updateRows(s);
  lv_timer_handler();
}

}  // namespace R48DisplayUi
