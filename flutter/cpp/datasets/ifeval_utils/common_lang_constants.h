/** @addtogroup Stemming
    @brief Library for stemming words down to their root words.
    @date 2004-2025
    @copyright Oleander Software, Ltd.
    @author Blake Madden
    @details This program is free software; you can redistribute it and/or modify
     it under the terms of the BSD License.

    SPDX-License-Identifier: BSD-3-Clause
* @{*/

#ifndef OLEAN_COMMON_LANG_CONSTANTS_H
#define OLEAN_COMMON_LANG_CONSTANTS_H

#include <cwctype>
#include <string>

namespace common_lang_constants
{
    constexpr wchar_t TAB = 0x09;
    constexpr wchar_t SPACE = 0x20;
    constexpr wchar_t COMMA = 0x2C;
    constexpr wchar_t COMMA_FULL_WIDTH = 0xFF0C;
    constexpr wchar_t LESS_THAN = 60;
    constexpr wchar_t GREATER_THAN = 62;
    constexpr wchar_t POUND = 35;
    constexpr wchar_t AMPERSAND = 0x26;
    constexpr wchar_t SEMICOLON = 59;
    constexpr wchar_t APOSTROPHE = 0x27;
    constexpr wchar_t DOUBLE_QUOTE = 0x22;
    constexpr wchar_t QUESTION_MARK = 0x3F;
    constexpr wchar_t QUESTION_MARK_FULL_WIDTH = 0xFF1F;
    constexpr wchar_t PERIOD = 0x2E;
    constexpr wchar_t PERIOD_FULL_WIDTH = 0xFF0E;
    constexpr wchar_t PERIOD_HALF_WIDTH = 0xFF61;
    constexpr wchar_t EXCLAMATION_MARK = 0x21;
    constexpr wchar_t EXCLAMATION_MARK_FULL_WIDTH = 0xFF01;
    constexpr wchar_t COLON = 0x3A;
    constexpr wchar_t COLON_FULL_WIDTH = 0xFF1A;
    constexpr wchar_t FORWARD_SLASH = 0x2F;
    constexpr wchar_t FORWARD_SLASH_FULL_WIDTH = 0xFF0F;
    constexpr wchar_t BACK_SLASH = 0x5C;
    constexpr wchar_t BACK_SLASH_FULL_WIDTH = 0xFF3C;
    constexpr wchar_t DOLLAR_SIGN = 0x24;
    constexpr wchar_t PERCENTAGE_SIGN = 0x25;
    constexpr wchar_t HYPHEN = 0x2D;
    constexpr wchar_t SOFT_HYPHEN = 0xAD;
    constexpr wchar_t HYPHEN_FULL_WIDTH = 0xFF0D;
    constexpr wchar_t LEFT_PARENTHESIS = 0x28;
    constexpr wchar_t LEFT_PARENTHESIS_FULL_WIDTH = 0xFF08;
    constexpr wchar_t RIGHT_PARENTHESIS = 0x29;
    constexpr wchar_t RIGHT_PARENTHESIS_FULL_WIDTH = 0xFF09;
    constexpr wchar_t RIGHT_BRACKET = 0x5D;
    constexpr wchar_t INTERROBANG = 0x203D;
    constexpr wchar_t COPYRIGHT_SYMBOL = 0xA9;
    constexpr wchar_t REGISTERED_SYMBOL = 0xAE;
    constexpr wchar_t TRADEMARK_SYMBOL = 0x2122;
    // numbers
    constexpr wchar_t NUMBER_0 = 0x30;
    constexpr wchar_t NUMBER_1 = 0x31;
    constexpr wchar_t NUMBER_2 = 0x32;
    constexpr wchar_t NUMBER_3 = 0x33;
    constexpr wchar_t NUMBER_4 = 0x34;
    constexpr wchar_t NUMBER_5 = 0x35;
    constexpr wchar_t NUMBER_6 = 0x36;
    constexpr wchar_t NUMBER_7 = 0x37;
    constexpr wchar_t NUMBER_8 = 0x38;
    constexpr wchar_t NUMBER_9 = 0x39;
    constexpr wchar_t NUMBER_0_FULL_WIDTH = 0xFF10;
    constexpr wchar_t NUMBER_1_FULL_WIDTH = 0xFF11;
    constexpr wchar_t NUMBER_2_FULL_WIDTH = 0xFF12;
    constexpr wchar_t NUMBER_3_FULL_WIDTH = 0xFF13;
    constexpr wchar_t NUMBER_4_FULL_WIDTH = 0xFF14;
    constexpr wchar_t NUMBER_5_FULL_WIDTH = 0xFF15;
    constexpr wchar_t NUMBER_6_FULL_WIDTH = 0xFF16;
    constexpr wchar_t NUMBER_7_FULL_WIDTH = 0xFF17;
    constexpr wchar_t NUMBER_8_FULL_WIDTH = 0xFF18;
    constexpr wchar_t NUMBER_9_FULL_WIDTH = 0xFF19;
    // letters
    constexpr wchar_t UPPER_A = 0x41;
    constexpr wchar_t LOWER_A = 0x61;
    constexpr wchar_t UPPER_B = 0x42;
    constexpr wchar_t LOWER_B = 0x62;
    constexpr wchar_t UPPER_C = 0x43;
    constexpr wchar_t LOWER_C = 0x63;
    constexpr wchar_t UPPER_D = 0x44;
    constexpr wchar_t LOWER_D = 0x64;
    constexpr wchar_t UPPER_E = 0x45;
    constexpr wchar_t LOWER_E = 0x65;
    constexpr wchar_t UPPER_F = 0x46;
    constexpr wchar_t LOWER_F = 0x66;
    constexpr wchar_t UPPER_G = 0x47;
    constexpr wchar_t LOWER_G = 0x67;
    constexpr wchar_t UPPER_H = 0x48;
    constexpr wchar_t LOWER_H = 0x68;
    constexpr wchar_t UPPER_I = 0x49;
    constexpr wchar_t LOWER_I = 0x69;
    constexpr wchar_t UPPER_J = 0x4A;
    constexpr wchar_t LOWER_J = 0x6A;
    constexpr wchar_t UPPER_K = 0x4B;
    constexpr wchar_t LOWER_K = 0x6B;
    constexpr wchar_t UPPER_L = 0x4C;
    constexpr wchar_t LOWER_L = 0x6C;
    constexpr wchar_t UPPER_M = 0x4D;
    constexpr wchar_t LOWER_M = 0x6D;
    constexpr wchar_t UPPER_N = 0x4E;
    constexpr wchar_t LOWER_N = 0x6E;
    constexpr wchar_t UPPER_O = 0x4F;
    constexpr wchar_t LOWER_O = 0x6F;
    constexpr wchar_t UPPER_P = 0x50;
    constexpr wchar_t LOWER_P = 0x70;
    constexpr wchar_t UPPER_Q = 0x51;
    constexpr wchar_t LOWER_Q = 0x71;
    constexpr wchar_t UPPER_R = 0x52;
    constexpr wchar_t LOWER_R = 0x72;
    constexpr wchar_t UPPER_S = 0x53;
    constexpr wchar_t LOWER_S = 0x73;
    constexpr wchar_t UPPER_T = 0x54;
    constexpr wchar_t LOWER_T = 0x74;
    constexpr wchar_t UPPER_U = 0x55;
    constexpr wchar_t LOWER_U = 0x75;
    constexpr wchar_t UPPER_V = 0x56;
    constexpr wchar_t LOWER_V = 0x76;
    constexpr wchar_t UPPER_W = 0x57;
    constexpr wchar_t LOWER_W = 0x77;
    constexpr wchar_t UPPER_X = 0x58;
    constexpr wchar_t LOWER_X = 0x78;
    constexpr wchar_t UPPER_Y = 0x59;
    constexpr wchar_t LOWER_Y = 0x79;
    constexpr wchar_t UPPER_Z = 0x5A;
    constexpr wchar_t LOWER_Z = 0x7A;

    constexpr wchar_t UPPER_A_ACUTE = 0xC1;
    constexpr wchar_t LOWER_A_ACUTE = 0xE1;
    constexpr wchar_t UPPER_E_ACUTE = 0xC9;
    constexpr wchar_t LOWER_E_ACUTE = 0xE9;
    constexpr wchar_t UPPER_I_ACUTE = 0xCD;
    constexpr wchar_t LOWER_I_ACUTE = 0xED;
    constexpr wchar_t UPPER_O_ACUTE = 0xD3;
    constexpr wchar_t LOWER_O_ACUTE = 0xF3;
    constexpr wchar_t LOWER_U_ACUTE = 0xFA;
    constexpr wchar_t UPPER_U_ACUTE = 0xDA;
    constexpr wchar_t UPPER_A_CIRCUMFLEX = 0xC2;
    constexpr wchar_t LOWER_A_CIRCUMFLEX = 0xE2;
    constexpr wchar_t UPPER_E_CIRCUMFLEX = 0xCA;
    constexpr wchar_t LOWER_E_CIRCUMFLEX = 0xEA;
    constexpr wchar_t UPPER_I_CIRCUMFLEX = 0xCE;
    constexpr wchar_t LOWER_I_CIRCUMFLEX = 0xEE;
    constexpr wchar_t UPPER_A_TILDE = 0xC3;
    constexpr wchar_t LOWER_A_TILDE = 0xE3;
    constexpr wchar_t UPPER_O_TILDE = 0xD5;
    constexpr wchar_t LOWER_O_TILDE = 0xF5;
    constexpr wchar_t UPPER_N_TILDE = 0xD1;
    constexpr wchar_t LOWER_N_TILDE = 0xF1;
    constexpr wchar_t UPPER_O_STROKE = 0xD8;
    constexpr wchar_t LOWER_O_STROKE = 0xF8;
    constexpr wchar_t UPPER_C_CEDILLA = 0xC7;
    constexpr wchar_t LOWER_C_CEDILLA = 0xE7;
    constexpr wchar_t UPPER_A_UMLAUTS = 0xC4;
    constexpr wchar_t LOWER_A_UMLAUTS = 0xE4;
    constexpr wchar_t UPPER_O_UMLAUTS = 0xD6;
    constexpr wchar_t LOWER_O_UMLAUTS = 0xF6;
    constexpr wchar_t UPPER_E_UMLAUTS = 0xCB;
    constexpr wchar_t LOWER_E_UMLAUTS = 0xEB;
    constexpr wchar_t UPPER_I_UMLAUTS = 0xCF;
    constexpr wchar_t LOWER_I_UMLAUTS = 0xEF;
    constexpr wchar_t UPPER_ETH = 0xD0;
    constexpr wchar_t LOWER_ETH = 0xF0;
    constexpr wchar_t UPPER_U_UMLAUTS = 0xDC;
    constexpr wchar_t LOWER_U_UMLAUTS = 0xFC;
    constexpr wchar_t TILDE = 0x7E;
    constexpr wchar_t UPPER_A_GRAVE = 0xC0;
    constexpr wchar_t LOWER_A_GRAVE = 0xE0;
    constexpr wchar_t UPPER_E_GRAVE = 0xC8;
    constexpr wchar_t LOWER_E_GRAVE = 0xE8;
    constexpr wchar_t UPPER_I_GRAVE = 0xCC;
    constexpr wchar_t LOWER_I_GRAVE = 0xEC;
    constexpr wchar_t UPPER_O_GRAVE = 0xD2;
    constexpr wchar_t LOWER_O_GRAVE = 0xF2;
    constexpr wchar_t UPPER_Y_ACUTE = 0xDD;
    constexpr wchar_t LOWER_Y_ACUTE = 0xFD;
    constexpr wchar_t ESZETT = 0xDF; // a.k.a. "sharp s"
    constexpr wchar_t Y_UMLAUT = 0xFF;
    constexpr wchar_t ELLIPSE = 0x2026;
    const std::wstring COMPOUND_WORD_SEPARATORS{ HYPHEN, HYPHEN_FULL_WIDTH, SOFT_HYPHEN,
                                                 FORWARD_SLASH, FORWARD_SLASH_FULL_WIDTH,
                                                 BACK_SLASH, BACK_SLASH_FULL_WIDTH };
    const std::wstring NUMBERS_AND_DOT{
        NUMBER_0, NUMBER_1, NUMBER_2, NUMBER_3, NUMBER_4,
        NUMBER_5, NUMBER_6, NUMBER_7, NUMBER_8, NUMBER_9,
        NUMBER_0_FULL_WIDTH, NUMBER_1_FULL_WIDTH,
        NUMBER_2_FULL_WIDTH, NUMBER_3_FULL_WIDTH,
        NUMBER_4_FULL_WIDTH, NUMBER_5_FULL_WIDTH,
        NUMBER_6_FULL_WIDTH, NUMBER_7_FULL_WIDTH,
        NUMBER_8_FULL_WIDTH, NUMBER_9_FULL_WIDTH,
        PERIOD };
}

/** @}*/

#endif // OLEAN_COMMON_LANG_CONSTANTS_H
