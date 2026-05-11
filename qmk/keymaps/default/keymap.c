/* Copyright 2026 Yiannis Michael (ymich9963)

   This program is free software: you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of
   the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <https://www.gnu.org/licenses/>.
   */

#include QMK_KEYBOARD_H

enum layer_names {
    _BL,    /* Base layer */
    _FL,    /* Fn layer */
    _ML,    /* Mouse layer */
    _DL,    /* MIDI layer */
    _AL,    /* Audio layer */
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BL] = LAYOUT(
            QK_GESC,KC_1   ,KC_2   ,KC_3   ,KC_4   ,KC_5   ,KC_6   ,KC_7   ,KC_8   ,KC_9   ,KC_0   ,KC_MINS,KC_EQL ,KC_BSPC,KC_DEL,
            KC_TAB ,KC_Q   ,KC_W   ,KC_E   ,KC_R   ,KC_T   ,        KC_Y   ,KC_U   ,KC_I   ,KC_O   ,KC_P   ,KC_LBRC,KC_RBRC,KC_ENT,
            KC_CAPS,KC_A   ,KC_S   ,KC_D   ,KC_F   ,KC_G   ,        KC_H   ,KC_J   ,KC_K   ,KC_L   ,KC_SCLN,KC_QUOT,KC_NUHS,KC_PGUP,
            KC_LSFT,KC_NUBS,KC_Z   ,KC_X   ,KC_C   ,KC_V   ,KC_B   ,KC_N   ,KC_M   ,KC_COMM,KC_DOT ,KC_SLSH,KC_RSFT,KC_UP  ,KC_PGDN,
            KC_LCTL,KC_LGUI,KC_LALT,KC_SPC ,                        KC_SPC ,        KC_RALT,MO(_FL),KC_RCTL,KC_LEFT,KC_DOWN,KC_RGHT
            ),
    [_FL] = LAYOUT(
            QK_BOOT,  KC_F1,  KC_F2,  KC_F3, KC_F4 ,  KC_F5,KC_F6  ,KC_F7  ,  KC_F8,  KC_F9, KC_F10,KC_F11 , KC_F12,KC_MUTE,KC_INS,
            _______,_______,_______,_______,_______,_______        ,_______,_______,_______,_______,KC_PSCR,_______,_______,_______,
            _______,TO(_AL),TO(_ML),TO(_DL),_______,_______        ,_______,_______,_______,KC_SCRL,_______,_______,_______,KC_HOME,
            _______,_______,_______,_______,_______,_______,_______,_______,_______,_______,_______,_______,_______,KC_VOLU,KC_END,
            _______,_______,_______,KC_MPLY,                        KC_MPLY,        _______,_______,_______,KC_MPRV,KC_VOLD,KC_MNXT
            ),
    [_ML] = LAYOUT(
            TO(_BL),MS_ACL1,MS_ACL2,_______,MS_BTN4,MS_BTN5,MS_BTN6,MS_BTN7,MS_BTN8,_______,MS_ACL0,_______,_______,_______,_______,
            _______,_______,_______,_______,_______,MS_BTN3        ,_______,_______,_______,_______,_______,_______,_______,_______,
            _______,_______,_______,_______,MS_BTN1,MS_BTN2        ,MS_LEFT,MS_DOWN,MS_UP  ,MS_RGHT,_______,_______,_______,_______,
            _______,_______,_______,_______,_______,_______,_______,MS_WHLL,MS_WHLD,MS_WHLU,MS_WHLR,_______,_______,_______,_______,
            _______,_______,_______,_______,                        _______,        _______,_______,_______,_______,_______,_______
            ),
    [_DL] = LAYOUT(
            TO(_BL),_______,_______,_______,_______,_______,_______,_______,_______,_______,_______,MI_CHND,MI_CHNU,MI_TOGG,_______,
            _______,_______,MI_Cs4 ,MI_Ds4 ,_______,MI_Fs4         ,MI_Gs4 ,MI_As4 ,_______,_______,_______,_______,_______,_______,
            _______,MI_C4  ,MI_D4  ,MI_E4  ,MI_F4  ,MI_G4          ,MI_A4  ,MI_B4  ,MI_C5  ,_______,_______,_______,_______,MI_MODU,
            MI_SUST,_______,MI_OCTD,MI_OCTU,MI_TRSD,MI_TRSU,MI_VELD,MI_VELU,_______,_______,_______,_______,MI_MOD ,MI_BNDU,MI_MODD,
            MI_PORT,_______,MI_SOST,MI_AOFF,                        MI_AOFF,        MI_SOFT,_______,MI_LEG ,_______,MI_BNDD,_______
            ),
    [_AL] = LAYOUT(
            TO(_BL),XXXXXXX,XXXXXXX,XXXXXXX,XXXXXXX,XXXXXXX,XXXXXXX,XXXXXXX,XXXXXXX,XXXXXXX,XXXXXXX,XXXXXXX,XXXXXXX,XXXXXXX,XXXXXXX,
            XXXXXXX,XXXXXXX,XXXXXXX,XXXXXXX,XXXXXXX,XXXXXXX        ,XXXXXXX,XXXXXXX,XXXXXXX,XXXXXXX,XXXXXXX,XXXXXXX,XXXXXXX,XXXXXXX,
            XXXXXXX,AU_TOGG,XXXXXXX,XXXXXXX,XXXXXXX,XXXXXXX        ,XXXXXXX,XXXXXXX,AU_NEXT,XXXXXXX,XXXXXXX,XXXXXXX,XXXXXXX,XXXXXXX,
            XXXXXXX,XXXXXXX,XXXXXXX,XXXXXXX,CK_TOGG,CK_UP  ,CK_DOWN,CK_RST ,MU_TOGG,MU_NEXT,XXXXXXX,XXXXXXX,XXXXXXX,XXXXXXX,XXXXXXX,
            XXXXXXX,XXXXXXX,XXXXXXX,XXXXXXX,                        XXXXXXX,        XXXXXXX,XXXXXXX,XXXXXXX,XXXXXXX,XXXXXXX,XXXXXXX
            ),
};
