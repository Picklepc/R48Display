#include "AnimLayer.h"
#include "BoardConfig.h"
#include <math.h>

namespace {

// ─── Particle pool ──────────────────────────────────────────────────────────
constexpr uint8_t POOL = 64;
constexpr int16_t W = DISPLAY_WIDTH;
constexpr int16_t H = DISPLAY_HEIGHT;

struct Dot {
  lv_obj_t *o    = nullptr;
  float x = 0,  y = 0;
  float vx = 0, vy = 0;
  float life = 0, maxLife = 1;
  uint32_t col = 0;
  uint8_t  sz  = 3;
  bool wrap = false;
  bool on   = false;
};

static Dot     pool[POOL];
static uint8_t curType    = 0;
static bool    curEnabled = false;
static uint32_t lastMs    = 0;
static uint32_t nextEvtMs = 0;
static uint32_t phase     = 0;
static uint32_t rng       = 0xDEADBEEFu;

// ─── Helpers ────────────────────────────────────────────────────────────────
static uint32_t r32() {
  rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5; return rng;
}
static float rf(float lo, float hi)    { return lo + (float)(r32() & 0xFFFF) * (hi - lo) / 65535.0f; }
static float rc(float centre, float r) { return rf(centre - r, centre + r); }

template<typename T> static T clamp(T v, T lo, T hi) { return v < lo ? lo : v > hi ? hi : v; }

// ─── Pool ops ───────────────────────────────────────────────────────────────
static Dot *alloc() {
  for (auto &d : pool) if (!d.on && d.o) { d.on = true; return &d; }
  return nullptr;
}

static void release(Dot &d) {
  d.on = false;
  if (d.o) lv_obj_add_flag(d.o, LV_OBJ_FLAG_HIDDEN);
}

static void spawn(float x, float y, float vx, float vy,
                  float life, uint32_t col, uint8_t sz, bool wrap = false) {
  Dot *d = alloc();
  if (!d) return;
  d->x = x; d->y = y; d->vx = vx; d->vy = vy;
  d->life = d->maxLife = life;
  d->col = col; d->sz = sz; d->wrap = wrap;
  lv_obj_clear_flag(d->o, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_pos(d->o, (int16_t)x, (int16_t)y);
  lv_obj_set_size(d->o, sz, sz);
  lv_obj_set_style_radius(d->o, sz / 2, 0);
  lv_obj_set_style_bg_color(d->o, lv_color_hex(col), 0);
  lv_obj_set_style_bg_opa(d->o, LV_OPA_COVER, 0);
}

// ─── Static flag ('Murica) ──────────────────────────────────────────────────
static void buildFlag(lv_obj_t *root) {
  constexpr int16_t FW = 80, FH = 52, SH = FH / 13;
  constexpr int16_t FX = W - FW - 10, FY = H - FH - 10;
  constexpr uint32_t sc[13] = {
    0xFF0000,0xFFFFFF,0xFF0000,0xFFFFFF,0xFF0000,0xFFFFFF,0xFF0000,
    0xFFFFFF,0xFF0000,0xFFFFFF,0xFF0000,0xFFFFFF,0xFF0000
  };
  auto mkobj = [&](int16_t x, int16_t y, int16_t w, int16_t h, uint32_t col, uint8_t opa, uint8_t r) {
    lv_obj_t *o = lv_obj_create(root);
    lv_obj_clear_flag(o, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_border_width(o, 0, 0);
    lv_obj_set_style_pad_all(o, 0, 0);
    lv_obj_set_style_radius(o, r, 0);
    lv_obj_set_pos(o, x, y);
    lv_obj_set_size(o, w, h);
    lv_obj_set_style_bg_color(o, lv_color_hex(col), 0);
    lv_obj_set_style_bg_opa(o, opa, 0);
  };
  for (uint8_t i = 0; i < 13; ++i)
    mkobj(FX, FY + i * SH, FW, i == 12 ? FH - 12 * SH : SH, sc[i], 150, 0);
  // Canton
  constexpr int16_t CW = FW * 4 / 10, CH = SH * 7;
  mkobj(FX, FY, CW, CH, 0x0A3161, 200, 0);
  // Stars (4×3 grid)
  constexpr int16_t SX0 = FX + 3, SY0 = FY + 3;
  constexpr int16_t SGX = (CW - 6) / 3, SGY = (CH - 6) / 2;
  for (uint8_t i = 0; i < 12; ++i)
    mkobj(SX0 + (i % 4) * SGX, SY0 + (i / 4) * SGY, 2, 2, 0xFFFFFF, 255, LV_RADIUS_CIRCLE);
}

// ─── Spawners — each returns ms until next event ─────────────────────────────
static uint32_t evtStars() {
  uint8_t n = 1 + r32() % 3;
  for (uint8_t i = 0; i < n; ++i)
    spawn(rf(10, W-10), rf(10, H-10), 0, 0,
          rf(1.2f, 3.2f), r32() % 4 == 0 ? 0xAADDFF : 0xFFFFFF, 1 + r32() % 2);
  return 2500 + r32() % 5000;
}

static uint32_t evtEmbers() {
  constexpr uint32_t p[] = {0xFF6600,0xFF9900,0xFFCC00,0xFF3300,0xFF5500};
  uint8_t n = 2 + r32() % 3;
  for (uint8_t i = 0; i < n; ++i)
    spawn(rf(80,280), rf(240,310), rc(0,25), rf(-90,-28),
          rf(1.8f,3.5f), p[r32()%5], 2+r32()%2);
  return 380 + r32() % 560;
}

static uint32_t evtLightning() {
  constexpr uint32_t p[] = {0xCCAAFF,0xFFFFFF,0x8844FF,0xFF88FF,0x44CCFF};
  float bx = rf(40,320);
  uint8_t n = 5 + r32() % 6;
  for (uint8_t i = 0; i < n; ++i)
    spawn(bx + rc(0,18), rf(0,100), rc(0,20), rf(60,230),
          rf(0.12f,0.45f), p[r32()%5], 2+r32()%2);
  return 2800 + r32() % 6000;
}

static uint32_t evtRipple() {
  constexpr uint32_t p[] = {0x4499FF,0x0066CC,0x00AAFF,0x88DDFF,0x7CFFCB};
  float cx = rf(60,300), cy = rf(60,300);
  uint8_t n = 8 + r32() % 4;
  for (uint8_t i = 0; i < n; ++i) {
    float a = (float)i * 6.283f / n;
    float spd = rf(28,72);
    spawn(cx, cy, cosf(a)*spd, sinf(a)*spd, rf(2.0f,4.0f), p[r32()%5], 2);
  }
  return 5000 + r32() % 8000;
}

static uint32_t evtSpeedLines() {
  constexpr uint32_t p[] = {0xFF1744,0xFF8888,0xFF5566,0xFFAABB};
  uint8_t n = 2 + r32() % 3;
  for (uint8_t i = 0; i < n; ++i) {
    float y = rf(30,330);
    float spd = rf(260,390);
    float life = rf(0.28f,0.65f);
    uint32_t col = p[r32()%4];
    spawn(W+18, y, -spd,       0, life,        col, 3);
    spawn(W+10, y, -spd, rc(0,3), life*0.75f,  col, 2);
    spawn(W+ 4, y, -spd, rc(0,3), life*0.45f,  col, 2);
  }
  return 650 + r32() % 1050;
}

static uint32_t evtRave() {
  constexpr uint32_t p[] = {
    0xFF00FF,0x00FFFF,0xFF0099,0x99FF00,
    0xFF4400,0x0044FF,0xFFFF00,0xFF88CC,
    0x00FF88,0xFF0044,0x44FF00,0xFFAA00
  };
  uint8_t n = 4 + r32() % 8;
  for (uint8_t i = 0; i < n; ++i) {
    float x, y, vx, vy;
    switch (r32() % 4) {
      case 0: x=0;   y=rf(0,H); vx=rf(90,250);  vy=rc(0,65); break;
      case 1: x=W;   y=rf(0,H); vx=rf(-250,-90); vy=rc(0,65); break;
      case 2: x=rf(0,W); y=0;   vy=rf(90,250);  vx=rc(0,65); break;
      default:x=rf(0,W); y=H;   vy=rf(-250,-90); vx=rc(0,65); break;
    }
    spawn(x, y, vx, vy, rf(0.5f,1.4f), p[r32()%12], 3+r32()%4, true);
  }
  return 55 + r32() % 80;
}

static uint32_t evtPixelRain() {
  constexpr uint32_t p[] = {0xD6D93B,0xF2D16B,0xCCBB22,0xF28C28,0xEEEE00};
  uint8_t n = 2 + r32() % 4;
  for (uint8_t i = 0; i < n; ++i)
    spawn(rf(5,W-5), -3, 0, rf(65,145), rf(2.5f,5.0f), p[r32()%5], 2);
  return 160 + r32() % 270;
}

static uint32_t evtGeometry() {
  constexpr uint32_t p[] = {0x79FFE1,0xFFD166,0xE5F0FF,0x7D8EA3,0xAABBC8};
  float a = rf(0,6.283f), spd = rf(10,32);
  spawn(rf(20,W-20), rf(20,H-20), cosf(a)*spd, sinf(a)*spd,
        rf(4.5f,9.0f), p[r32()%5], 3+r32()%4);
  return 1700 + r32() % 2800;
}

static void burstFireworks(float bx, float by) {
  constexpr uint32_t palMurica[] = {0xFF1818,0xFFFFFF,0x4499FF,0xFFCC00,0xFF88FF};
  constexpr uint32_t palNY[]     = {0xFFD700,0xFFFFFF,0xC0C0C0,0xFF6633,0xFF4444};
  const uint32_t *pal = (curType == Anim::FireworksFlag) ? palMurica : palNY;
  uint32_t bc = pal[r32()%5];
  uint8_t  n  = 22 + r32() % 12;
  for (uint8_t i = 0; i < n; ++i) {
    float a = rf(0,6.283f), spd = rf(50,145);
    spawn(bx, by, cosf(a)*spd, sinf(a)*spd, rf(1.0f,2.5f), bc, 2+r32()%2);
  }
  uint8_t ns = 8 + r32() % 6;
  for (uint8_t i = 0; i < ns; ++i) {
    float a = rf(0,6.283f), spd = rf(18,55);
    spawn(bx, by, cosf(a)*spd, sinf(a)*spd, rf(0.5f,1.1f), 0xFFFFFF, 2);
  }
}

static uint32_t evtFireworks(bool hasFlag) {
  float bx = rf(40,320);
  float by = hasFlag ? rf(20,185) : rf(20,280);
  burstFireworks(bx, by);
  return 4500 + r32() % 9000;
}

static uint32_t evtBats() {
  constexpr uint32_t p[] = {0x330055,0x220033,0x440066,0x1A0033};
  uint8_t n = 1 + r32() % 2;
  for (uint8_t i = 0; i < n; ++i) {
    bool left = r32() % 2;
    float x = left ? -6.0f : (float)(W+6);
    float vx = left ? rf(55,115) : rf(-115,-55);
    spawn(x, rf(40,260), vx, rc(0,14), rf(3.5f,5.5f), p[r32()%4], 5+r32()%3);
  }
  return 3500 + r32() % 7000;
}

static uint32_t evtSnow() {
  constexpr uint32_t p[] = {0xEEF4FF,0xFFFFFF,0xCCDDFF,0xDDEEFF};
  uint8_t n = 2 + r32() % 3;
  for (uint8_t i = 0; i < n; ++i)
    spawn(rf(5,W-5), rf(-8,0), rc(0,18), rf(26,65), rf(4.5f,8.5f), p[r32()%4], 2+r32()%3);
  return 210 + r32() % 360;
}

static uint32_t evtLeaves() {
  constexpr uint32_t p[] = {0xFF6600,0xCC3300,0xFFAA00,0xDD5500,0xFF8800,0xBB4400};
  uint8_t n = 1 + r32() % 2;
  for (uint8_t i = 0; i < n; ++i)
    spawn(rf(5,W-5), rf(-12,0), rc(0,38), rf(38,90), rf(3.5f,6.5f), p[r32()%6], 4+r32()%3);
  return 520 + r32() % 980;
}

static uint32_t evtHearts() {
  constexpr uint32_t p[] = {0xFF1493,0xFF69B4,0xFF0055,0xFF88CC,0xFF3366};
  uint8_t n = 1 + r32() % 2;
  for (uint8_t i = 0; i < n; ++i)
    spawn(rf(40,W-40), H+4, rc(0,20), rf(-68,-26), rf(4.0f,7.0f), p[r32()%5], 5+r32()%3);
  return 1100 + r32() % 2100;
}

static uint32_t evtGrass() {
  constexpr uint32_t p[] = {0x88CC00,0xAAFF00,0x66BB00,0xCCFF00,0x99DD00};
  float cx = rf(60,300);
  uint8_t n = 5 + r32() % 6;
  for (uint8_t i = 0; i < n; ++i)
    spawn(cx + rc(0,45), rf(200,280), rc(0,80), rf(-125,-28),
          rf(0.8f,2.2f), p[r32()%5], 2+r32()%2);
  return 2800 + r32() % 5000;
}

// ─── Physics step ───────────────────────────────────────────────────────────
static void stepDots(float dt) {
  const bool isRave = curType == Anim::Rave;

  for (auto &d : pool) {
    if (!d.on) continue;
    d.life -= dt;
    if (d.life <= 0) { release(d); continue; }
    d.x += d.vx * dt;
    d.y += d.vy * dt;

    switch (curType) {
      case Anim::FireworksFlag:
      case Anim::Fireworks:
        d.vy += 55.0f * dt; d.vx *= 0.985f; break;
      case Anim::Snow:
        d.vy  = clamp(d.vy + 4.0f * dt, 8.0f, 95.0f);
        d.vx += rc(0, 6.0f) * dt; break;
      case Anim::Leaves:
        d.vy  = clamp(d.vy + 6.0f * dt, 14.0f, 115.0f);
        d.vx += rc(0, 12.0f) * dt; break;
      case Anim::Hearts:
        d.vx += rc(0, 5.0f) * dt; break;
      case Anim::Embers:
      case Anim::Grass:
        d.vy -= 10.0f * dt; d.vx *= 0.994f; break;
      case Anim::Ripple:
        d.vx *= 0.97f; d.vy *= 0.97f; break;
      case Anim::Rave:
        d.vx = clamp(d.vx + rc(0, 45.0f) * dt, -290.0f, 290.0f);
        d.vy = clamp(d.vy + rc(0, 45.0f) * dt, -290.0f, 290.0f); break;
      default: break;
    }

    // Edge handling
    if (d.wrap) {
      if (d.x < -12) d.x = W+12; else if (d.x > W+12) d.x = -12;
      if (d.y < -12) d.y = H+12; else if (d.y > H+12) d.y = -12;
    } else {
      bool dead = d.y > H+30 || d.y < -30;
      if (!dead && (curType == Anim::SpeedLines || curType == Anim::Bats))
        dead = d.x < -30 || d.x > W+30;
      if (dead) { release(d); continue; }
    }

    // Opacity / color update
    lv_opa_t opa;
    if (isRave) {
      opa = LV_OPA_COVER;
      // Strobe: random particles flash white every ~11 frames
      if (phase % 11 == 0 && r32() % 6 == 0)
        lv_obj_set_style_bg_color(d.o, lv_color_hex(0xFFFFFF), 0);
      // Continuous color cycling every 4 frames
      else if (phase % 4 == 0) {
        constexpr uint32_t rp[] = {0xFF00FF,0x00FFFF,0xFF0099,0x99FF00,
                                   0xFF4400,0x0044FF,0xFFFF00,0xFF88CC};
        lv_obj_set_style_bg_color(d.o, lv_color_hex(rp[(phase/4 + (uint8_t)(&d-pool)) % 8]), 0);
      }
    } else if (curType == Anim::Stars) {
      float t = d.life / d.maxLife;
      float bright = t < 0.5f ? t * 2.0f : (1.0f - t) * 2.0f;
      opa = (lv_opa_t)(LV_OPA_COVER * bright);
    } else {
      opa = (lv_opa_t)(LV_OPA_COVER * (d.life / d.maxLife));
    }
    lv_obj_set_pos(d.o, (int16_t)d.x, (int16_t)d.y);
    lv_obj_set_style_bg_opa(d.o, opa, 0);
  }
}

static uint32_t fireEvent(uint32_t now) {
  switch (curType) {
    case Anim::Stars:         return now + evtStars();
    case Anim::Embers:        return now + evtEmbers();
    case Anim::Lightning:     return now + evtLightning();
    case Anim::Ripple:        return now + evtRipple();
    case Anim::SpeedLines:    return now + evtSpeedLines();
    case Anim::Rave:          return now + evtRave();
    case Anim::PixelRain:     return now + evtPixelRain();
    case Anim::Geometry:      return now + evtGeometry();
    case Anim::FireworksFlag: return now + evtFireworks(true);
    case Anim::Fireworks:     return now + evtFireworks(false);
    case Anim::Bats:          return now + evtBats();
    case Anim::Snow:          return now + evtSnow();
    case Anim::Leaves:        return now + evtLeaves();
    case Anim::Hearts:        return now + evtHearts();
    case Anim::Grass:         return now + evtGrass();
    default:                  return now + 10000;
  }
}

}  // anonymous namespace

// ─── Public API ─────────────────────────────────────────────────────────────
namespace AnimLayer {

void build(lv_obj_t *root, uint8_t animType, bool enabled) {
  curType    = animType;
  curEnabled = enabled;
  lastMs     = 0;
  nextEvtMs  = 0;
  phase      = 0;

  for (auto &d : pool) { d.o = nullptr; d.on = false; d.life = 0; }

  if (animType == Anim::None || !root) return;

  for (auto &d : pool) {
    d.o = lv_obj_create(root);
    lv_obj_clear_flag(d.o, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_border_width(d.o, 0, 0);
    lv_obj_set_style_pad_all(d.o, 0, 0);
    lv_obj_set_style_bg_opa(d.o, LV_OPA_TRANSP, 0);
    lv_obj_add_flag(d.o, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_size(d.o, 4, 4);
    lv_obj_set_style_radius(d.o, 2, 0);
  }

  if (animType == Anim::FireworksFlag) buildFlag(root);
}

void step(uint32_t now) {
  if (!curEnabled || curType == Anim::None) return;
  if (lastMs == 0) { lastMs = now; nextEvtMs = now + 1200; return; }
  if (now - lastMs < 33) return;

  const float dt = (float)(now - lastMs) * 0.001f;
  lastMs = now;
  ++phase;

  stepDots(dt);
  if (now >= nextEvtMs) nextEvtMs = fireEvent(now);
}

void setEnabled(bool enabled) {
  if (curEnabled == enabled) return;
  curEnabled = enabled;
  if (!enabled)
    for (auto &d : pool) if (d.on) release(d);
}

}  // namespace AnimLayer
