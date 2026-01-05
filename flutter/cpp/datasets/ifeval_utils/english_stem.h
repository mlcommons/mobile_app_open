/** @addtogroup Stemming
    @brief Library for stemming words down to their root words.
    @date 2004-2025
    @copyright Oleander Software, Ltd.
    @author Blake Madden
    @details This program is free software; you can redistribute it and/or modify
     it under the terms of the BSD License.

    SPDX-License-Identifier: BSD-3-Clause
* @{*/

#ifndef OLEAN_ENGLISH_STEM_H
#define OLEAN_ENGLISH_STEM_H

#include "flutter/cpp/datasets/ifeval_utils/stemming.h"

namespace stemming
    {
    /**
        @brief English stemmer.
    */
    //------------------------------------------------------
    template <typename string_typeT = std::wstring>
    class english_stem final : public stem<string_typeT>
        {
    public:
        /** @brief Stems an English string.
            @param[in,out] text English string to stem.*/
        void operator()(string_typeT& text) final
            {
            // reset internal data
            m_first_vowel = string_typeT::npos;
            stem<string_typeT>::reset_r_values();

            std::transform(text.begin(), text.end(), text.begin(), full_width_to_narrow);
            stem<string_typeT>::remove_possessive_suffix(text);

            if (text.length() < 3)
                { return; }

            // handle exceptions first
            if (is_exception(text) )
                { return; }

            stem<string_typeT>::hash_y(text, L"aeiouyAEIOUY");
            m_first_vowel = text.find_first_of(L"aeiouyAEIOUY");
            if (m_first_vowel == string_typeT::npos)
                { return; }

            if (text.length() >= 5 &&
                /*gener*/
                (stem<string_typeT>::is_either(text[0],
                        common_lang_constants::LOWER_G, common_lang_constants::UPPER_G) &&
                    stem<string_typeT>::is_either(text[1],
                        common_lang_constants::LOWER_E, common_lang_constants::UPPER_E) &&
                    stem<string_typeT>::is_either(text[2],
                        common_lang_constants::LOWER_N, common_lang_constants::UPPER_N) &&
                    stem<string_typeT>::is_either(text[3],
                        common_lang_constants::LOWER_E, common_lang_constants::UPPER_E) &&
                    stem<string_typeT>::is_either(text[4],
                        common_lang_constants::LOWER_R, common_lang_constants::UPPER_R) ) )
                {
                stem<string_typeT>::set_r1(5);
                }
            else if (text.length() >= 6 &&
                /*commun*/
                (stem<string_typeT>::is_either(text[0],
                        common_lang_constants::LOWER_C, common_lang_constants::UPPER_C) &&
                    stem<string_typeT>::is_either(text[1],
                        common_lang_constants::LOWER_O, common_lang_constants::UPPER_O) &&
                    stem<string_typeT>::is_either(text[2],
                        common_lang_constants::LOWER_M, common_lang_constants::UPPER_M) &&
                    stem<string_typeT>::is_either(text[3],
                        common_lang_constants::LOWER_M, common_lang_constants::UPPER_M) &&
                    stem<string_typeT>::is_either(text[4],
                        common_lang_constants::LOWER_U, common_lang_constants::UPPER_U) &&
                    stem<string_typeT>::is_either(text[5],
                        common_lang_constants::LOWER_N, common_lang_constants::UPPER_N) ) )
                {
                stem<string_typeT>::set_r1(6);
                }
            else if (text.length() >= 5 &&
                /*arsen*/
                (stem<string_typeT>::is_either(text[0],
                        common_lang_constants::LOWER_A, common_lang_constants::UPPER_A) &&
                    stem<string_typeT>::is_either(text[1],
                        common_lang_constants::LOWER_R, common_lang_constants::UPPER_R) &&
                    stem<string_typeT>::is_either(text[2],
                        common_lang_constants::LOWER_S, common_lang_constants::UPPER_S) &&
                    stem<string_typeT>::is_either(text[3],
                        common_lang_constants::LOWER_E, common_lang_constants::UPPER_E) &&
                    stem<string_typeT>::is_either(text[4],
                        common_lang_constants::LOWER_N, common_lang_constants::UPPER_N) ) )
                {
                stem<string_typeT>::set_r1(5);
                }
            else if (text.length() >= 4 &&
                /*past*/
                (stem<string_typeT>::is_either(text[0],
                        common_lang_constants::LOWER_P, common_lang_constants::UPPER_P) &&
                    stem<string_typeT>::is_either(text[1],
                        common_lang_constants::LOWER_A, common_lang_constants::UPPER_A) &&
                    stem<string_typeT>::is_either(text[2],
                        common_lang_constants::LOWER_S, common_lang_constants::UPPER_S) &&
                    stem<string_typeT>::is_either(text[3],
                        common_lang_constants::LOWER_T, common_lang_constants::UPPER_T) ) )
                {
                stem<string_typeT>::set_r1(4);
                }
            else if (text.length() >= 7 &&
                /*univers*/
                (stem<string_typeT>::is_either(text[0],
                        common_lang_constants::LOWER_U, common_lang_constants::UPPER_U) &&
                    stem<string_typeT>::is_either(text[1],
                        common_lang_constants::LOWER_N, common_lang_constants::UPPER_N) &&
                    stem<string_typeT>::is_either(text[2],
                        common_lang_constants::LOWER_I, common_lang_constants::UPPER_I) &&
                    stem<string_typeT>::is_either(text[3],
                        common_lang_constants::LOWER_V, common_lang_constants::UPPER_V) &&
                    stem<string_typeT>::is_either(text[4],
                        common_lang_constants::LOWER_E, common_lang_constants::UPPER_E) &&
                    stem<string_typeT>::is_either(text[5],
                        common_lang_constants::LOWER_R, common_lang_constants::UPPER_R) &&
                    stem<string_typeT>::is_either(text[6],
                        common_lang_constants::LOWER_S, common_lang_constants::UPPER_S)) )
                {
                stem<string_typeT>::set_r1(7);
                }
            else if (text.length() >= 5 &&
                /*later*/
                (stem<string_typeT>::is_either(text[0],
                        common_lang_constants::LOWER_L, common_lang_constants::UPPER_L) &&
                    stem<string_typeT>::is_either(text[1],
                        common_lang_constants::LOWER_A, common_lang_constants::UPPER_A) &&
                    stem<string_typeT>::is_either(text[2],
                        common_lang_constants::LOWER_T, common_lang_constants::UPPER_T) &&
                    stem<string_typeT>::is_either(text[3],
                        common_lang_constants::LOWER_E, common_lang_constants::UPPER_E) &&
                    stem<string_typeT>::is_either(text[4],
                        common_lang_constants::LOWER_R, common_lang_constants::UPPER_R) ) )
                {
                stem<string_typeT>::set_r1(5);
                }
            else if (text.length() >= 5 &&
                /*emerg*/
                (stem<string_typeT>::is_either(text[0],
                        common_lang_constants::LOWER_E, common_lang_constants::UPPER_E) &&
                    stem<string_typeT>::is_either(text[1],
                        common_lang_constants::LOWER_M, common_lang_constants::UPPER_M) &&
                    stem<string_typeT>::is_either(text[2],
                        common_lang_constants::LOWER_E, common_lang_constants::UPPER_E) &&
                    stem<string_typeT>::is_either(text[3],
                        common_lang_constants::LOWER_R, common_lang_constants::UPPER_R) &&
                    stem<string_typeT>::is_either(text[4],
                        common_lang_constants::LOWER_G, common_lang_constants::UPPER_G) ) )
                {
                stem<string_typeT>::set_r1(5);
                }
            else if (text.length() >= 5 &&
                /*organ*/
                (stem<string_typeT>::is_either(text[0],
                        common_lang_constants::LOWER_O, common_lang_constants::UPPER_O) &&
                    stem<string_typeT>::is_either(text[1],
                        common_lang_constants::LOWER_R, common_lang_constants::UPPER_R) &&
                    stem<string_typeT>::is_either(text[2],
                        common_lang_constants::LOWER_G, common_lang_constants::UPPER_G) &&
                    stem<string_typeT>::is_either(text[3],
                        common_lang_constants::LOWER_A, common_lang_constants::UPPER_A) &&
                    stem<string_typeT>::is_either(text[4],
                        common_lang_constants::LOWER_N, common_lang_constants::UPPER_N) ) )
                {
                stem<string_typeT>::set_r1(5);
                }
            else
                {
                stem<string_typeT>::find_r1(text, L"aeiouyAEIOUY");
                }

            stem<string_typeT>::find_r2(text, L"aeiouyAEIOUY");

            // step 1a:
            step_1a(text);
            // step 1b:
            step_1b(text);
            // step 1c:
            step_1c(text);
            // step 2:
            step_2(text);
            // step 3:
            step_3(text);
            // step 4:
            step_4(text);
            // step 5:
            step_5(text);

            stem<string_typeT>::unhash_y(text);
            }

        /// @returns The stemmer's language.
        [[nodiscard]]
        stemming_type get_language() const noexcept final
            { return stemming_type::english; }
    private:
        //---------------------------------------------
        bool is_exception(string_typeT& text) const
            {
            // exception #0
            /*skis*/
            if (text.length() == 4 &&
                stem<string_typeT>::is_either(text[0],
                    common_lang_constants::LOWER_S, common_lang_constants::UPPER_S) &&
                stem<string_typeT>::is_either(text[1],
                    common_lang_constants::LOWER_K, common_lang_constants::UPPER_K) &&
                stem<string_typeT>::is_either(text[2],
                    common_lang_constants::LOWER_I, common_lang_constants::UPPER_I) &&
                stem<string_typeT>::is_either(text[3],
                    common_lang_constants::LOWER_S, common_lang_constants::UPPER_S) )
                {
                text = L"ski";
                return true;
                }
            /*skies*/
            else if (text.length() == 5 &&
                    stem<string_typeT>::is_either(text[0],
                        common_lang_constants::LOWER_S, common_lang_constants::UPPER_S) &&
                    stem<string_typeT>::is_either(text[1],
                        common_lang_constants::LOWER_K, common_lang_constants::UPPER_K) &&
                    stem<string_typeT>::is_either(text[2],
                        common_lang_constants::LOWER_I, common_lang_constants::UPPER_I) &&
                    stem<string_typeT>::is_either(text[3],
                        common_lang_constants::LOWER_E, common_lang_constants::UPPER_E) &&
                    stem<string_typeT>::is_either(text[4],
                        common_lang_constants::LOWER_S, common_lang_constants::UPPER_S) )
                {
                text = L"sky";
                return true;
                }
            /*dying*/
            else if (text.length() == 5 &&
                    stem<string_typeT>::is_either(text[0],
                        common_lang_constants::LOWER_D, common_lang_constants::UPPER_D) &&
                    stem<string_typeT>::is_either(text[1],
                        common_lang_constants::LOWER_Y, common_lang_constants::UPPER_Y) &&
                    stem<string_typeT>::is_either(text[2],
                        common_lang_constants::LOWER_I, common_lang_constants::UPPER_I) &&
                    stem<string_typeT>::is_either(text[3],
                        common_lang_constants::LOWER_N, common_lang_constants::UPPER_N) &&
                    stem<string_typeT>::is_either(text[4],
                        common_lang_constants::LOWER_G, common_lang_constants::UPPER_G) )
                {
                text = L"die";
                return true;
                }
            /*lying*/
            else if (text.length() == 5 &&
                    stem<string_typeT>::is_either(text[0],
                        common_lang_constants::LOWER_L, common_lang_constants::UPPER_L) &&
                    stem<string_typeT>::is_either(text[1],
                        common_lang_constants::LOWER_Y, common_lang_constants::UPPER_Y) &&
                    stem<string_typeT>::is_either(text[2],
                        common_lang_constants::LOWER_I, common_lang_constants::UPPER_I) &&
                    stem<string_typeT>::is_either(text[3],
                        common_lang_constants::LOWER_N, common_lang_constants::UPPER_N) &&
                    stem<string_typeT>::is_either(text[4],
                       common_lang_constants::LOWER_G, common_lang_constants::UPPER_G) )
                {
                text = L"lie";
                return true;
                }
            /*tying*/
            else if (text.length() == 5 &&
                    stem<string_typeT>::is_either(text[0],
                        common_lang_constants::LOWER_T, common_lang_constants::UPPER_T) &&
                    stem<string_typeT>::is_either(text[1],
                        common_lang_constants::LOWER_Y, common_lang_constants::UPPER_Y) &&
                    stem<string_typeT>::is_either(text[2],
                        common_lang_constants::LOWER_I, common_lang_constants::UPPER_I) &&
                    stem<string_typeT>::is_either(text[3],
                        common_lang_constants::LOWER_N, common_lang_constants::UPPER_N) &&
                    stem<string_typeT>::is_either(text[4],
                        common_lang_constants::LOWER_G, common_lang_constants::UPPER_G) )
                {
                text = L"tie";
                return true;
                }
            /*idly*/
            else if (text.length() == 4 &&
                    stem<string_typeT>::is_either(text[0],
                        common_lang_constants::LOWER_I, common_lang_constants::UPPER_I) &&
                    stem<string_typeT>::is_either(text[1],
                        common_lang_constants::LOWER_D, common_lang_constants::UPPER_D) &&
                    stem<string_typeT>::is_either(text[2],
                        common_lang_constants::LOWER_L, common_lang_constants::UPPER_L) &&
                    stem<string_typeT>::is_either(text[3],
                        common_lang_constants::LOWER_Y, common_lang_constants::UPPER_Y) )
                {
                text = L"idl";
                return true;
                }
            /*gently*/
            else if (text.length() == 6 &&
                    stem<string_typeT>::is_either(text[0],
                        common_lang_constants::LOWER_G, common_lang_constants::UPPER_G) &&
                    stem<string_typeT>::is_either(text[1],
                        common_lang_constants::LOWER_E, common_lang_constants::UPPER_E) &&
                    stem<string_typeT>::is_either(text[2],
                        common_lang_constants::LOWER_N, common_lang_constants::UPPER_N) &&
                    stem<string_typeT>::is_either(text[3],
                        common_lang_constants::LOWER_T, common_lang_constants::UPPER_T) &&
                    stem<string_typeT>::is_either(text[4],
                        common_lang_constants::LOWER_L, common_lang_constants::UPPER_L) &&
                    stem<string_typeT>::is_either(text[5],
                        common_lang_constants::LOWER_Y, common_lang_constants::UPPER_Y) )
                {
                text = L"gentl";
                return true;
                }
            /*ugly*/
            else if (text.length() == 4 &&
                    stem<string_typeT>::is_either(text[0],
                        common_lang_constants::LOWER_U, common_lang_constants::UPPER_U) &&
                    stem<string_typeT>::is_either(text[1],
                        common_lang_constants::LOWER_G, common_lang_constants::UPPER_G) &&
                    stem<string_typeT>::is_either(text[2],
                        common_lang_constants::LOWER_L, common_lang_constants::UPPER_L) &&
                    stem<string_typeT>::is_either(text[3],
                        common_lang_constants::LOWER_Y, common_lang_constants::UPPER_Y) )
                {
                text = L"ugli";
                return true;
                }
            /*early*/
            else if (text.length() == 5 &&
                    stem<string_typeT>::is_either(text[0],
                        common_lang_constants::LOWER_E, common_lang_constants::UPPER_E) &&
                    stem<string_typeT>::is_either(text[1],
                            common_lang_constants::LOWER_A, common_lang_constants::UPPER_A) &&
                    stem<string_typeT>::is_either(text[2],
                        common_lang_constants::LOWER_R, common_lang_constants::UPPER_R) &&
                    stem<string_typeT>::is_either(text[3],
                        common_lang_constants::LOWER_L, common_lang_constants::UPPER_L) &&
                    stem<string_typeT>::is_either(text[4],
                        common_lang_constants::LOWER_Y, common_lang_constants::UPPER_Y) )
                {
                text = L"earli";
                return true;
                }
            /*only*/
            else if (text.length() == 4 &&
                    stem<string_typeT>::is_either(text[0],
                        common_lang_constants::LOWER_O, common_lang_constants::UPPER_O) &&
                    stem<string_typeT>::is_either(text[1],
                        common_lang_constants::LOWER_N, common_lang_constants::UPPER_N) &&
                    stem<string_typeT>::is_either(text[2],
                        common_lang_constants::LOWER_L, common_lang_constants::UPPER_L) &&
                    stem<string_typeT>::is_either(text[3],
                        common_lang_constants::LOWER_Y, common_lang_constants::UPPER_Y) )
                {
                text = L"onli";
                return true;
                }
            /*singly*/
            else if (text.length() == 6 &&
                    stem<string_typeT>::is_either(text[0],
                        common_lang_constants::LOWER_S, common_lang_constants::UPPER_S) &&
                    stem<string_typeT>::is_either(text[1],
                        common_lang_constants::LOWER_I, common_lang_constants::UPPER_I) &&
                    stem<string_typeT>::is_either(text[2],
                        common_lang_constants::LOWER_N, common_lang_constants::UPPER_N) &&
                    stem<string_typeT>::is_either(text[3],
                        common_lang_constants::LOWER_G, common_lang_constants::UPPER_G) &&
                    stem<string_typeT>::is_either(text[4],
                        common_lang_constants::LOWER_L, common_lang_constants::UPPER_L) &&
                    stem<string_typeT>::is_either(text[5],
                        common_lang_constants::LOWER_Y, common_lang_constants::UPPER_Y) )
                {
                text = L"singl";
                return true;
                }
            // exception #1
            else if (
                /*sky*/
                (text.length() == 3 &&
                    stem<string_typeT>::is_either(text[0],
                        common_lang_constants::LOWER_S, common_lang_constants::UPPER_S) &&
                    stem<string_typeT>::is_either(text[1],
                        common_lang_constants::LOWER_K, common_lang_constants::UPPER_K) &&
                    stem<string_typeT>::is_either(text[2],
                        common_lang_constants::LOWER_Y, common_lang_constants::UPPER_Y) ) ||
                /*news*/
                (text.length() == 4 &&
                    stem<string_typeT>::is_either(text[0],
                        common_lang_constants::LOWER_N, common_lang_constants::UPPER_N) &&
                    stem<string_typeT>::is_either(text[1],
                        common_lang_constants::LOWER_E, common_lang_constants::UPPER_E) &&
                    stem<string_typeT>::is_either(text[2],
                        common_lang_constants::LOWER_W, common_lang_constants::UPPER_W) &&
                    stem<string_typeT>::is_either(text[3],
                        common_lang_constants::LOWER_S, common_lang_constants::UPPER_S) ) ||
                /*howe*/
                (text.length() == 4 &&
                    stem<string_typeT>::is_either(text[0],
                        common_lang_constants::LOWER_H, common_lang_constants::UPPER_H) &&
                    stem<string_typeT>::is_either(text[1],
                        common_lang_constants::LOWER_O, common_lang_constants::UPPER_O) &&
                    stem<string_typeT>::is_either(text[2],
                        common_lang_constants::LOWER_W, common_lang_constants::UPPER_W) &&
                    stem<string_typeT>::is_either(text[3],
                        common_lang_constants::LOWER_E, common_lang_constants::UPPER_E) ) ||
                /*atlas*/
                (text.length() == 5 &&
                    stem<string_typeT>::is_either(text[0],
                        common_lang_constants::LOWER_A, common_lang_constants::UPPER_A) &&
                    stem<string_typeT>::is_either(text[1],
                        common_lang_constants::LOWER_T, common_lang_constants::UPPER_T) &&
                    stem<string_typeT>::is_either(text[2],
                        common_lang_constants::LOWER_L, common_lang_constants::UPPER_L) &&
                    stem<string_typeT>::is_either(text[3],
                        common_lang_constants::LOWER_A, common_lang_constants::UPPER_A) &&
                    stem<string_typeT>::is_either(text[4],
                        common_lang_constants::LOWER_S, common_lang_constants::UPPER_S) ) ||
                /*cosmos*/
                (text.length() == 6 &&
                    stem<string_typeT>::is_either(text[0],
                        common_lang_constants::LOWER_C, common_lang_constants::UPPER_C) &&
                    stem<string_typeT>::is_either(text[1],
                        common_lang_constants::LOWER_O, common_lang_constants::UPPER_O) &&
                    stem<string_typeT>::is_either(text[2],
                        common_lang_constants::LOWER_S, common_lang_constants::UPPER_S) &&
                    stem<string_typeT>::is_either(text[3],
                        common_lang_constants::LOWER_M, common_lang_constants::UPPER_M) &&
                    stem<string_typeT>::is_either(text[4],
                        common_lang_constants::LOWER_O, common_lang_constants::UPPER_O) &&
                    stem<string_typeT>::is_either(text[5],
                        common_lang_constants::LOWER_S, common_lang_constants::UPPER_S) ) ||
                /*bias*/
                (text.length() == 4 &&
                    stem<string_typeT>::is_either(text[0],
                        common_lang_constants::LOWER_B, common_lang_constants::UPPER_B) &&
                    stem<string_typeT>::is_either(text[1],
                        common_lang_constants::LOWER_I, common_lang_constants::UPPER_I) &&
                    stem<string_typeT>::is_either(text[2],
                        common_lang_constants::LOWER_A, common_lang_constants::UPPER_A) &&
                    stem<string_typeT>::is_either(text[3],
                        common_lang_constants::LOWER_S, common_lang_constants::UPPER_S) ) ||
                /*andes*/
                (text.length() == 5 &&
                    stem<string_typeT>::is_either(text[0],
                        common_lang_constants::LOWER_A, common_lang_constants::UPPER_A) &&
                    stem<string_typeT>::is_either(text[1],
                        common_lang_constants::LOWER_N, common_lang_constants::UPPER_N) &&
                    stem<string_typeT>::is_either(text[2],
                        common_lang_constants::LOWER_D, common_lang_constants::UPPER_D) &&
                    stem<string_typeT>::is_either(text[3],
                        common_lang_constants::LOWER_E, common_lang_constants::UPPER_E) &&
                    stem<string_typeT>::is_either(text[4],
                        common_lang_constants::LOWER_S, common_lang_constants::UPPER_S) ) )
                {
                return true;
                }
            return false;
            }

        //---------------------------------------------
        void step_1a(string_typeT& text)
            {
            if (stem<string_typeT>::is_suffix(text,
                /*sses*/
                common_lang_constants::LOWER_S, common_lang_constants::UPPER_S,
                common_lang_constants::LOWER_S, common_lang_constants::UPPER_S,
                common_lang_constants::LOWER_E, common_lang_constants::UPPER_E,
                common_lang_constants::LOWER_S, common_lang_constants::UPPER_S) )
                {
                text.erase(text.length()-2);
                stem<string_typeT>::update_r_sections(text);
                }
            else if (stem<string_typeT>::is_suffix(text,
                        /*ied*/
                        common_lang_constants::LOWER_I, common_lang_constants::UPPER_I,
                        common_lang_constants::LOWER_E, common_lang_constants::UPPER_E,
                        common_lang_constants::LOWER_D, common_lang_constants::UPPER_D) ||
                    stem<string_typeT>::is_suffix(text,
                        /*ies*/
                        common_lang_constants::LOWER_I, common_lang_constants::UPPER_I,
                        common_lang_constants::LOWER_E, common_lang_constants::UPPER_E,
                        common_lang_constants::LOWER_S, common_lang_constants::UPPER_S) )
                {
                if (text.length() == 3 || text.length() == 4)
                    {
                    text.erase(text.length()-1);
                    stem<string_typeT>::update_r_sections(text);
                    }
                else
                    {
                    text.erase(text.length()-2);
                    stem<string_typeT>::update_r_sections(text);
                    }
                }
            else if (text.length() >= 2 &&
                    stem<string_typeT>::is_either(text[text.length()-1],
                        common_lang_constants::LOWER_S, common_lang_constants::UPPER_S) &&
                    m_first_vowel < text.length()-2 &&
                    !stem<string_typeT>::is_one_of(text[text.length()-2], L"suSU") )
                {
                text.erase(text.length()-1);
                stem<string_typeT>::update_r_sections(text);
                }
            }
        //---------------------------------------------

        void step_1b(string_typeT& text)
            {
            // if the preceding word contains a vowel
            bool regress_trim = false;

            // exceptions
            if (stem<string_typeT>::is_suffix(text,
                /*eed*/
                common_lang_constants::LOWER_P, common_lang_constants::UPPER_P,
                common_lang_constants::LOWER_R, common_lang_constants::UPPER_R,
                common_lang_constants::LOWER_O, common_lang_constants::UPPER_O,
                common_lang_constants::LOWER_C, common_lang_constants::UPPER_C,
                common_lang_constants::LOWER_E, common_lang_constants::UPPER_E,
                common_lang_constants::LOWER_E, common_lang_constants::UPPER_E,
                common_lang_constants::LOWER_D, common_lang_constants::UPPER_D))
                {
                return;
                }
            else if (stem<string_typeT>::is_suffix(text,
                /*eed*/
                common_lang_constants::LOWER_S, common_lang_constants::UPPER_S,
                common_lang_constants::LOWER_U, common_lang_constants::UPPER_U,
                common_lang_constants::LOWER_C, common_lang_constants::UPPER_C,
                common_lang_constants::LOWER_C, common_lang_constants::UPPER_C,
                common_lang_constants::LOWER_E, common_lang_constants::UPPER_E,
                common_lang_constants::LOWER_E, common_lang_constants::UPPER_E,
                common_lang_constants::LOWER_D, common_lang_constants::UPPER_D))
                {
                return;
                }
            else if (stem<string_typeT>::is_suffix(text,
                /*eed*/
                common_lang_constants::LOWER_E, common_lang_constants::UPPER_E,
                common_lang_constants::LOWER_X, common_lang_constants::UPPER_X,
                common_lang_constants::LOWER_C, common_lang_constants::UPPER_C,
                common_lang_constants::LOWER_E, common_lang_constants::UPPER_E,
                common_lang_constants::LOWER_E, common_lang_constants::UPPER_E,
                common_lang_constants::LOWER_D, common_lang_constants::UPPER_D))
                {
                return;
                }
            else if (stem<string_typeT>::is_suffix(text,
                /*eedly*/
                common_lang_constants::LOWER_P, common_lang_constants::UPPER_P,
                common_lang_constants::LOWER_R, common_lang_constants::UPPER_R,
                common_lang_constants::LOWER_O, common_lang_constants::UPPER_O,
                common_lang_constants::LOWER_C, common_lang_constants::UPPER_C,
                common_lang_constants::LOWER_E, common_lang_constants::UPPER_E,
                common_lang_constants::LOWER_E, common_lang_constants::UPPER_E,
                common_lang_constants::LOWER_D, common_lang_constants::UPPER_D,
                common_lang_constants::LOWER_L, common_lang_constants::UPPER_L,
                common_lang_constants::LOWER_Y, common_lang_constants::UPPER_Y))
                {
                return;
                }
            else if (stem<string_typeT>::is_suffix(text,
                /*eedly*/
                common_lang_constants::LOWER_S, common_lang_constants::UPPER_S,
                common_lang_constants::LOWER_U, common_lang_constants::UPPER_U,
                common_lang_constants::LOWER_C, common_lang_constants::UPPER_C,
                common_lang_constants::LOWER_C, common_lang_constants::UPPER_C,
                common_lang_constants::LOWER_E, common_lang_constants::UPPER_E,
                common_lang_constants::LOWER_E, common_lang_constants::UPPER_E,
                common_lang_constants::LOWER_D, common_lang_constants::UPPER_D,
                common_lang_constants::LOWER_L, common_lang_constants::UPPER_L,
                common_lang_constants::LOWER_Y, common_lang_constants::UPPER_Y))
                {
                return;
                }
            else if (stem<string_typeT>::is_suffix(text,
                /*eedly*/
                common_lang_constants::LOWER_E, common_lang_constants::UPPER_E,
                common_lang_constants::LOWER_X, common_lang_constants::UPPER_X,
                common_lang_constants::LOWER_C, common_lang_constants::UPPER_C,
                common_lang_constants::LOWER_E, common_lang_constants::UPPER_E,
                common_lang_constants::LOWER_E, common_lang_constants::UPPER_E,
                common_lang_constants::LOWER_D, common_lang_constants::UPPER_D,
                common_lang_constants::LOWER_L, common_lang_constants::UPPER_L,
                common_lang_constants::LOWER_Y, common_lang_constants::UPPER_Y))
                {
                return;
                }

            if (stem<string_typeT>::is_suffix(text,
                /*eed*/
                common_lang_constants::LOWER_E, common_lang_constants::UPPER_E,
                common_lang_constants::LOWER_E, common_lang_constants::UPPER_E,
                common_lang_constants::LOWER_D, common_lang_constants::UPPER_D) )
                {
                if (stem<string_typeT>::get_r1() <= text.length()-3)
                    {
                    text.erase(text.length()-1);
                    stem<string_typeT>::update_r_sections(text);
                    }
                }
            else if (stem<string_typeT>::is_suffix(text,
                /*eedly*/
                common_lang_constants::LOWER_E, common_lang_constants::UPPER_E,
                common_lang_constants::LOWER_E, common_lang_constants::UPPER_E,
                common_lang_constants::LOWER_D, common_lang_constants::UPPER_D,
                common_lang_constants::LOWER_L, common_lang_constants::UPPER_L,
                common_lang_constants::LOWER_Y, common_lang_constants::UPPER_Y) )
                {
                if (stem<string_typeT>::get_r1() <= text.length()-5)
                    {
                    text.erase(text.length()-3);
                    stem<string_typeT>::update_r_sections(text);
                    }
                }
            else if (stem<string_typeT>::is_suffix(text,
                /*ed*/
                common_lang_constants::LOWER_E, common_lang_constants::UPPER_E,
                common_lang_constants::LOWER_D, common_lang_constants::UPPER_D) &&
                m_first_vowel < text.length()-2)
                {
                text.erase(text.length()-2);
                stem<string_typeT>::update_r_sections(text);
                regress_trim = true;
                }
            else if (stem<string_typeT>::is_suffix(text,
                /*edly*/
                common_lang_constants::LOWER_E, common_lang_constants::UPPER_E,
                common_lang_constants::LOWER_D, common_lang_constants::UPPER_D,
                common_lang_constants::LOWER_L, common_lang_constants::UPPER_L,
                common_lang_constants::LOWER_Y, common_lang_constants::UPPER_Y) &&
                m_first_vowel < text.length()-4)
                {
                text.erase(text.length()-4);
                stem<string_typeT>::update_r_sections(text);
                regress_trim = true;
                }
            else if (stem<string_typeT>::is_suffix(text,
                /*ing*/
                common_lang_constants::LOWER_I, common_lang_constants::UPPER_I,
                common_lang_constants::LOWER_N, common_lang_constants::UPPER_N,
                common_lang_constants::LOWER_G, common_lang_constants::UPPER_G) &&
                m_first_vowel < text.length()-3)
                {
                if (text.length() == 5 &&
                    stem<string_typeT>::is_either(text[text.length() - 4],
                        common_lang_constants::LOWER_Y, LOWER_Y_HASH) &&
                    !is_vowel(text[text.length() - 5]))
                    {
                    text.erase(text.length() - 2);
                    text[text.length() - 2] = common_lang_constants::LOWER_I;
                    text[text.length() - 1] = common_lang_constants::LOWER_E;
                    stem<string_typeT>::update_r_sections(text);
                    return;
                    }
                else if (text.length() == 6 &&
                    ((stem<string_typeT>::is_either(text[0],
                        common_lang_constants::LOWER_I, common_lang_constants::UPPER_I) &&
                     stem<string_typeT>::is_either(text[1],
                        common_lang_constants::LOWER_N, common_lang_constants::UPPER_N) &&
                     stem<string_typeT>::is_either(text[2],
                        common_lang_constants::LOWER_N, common_lang_constants::UPPER_N)) ||

                    (stem<string_typeT>::is_either(text[0],
                        common_lang_constants::LOWER_O, common_lang_constants::UPPER_O) &&
                     stem<string_typeT>::is_either(text[1],
                        common_lang_constants::LOWER_U, common_lang_constants::UPPER_U) &&
                     stem<string_typeT>::is_either(text[2],
                        common_lang_constants::LOWER_T, common_lang_constants::UPPER_T))))
                    {
                    return;
                    }
                else if (text.length() == 7 &&
                    ((stem<string_typeT>::is_either(text[0],
                        common_lang_constants::LOWER_C, common_lang_constants::UPPER_C) &&
                     stem<string_typeT>::is_either(text[1],
                        common_lang_constants::LOWER_A, common_lang_constants::UPPER_A) &&
                     stem<string_typeT>::is_either(text[2],
                        common_lang_constants::LOWER_N, common_lang_constants::UPPER_N) &&
                     stem<string_typeT>::is_either(text[3],
                        common_lang_constants::LOWER_N, common_lang_constants::UPPER_N)) ||

                    (stem<string_typeT>::is_either(text[0],
                        common_lang_constants::LOWER_H, common_lang_constants::UPPER_H) &&
                     stem<string_typeT>::is_either(text[1],
                        common_lang_constants::LOWER_E, common_lang_constants::UPPER_E) &&
                     stem<string_typeT>::is_either(text[2],
                        common_lang_constants::LOWER_R, common_lang_constants::UPPER_R) &&
                     stem<string_typeT>::is_either(text[3],
                        common_lang_constants::LOWER_R, common_lang_constants::UPPER_R)) ||

                    (stem<string_typeT>::is_either(text[0],
                        common_lang_constants::LOWER_E, common_lang_constants::UPPER_E) &&
                     stem<string_typeT>::is_either(text[1],
                        common_lang_constants::LOWER_V, common_lang_constants::UPPER_V) &&
                     stem<string_typeT>::is_either(text[2],
                        common_lang_constants::LOWER_E, common_lang_constants::UPPER_E) &&
                     stem<string_typeT>::is_either(text[3],
                        common_lang_constants::LOWER_N, common_lang_constants::UPPER_N)) ||
                        
                    (stem<string_typeT>::is_either(text[0],
                        common_lang_constants::LOWER_E, common_lang_constants::UPPER_E) &&
                     stem<string_typeT>::is_either(text[1],
                        common_lang_constants::LOWER_A, common_lang_constants::UPPER_A) &&
                     stem<string_typeT>::is_either(text[2],
                        common_lang_constants::LOWER_R, common_lang_constants::UPPER_R) &&
                     stem<string_typeT>::is_either(text[3],
                        common_lang_constants::LOWER_R, common_lang_constants::UPPER_R))))
                    {
                    return;
                    }
                text.erase(text.length() - 3);
                stem<string_typeT>::update_r_sections(text);
                regress_trim = true;
                }
            else if (stem<string_typeT>::is_suffix(text,
                /*ingly*/
                common_lang_constants::LOWER_I, common_lang_constants::UPPER_I,
                common_lang_constants::LOWER_N, common_lang_constants::UPPER_N,
                common_lang_constants::LOWER_G, common_lang_constants::UPPER_G,
                common_lang_constants::LOWER_L, common_lang_constants::UPPER_L,
                common_lang_constants::LOWER_Y, common_lang_constants::UPPER_Y) &&
                m_first_vowel < text.length()-5)
                {
                text.erase(text.length()-5);
                stem<string_typeT>::update_r_sections(text);
                regress_trim = true;
                }
            if (regress_trim)
                {
                const bool isExactly3NotAEOStart
                    {
                    text.length() == 3 &&
                    !(stem<string_typeT>::is_either(text[0],
                        common_lang_constants::LOWER_A, common_lang_constants::UPPER_A) ||
                      stem<string_typeT>::is_either(text[0],
                         common_lang_constants::LOWER_E, common_lang_constants::UPPER_E) ||
                      stem<string_typeT>::is_either(text[0],
                         common_lang_constants::LOWER_O, common_lang_constants::UPPER_O))
                    };
                if (stem<string_typeT>::is_suffix(text,
                        /*at*/common_lang_constants::LOWER_A, common_lang_constants::UPPER_A,
                        common_lang_constants::LOWER_T, common_lang_constants::UPPER_T) ||
                    stem<string_typeT>::is_suffix(text,
                        /*bl*/common_lang_constants::LOWER_B, common_lang_constants::UPPER_B,
                        common_lang_constants::LOWER_L, common_lang_constants::UPPER_L) ||
                    stem<string_typeT>::is_suffix(text,
                        /*iz*/common_lang_constants::LOWER_I, common_lang_constants::UPPER_I,
                        common_lang_constants::LOWER_Z, common_lang_constants::UPPER_Z) )
                    {
                    text += common_lang_constants::LOWER_E;
                    // need to search for r2 again because the 'e' added here may change that
                    stem<string_typeT>::find_r2(text, L"aeiouyAEIOUY");
                    }
                // undouble
                else if ((text.length() > 3 || isExactly3NotAEOStart) &&
                        (stem<string_typeT>::is_suffix(text,
                            /*bb*/
                            common_lang_constants::LOWER_B, common_lang_constants::UPPER_B,
                            common_lang_constants::LOWER_B, common_lang_constants::UPPER_B) ||
                        stem<string_typeT>::is_suffix(text,
                            /*dd*/
                            common_lang_constants::LOWER_D, common_lang_constants::UPPER_D,
                            common_lang_constants::LOWER_D, common_lang_constants::UPPER_D) ||
                        stem<string_typeT>::is_suffix(text,
                            /*ff*/
                            common_lang_constants::LOWER_F, common_lang_constants::UPPER_F,
                            common_lang_constants::LOWER_F, common_lang_constants::UPPER_F) ||
                        stem<string_typeT>::is_suffix(text,
                            /*gg*/
                            common_lang_constants::LOWER_G, common_lang_constants::UPPER_G,
                            common_lang_constants::LOWER_G, common_lang_constants::UPPER_G) ||
                        stem<string_typeT>::is_suffix(text,
                            /*mm*/common_lang_constants::LOWER_M, common_lang_constants::UPPER_M,
                            common_lang_constants::LOWER_M, common_lang_constants::UPPER_M) ||
                        stem<string_typeT>::is_suffix(text,
                            /*nn*/
                            common_lang_constants::LOWER_N, common_lang_constants::UPPER_N,
                            common_lang_constants::LOWER_N, common_lang_constants::UPPER_N) ||
                        stem<string_typeT>::is_suffix(text,
                            /*pp*/
                            common_lang_constants::LOWER_P, common_lang_constants::UPPER_P,
                            common_lang_constants::LOWER_P, common_lang_constants::UPPER_P) ||
                        stem<string_typeT>::is_suffix(text,
                            /*rr*/
                            common_lang_constants::LOWER_R, common_lang_constants::UPPER_R,
                            common_lang_constants::LOWER_R, common_lang_constants::UPPER_R) ||
                        stem<string_typeT>::is_suffix(text,
                            /*tt*/
                            common_lang_constants::LOWER_T, common_lang_constants::UPPER_T,
                            common_lang_constants::LOWER_T, common_lang_constants::UPPER_T)) )
                    {
                    text.erase(text.length()-1);
                    stem<string_typeT>::update_r_sections(text);
                    }
                else if ((text.length() < 2 ||
                          stem<string_typeT>::tolower_western(text[text.length() - 1]) !=
                          stem<string_typeT>::tolower_western(text[text.length() - 2]) ) &&
                    is_short_word(text, text.length() ) )
                    {
                    text += common_lang_constants::LOWER_E;
                    // need to search for R2 again because the 'e' added here may change that
                    stem<string_typeT>::find_r2(text, L"aeiouyAEIOUY");
                    }
                }
            }
        //---------------------------------------------

        //---------------------------------------------
        void step_1c(string_typeT& text)
            {
            // proceeding consonant cannot be first letter in word
            if (text.length() > 2 &&
                !is_vowel(text[text.length()-2]) )
                {
                if (stem<string_typeT>::is_either(text[text.length()-1],
                    common_lang_constants::LOWER_Y, LOWER_Y_HASH) )
                    {
                    text[text.length()-1] = common_lang_constants::LOWER_I;
                    }
                else if (stem<string_typeT>::is_either(text[text.length()-1],
                    common_lang_constants::UPPER_Y, UPPER_Y_HASH) )
                    {
                    text[text.length()-1] = common_lang_constants::UPPER_I;
                    }
                }
            }

        //---------------------------------------------
        void step_2(string_typeT& text)
            {
            if (text.length() >= 7 &&
                (stem<string_typeT>::is_suffix(text,
                    /*ization*/
                    common_lang_constants::LOWER_I, common_lang_constants::UPPER_I,
                    common_lang_constants::LOWER_Z, common_lang_constants::UPPER_Z,
                    common_lang_constants::LOWER_A, common_lang_constants::UPPER_A,
                    common_lang_constants::LOWER_T, common_lang_constants::UPPER_T,
                    common_lang_constants::LOWER_I, common_lang_constants::UPPER_I,
                    common_lang_constants::LOWER_O, common_lang_constants::UPPER_O,
                    common_lang_constants::LOWER_N, common_lang_constants::UPPER_N) ||
                stem<string_typeT>::is_suffix(text,
                    /*ational*/
                    common_lang_constants::LOWER_A, common_lang_constants::UPPER_A,
                    common_lang_constants::LOWER_T, common_lang_constants::UPPER_T,
                    common_lang_constants::LOWER_I, common_lang_constants::UPPER_I,
                    common_lang_constants::LOWER_O, common_lang_constants::UPPER_O,
                    common_lang_constants::LOWER_N, common_lang_constants::UPPER_N,
                    common_lang_constants::LOWER_A, common_lang_constants::UPPER_A,
                    common_lang_constants::LOWER_L, common_lang_constants::UPPER_L) ) )
                {
                if (stem<string_typeT>::get_r1() <= text.length()-7)
                    {
                    text.erase(text.length()-4);
                    text[static_cast<int>(text.length()-1)] = common_lang_constants::LOWER_E;
                    stem<string_typeT>::update_r_sections(text);
                    }
                }
            else if (text.length() >= 7 &&
                (stem<string_typeT>::is_suffix(text,
                    /*fulness*/
                    common_lang_constants::LOWER_F, common_lang_constants::UPPER_F,
                    common_lang_constants::LOWER_U, common_lang_constants::UPPER_U,
                    common_lang_constants::LOWER_L, common_lang_constants::UPPER_L,
                    common_lang_constants::LOWER_N, common_lang_constants::UPPER_N,
                    common_lang_constants::LOWER_E, common_lang_constants::UPPER_E,
                    common_lang_constants::LOWER_S, common_lang_constants::UPPER_S,
                    common_lang_constants::LOWER_S, common_lang_constants::UPPER_S) ||
                stem<string_typeT>::is_suffix(text,
                    /*ousness*/
                    common_lang_constants::LOWER_O, common_lang_constants::UPPER_O,
                    common_lang_constants::LOWER_U, common_lang_constants::UPPER_U,
                    common_lang_constants::LOWER_S, common_lang_constants::UPPER_S,
                    common_lang_constants::LOWER_N, common_lang_constants::UPPER_N,
                    common_lang_constants::LOWER_E, common_lang_constants::UPPER_E,
                    common_lang_constants::LOWER_S, common_lang_constants::UPPER_S,
                    common_lang_constants::LOWER_S, common_lang_constants::UPPER_S) ||
                stem<string_typeT>::is_suffix(text,
                    /*iveness*/
                    common_lang_constants::LOWER_I, common_lang_constants::UPPER_I,
                    common_lang_constants::LOWER_V, common_lang_constants::UPPER_V,
                    common_lang_constants::LOWER_E, common_lang_constants::UPPER_E,
                    common_lang_constants::LOWER_N, common_lang_constants::UPPER_N,
                    common_lang_constants::LOWER_E, common_lang_constants::UPPER_E,
                    common_lang_constants::LOWER_S, common_lang_constants::UPPER_S,
                    common_lang_constants::LOWER_S, common_lang_constants::UPPER_S) ) )
                {
                if (stem<string_typeT>::get_r1() <= text.length()-7)
                    {
                    text.erase(text.length()-4);
                    stem<string_typeT>::update_r_sections(text);
                    }
                }
            else if (text.length() >= 6 &&
                (stem<string_typeT>::is_suffix(text,
                    /*tional*/
                    common_lang_constants::LOWER_T, common_lang_constants::UPPER_T,
                    common_lang_constants::LOWER_I, common_lang_constants::UPPER_I,
                    common_lang_constants::LOWER_O, common_lang_constants::UPPER_O,
                    common_lang_constants::LOWER_N, common_lang_constants::UPPER_N,
                    common_lang_constants::LOWER_A, common_lang_constants::UPPER_A,
                    common_lang_constants::LOWER_L, common_lang_constants::UPPER_L) ||
                stem<string_typeT>::is_suffix(text,
                    /*lessli*/
                    common_lang_constants::LOWER_L, common_lang_constants::UPPER_L,
                    common_lang_constants::LOWER_E, common_lang_constants::UPPER_E,
                    common_lang_constants::LOWER_S, common_lang_constants::UPPER_S,
                    common_lang_constants::LOWER_S, common_lang_constants::UPPER_S,
                    common_lang_constants::LOWER_L, common_lang_constants::UPPER_L,
                    common_lang_constants::LOWER_I, common_lang_constants::UPPER_I) ) )
                {
                if (stem<string_typeT>::get_r1() <= text.length()-6)
                    {
                    text.erase(text.length()-2);
                    stem<string_typeT>::update_r_sections(text);
                    }
                }
            else if (text.length() >= 6 &&
                stem<string_typeT>::is_suffix(text,
                    /*biliti*/
                    common_lang_constants::LOWER_B, common_lang_constants::UPPER_B,
                    common_lang_constants::LOWER_I, common_lang_constants::UPPER_I,
                    common_lang_constants::LOWER_L, common_lang_constants::UPPER_L,
                    common_lang_constants::LOWER_I, common_lang_constants::UPPER_I,
                    common_lang_constants::LOWER_T, common_lang_constants::UPPER_T,
                    common_lang_constants::LOWER_I, common_lang_constants::UPPER_I) )
                {
                if (stem<string_typeT>::get_r1() <= text.length()-6)
                    {
                    text.erase(text.length()-3);
                    text[text.length()-2] = common_lang_constants::LOWER_L;
                    text[text.length()-1] = common_lang_constants::LOWER_E;
                    stem<string_typeT>::update_r_sections(text);
                    }
                }
            else if (text.length() >= 5 &&
                (stem<string_typeT>::is_suffix(text,
                    /*iviti*/
                    common_lang_constants::LOWER_I, common_lang_constants::UPPER_I,
                    common_lang_constants::LOWER_V, common_lang_constants::UPPER_V,
                    common_lang_constants::LOWER_I, common_lang_constants::UPPER_I,
                    common_lang_constants::LOWER_T, common_lang_constants::UPPER_T,
                    common_lang_constants::LOWER_I, common_lang_constants::UPPER_I) ||
                stem<string_typeT>::is_suffix(text,
                    /*ation*/
                    common_lang_constants::LOWER_A, common_lang_constants::UPPER_A,
                    common_lang_constants::LOWER_T, common_lang_constants::UPPER_T,
                    common_lang_constants::LOWER_I, common_lang_constants::UPPER_I,
                    common_lang_constants::LOWER_O, common_lang_constants::UPPER_O,
                    common_lang_constants::LOWER_N, common_lang_constants::UPPER_N) ) )
                {
                if (stem<string_typeT>::get_r1() <= text.length()-5)
                    {
                    text.erase(text.length()-2);
                    text[text.length()-1] = common_lang_constants::LOWER_E;
                    stem<string_typeT>::update_r_sections(text);
                    }
                }
            else if (text.length() >= 5 &&
                (stem<string_typeT>::is_suffix(text,
                    /*alism*/
                    common_lang_constants::LOWER_A, common_lang_constants::UPPER_A,
                    common_lang_constants::LOWER_L, common_lang_constants::UPPER_L,
                    common_lang_constants::LOWER_I, common_lang_constants::UPPER_I,
                    common_lang_constants::LOWER_S, common_lang_constants::UPPER_S,
                    common_lang_constants::LOWER_M, common_lang_constants::UPPER_M) ||
                stem<string_typeT>::is_suffix(text,
                    /*aliti*/
                    common_lang_constants::LOWER_A, common_lang_constants::UPPER_A,
                    common_lang_constants::LOWER_L, common_lang_constants::UPPER_L,
                    common_lang_constants::LOWER_I, common_lang_constants::UPPER_I,
                    common_lang_constants::LOWER_T, common_lang_constants::UPPER_T,
                    common_lang_constants::LOWER_I, common_lang_constants::UPPER_I) ||
                stem<string_typeT>::is_suffix(text,
                    /*ogist*/
                    common_lang_constants::LOWER_O, common_lang_constants::UPPER_O,
                    common_lang_constants::LOWER_G, common_lang_constants::UPPER_G,
                    common_lang_constants::LOWER_I, common_lang_constants::UPPER_I,
                    common_lang_constants::LOWER_S, common_lang_constants::UPPER_S,
                    common_lang_constants::LOWER_T, common_lang_constants::UPPER_T)) )
                {
                if (stem<string_typeT>::get_r1() <= text.length() - 5)
                    {
                    text.erase(text.length() - 3);
                    stem<string_typeT>::update_r_sections(text);
                    }
                }
            else if (text.length() >= 5 &&
                (stem<string_typeT>::is_suffix(text,
                    /*ousli*/
                    common_lang_constants::LOWER_O, common_lang_constants::UPPER_O,
                    common_lang_constants::LOWER_U, common_lang_constants::UPPER_U,
                    common_lang_constants::LOWER_S, common_lang_constants::UPPER_S,
                    common_lang_constants::LOWER_L, common_lang_constants::UPPER_L,
                    common_lang_constants::LOWER_I, common_lang_constants::UPPER_I) ||
                stem<string_typeT>::is_suffix(text,
                    /*entli*/
                    common_lang_constants::LOWER_E, common_lang_constants::UPPER_E,
                    common_lang_constants::LOWER_N, common_lang_constants::UPPER_N,
                    common_lang_constants::LOWER_T, common_lang_constants::UPPER_T,
                    common_lang_constants::LOWER_L, common_lang_constants::UPPER_L,
                    common_lang_constants::LOWER_I, common_lang_constants::UPPER_I) ||
                stem<string_typeT>::is_suffix(text,
                    /*fulli*/
                    common_lang_constants::LOWER_F, common_lang_constants::UPPER_F,
                    common_lang_constants::LOWER_U, common_lang_constants::UPPER_U,
                    common_lang_constants::LOWER_L, common_lang_constants::UPPER_L,
                    common_lang_constants::LOWER_L, common_lang_constants::UPPER_L,
                    common_lang_constants::LOWER_I, common_lang_constants::UPPER_I) ) )
                {
                if (stem<string_typeT>::get_r1() <= text.length()-5)
                    {
                    text.erase(text.length()-2);
                    stem<string_typeT>::update_r_sections(text);
                    }
                }
            else if (text.length() >= 4 && stem<string_typeT>::is_suffix(text,
                /*alli*/
                common_lang_constants::LOWER_A, common_lang_constants::UPPER_A,
                common_lang_constants::LOWER_L, common_lang_constants::UPPER_L,
                common_lang_constants::LOWER_L, common_lang_constants::UPPER_L,
                common_lang_constants::LOWER_I, common_lang_constants::UPPER_I) )
                {
                if (stem<string_typeT>::get_r1() <= text.length()-4)
                    {
                    text.erase(text.length()-2);
                    stem<string_typeT>::update_r_sections(text);
                    }
                }
            else if (text.length() >= 4 &&
                (stem<string_typeT>::is_suffix(text,
                    /*enci*/
                    common_lang_constants::LOWER_E, common_lang_constants::UPPER_E,
                    common_lang_constants::LOWER_N, common_lang_constants::UPPER_N,
                    common_lang_constants::LOWER_C, common_lang_constants::UPPER_C,
                    common_lang_constants::LOWER_I, common_lang_constants::UPPER_I) ||
                stem<string_typeT>::is_suffix(text,
                    /*anci*/
                    common_lang_constants::LOWER_A, common_lang_constants::UPPER_A,
                    common_lang_constants::LOWER_N, common_lang_constants::UPPER_N,
                    common_lang_constants::LOWER_C, common_lang_constants::UPPER_C,
                    common_lang_constants::LOWER_I, common_lang_constants::UPPER_I) ||
                stem<string_typeT>::is_suffix(text,
                    /*abli*/
                    common_lang_constants::LOWER_A, common_lang_constants::UPPER_A,
                    common_lang_constants::LOWER_B, common_lang_constants::UPPER_B,
                    common_lang_constants::LOWER_L, common_lang_constants::UPPER_L,
                    common_lang_constants::LOWER_I, common_lang_constants::UPPER_I) ) )
                {
                if (stem<string_typeT>::get_r1() <= text.length()-4)
                    {
                    text[text.length()-1] = common_lang_constants::LOWER_E;
                    }
                }
            else if (text.length() >= 4 && stem<string_typeT>::is_suffix(text,
                /*izer*/
                common_lang_constants::LOWER_I, common_lang_constants::UPPER_I,
                common_lang_constants::LOWER_Z, common_lang_constants::UPPER_Z,
                common_lang_constants::LOWER_E, common_lang_constants::UPPER_E,
                common_lang_constants::LOWER_R, common_lang_constants::UPPER_R) )
                {
                if (stem<string_typeT>::get_r1() <= text.length()-4)
                    {
                    text.erase(text.length()-1);
                    stem<string_typeT>::update_r_sections(text);
                    }
                }
            else if (text.length() >= 4 &&
                stem<string_typeT>::is_suffix(text,
                    /*ator*/
                    common_lang_constants::LOWER_A, common_lang_constants::UPPER_A,
                    common_lang_constants::LOWER_T, common_lang_constants::UPPER_T,
                    common_lang_constants::LOWER_O, common_lang_constants::UPPER_O,
                    common_lang_constants::LOWER_R, common_lang_constants::UPPER_R) )
                {
                if (stem<string_typeT>::get_r1() <= text.length()-4)
                    {
                    text.erase(text.length()-1);
                    text[text.length()-1] = common_lang_constants::LOWER_E;
                    stem<string_typeT>::update_r_sections(text);
                    }
                }
            else if (text.length() >= 3 &&
                stem<string_typeT>::get_r1() <= (text.length()-3) &&
                stem<string_typeT>::is_suffix(text,
                    /*bli*/
                    common_lang_constants::LOWER_B, common_lang_constants::UPPER_B,
                    common_lang_constants::LOWER_L, common_lang_constants::UPPER_L,
                    common_lang_constants::LOWER_I, common_lang_constants::UPPER_I) )
                {
                text[text.length()-1] = common_lang_constants::LOWER_E;
                }
            else if (text.length() >= 3 &&
                stem<string_typeT>::get_r1() <= (text.length()-3) &&
                stem<string_typeT>::is_suffix(text,
                    /*ogi*/
                    common_lang_constants::LOWER_O, common_lang_constants::UPPER_O,
                    common_lang_constants::LOWER_G, common_lang_constants::UPPER_G,
                    common_lang_constants::LOWER_I, common_lang_constants::UPPER_I) )
                {
                if (stem<string_typeT>::is_either(text[text.length()-4],
                    common_lang_constants::LOWER_L, common_lang_constants::UPPER_L) )
                    {
                    text.erase(text.length()-1);
                    stem<string_typeT>::update_r_sections(text);
                    }
                }
            else if (text.length() >= 3 &&
                    stem<string_typeT>::get_r1() <= (text.length()-2) &&
                    stem<string_typeT>::is_suffix(text,
                        /*li*/
                        common_lang_constants::LOWER_L, common_lang_constants::UPPER_L,
                        common_lang_constants::LOWER_I, common_lang_constants::UPPER_I) )
                {
                if (stem<string_typeT>::is_one_of(text[text.length()-3], L"cdeghkmnrtCDEGHKMNRT") )
                    {
                    text.erase(text.length()-2);
                    stem<string_typeT>::update_r_sections(text);
                    }
                }
            }

        //---------------------------------------------
        void step_3(string_typeT& text)
            {
            if (text.length() >= 7 && stem<string_typeT>::is_suffix(text,
                /*ational*/
                common_lang_constants::LOWER_A, common_lang_constants::UPPER_A,
                common_lang_constants::LOWER_T, common_lang_constants::UPPER_T,
                common_lang_constants::LOWER_I, common_lang_constants::UPPER_I,
                common_lang_constants::LOWER_O, common_lang_constants::UPPER_O,
                common_lang_constants::LOWER_N, common_lang_constants::UPPER_N,
                common_lang_constants::LOWER_A, common_lang_constants::UPPER_A,
                common_lang_constants::LOWER_L, common_lang_constants::UPPER_L) )
                {
                if (stem<string_typeT>::get_r1() <= text.length()-7)
                    {
                    text.erase(text.length()-4);
                    text[text.length()-1] = common_lang_constants::LOWER_E;
                    stem<string_typeT>::update_r_sections(text);
                    }
                }
            else if (text.length() >= 6 && stem<string_typeT>::is_suffix(text,
                /*tional*/
                common_lang_constants::LOWER_T, common_lang_constants::UPPER_T,
                common_lang_constants::LOWER_I, common_lang_constants::UPPER_I,
                common_lang_constants::LOWER_O, common_lang_constants::UPPER_O,
                common_lang_constants::LOWER_N, common_lang_constants::UPPER_N,
                common_lang_constants::LOWER_A, common_lang_constants::UPPER_A,
                common_lang_constants::LOWER_L, common_lang_constants::UPPER_L) )
                {
                if (stem<string_typeT>::get_r1() <= text.length()-6)
                    {
                    text.erase(text.length()-2);
                    stem<string_typeT>::update_r_sections(text);
                    }
                }
            else if (text.length() >= 5 &&
                (stem<string_typeT>::is_suffix(text,
                    /*icate*/
                    common_lang_constants::LOWER_I, common_lang_constants::UPPER_I,
                    common_lang_constants::LOWER_C, common_lang_constants::UPPER_C,
                    common_lang_constants::LOWER_A, common_lang_constants::UPPER_A,
                    common_lang_constants::LOWER_T, common_lang_constants::UPPER_T,
                    common_lang_constants::LOWER_E, common_lang_constants::UPPER_E) ||
                stem<string_typeT>::is_suffix(text,
                    /*iciti*/
                    common_lang_constants::LOWER_I, common_lang_constants::UPPER_I,
                    common_lang_constants::LOWER_C, common_lang_constants::UPPER_C,
                    common_lang_constants::LOWER_I, common_lang_constants::UPPER_I,
                    common_lang_constants::LOWER_T, common_lang_constants::UPPER_T,
                    common_lang_constants::LOWER_I, common_lang_constants::UPPER_I) ||
                stem<string_typeT>::is_suffix(text,
                    /*alize*/
                    common_lang_constants::LOWER_A, common_lang_constants::UPPER_A,
                    common_lang_constants::LOWER_L, common_lang_constants::UPPER_L,
                    common_lang_constants::LOWER_I, common_lang_constants::UPPER_I,
                    common_lang_constants::LOWER_Z, common_lang_constants::UPPER_Z,
                    common_lang_constants::LOWER_E, common_lang_constants::UPPER_E) ) )
                {
                if (stem<string_typeT>::get_r1() <= text.length()-5)
                    {
                    text.erase(text.length()-3);
                    stem<string_typeT>::update_r_sections(text);
                    }
                }
            else if (text.length() >= 5 && stem<string_typeT>::is_suffix(text,
                /*ative*/
                common_lang_constants::LOWER_A, common_lang_constants::UPPER_A,
                common_lang_constants::LOWER_T, common_lang_constants::UPPER_T,
                common_lang_constants::LOWER_I, common_lang_constants::UPPER_I,
                common_lang_constants::LOWER_V, common_lang_constants::UPPER_V,
                common_lang_constants::LOWER_E, common_lang_constants::UPPER_E) )
                {
                if (stem<string_typeT>::get_r2() <= text.length()-5)
                    {
                    text.erase(text.length()-5);
                    stem<string_typeT>::update_r_sections(text);
                    }
                }
            else if (text.length() >= 4 && stem<string_typeT>::is_suffix(text,
                /*ical*/
                common_lang_constants::LOWER_I, common_lang_constants::UPPER_I,
                common_lang_constants::LOWER_C, common_lang_constants::UPPER_C,
                common_lang_constants::LOWER_A, common_lang_constants::UPPER_A,
                common_lang_constants::LOWER_L, common_lang_constants::UPPER_L) )
                {
                if (stem<string_typeT>::get_r1() <= text.length()-4)
                    {
                    text.erase(text.length()-2);
                    stem<string_typeT>::update_r_sections(text);
                    }
                }
            else if (text.length() >= 4 && stem<string_typeT>::is_suffix(text,
                /*ness*/
                common_lang_constants::LOWER_N, common_lang_constants::UPPER_N,
                common_lang_constants::LOWER_E, common_lang_constants::UPPER_E,
                common_lang_constants::LOWER_S, common_lang_constants::UPPER_S,
                common_lang_constants::LOWER_S, common_lang_constants::UPPER_S) )
                {
                if (stem<string_typeT>::get_r1() <= text.length()-4)
                    {
                    text.erase(text.length()-4);
                    stem<string_typeT>::update_r_sections(text);
                    }
                }
            else if (text.length() >= 3 && stem<string_typeT>::is_suffix(text,
                /*ful*/
                common_lang_constants::LOWER_F, common_lang_constants::UPPER_F,
                common_lang_constants::LOWER_U, common_lang_constants::UPPER_U,
                common_lang_constants::LOWER_L, common_lang_constants::UPPER_L) )
                {
                if (stem<string_typeT>::get_r1() <= text.length()-3)
                    {
                    text.erase(text.length()-3);
                    stem<string_typeT>::update_r_sections(text);
                    }
                }
            }

        //---------------------------------------------
        void step_4(string_typeT& text)
            {
            if (text.length() >= 5 &&
                stem<string_typeT>::is_suffix(text,
                    /*ement*/
                    common_lang_constants::LOWER_E, common_lang_constants::UPPER_E,
                    common_lang_constants::LOWER_M, common_lang_constants::UPPER_M,
                    common_lang_constants::LOWER_E, common_lang_constants::UPPER_E,
                    common_lang_constants::LOWER_N, common_lang_constants::UPPER_N,
                    common_lang_constants::LOWER_T, common_lang_constants::UPPER_T) )
                {
                if (stem<string_typeT>::get_r2() <= text.length()-5)
                    {
                    text.erase(text.length()-5);
                    stem<string_typeT>::update_r_sections(text);
                    }
                }
            else if (text.length() >= 4 &&
                    (stem<string_typeT>::is_suffix(text,
                        /*able*/
                        common_lang_constants::LOWER_A, common_lang_constants::UPPER_A,
                        common_lang_constants::LOWER_B, common_lang_constants::UPPER_B,
                        common_lang_constants::LOWER_L, common_lang_constants::UPPER_L,
                        common_lang_constants::LOWER_E, common_lang_constants::UPPER_E) ||
                    stem<string_typeT>::is_suffix(text,
                        /*ible*/
                        common_lang_constants::LOWER_I, common_lang_constants::UPPER_I,
                        common_lang_constants::LOWER_B, common_lang_constants::UPPER_B,
                        common_lang_constants::LOWER_L, common_lang_constants::UPPER_L,
                        common_lang_constants::LOWER_E, common_lang_constants::UPPER_E) ||
                    stem<string_typeT>::is_suffix(text,
                        /*ment*/
                        common_lang_constants::LOWER_M, common_lang_constants::UPPER_M,
                        common_lang_constants::LOWER_E, common_lang_constants::UPPER_E,
                        common_lang_constants::LOWER_N, common_lang_constants::UPPER_N,
                        common_lang_constants::LOWER_T, common_lang_constants::UPPER_T) ||
                    stem<string_typeT>::is_suffix(text,
                        /*ence*/
                        common_lang_constants::LOWER_E, common_lang_constants::UPPER_E,
                        common_lang_constants::LOWER_N, common_lang_constants::UPPER_N,
                        common_lang_constants::LOWER_C, common_lang_constants::UPPER_C,
                        common_lang_constants::LOWER_E, common_lang_constants::UPPER_E) ||
                    stem<string_typeT>::is_suffix(text,
                        /*ance*/
                        common_lang_constants::LOWER_A, common_lang_constants::UPPER_A,
                        common_lang_constants::LOWER_N, common_lang_constants::UPPER_N,
                        common_lang_constants::LOWER_C, common_lang_constants::UPPER_C,
                        common_lang_constants::LOWER_E, common_lang_constants::UPPER_E)) )
                {
                if (stem<string_typeT>::get_r2() <= text.length()-4)
                    {
                    text.erase(text.length()-4);
                    stem<string_typeT>::update_r_sections(text);
                    }
                }
            else if (text.length() >= 4 &&
                    (stem<string_typeT>::is_suffix(text,
                        /*sion*/
                        common_lang_constants::LOWER_S, common_lang_constants::UPPER_S,
                        common_lang_constants::LOWER_I, common_lang_constants::UPPER_I,
                        common_lang_constants::LOWER_O, common_lang_constants::UPPER_O,
                        common_lang_constants::LOWER_N, common_lang_constants::UPPER_N) ||
                    stem<string_typeT>::is_suffix(text,
                        /*tion*/
                        common_lang_constants::LOWER_T, common_lang_constants::UPPER_T,
                        common_lang_constants::LOWER_I, common_lang_constants::UPPER_I,
                        common_lang_constants::LOWER_O, common_lang_constants::UPPER_O,
                        common_lang_constants::LOWER_N, common_lang_constants::UPPER_N)) )
                {
                if (stem<string_typeT>::get_r2() <= text.length()-3)
                    {
                    text.erase(text.length()-3);
                    stem<string_typeT>::update_r_sections(text);
                    }
                }
            else if (text.length() >= 3 &&
                    (stem<string_typeT>::is_suffix(text,
                        /*ant*/
                        common_lang_constants::LOWER_A, common_lang_constants::UPPER_A,
                        common_lang_constants::LOWER_N, common_lang_constants::UPPER_N,
                        common_lang_constants::LOWER_T, common_lang_constants::UPPER_T) ||
                    stem<string_typeT>::is_suffix(text,
                        /*ent*/
                        common_lang_constants::LOWER_E, common_lang_constants::UPPER_E,
                        common_lang_constants::LOWER_N, common_lang_constants::UPPER_N,
                        common_lang_constants::LOWER_T, common_lang_constants::UPPER_T) ||
                    stem<string_typeT>::is_suffix(text,
                        /*ism*/
                        common_lang_constants::LOWER_I, common_lang_constants::UPPER_I,
                        common_lang_constants::LOWER_S, common_lang_constants::UPPER_S,
                        common_lang_constants::LOWER_M, common_lang_constants::UPPER_M) ||
                    stem<string_typeT>::is_suffix(text,
                        /*ate*/
                        common_lang_constants::LOWER_A, common_lang_constants::UPPER_A,
                        common_lang_constants::LOWER_T, common_lang_constants::UPPER_T,
                        common_lang_constants::LOWER_E, common_lang_constants::UPPER_E) ||
                    stem<string_typeT>::is_suffix(text,
                        /*iti*/
                        common_lang_constants::LOWER_I, common_lang_constants::UPPER_I,
                        common_lang_constants::LOWER_T, common_lang_constants::UPPER_T,
                        common_lang_constants::LOWER_I, common_lang_constants::UPPER_I) ||
                    stem<string_typeT>::is_suffix(text,
                        /*ous*/
                        common_lang_constants::LOWER_O, common_lang_constants::UPPER_O,
                        common_lang_constants::LOWER_U, common_lang_constants::UPPER_U,
                        common_lang_constants::LOWER_S, common_lang_constants::UPPER_S) ||
                    stem<string_typeT>::is_suffix(text,
                        /*ive*/
                        common_lang_constants::LOWER_I, common_lang_constants::UPPER_I,
                        common_lang_constants::LOWER_V, common_lang_constants::UPPER_V,
                        common_lang_constants::LOWER_E, common_lang_constants::UPPER_E) ||
                    stem<string_typeT>::is_suffix(text,
                        /*ize*/
                        common_lang_constants::LOWER_I, common_lang_constants::UPPER_I,
                        common_lang_constants::LOWER_Z, common_lang_constants::UPPER_Z,
                        common_lang_constants::LOWER_E, common_lang_constants::UPPER_E)) )
                {
                if (stem<string_typeT>::get_r2() <= text.length()-3)
                    {
                    text.erase(text.length()-3);
                    stem<string_typeT>::update_r_sections(text);
                    }
                }
            else if (text.length() >= 2 &&
                    (stem<string_typeT>::is_suffix(text,
                        /*al*/
                        common_lang_constants::LOWER_A, common_lang_constants::UPPER_A,
                        common_lang_constants::LOWER_L, common_lang_constants::UPPER_L) ||
                    stem<string_typeT>::is_suffix(text,
                        /*er*/
                        common_lang_constants::LOWER_E, common_lang_constants::UPPER_E,
                        common_lang_constants::LOWER_R, common_lang_constants::UPPER_R) ||
                    stem<string_typeT>::is_suffix(text,
                        /*ic*/
                        common_lang_constants::LOWER_I, common_lang_constants::UPPER_I,
                        common_lang_constants::LOWER_C, common_lang_constants::UPPER_C)) )
                {
                if (stem<string_typeT>::get_r2() <= text.length()-2)
                    {
                    text.erase(text.length()-2);
                    stem<string_typeT>::update_r_sections(text);
                    }
                }
            }

        //---------------------------------------------
        void step_5(string_typeT& text)
            {
            if (text.length() >= 1 &&
                stem<string_typeT>::is_either(text[text.length()-1],
                    common_lang_constants::LOWER_E, common_lang_constants::UPPER_E) )
                {
                if (stem<string_typeT>::get_r2() != text.length())
                    {
                    text.erase(text.length()-1);
                    stem<string_typeT>::update_r_sections(text);
                    }
                else if (stem<string_typeT>::get_r1() != text.length() &&
                    text.length() >= 2 &&
                    // look at the part of the word in front of the last 'e' to see if it ends with
                    // a short syllable.
                    !ends_with_short_syllable(text, text.length()-1))
                    {
                    text.erase(text.length()-1);
                    stem<string_typeT>::update_r_sections(text);
                    }
                }
            else if (stem<string_typeT>::get_r2() != text.length() &&
                stem<string_typeT>::is_suffix(text,
                    /*ll*/
                    common_lang_constants::LOWER_L, common_lang_constants::UPPER_L,
                    common_lang_constants::LOWER_L, common_lang_constants::UPPER_L) )
                {
                text.erase(text.length()-1);
                stem<string_typeT>::update_r_sections(text);
                }
            }

        /** Define a short syllable in a word as either
            (a) a vowel followed by a non-vowel other than w, x or Y and preceded by a non-vowel, or 
            (b) a vowel at the beginning of the word followed by a non-vowel, or
            (c) past

            So rap, trap, entrap end with a short syllable, and ow, on, at,
            past are classed as short syllables.
            But uproot, bestow, disturb do not end with a short syllable.*/
        //---------------------------------------------
        bool ends_with_short_syllable(const string_typeT& text, const size_t length) const
            {
            if (length == 2)
                {
                if (is_vowel(text[0]) )
                    { return (!is_vowel(text[1]) ); }
                else
                    { return false; }
                }
            else if (length == 4 &&
                /*past*/
                (stem<string_typeT>::is_either(text[0],
                    common_lang_constants::LOWER_P, common_lang_constants::UPPER_P) &&
                 stem<string_typeT>::is_either(text[1],
                    common_lang_constants::LOWER_A, common_lang_constants::UPPER_A) &&
                 stem<string_typeT>::is_either(text[2],
                    common_lang_constants::LOWER_S, common_lang_constants::UPPER_S) &&
                 stem<string_typeT>::is_either(text[3],
                    common_lang_constants::LOWER_T, common_lang_constants::UPPER_T)))
                {
                return true;
                }
            else if (length > 2)
                {
                const size_t start = text.find_last_of(L"aeiouyAEIOUY", length-1);
                if (start == string_typeT::npos)
                    { return false; }
                if (start > 0 &&
                    start == (length-2) &&
                    // following letter
                    (!is_vowel(text[start+1]) &&
                    !stem<string_typeT>::is_one_of(text[start+1], L"wxWX") &&
                    stem<string_typeT>::is_neither(text[start+1], LOWER_Y_HASH, UPPER_Y_HASH)) &&
                    // proceeding letter
                    !is_vowel(text[start-1]) )
                    { return true; }
                else
                    { return false; }
                }
            else
                { return false; }
            }

        /// A word is called short if it ends in a short syllable, and if R1 is null.
        //---------------------------------------------
        inline bool is_short_word(const string_typeT& text, const size_t length) const
            {
            return (ends_with_short_syllable(text, length) &&
                    stem<string_typeT>::get_r1() == text.length());
            }

        //---------------------------------------------
        inline bool is_vowel(const wchar_t character) const noexcept
            { return (stem<string_typeT>::is_one_of(character, L"aeiouyAEIOUY") ); }

        size_t m_first_vowel{ string_typeT::npos };
        };
    }

/** @}*/

#endif // OLEAN_ENGLISH_STEM_H
