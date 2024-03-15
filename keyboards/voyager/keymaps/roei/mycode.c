// --------------------------------------------------------
// --- custom code
// https://github.com/filterpaper/qmk_userspace
// --------------------------------------------------------

#include "print.h"

// Basic keycode filter for tap-hold keys
#define GET_TAP_KEYCODE(kc) ((kc) & 0xff)

// Tap-hold decision helper macros
#define IS_HOMEROW(r)        (r->event.key.row == 2 || r->event.key.row == 8)
#define IS_LAYER_TAP(kc)     (IS_QK_LAYER_TAP(kc) && QK_LAYER_TAP_GET_LAYER(kc))
#define IS_TEXT(kc)          (KC_A <= (uint8_t)(kc) && (uint8_t)(kc) <= KC_SLSH)
#define IS_TYPING(kc)        (last_input_activity_elapsed() < QUICK_TAP_TERM && IS_TEXT(kc) && !IS_LAYER_TAP(kc))
#define IS_MOD_TAP_SHIFT(kc) (IS_QK_MOD_TAP(kc) && (kc) & (QK_LSFT))
#define IS_MOD_TAP_CS(kc)    (IS_QK_MOD_TAP(kc) && (kc) & (QK_LCTL | QK_LSFT))
#define IS_MOD_TAP_CAG(kc)   (IS_QK_MOD_TAP(kc) && (kc) & (QK_LCTL | QK_LALT | QK_LGUI))

// Home row mod-tap and the following key are on the same side of the keyboard
#define IS_UNILATERAL(r, n) ( \
    (0 <= r->event.key.row && r->event.key.row <= 4 && 0 <= n.event.key.row && n.event.key.row <= 4) || \
    (6 <= r->event.key.row && r->event.key.row <= 10 && 6 <= n.event.key.row && n.event.key.row <= 10) )

// Home row mod-tap and the following key are on opposite sides of the keyboard
#define IS_BILATERAL(r, n) ( \
    (r->event.key.row == 2 && 6 <= n.event.key.row && n.event.key.row <= 10) || \
    (r->event.key.row == 8 && 0 <= n.event.key.row && n.event.key.row <= 4) )

// store next key
static uint16_t    next_keycode;
static keyrecord_t next_record;

/*
 * Instant tap
 * Tap-hold key-up delays can be bothersome and unnecessary while typing quickly.
 * To eliminate these delays, the tap-hold key is replaced with its tap keycode if prior key is a text keycode
 * and input was less than the QUICK_TAP_TERM duration in milliseconds.
 */
bool pre_process_record_user(uint16_t keycode, keyrecord_t *record) {
    static uint16_t prev_keycode;
    static bool     is_pressed[UINT8_MAX];

    // Cache previous and next input for tap-hold decisions
    if (record->event.pressed) {
        prev_keycode = next_keycode;
        next_keycode = keycode;
        next_record  = *record;
    }

    // Override non-Shift tap-hold keys based on previous input
    if (IS_HOMEROW(record) && IS_MOD_TAP_CAG(keycode)) {
        uint8_t const tap_keycode = GET_TAP_KEYCODE(keycode);
        // Press the tap keycode when precedeed by short text input interval
        if (record->event.pressed && IS_TYPING(prev_keycode)) {
            record->keycode = tap_keycode;
            is_pressed[tap_keycode] = true;
        }
        // Release the tap keycode if pressed
        else if (!record->event.pressed && is_pressed[tap_keycode]) {
            record->keycode = tap_keycode;
            is_pressed[tap_keycode] = false;
        }
    }

#ifdef CONSOLE_ENABLE
    uprintf("KL: kc: 0x%04X, col: %2u, row: %2u, pressed: %u, time: %5u, int: %u, count: %u\n", keycode, record->event.key.col, record->event.key.row, record->event.pressed, record->event.time, record->tap.interrupted, record->tap.count);
#endif

    return true;
}

/*
 * Permissive bilateral hold
 * Modifiers should be triggered when a mod-tap key is held down and another key is tapped with the opposite hand.
 * required: #define PERMISSIVE_HOLD_PER_KEY
 */
bool get_permissive_hold(uint16_t keycode, keyrecord_t *record) {
    // Send Control or Shift with a nested key press on the opposite hand
    return IS_BILATERAL(record, next_record) && IS_MOD_TAP_CS(keycode);
}

/*
 * Immediately select the hold action when another key is pressed.
 */
bool get_hold_on_other_key_press(uint16_t keycode, keyrecord_t *record) {
    // Activate layer with another key press
    if (IS_LAYER_TAP(keycode)) return true;

    // Sent its tap keycode when non-Shift overlaps with another key on the same hand
    if (IS_BILATERAL(record, next_record) && IS_MOD_TAP_SHIFT(keycode)) {
        return true;
    }

    return false;
}
