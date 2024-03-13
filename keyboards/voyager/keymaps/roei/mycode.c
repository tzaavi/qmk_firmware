// --------------------------------------------------------
// --- custom code
// https://github.com/filterpaper/qmk_userspace
// --------------------------------------------------------

// Basic keycode filter for tap-hold keys
#define GET_TAP_KEYCODE(kc) ((kc) & 0xff)

// Tap-hold decision helper macros
#define IS_HOMEROW(r)        (r->event.key.row == 2 || r->event.key.row == 8)
#define IS_MOD_TAP_CAG(kc)   (IS_QK_MOD_TAP(kc) && (kc) & (QK_LCTL | QK_LALT | QK_LGUI))
#define IS_LAYER_TAP(kc)     (IS_QK_LAYER_TAP(kc) && QK_LAYER_TAP_GET_LAYER(kc))
#define IS_TEXT(kc)          (KC_A <= (uint8_t)(kc) && (uint8_t)(kc) <= KC_SLSH)
#define IS_TYPING(kc)        (last_input_activity_elapsed() < QUICK_TAP_TERM && IS_TEXT(kc) && !IS_LAYER_TAP(kc))

static uint16_t    next_keycode;
static keyrecord_t next_record;

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

    return true;
}
