// ============================================================================
//  config.h — réglages de la télécommande
// ============================================================================
#pragma once

// Nom affiché dans les réglages Bluetooth du PC
#define BLE_NAME          "M5 StickC Remote"

// Appli de visio pour la page APPEL (bouton "Mute micro") :
//   1 = Discord       (mute = Ctrl+Shift+M,  B = Deafen Ctrl+Shift+D)
//   2 = Microsoft Teams (mute = Ctrl+Shift+M, B = Camera Ctrl+Shift+O)
//   3 = Google Meet   (mute = Ctrl+D,         B = Camera Ctrl+E)
//   4 = Zoom          (mute = Alt+A,          B = Camera Alt+V)
#define CALL_APP          1

// Secouer le stick coupe/réactive le micro (raccourci de l'appli ci-dessus)
#define SHAKE_TO_MUTE     true
#define SHAKE_THRESHOLD   2.2    // en g — plus haut = moins sensible

// Écran / buzzer
#define SCREEN_ROTATION   0      // 0/2 = portrait, 1/3 = paysage
#define SCREEN_BRIGHTNESS 90     // 0-255
#define BUZZER_VOLUME     120    // 0-255
