// ============================================================================
//  M5 PC Remote — M5StickC Plus2
//  ---------------------------------------------------------------------------
//  Télécommande Bluetooth (HID clavier + touches multimédia) pour PC.
//  S'appaire comme un clavier BLE : aucun dongle, aucun driver.
//
//  Bouton POWER (côté) : change de page
//  Boutons A / B (clic + maintien) : actions de la page courante
//  Secousse : coupe/réactive le micro (page APPEL)
// ============================================================================

#include <M5Unified.h>
#include <BleKeyboard.h>
#include <math.h>
#include "config.h"

BleKeyboard bleKeyboard(BLE_NAME, "M5Stack", 100);
static M5Canvas canvas(&M5.Display);

enum Page { PG_MEDIA, PG_VOLUME, PG_CALL, PG_PRESENT, PG_MACRO, PG_COUNT };
static int      page         = PG_MEDIA;
static bool     comboActive  = false;
static bool     wasConnected = false;
static uint32_t lastBatt     = 0;
static uint32_t lastShake    = 0;
static uint32_t volRepeat    = 0;

static uint16_t colAccent, colGreen, colRed, colBar, colDim, colHead;

static void initColors() {
  colAccent = canvas.color565(155,  89, 182);  // violet
  colGreen  = canvas.color565( 46, 204, 113);
  colRed    = canvas.color565(231,  76,  60);
  colBar    = canvas.color565( 28,  28,  32);
  colDim    = canvas.color565(140, 140, 150);
  colHead   = canvas.color565( 44,  62,  80);
}

// ---------------------------------------------------------------------------
//  Envoi de touches (rien n'est envoyé si le PC n'est pas connecté)
// ---------------------------------------------------------------------------
static inline bool connected() { return bleKeyboard.isConnected(); }

#define MEDIA(k)  do { if (connected()) bleKeyboard.write(k); } while (0)

static void tapKey(uint8_t k) {
  if (connected()) bleKeyboard.write(k);
}

// combo : jusqu'à 2 modificateurs + 1 touche (mettre mod2 = 0 si inutile)
static void combo(uint8_t mod1, uint8_t mod2, uint8_t key) {
  if (!connected()) return;
  if (mod1) bleKeyboard.press(mod1);
  if (mod2) bleKeyboard.press(mod2);
  bleKeyboard.press(key);
  delay(20);
  bleKeyboard.releaseAll();
}

// ---- Raccourcis de visio, selon CALL_APP -----------------------------------
static void muteMic() {
#if   CALL_APP == 1
  combo(KEY_LEFT_CTRL, KEY_LEFT_SHIFT, 'm');   // Discord
#elif CALL_APP == 2
  combo(KEY_LEFT_CTRL, KEY_LEFT_SHIFT, 'm');   // Teams
#elif CALL_APP == 3
  combo(KEY_LEFT_CTRL, 0, 'd');                // Google Meet
#elif CALL_APP == 4
  combo(KEY_LEFT_ALT, 0, 'a');                 // Zoom
#endif
}

static void callSecondary() {
#if   CALL_APP == 1
  combo(KEY_LEFT_CTRL, KEY_LEFT_SHIFT, 'd');   // Discord : deafen
#elif CALL_APP == 2
  combo(KEY_LEFT_CTRL, KEY_LEFT_SHIFT, 'o');   // Teams : caméra
#elif CALL_APP == 3
  combo(KEY_LEFT_CTRL, 0, 'e');                // Meet : caméra
#elif CALL_APP == 4
  combo(KEY_LEFT_ALT, 0, 'v');                 // Zoom : caméra
#endif
}

#if   CALL_APP == 1
  #define CALL_NAME "DISCORD"
  #define CALL_B    "Deafen"
#elif CALL_APP == 2
  #define CALL_NAME "TEAMS"
  #define CALL_B    "Camera"
#elif CALL_APP == 3
  #define CALL_NAME "MEET"
  #define CALL_B    "Camera"
#elif CALL_APP == 4
  #define CALL_NAME "ZOOM"
  #define CALL_B    "Camera"
#endif

// ---------------------------------------------------------------------------
//  Boutons
// ---------------------------------------------------------------------------
static void beep(int f) { M5.Speaker.tone(f, 60); }

static void nextPage() {
  page = (page + 1) % PG_COUNT;
  beep(1400);
}

static void handleButtons() {
  bool aP = M5.BtnA.isPressed();
  bool bP = M5.BtnB.isPressed();

  // --- Changer de page : A + B pressés ensemble ---
  if (aP && bP) {
    if (!comboActive) { comboActive = true; nextPage(); }
    return;                       // on ne déclenche pas les actions pendant le combo
  }
  if (comboActive) {              // attendre le relâchement des 2 boutons
    if (!aP && !bP) comboActive = false;
    return;
  }

  // Bonus : bouton power (marche sur certaines unités seulement)
  if (M5.BtnPWR.wasClicked()) nextPage();

  // Page VOLUME : maintien = répétition continue
  if (page == PG_VOLUME) {
    if (M5.BtnA.isPressed() && millis() - volRepeat > 140) {
      MEDIA(KEY_MEDIA_VOLUME_UP);   volRepeat = millis();
    } else if (M5.BtnB.isPressed() && millis() - volRepeat > 140) {
      MEDIA(KEY_MEDIA_VOLUME_DOWN); volRepeat = millis();
    }
    return;
  }

  bool aC = M5.BtnA.wasClicked(), aH = M5.BtnA.wasHold();
  bool bC = M5.BtnB.wasClicked(), bH = M5.BtnB.wasHold();

  switch (page) {
    case PG_MEDIA:
      if (aC) MEDIA(KEY_MEDIA_PLAY_PAUSE);
      if (aH) MEDIA(KEY_MEDIA_MUTE);
      if (bC) MEDIA(KEY_MEDIA_NEXT_TRACK);
      if (bH) MEDIA(KEY_MEDIA_PREVIOUS_TRACK);
      break;
    case PG_CALL:
      if (aC) muteMic();
      if (bC) callSecondary();
      break;
    case PG_PRESENT:
      if (aC) tapKey(KEY_RIGHT_ARROW);
      if (aH) tapKey(KEY_F5);
      if (bC) tapKey(KEY_LEFT_ARROW);
      if (bH) tapKey(KEY_ESC);
      break;
    case PG_MACRO:
      if (aC) combo(KEY_LEFT_GUI, KEY_LEFT_SHIFT, 's');  // capture écran
      if (aH) combo(KEY_LEFT_GUI, 0, 'd');               // bureau
      if (bC) combo(KEY_LEFT_GUI, 0, 'l');               // verrouiller
      if (bH) combo(KEY_LEFT_GUI, 0, 'e');               // explorateur
      break;
  }
}

static void checkShake() {
#if SHAKE_TO_MUTE
  float ax, ay, az;
  if (M5.Imu.getAccel(&ax, &ay, &az)) {
    float mag = sqrtf(ax * ax + ay * ay + az * az);
    if (mag > SHAKE_THRESHOLD && millis() - lastShake > 900) {
      lastShake = millis();
      muteMic();
      beep(1900);
    }
  }
#endif
}

// ---------------------------------------------------------------------------
//  Rendu
// ---------------------------------------------------------------------------
static const char* pageName(int p) {
  switch (p) {
    case PG_MEDIA:   return "MEDIA";
    case PG_VOLUME:  return "VOLUME";
    case PG_CALL:    return CALL_NAME;
    case PG_PRESENT: return "SLIDES";
    case PG_MACRO:   return "MACROS";
  }
  return "?";
}

static void legend(const char* tag, const char* desc, int y, uint16_t tagCol) {
  canvas.setTextDatum(TL_DATUM);
  canvas.setTextColor(tagCol);
  canvas.drawString(tag, 6, y);
  canvas.setTextColor(TFT_WHITE);
  canvas.drawString(desc, 52, y);
}

static void render() {
  const int W = canvas.width();
  const int H = canvas.height();
  bool con = connected();
  canvas.fillSprite(TFT_BLACK);

  // ---- En-tête : nom de la page ----
  canvas.fillRect(0, 0, W, 30, con ? colAccent : colHead);
  canvas.setTextColor(TFT_WHITE);
  canvas.setTextDatum(MC_DATUM);
  canvas.setTextSize(2);
  canvas.drawString(pageName(page), W / 2, 15);
  canvas.setTextSize(1);
  canvas.setTextDatum(TR_DATUM);
  char idx[8];
  snprintf(idx, sizeof(idx), "%d/%d", page + 1, PG_COUNT);
  canvas.drawString(idx, W - 3, 3);

  // ---- Légende des boutons ----
  int y0 = 42, dy = 24;
  switch (page) {
    case PG_MEDIA:
      legend("A",      "Play / Pause",   y0 + 0 * dy, colAccent);
      legend("A long", "Mute son",       y0 + 1 * dy, colDim);
      legend("B",      "Suivant",        y0 + 2 * dy, colAccent);
      legend("B long", "Precedent",      y0 + 3 * dy, colDim);
      break;
    case PG_VOLUME:
      legend("A", "Volume +", y0 + 0 * dy, colAccent);
      legend("B", "Volume -", y0 + 1 * dy, colAccent);
      canvas.setTextColor(colDim);
      canvas.setTextDatum(TL_DATUM);
      canvas.drawString("Maintiens pour repeter", 6, y0 + 3 * dy);
      break;
    case PG_CALL:
      legend("A", "Mute micro", y0 + 0 * dy, colAccent);
      legend("B", CALL_B,       y0 + 1 * dy, colAccent);
      canvas.setTextColor(colDim);
      canvas.setTextDatum(TL_DATUM);
      canvas.drawString("Secoue = mute", 6, y0 + 3 * dy);
      break;
    case PG_PRESENT:
      legend("A",      "Suivant",   y0 + 0 * dy, colAccent);
      legend("A long", "Demarrer",  y0 + 1 * dy, colDim);
      legend("B",      "Precedent", y0 + 2 * dy, colAccent);
      legend("B long", "Quitter",   y0 + 3 * dy, colDim);
      break;
    case PG_MACRO:
      legend("A",      "Capture ecran", y0 + 0 * dy, colAccent);
      legend("A long", "Bureau",        y0 + 1 * dy, colDim);
      legend("B",      "Verrouiller",   y0 + 2 * dy, colAccent);
      legend("B long", "Explorateur",   y0 + 3 * dy, colDim);
      break;
  }

  // ---- Pied : statut BLE + batterie + rappel PWR ----
  canvas.fillRect(0, H - 12, W, 12, colBar);
  canvas.setTextDatum(ML_DATUM);
  canvas.setTextColor(con ? colGreen : colRed);
  canvas.drawString(con ? "connecte" : "appairage...", 3, H - 6);

  canvas.setTextDatum(MC_DATUM);
  canvas.setTextColor(colDim);
  canvas.drawString("A+B = page", W / 2, H - 6);

  int bat = M5.Power.getBatteryLevel();
  char b[8];
  snprintf(b, sizeof(b), "%d%%", bat);
  canvas.setTextDatum(MR_DATUM);
  canvas.setTextColor(bat >= 0 && bat < 20 ? colRed : colDim);
  canvas.drawString(b, W - 3, H - 6);

  canvas.pushSprite(0, 0);
}

// ---------------------------------------------------------------------------
//  Setup / Loop
// ---------------------------------------------------------------------------
void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  Serial.begin(115200);
  M5.Display.setRotation(SCREEN_ROTATION);
  M5.Display.setBrightness(SCREEN_BRIGHTNESS);

  canvas.setColorDepth(16);
  canvas.createSprite(M5.Display.width(), M5.Display.height());
  canvas.setTextWrap(false);
  initColors();

  M5.Speaker.setVolume(BUZZER_VOLUME);

  bleKeyboard.begin();
  Serial.println("\n=== M5 PC Remote — en attente d'appairage ===");
}

void loop() {
  M5.update();
  handleButtons();
  checkShake();

  bool con = connected();
  if (con && !wasConnected) { beep(1900); Serial.println("[ble] PC connecte"); }
  if (!con && wasConnected) { beep(400);  Serial.println("[ble] deconnecte"); }
  wasConnected = con;

  if (con && millis() - lastBatt > 30000) {
    lastBatt = millis();
    bleKeyboard.setBatteryLevel(M5.Power.getBatteryLevel());
  }

  render();
  delay(30);
}
