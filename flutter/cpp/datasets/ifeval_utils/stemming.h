// clang-format off
/** @addtogroup Stemming
    @brief Library for stemming words down to their root words.
    @date 2004-2025
    @copyright Oleander Software, Ltd.
    @author Blake Madden
    @details This program is free software; you can redistribute it and/or modify
    it under the terms of the BSD License.

    SPDX-License-Identifier: BSD-3-Clause
* @{*/

#ifndef OLEAN_STEM_H
#define OLEAN_STEM_H

#include <algorithm>
#include <string_view>
#include <array>
#include <cassert>
#include "flutter/cpp/datasets/ifeval_utils/common_lang_constants.h"


// TODO remove parts not related to english stemmer
/// @brief Namespace for stemming classes.
namespace stemming
    {
    /// @brief The library's major version.
    constexpr int OLEANDER_STEM_MAJOR_VERSION = 2025;
    /// @brief The library's minor version.
    constexpr int OLEANDER_STEM_MINOR_VERSION = 0;
    /// @brief The library's patch version.
    constexpr int OLEANDER_STEM_PATCH_VERSION = 1;
    /// @brief The library's tweak version.
    constexpr int OLEANDER_STEM_TWEAK_VERSION = 1;

    /// @brief The library's copyright notice.
    constexpr wchar_t OLEANDER_STEM_COPYRIGHT[] = L"Copyright (c) 2004-2025 Blake Madden";

    /// @brief The Snowball standard implemented by the library
    ///     (major version).
    constexpr int SNOWBALL_MAJOR_VERSION = 3;
    /// @brief The Snowball standard implemented by the library
    ///     (minor version).
    constexpr int SNOWBALL_MINOR_VERSION = 0;
    /// @brief The Snowball standard implemented by the library
    ///     (minor version).
    constexpr int SNOWBALL_PATCH_VERSION = 1;

    /// @brief Languages available for stemming.
    enum class stemming_type
        {
        /// @brief A no-op stemmer.
        no_stemming,
        /// @brief Danish
        danish,
        /// @brief Dutch
        dutch,
        /// @private
        /// @internal Use Porter's Dutch algorithm for now.
        dutch_porter = dutch,
        /// @brief English
        english,
        /// @brief Finnish
        finnish,
        /// @brief french
        french,
        /// @brief German
        german,
        /// @brief Italian
        italian,
        /// @brief Norwegian
        norwegian,
        /// @brief Portuguese
        portuguese,
        /// @brief Spanish
        spanish,
        /// @brief Swedish
        swedish,
        /// @brief Russian
        russian,
        /// @private
        STEMMING_TYPE_COUNT
        };

    // these characters should not appear in an indexed word
    constexpr wchar_t UPPER_Y_HASH = 7;  // bell
    constexpr wchar_t LOWER_Y_HASH = 9;  // tab
    constexpr wchar_t UPPER_I_HASH = 10; // line feed
    constexpr wchar_t LOWER_I_HASH = 11; // vertical tab
    constexpr wchar_t UPPER_U_HASH = 12; // form feed (new page)
    constexpr wchar_t LOWER_U_HASH = 13; // carriage return
    constexpr wchar_t DIARESIS_HASH = 14; // shift out

    // language constants
    static const wchar_t FRENCH_VOWELS[] = { 97, 101, 105, 111, 117, 121, 0xE2,
        0xE0, 0xEB, 0xE9,
        0xEA, 0xE8, 0xEF,
        0xEE, 0xF4, 0xFB,
        0xF9, 65, 69, 73, 79, 85, 89, 0xC2,
        0xC0, 0xCB, 0xC9,
        0xCA, 0xC8, 0xCF,
        0xCE, 0xD4, 0xDB,
        0xD9, 0 };
    static const wchar_t FRENCH_ACCENTED_E[] = { 0xE9, 0xE8,
        0xC9, 0xC8, 0 };
    static const wchar_t FRENCH_AIOUES[] = { 97, 105, 111, 117, 0xE8, 115, 65, 73, 79, 85,
        0xC8, 83, 0 };

    static const wchar_t GERMAN_VOWELS[] = { 97, 101, 105, 111, 117, 0xFC, 121,
        0xE4, 0xF6, 65, 0xC4,
        69, 73, 79, 0xD6, 85, 0xDC, 89, 0 };

    static const wchar_t DANISH_VOWELS[] = { 97, 101, 105, 111, 117, 121, 0xE6,
        0xE5, 0xF8, 65, 69, 73, 79, 85, 89,
        0xC6, 0xC5, 0xD8, 0 };
    static const wchar_t DANISH_ALPHABET[] = { 97, 98, 99, 100, 102, 103, 104, 106, 107,
        108, 109, 110, 111, 112, 114, 116, 118, 121, 122, 0xE5, 65, 66, 67, 68, 70, 71,
        72, 74, 75, 76, 77, 78, 79, 80, 82, 84, 86, 89, 90, 0xC5, 0 };

    static const wchar_t FINNISH_VOWELS[] = { 97, 101, 105, 111, 117, 121, 0xE4, 0xF6, 65,
        69, 73, 79, 85, 89, 0xC4, 0xD6, 0 };
    static const wchar_t FINNISH_VOWELS_NO_Y[] = { 97, 101, 105, 111, 117, 0xE4, 0xF6, 65,
        69, 73, 79, 85, 0xC4, 0xD6, 0 };
    static const wchar_t FINNISH_VOWELS_SIMPLE[] = { 97, 101, 105, 0xE4, 65, 69, 73, 0xC4, 0 };
    static constexpr wchar_t FINNISH_CONSONANTS[] =
        { L'b', L'c', L'd', L'f', L'g', L'h', L'j', L'k', L'l', L'm', L'n', L'p', L'q', L'r', L's',
          L't', L'v', L'w', L'x', L'z', L'B', L'C', L'D', L'F', L'G', L'H', L'J', L'K', L'L', L'M',
          L'N', L'P', L'Q', L'R', L'S', L'T', L'V', L'W', L'X', L'Z', 0 };
    static const wchar_t FINNISH_STEP_1_SUFFIX[] = { 110, 116, 97, 101, 105, 111, 117, 121, 0xE4,
        0xF6, 78, 84, 65, 69, 73, 79, 85, 89, 0xC4, 0xD6, 0 };

    static const wchar_t DUTCH_VOWELS[] = { 97, 101, 105, 111, 117, 121, 0xE8,
        65, 69, 73, 79, 85, 89, 0xC8, 0 };
    static const wchar_t DUTCH_KDT[] = { 107, 100, 116, 75, 68, 84, 0 };
    static const wchar_t DUTCH_S_ENDING[] = { 97, 101, 0xE8, 105, 111, 117, 121, 106, 65, 69,
        0xC8, 73, 79, 85, 89, 74, 0 };

    static const wchar_t NORWEGIAN_VOWELS[] = { L'a', L'e', L'ê', L'i', L'o', L'ò', L'ó',
        L'ô', L'u', L'y', L'æ', L'å', L'ø',
        L'A', L'E', L'Ê', L'I', L'O', L'Ò', L'Ó',
        L'Ô', L'U', L'Y', L'Æ', L'Å', L'Ø', 0 };
    static const wchar_t PORTUGUESE_VOWELS[] = { 97, 101, 105, 111, 117, 0xE1, 0xE9,
        0xED, 0xF3, 0xFA, 0xE2,
        0xEA, 0xF4, 65, 69, 73, 79, 85, 0xC1,
        0xC9, 0xCD, 0xD3, 0xDA,
        0xC2, 0xCA, 0xD4, 0 };
    static const wchar_t SPANISH_VOWELS[] = { 97, 101, 105, 111, 117, 0xE1, 0xE9,
        0xED, 0xF3, 0xFA, 0xFC,
        65, 69, 73, 79, 85, 0xC1, 0xC9, 0xCD,
        0xD3, 0xDA, 0xDC, 0 };

    static const wchar_t SWEDISH_VOWELS[] = { 97, 101, 105, 111, 117, 121, 0xE5,
        0xE4, 0xF6, 65, 69, 73, 79, 85, 89,
        0xC5, 0xC4, 0xD6, 0 };

    static const wchar_t ITALIAN_VOWELS[] = { 97, 101, 105, 111, 117, 0xE0,
        0xE8, 0xEC, 0xF2,
        0xF9, 65, 69, 73, 79, 85, 0xC0,
        0xC8, 0xCC, 0xD2,
        0xD9, 0 };
    static const wchar_t ITALIAN_VOWELS_SIMPLE[] = { 97, 101, 105, 111, 0xE0,
        0xE8, 0xEC, 0xF2,
        65, 69, 73, 79, 0xC0, 0xC8,
        0xCC, 0xD2, 0 };

    /** @brief Converts a full-width number/English letter/various symbols
            into its "narrow" counterpart.
        @param ch The character to convert.
        @returns The narrow version of a character, or the character if not full-width.*/
    [[nodiscard]]
    inline constexpr wchar_t full_width_to_narrow(const wchar_t ch) noexcept
        {
        return
            // not in the fullwidth/halfwidth Unicode ranges; return character unchanged
            (ch < 65'000) ? ch :
            // fullwidth Latin letters, digits, and punctuation
            (ch >= 65'281 && ch <= 65'374) ? (ch - 65'248) :
            // cent and pound sterling
            (ch >= 65'504 && ch <= 65'505) ? (ch - 65'342) :
            // Yen
            (ch == 65'509) ? 165 :
            // Not
            (ch == 65'506) ? 172 :
            // macron
            (ch == 65'507) ? 175 :
            // broken bar
            (ch == 65'508) ? 166 :
            ch;
        }

    /** @brief The base class for language-specific stemmers.
        @details The template argument for the stemmers are the type
        of `std::basic_string` that you are trying to stem,
        by default `std::wstring` (double-byte strings).
        As long as the char type of your `basic_string` is `wchar_t`,
        then you can use any type of `basic_string`.
        This is to say, if your `basic_string` has a custom character traits or allocator,
        then just specify it in your template argument to the stemmer.

        @par Example:
        @code
        using myString = std::basic_string<wchar_t, myTraits, myAllocator>;
        myString word(L"documentation");
        stemming::english_stem<myString> StemEnglish;
        StemEnglish(word);
        @endcode
    */
    template <typename string_typeT = std::wstring>
    class stem
        {
    public:
        /// @brief The string type that this class will accept.
        using string_type = string_typeT;
        /// @brief The main interface for stemming a word.
        /// @param[in,out] text The text to stem.
        virtual void operator()(string_typeT& text) = 0;
        /// @returns The stemmer's language.
        [[nodiscard]]
        virtual stemming_type get_language() const noexcept = 0;
        /// Destructor.
        virtual ~stem() = default;
    protected:
        // R1, R2, RV functions
        /// @brief Finds the start of R1.
        /// @param text The string to review.
        /// @param vowel_list The list of vowels by the stemmer's language.
        void find_r1(const string_typeT& text,
                     const wchar_t* vowel_list) noexcept
            {
            // see where the R1 section begin
            // R1 is the region after the first non-vowel after the first vowel
            size_t start = text.find_first_of(vowel_list, 0);
            if (start == string_typeT::npos)
                {
                // we need at least need a vowel somewhere in the word
                m_r1 = text.length();
                return;
                }

            m_r1 = text.find_first_not_of(vowel_list,++start);
            if (get_r1() == string_typeT::npos)
                {
                m_r1 = text.length();
                }
            else
                {
                ++m_r1;
                }
            }

        /// @brief Finds the start of R2.
        /// @param text The string to review.
        /// @param vowel_list The list of vowels by the stemmer's language.
        void find_r2(const string_typeT& text,
                     const wchar_t* vowel_list) noexcept
            {
            size_t start = 0;
            // look for R2--not required for all criteria.
            // R2 is the region after the first non-vowel after the first vowel after R1
            if (get_r1() != text.length() )
                {
                start = text.find_first_of(vowel_list, get_r1());
                }
            else
                {
                start = string_typeT::npos;
                }
            if (start != string_typeT::npos &&
                static_cast<int>(start) != static_cast<int>(text.length())-1)
                {
                m_r2 = text.find_first_not_of(vowel_list,++start);
                if (get_r2() == string_typeT::npos)
                    {
                    m_r2 = text.length();
                    }
                else
                    {
                    ++m_r2;
                    }
                }
            else
                {
                m_r2 = text.length();
                }
            }

        /// @brief Finds the start of RV (Spanish stemmer).
        /// @param text The string to review.
        /// @param vowel_list The list of vowels by the stemmer's language.
        void find_spanish_rv(const string_typeT& text,
                             const wchar_t* vowel_list)
            {
            // see where the RV section begin
            if (text.length() < 4)
                {
                m_rv = text.length();
                return;
                }
            // if second letter is a consonant
            if (!stem<string_typeT>::is_one_of(text[1], vowel_list) )
                {
                const size_t start = text.find_first_of(vowel_list, 2);
                if (start == string_typeT::npos)
                    {
                    // can't find next vowel
                    m_rv = text.length();
                    return;
                    }
                else
                    {
                    m_rv = start+1;
                    }
                }
            // if first two letters are vowels
            else if (stem<string_typeT>::is_one_of(text[0], vowel_list) &&
                     stem<string_typeT>::is_one_of(text[1], vowel_list))
                {
                const size_t start = text.find_first_not_of(vowel_list, 2);
                if (start == string_typeT::npos)
                    {
                    // can't find next consonant
                    m_rv = text.length();
                    return;
                    }
                else
                    {
                    m_rv = start+1;
                    }
                }
            // consonant/vowel at beginning
            else if (!stem<string_typeT>::is_one_of(text[0], vowel_list) &&
                     stem<string_typeT>::is_one_of(text[1], vowel_list))
                {
                m_rv = 3;
                }
            else
                {
                m_rv = text.length();
                }
            }

        /* @brief Finds the start of RV (French stemmer).
           @param text The string to review.
           @param vowel_list The list of vowels by the stemmer's language.
           @note If the word begins with two vowels, RV is the region after the third letter,
            otherwise the region after the first vowel not at the beginning of the word,
            or the end of the word if these positions cannot be found.
            (Exceptionally, par, col, tap, or ni[vowel] at the beginning of a word is also taken
             to be the region before RV.)*/
        void find_french_rv(const string_typeT& text,
                            const wchar_t* vowel_list)
            {
            // see where the RV section begin
            if (text.length() < 3)
                {
                m_rv = text.length();
                return;
                }
            /* Exceptions: If the word begins with these then RV goes right after them,
               whether it be a letter or simply the end of the word.*/
            if (text.length() >= 3 &&
                ((stem<string_typeT>::is_either(text[0], common_lang_constants::LOWER_P,
                                                common_lang_constants::UPPER_P) &&
                stem<string_typeT>::is_either(text[1], common_lang_constants::LOWER_A,
                                              common_lang_constants::UPPER_A) &&
                stem<string_typeT>::is_either(text[2], common_lang_constants::LOWER_R,
                                              common_lang_constants::UPPER_R)) || // par

                (stem<string_typeT>::is_either(text[0], common_lang_constants::LOWER_C,
                                               common_lang_constants::UPPER_C) &&
                stem<string_typeT>::is_either(text[1], common_lang_constants::LOWER_O,
                                              common_lang_constants::UPPER_O) &&
                stem<string_typeT>::is_either(text[2], common_lang_constants::LOWER_L,
                                              common_lang_constants::UPPER_L)) || // col

                (stem<string_typeT>::is_either(text[0], common_lang_constants::LOWER_T,
                                               common_lang_constants::UPPER_T) &&
                stem<string_typeT>::is_either(text[1], common_lang_constants::LOWER_A,
                                              common_lang_constants::UPPER_A) &&
                stem<string_typeT>::is_either(text[2], common_lang_constants::LOWER_P,
                                              common_lang_constants::UPPER_P)) ||

                (stem<string_typeT>::is_either(text[0], common_lang_constants::LOWER_N,
                                               common_lang_constants::UPPER_N) &&
                 stem<string_typeT>::is_either(text[1], common_lang_constants::LOWER_I,
                                              common_lang_constants::UPPER_I) &&
                 stem<string_typeT>::is_one_of(text[2], vowel_list))) // ni[vowel]
                )
                {
                m_rv = 3;
                return;
                }
            // if first two letters are vowels
            if (stem<string_typeT>::is_one_of(text[0], vowel_list) &&
                stem<string_typeT>::is_one_of(text[1], vowel_list))
                {
                m_rv = 3;
                }
            else
                {
                size_t start = text.find_first_not_of(vowel_list, 0);
                if (start == string_typeT::npos)
                    {
                    // can't find first consonant
                    m_rv = text.length();
                    return;
                    }
                start = text.find_first_of(vowel_list, start);
                if (start == string_typeT::npos)
                    {
                    // can't find first vowel
                    m_rv = text.length();
                    return;
                    }
                m_rv = start+1;
                }
            }

        /* @brief Finds the start of RV (Russian stemmer).
           @param text The string to review.
           @param vowel_list The list of vowels by the stemmer's language.*/
        void find_russian_rv(const string_typeT& text,
                             const wchar_t* vowel_list) noexcept
            {
            const size_t start = text.find_first_of(vowel_list);
            if (start == string_typeT::npos)
                {
                // can't find first vowel
                m_rv = text.length();
                return;
                }
            else
                {
                m_rv = start+1;
                }
            }

        /// @brief Updates positions of the R sections.
        /// @param text The string being reviewed.
        inline void update_r_sections(const string_typeT& text) noexcept
            {
            if (get_r1() > text.length() )
                { m_r1 = text.length(); }
            if (get_r2() > text.length() )
                { m_r2 = text.length(); }
            if (get_rv() > text.length() )
                { m_rv = text.length(); }
            }
        /** @brief Determines if a character is an apostrophe (includes straight single quotes).
            @param ch The letter to be analyzed.
            @returns @c true if character is an apostrophe.*/
        [[nodiscard]]
        constexpr bool is_apostrophe(const wchar_t& ch) const noexcept
            {
            return (ch == 39) ?         // '
                true : (ch == 146) ?    // apostrophe
                true : (ch == 180) ?    // apostrophe
                true : (ch == 0x2019) ? // right single apostrophe
                true : false;
            }

        /// @brief Removes possessive suffix (apostrophe and "'s") from the end of a string.
        /// @param[in,out] text The string to trim.
        void remove_possessive_suffix(string_typeT& text) const
            {
            // handle trash like "there's'"
            while (text.length() >= 1 &&
                is_apostrophe(text.back()))
                {
                text.pop_back();
                }

            if (text.length() >= 2 &&
                is_apostrophe(text[text.length()-2]) &&
                stem<string_typeT>::is_either(text.back(), common_lang_constants::LOWER_S,
                                                           common_lang_constants::UPPER_S) )
                { text.erase(text.length()-2); }

            while (text.length() >= 1 &&
                is_apostrophe(text.back()))
                { text.pop_back(); }
            }

        // suffix determinant functions
        //------------------------------------
        /// @brief is_suffix for one character.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @returns @c true if characters match suffix.
        [[nodiscard]]
        inline static bool is_suffix(const string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U) noexcept
            {
            if (text.length() < 1)
                { return false; }
            return stem<string_typeT>::is_either(text[text.length()-1], suffix1L, suffix1U);
            }
        /// @brief is_suffix for two characters.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @returns @c true if characters match suffix.
        [[nodiscard]]
        inline static bool is_suffix(const string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U) noexcept
            {
            if (text.length() < 2)
                { return false; }
            return stem<string_typeT>::is_either(text[text.length()-2], suffix1L, suffix1U) &&
                    stem<string_typeT>::is_either(text[text.length()-1], suffix2L, suffix2U);
            }

        /// @brief is_suffix for three characters.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @returns @c true if characters match suffix.
        [[nodiscard]]
        inline static bool is_suffix(const string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U) noexcept
            {
            if (text.length() < 3)
                { return false; }
            return stem<string_typeT>::is_either(text[text.length()-3], suffix1L, suffix1U) &&
                    stem<string_typeT>::is_either(text[text.length()-2], suffix2L, suffix2U) &&
                    stem<string_typeT>::is_either(text[text.length()-1], suffix3L, suffix3U);
            }
        /// @brief is_suffix for four characters.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @param suffix4L The lowercased version of the fourth character of the suffix.
        /// @param suffix4U The uppercased version of the fourth character of the suffix.
        /// @returns @c true if characters match suffix.
        [[nodiscard]]
        inline static bool is_suffix(const string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U,
                    const wchar_t suffix4L, const wchar_t suffix4U) noexcept
            {
            if (text.length() < 4)
                { return false; }
            return stem<string_typeT>::is_either(text[text.length()-4], suffix1L, suffix1U) &&
                    stem<string_typeT>::is_either(text[text.length()-3], suffix2L, suffix2U) &&
                    stem<string_typeT>::is_either(text[text.length()-2], suffix3L, suffix3U) &&
                    stem<string_typeT>::is_either(text[text.length()-1], suffix4L, suffix4U);
            }
        /// @brief is_suffix for five characters.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @param suffix4L The lowercased version of the fourth character of the suffix.
        /// @param suffix4U The uppercased version of the fourth character of the suffix.
        /// @param suffix5L The lowercased version of the fifth character of the suffix.
        /// @param suffix5U The uppercased version of the fifth character of the suffix.
        /// @returns @c true if characters match suffix.
        [[nodiscard]]
        inline static bool is_suffix(const string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U,
                    const wchar_t suffix4L, const wchar_t suffix4U,
                    const wchar_t suffix5L, const wchar_t suffix5U) noexcept
            {
            if (text.length() < 5)
                { return false; }
            return stem<string_typeT>::is_either(text[text.length()-5], suffix1L, suffix1U) &&
                    stem<string_typeT>::is_either(text[text.length()-4], suffix2L, suffix2U) &&
                    stem<string_typeT>::is_either(text[text.length()-3], suffix3L, suffix3U) &&
                    stem<string_typeT>::is_either(text[text.length()-2], suffix4L, suffix4U) &&
                    stem<string_typeT>::is_either(text[text.length()-1], suffix5L, suffix5U);
            }
        /// @brief is_suffix for six characters.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @param suffix4L The lowercased version of the fourth character of the suffix.
        /// @param suffix4U The uppercased version of the fourth character of the suffix.
        /// @param suffix5L The lowercased version of the fifth character of the suffix.
        /// @param suffix5U The uppercased version of the fifth character of the suffix.
        /// @param suffix6L The lowercased version of the sixth character of the suffix.
        /// @param suffix6U The uppercased version of the sixth character of the suffix.
        /// @returns @c true if characters match suffix.
        [[nodiscard]]
        inline static bool is_suffix(const string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U,
                    const wchar_t suffix4L, const wchar_t suffix4U,
                    const wchar_t suffix5L, const wchar_t suffix5U,
                    const wchar_t suffix6L, const wchar_t suffix6U) noexcept
            {
            if (text.length() < 6)
                { return false; }
            return stem<string_typeT>::is_either(text[text.length()-6], suffix1L, suffix1U) &&
                    stem<string_typeT>::is_either(text[text.length()-5], suffix2L, suffix2U) &&
                    stem<string_typeT>::is_either(text[text.length()-4], suffix3L, suffix3U) &&
                    stem<string_typeT>::is_either(text[text.length()-3], suffix4L, suffix4U) &&
                    stem<string_typeT>::is_either(text[text.length()-2], suffix5L, suffix5U) &&
                    stem<string_typeT>::is_either(text[text.length()-1], suffix6L, suffix6U);
            }
        /// @brief is_suffix for seven characters.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @param suffix4L The lowercased version of the fourth character of the suffix.
        /// @param suffix4U The uppercased version of the fourth character of the suffix.
        /// @param suffix5L The lowercased version of the fifth character of the suffix.
        /// @param suffix5U The uppercased version of the fifth character of the suffix.
        /// @param suffix6L The lowercased version of the sixth character of the suffix.
        /// @param suffix6U The uppercased version of the sixth character of the suffix.
        /// @param suffix7L The lowercased version of the seventh character of the suffix.
        /// @param suffix7U The uppercased version of the seventh character of the suffix.
        /// @returns @c true if characters match suffix.
        [[nodiscard]]
        inline static bool is_suffix(const string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U,
                    const wchar_t suffix4L, const wchar_t suffix4U,
                    const wchar_t suffix5L, const wchar_t suffix5U,
                    const wchar_t suffix6L, const wchar_t suffix6U,
                    const wchar_t suffix7L, const wchar_t suffix7U) noexcept
            {
            if (text.length() < 7)
                { return false; }
            return stem<string_typeT>::is_either(text[text.length()-7], suffix1L, suffix1U) &&
                    stem<string_typeT>::is_either(text[text.length()-6], suffix2L, suffix2U) &&
                    stem<string_typeT>::is_either(text[text.length()-5], suffix3L, suffix3U) &&
                    stem<string_typeT>::is_either(text[text.length()-4], suffix4L, suffix4U) &&
                    stem<string_typeT>::is_either(text[text.length()-3], suffix5L, suffix5U) &&
                    stem<string_typeT>::is_either(text[text.length()-2], suffix6L, suffix6U) &&
                    stem<string_typeT>::is_either(text[text.length()-1], suffix7L, suffix7U);
            }
        /// @brief is_suffix for eight characters.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @param suffix4L The lowercased version of the fourth character of the suffix.
        /// @param suffix4U The uppercased version of the fourth character of the suffix.
        /// @param suffix5L The lowercased version of the fifth character of the suffix.
        /// @param suffix5U The uppercased version of the fifth character of the suffix.
        /// @param suffix6L The lowercased version of the sixth character of the suffix.
        /// @param suffix6U The uppercased version of the sixth character of the suffix.
        /// @param suffix7L The lowercased version of the seventh character of the suffix.
        /// @param suffix7U The uppercased version of the seventh character of the suffix.
        /// @param suffix8L The lowercased version of the eighth character of the suffix.
        /// @param suffix8U The uppercased version of the eighth character of the suffix.
        /// @returns @c true if characters match suffix.
        [[nodiscard]]
        inline static bool is_suffix(const string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U,
                    const wchar_t suffix4L, const wchar_t suffix4U,
                    const wchar_t suffix5L, const wchar_t suffix5U,
                    const wchar_t suffix6L, const wchar_t suffix6U,
                    const wchar_t suffix7L, const wchar_t suffix7U,
                    const wchar_t suffix8L, const wchar_t suffix8U) noexcept
            {
            if (text.length() < 8)
                { return false; }
            return stem<string_typeT>::is_either(text[text.length()-8], suffix1L, suffix1U) &&
                    stem<string_typeT>::is_either(text[text.length()-7], suffix2L, suffix2U) &&
                    stem<string_typeT>::is_either(text[text.length()-6], suffix3L, suffix3U) &&
                    stem<string_typeT>::is_either(text[text.length()-5], suffix4L, suffix4U) &&
                    stem<string_typeT>::is_either(text[text.length()-4], suffix5L, suffix5U) &&
                    stem<string_typeT>::is_either(text[text.length()-3], suffix6L, suffix6U) &&
                    stem<string_typeT>::is_either(text[text.length()-2], suffix7L, suffix7U) &&
                    stem<string_typeT>::is_either(text[text.length()-1], suffix8L, suffix8U);
            }
        /// @brief is_suffix for nine characters.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @param suffix4L The lowercased version of the fourth character of the suffix.
        /// @param suffix4U The uppercased version of the fourth character of the suffix.
        /// @param suffix5L The lowercased version of the fifth character of the suffix.
        /// @param suffix5U The uppercased version of the fifth character of the suffix.
        /// @param suffix6L The lowercased version of the sixth character of the suffix.
        /// @param suffix6U The uppercased version of the sixth character of the suffix.
        /// @param suffix7L The lowercased version of the seventh character of the suffix.
        /// @param suffix7U The uppercased version of the seventh character of the suffix.
        /// @param suffix8L The lowercased version of the eighth character of the suffix.
        /// @param suffix8U The uppercased version of the eighth character of the suffix.
        /// @param suffix9L The lowercased version of the ninth character of the suffix.
        /// @param suffix9U The uppercased version of the ninth character of the suffix.
        /// @returns @c true if characters match suffix.
        [[nodiscard]]
        inline static bool is_suffix(const string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U,
                    const wchar_t suffix4L, const wchar_t suffix4U,
                    const wchar_t suffix5L, const wchar_t suffix5U,
                    const wchar_t suffix6L, const wchar_t suffix6U,
                    const wchar_t suffix7L, const wchar_t suffix7U,
                    const wchar_t suffix8L, const wchar_t suffix8U,
                    const wchar_t suffix9L, const wchar_t suffix9U) noexcept
            {
            if (text.length() < 9)
                { return false; }
            return stem<string_typeT>::is_either(text[text.length()-9], suffix1L, suffix1U) &&
                    stem<string_typeT>::is_either(text[text.length()-8], suffix2L, suffix2U) &&
                    stem<string_typeT>::is_either(text[text.length()-7], suffix3L, suffix3U) &&
                    stem<string_typeT>::is_either(text[text.length()-6], suffix4L, suffix4U) &&
                    stem<string_typeT>::is_either(text[text.length()-5], suffix5L, suffix5U) &&
                    stem<string_typeT>::is_either(text[text.length()-4], suffix6L, suffix6U) &&
                    stem<string_typeT>::is_either(text[text.length()-3], suffix7L, suffix7U) &&
                    stem<string_typeT>::is_either(text[text.length()-2], suffix8L, suffix8U) &&
                    stem<string_typeT>::is_either(text[text.length()-1], suffix9L, suffix9U);
            }

        /// @brief Comparison for two characters.
        /// @param text The string being reviewed.
        /// @param start_index Where to start the suffix comparison.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @returns @c true if characters are a partial suffix.
        [[nodiscard]]
        inline static bool is_partial_suffix(const string_typeT& text,
                    const size_t start_index,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U) noexcept
            {
            if ((start_index+2) >= text.length())
                { return false; }
            return (stem<string_typeT>::is_either(text[start_index], suffix1L, suffix1U) &&
                    stem<string_typeT>::is_either(text[start_index+1], suffix2L, suffix2U));
            }
        /// @brief Comparison for three characters.
        /// @param text The string being reviewed.
        /// @param start_index Where to start the suffix comparison.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @returns @c true if characters are a partial suffix.
        [[nodiscard]]
        inline static bool is_partial_suffix(const string_typeT& text,
                    const size_t start_index,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U) noexcept
            {
            if ((start_index+3) >= text.length())
                { return false; }
            return (stem<string_typeT>::is_either(text[start_index], suffix1L, suffix1U) &&
                    stem<string_typeT>::is_either(text[start_index+1], suffix2L, suffix2U) &&
                    stem<string_typeT>::is_either(text[start_index+2], suffix3L, suffix3U));
            }

        // RV suffix functions
        //-------------------------------------------------
        /// @brief RV suffix comparison for one character.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @returns @c true if suffix is in RV.
        [[nodiscard]]
        inline bool is_suffix_in_rv(const string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U) noexcept
            {
            if (text.length() < 1)
                { return false; }
            return (stem<string_typeT>::is_either(text[text.length()-1], suffix1L, suffix1U) &&
                    (get_rv() <= text.length()-1) );
            }
        /// @brief RV suffix comparison for two characters.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @returns @c true if suffix is in RV.
        [[nodiscard]]
        inline bool is_suffix_in_rv(const string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U) noexcept
            {
            if (text.length() < 2)
                { return false; }
            return ((stem<string_typeT>::is_either(text[text.length()-2], suffix1L, suffix1U) &&
                    stem<string_typeT>::is_either(text[text.length()-1], suffix2L, suffix2U) ) &&
                    (get_rv() <= text.length()-2) );
            }
        /// @brief RV suffix comparison for three characters.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @returns @c true if suffix is in RV.
        [[nodiscard]]
        inline bool is_suffix_in_rv(const string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U) noexcept
            {
            if (text.length() < 3)
                { return false; }
            return ((stem<string_typeT>::is_either(text[text.length()-3], suffix1L, suffix1U) &&
                    stem<string_typeT>::is_either(text[text.length()-2], suffix2L, suffix2U) &&
                    stem<string_typeT>::is_either(text[text.length()-1], suffix3L, suffix3U) ) &&
                    (get_rv() <= text.length()-3) );
            }
        /// @brief RV suffix comparison for four characters.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @param suffix4L The lowercased version of the fourth character of the suffix.
        /// @param suffix4U The uppercased version of the fourth character of the suffix.
        /// @returns @c true if suffix is in RV.
        [[nodiscard]]
        inline bool is_suffix_in_rv(const string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U,
                    const wchar_t suffix4L, const wchar_t suffix4U) noexcept
            {
            if (text.length() < 4)
                { return false; }
            return ((stem<string_typeT>::is_either(text[text.length()-4], suffix1L, suffix1U) &&
                    stem<string_typeT>::is_either(text[text.length()-3], suffix2L, suffix2U) &&
                    stem<string_typeT>::is_either(text[text.length()-2], suffix3L, suffix3U) &&
                    stem<string_typeT>::is_either(text[text.length()-1], suffix4L, suffix4U) ) &&
                    (get_rv() <= text.length()-4) );
            }
        /// @brief RV suffix comparison for five characters.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @param suffix4L The lowercased version of the fourth character of the suffix.
        /// @param suffix4U The uppercased version of the fourth character of the suffix.
        /// @param suffix5L The lowercased version of the fifth character of the suffix.
        /// @param suffix5U The uppercased version of the fifth character of the suffix.
        /// @returns @c true if suffix is in RV.
        [[nodiscard]]
        inline bool is_suffix_in_rv(const string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U,
                    const wchar_t suffix4L, const wchar_t suffix4U,
                    const wchar_t suffix5L, const wchar_t suffix5U) noexcept
            {
            if (text.length() < 5)
                { return false; }
            return ((stem<string_typeT>::is_either(text[text.length()-5], suffix1L, suffix1U) &&
                    stem<string_typeT>::is_either(text[text.length()-4], suffix2L, suffix2U) &&
                    stem<string_typeT>::is_either(text[text.length()-3], suffix3L, suffix3U) &&
                    stem<string_typeT>::is_either(text[text.length()-2], suffix4L, suffix4U) &&
                    stem<string_typeT>::is_either(text[text.length()-1], suffix5L, suffix5U) ) &&
                    (get_rv() <= text.length()-5) );
            }
        /// @brief RV suffix comparison for six characters.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @param suffix4L The lowercased version of the fourth character of the suffix.
        /// @param suffix4U The uppercased version of the fourth character of the suffix.
        /// @param suffix5L The lowercased version of the fifth character of the suffix.
        /// @param suffix5U The uppercased version of the fifth character of the suffix.
        /// @param suffix6L The lowercased version of the sixth character of the suffix.
        /// @param suffix6U The uppercased version of the sixth character of the suffix.
        /// @returns @c true if suffix is in RV.
        [[nodiscard]]
        inline bool is_suffix_in_rv(const string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U,
                    const wchar_t suffix4L, const wchar_t suffix4U,
                    const wchar_t suffix5L, const wchar_t suffix5U,
                    const wchar_t suffix6L, const wchar_t suffix6U) noexcept
            {
            if (text.length() < 6)
                { return false; }
            return ((stem<string_typeT>::is_either(text[text.length()-6], suffix1L, suffix1U) &&
                    stem<string_typeT>::is_either(text[text.length()-5], suffix2L, suffix2U) &&
                    stem<string_typeT>::is_either(text[text.length()-4], suffix3L, suffix3U) &&
                    stem<string_typeT>::is_either(text[text.length()-3], suffix4L, suffix4U) &&
                    stem<string_typeT>::is_either(text[text.length()-2], suffix5L, suffix5U) &&
                    stem<string_typeT>::is_either(text[text.length()-1], suffix6L, suffix6U) ) &&
                    (get_rv() <= text.length()-6) );
            }
        /// @brief RV suffix comparison for seven characters.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @param suffix4L The lowercased version of the fourth character of the suffix.
        /// @param suffix4U The uppercased version of the fourth character of the suffix.
        /// @param suffix5L The lowercased version of the fifth character of the suffix.
        /// @param suffix5U The uppercased version of the fifth character of the suffix.
        /// @param suffix6L The lowercased version of the sixth character of the suffix.
        /// @param suffix6U The uppercased version of the sixth character of the suffix.
        /// @param suffix7L The lowercased version of the seventh character of the suffix.
        /// @param suffix7U The uppercased version of the seventh character of the suffix.
        /// @returns @c true if suffix is in RV.
        [[nodiscard]]
        inline bool is_suffix_in_rv(const string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U,
                    const wchar_t suffix4L, const wchar_t suffix4U,
                    const wchar_t suffix5L, const wchar_t suffix5U,
                    const wchar_t suffix6L, const wchar_t suffix6U,
                    const wchar_t suffix7L, const wchar_t suffix7U) noexcept
            {
            if (text.length() < 7)
                { return false; }
            return ((stem<string_typeT>::is_either(text[text.length()-7], suffix1L, suffix1U) &&
                    stem<string_typeT>::is_either(text[text.length()-6], suffix2L, suffix2U) &&
                    stem<string_typeT>::is_either(text[text.length()-5], suffix3L, suffix3U) &&
                    stem<string_typeT>::is_either(text[text.length()-4], suffix4L, suffix4U) &&
                    stem<string_typeT>::is_either(text[text.length()-3], suffix5L, suffix5U) &&
                    stem<string_typeT>::is_either(text[text.length()-2], suffix6L, suffix6U) &&
                    stem<string_typeT>::is_either(text[text.length()-1], suffix7L, suffix7U) ) &&
                    (get_rv() <= text.length()-7) );
            }
        /// @brief RV suffix comparison for eight characters.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @param suffix4L The lowercased version of the fourth character of the suffix.
        /// @param suffix4U The uppercased version of the fourth character of the suffix.
        /// @param suffix5L The lowercased version of the fifth character of the suffix.
        /// @param suffix5U The uppercased version of the fifth character of the suffix.
        /// @param suffix6L The lowercased version of the sixth character of the suffix.
        /// @param suffix6U The uppercased version of the sixth character of the suffix.
        /// @param suffix7L The lowercased version of the seventh character of the suffix.
        /// @param suffix7U The uppercased version of the seventh character of the suffix.
        /// @param suffix8L The lowercased version of the eighth character of the suffix.
        /// @param suffix8U The uppercased version of the eighth character of the suffix.
        /// @returns @c true if suffix is in RV.
        [[nodiscard]]
        inline bool is_suffix_in_rv(const string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U,
                    const wchar_t suffix4L, const wchar_t suffix4U,
                    const wchar_t suffix5L, const wchar_t suffix5U,
                    const wchar_t suffix6L, const wchar_t suffix6U,
                    const wchar_t suffix7L, const wchar_t suffix7U,
                    const wchar_t suffix8L, const wchar_t suffix8U) noexcept
            {
            if (text.length() < 8)
                { return false; }
            return ((stem<string_typeT>::is_either(text[text.length()-8], suffix1L, suffix1U) &&
                    stem<string_typeT>::is_either(text[text.length()-7], suffix2L, suffix2U) &&
                    stem<string_typeT>::is_either(text[text.length()-6], suffix3L, suffix3U) &&
                    stem<string_typeT>::is_either(text[text.length()-5], suffix4L, suffix4U) &&
                    stem<string_typeT>::is_either(text[text.length()-4], suffix5L, suffix5U) &&
                    stem<string_typeT>::is_either(text[text.length()-3], suffix6L, suffix6U) &&
                    stem<string_typeT>::is_either(text[text.length()-2], suffix7L, suffix7U) &&
                    stem<string_typeT>::is_either(text[text.length()-1], suffix8L, suffix8U) ) &&
                    (get_rv() <= text.length()-8) );
            }

        // R1 suffix functions
        //-------------------------------------------------
        /// @brief R1 suffix comparison for one character.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @returns @c true if suffix is in R1.
        [[nodiscard]]
        inline bool is_suffix_in_r1(const string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U) noexcept
            {
            if (text.length() < 1)
                { return false; }
            return (stem<string_typeT>::is_either(text[text.length()-1], suffix1L, suffix1U) &&
                    (get_r1() <= text.length()-1) );
            }
        /// @brief 1 suffix comparison for two characters.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @returns @c true if suffix is in R1.
        [[nodiscard]]
        inline bool is_suffix_in_r1(const string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U) noexcept
            {
            if (text.length() < 2)
                { return false; }
            return ((stem<string_typeT>::is_either(text[text.length()-2], suffix1L, suffix1U) &&
                    stem<string_typeT>::is_either(text[text.length()-1], suffix2L, suffix2U) ) &&
                    (get_r1() <= text.length()-2) );
            }
        /// @brief R1 suffix comparison for three characters.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @returns @c true if suffix is in R1.
        [[nodiscard]]
        inline bool is_suffix_in_r1(const string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U) noexcept
            {
            if (text.length() < 3)
                { return false; }
            return ((stem<string_typeT>::is_either(text[text.length()-3], suffix1L, suffix1U) &&
                    stem<string_typeT>::is_either(text[text.length()-2], suffix2L, suffix2U) &&
                    stem<string_typeT>::is_either(text[text.length()-1], suffix3L, suffix3U) ) &&
                    (get_r1() <= text.length()-3) );
            }
        /// @brief R1 suffix comparison for four characters.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @param suffix4L The lowercased version of the fourth character of the suffix.
        /// @param suffix4U The uppercased version of the fourth character of the suffix.
        /// @returns @c true if suffix is in R1.
        [[nodiscard]]
        inline bool is_suffix_in_r1(const string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U,
                    const wchar_t suffix4L, const wchar_t suffix4U) noexcept
            {
            if (text.length() < 4)
                { return false; }
            return ((stem<string_typeT>::is_either(text[text.length()-4], suffix1L, suffix1U) &&
                    stem<string_typeT>::is_either(text[text.length()-3], suffix2L, suffix2U) &&
                    stem<string_typeT>::is_either(text[text.length()-2], suffix3L, suffix3U) &&
                    stem<string_typeT>::is_either(text[text.length()-1], suffix4L, suffix4U) ) &&
                    (get_r1() <= text.length()-4) );
            }
        /// @brief R1 suffix comparison for five characters.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @param suffix4L The lowercased version of the fourth character of the suffix.
        /// @param suffix4U The uppercased version of the fourth character of the suffix.
        /// @param suffix5L The lowercased version of the fifth character of the suffix.
        /// @param suffix5U The uppercased version of the fifth character of the suffix.
        /// @returns @c true if suffix is in R1.
        [[nodiscard]]
        inline bool is_suffix_in_r1(const string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U,
                    const wchar_t suffix4L, const wchar_t suffix4U,
                    const wchar_t suffix5L, const wchar_t suffix5U) noexcept
            {
            if (text.length() < 5)
                { return false; }
            return ((stem<string_typeT>::is_either(text[text.length()-5], suffix1L, suffix1U) &&
                    stem<string_typeT>::is_either(text[text.length()-4], suffix2L, suffix2U) &&
                    stem<string_typeT>::is_either(text[text.length()-3], suffix3L, suffix3U) &&
                    stem<string_typeT>::is_either(text[text.length()-2], suffix4L, suffix4U) &&
                    stem<string_typeT>::is_either(text[text.length()-1], suffix5L, suffix5U) ) &&
                    (get_r1() <= text.length()-5) );
            }
        /// @brief R1 suffix comparison for six characters.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @param suffix4L The lowercased version of the fourth character of the suffix.
        /// @param suffix4U The uppercased version of the fourth character of the suffix.
        /// @param suffix5L The lowercased version of the fifth character of the suffix.
        /// @param suffix5U The uppercased version of the fifth character of the suffix.
        /// @param suffix6L The lowercased version of the sixth character of the suffix.
        /// @param suffix6U The uppercased version of the sixth character of the suffix.
        /// @returns @c true if suffix is in R1.
        [[nodiscard]]
        inline bool is_suffix_in_r1(const string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U,
                    const wchar_t suffix4L, const wchar_t suffix4U,
                    const wchar_t suffix5L, const wchar_t suffix5U,
                    const wchar_t suffix6L, const wchar_t suffix6U) noexcept
            {
            if (text.length() < 6)
                { return false; }
            return ((stem<string_typeT>::is_either(text[text.length()-6], suffix1L, suffix1U) &&
                    stem<string_typeT>::is_either(text[text.length()-5], suffix2L, suffix2U) &&
                    stem<string_typeT>::is_either(text[text.length()-4], suffix3L, suffix3U) &&
                    stem<string_typeT>::is_either(text[text.length()-3], suffix4L, suffix4U) &&
                    stem<string_typeT>::is_either(text[text.length()-2], suffix5L, suffix5U) &&
                    stem<string_typeT>::is_either(text[text.length()-1], suffix6L, suffix6U) ) &&
                    (get_r1() <= text.length()-6) );
            }

        // R2 suffix functions
        //-------------------------------------------------
        /// @brief R2 suffix comparison for one character.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @returns @c true if suffix is in R21.
        [[nodiscard]]
        inline bool is_suffix_in_r2(const string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U) noexcept
            {
            if (text.length() < 1)
                { return false; }
            return (stem<string_typeT>::is_either(text[text.length()-1], suffix1L, suffix1U) &&
                    (get_r2() <= text.length()-1) );
            }
        /// @brief R2 suffix comparison for two characters.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @returns @c true if suffix is in R2.
        [[nodiscard]]
        inline bool is_suffix_in_r2(const string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U) noexcept
            {
            if (text.length() < 2)
                { return false; }
            return ((stem<string_typeT>::is_either(text[text.length()-2], suffix1L, suffix1U) &&
                    stem<string_typeT>::is_either(text[text.length()-1], suffix2L, suffix2U) ) &&
                    (get_r2() <= text.length()-2) );
            }
        /// @brief R2 suffix comparison for three characters.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @returns @c true if suffix is in R2.
        [[nodiscard]]
        inline bool is_suffix_in_r2(const string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U) noexcept
            {
            if (text.length() < 3)
                { return false; }
            return ((stem<string_typeT>::is_either(text[text.length()-3], suffix1L, suffix1U) &&
                    stem<string_typeT>::is_either(text[text.length()-2], suffix2L, suffix2U) &&
                    stem<string_typeT>::is_either(text[text.length()-1], suffix3L, suffix3U) ) &&
                    (get_r2() <= text.length()-3) );
            }
        /// @brief R2 suffix comparison for four characters.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @param suffix4L The lowercased version of the fourth character of the suffix.
        /// @param suffix4U The uppercased version of the fourth character of the suffix.
        /// @returns @c true if suffix is in R2.
        [[nodiscard]]
        inline bool is_suffix_in_r2(const string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U,
                    const wchar_t suffix4L, const wchar_t suffix4U) noexcept
            {
            if (text.length() < 4)
                { return false; }
            return ((stem<string_typeT>::is_either(text[text.length()-4], suffix1L, suffix1U) &&
                    stem<string_typeT>::is_either(text[text.length()-3], suffix2L, suffix2U) &&
                    stem<string_typeT>::is_either(text[text.length()-2], suffix3L, suffix3U) &&
                    stem<string_typeT>::is_either(text[text.length()-1], suffix4L, suffix4U) ) &&
                    (get_r2() <= text.length()-4) );
            }
        /// @brief R2 suffix comparison for five characters.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @param suffix4L The lowercased version of the fourth character of the suffix.
        /// @param suffix4U The uppercased version of the fourth character of the suffix.
        /// @param suffix5L The lowercased version of the fifth character of the suffix.
        /// @param suffix5U The uppercased version of the fifth character of the suffix.
        /// @returns @c true if suffix is in R2.
        [[nodiscard]]
        inline bool is_suffix_in_r2(const string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U,
                    const wchar_t suffix4L, const wchar_t suffix4U,
                    const wchar_t suffix5L, const wchar_t suffix5U) noexcept
            {
            if (text.length() < 5)
                { return false; }
            return ((stem<string_typeT>::is_either(text[text.length()-5], suffix1L, suffix1U) &&
                    stem<string_typeT>::is_either(text[text.length()-4], suffix2L, suffix2U) &&
                    stem<string_typeT>::is_either(text[text.length()-3], suffix3L, suffix3U) &&
                    stem<string_typeT>::is_either(text[text.length()-2], suffix4L, suffix4U) &&
                    stem<string_typeT>::is_either(text[text.length()-1], suffix5L, suffix5U) ) &&
                    (get_r2() <= text.length()-5) );
            }
        /// @brief R2 suffix comparison for six characters.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @param suffix4L The lowercased version of the fourth character of the suffix.
        /// @param suffix4U The uppercased version of the fourth character of the suffix.
        /// @param suffix5L The lowercased version of the fifth character of the suffix.
        /// @param suffix5U The uppercased version of the fifth character of the suffix.
        /// @param suffix6L The lowercased version of the sixth character of the suffix.
        /// @param suffix6U The uppercased version of the sixth character of the suffix.
        /// @returns @c true if suffix is in R2.
        [[nodiscard]]
        inline bool is_suffix_in_r2(string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U,
                    const wchar_t suffix4L, const wchar_t suffix4U,
                    const wchar_t suffix5L, const wchar_t suffix5U,
                    const wchar_t suffix6L, const wchar_t suffix6U) noexcept
            {
            if (text.length() < 6)
                { return false; }
            return ((stem<string_typeT>::is_either(text[text.length()-6], suffix1L, suffix1U) &&
                    stem<string_typeT>::is_either(text[text.length()-5], suffix2L, suffix2U) &&
                    stem<string_typeT>::is_either(text[text.length()-4], suffix3L, suffix3U) &&
                    stem<string_typeT>::is_either(text[text.length()-3], suffix4L, suffix4U) &&
                    stem<string_typeT>::is_either(text[text.length()-2], suffix5L, suffix5U) &&
                    stem<string_typeT>::is_either(text[text.length()-1], suffix6L, suffix6U) ) &&
                    (get_r2() <= text.length()-6) );
            }
        /// @brief R2 suffix comparison for seven characters.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @param suffix4L The lowercased version of the fourth character of the suffix.
        /// @param suffix4U The uppercased version of the fourth character of the suffix.
        /// @param suffix5L The lowercased version of the fifth character of the suffix.
        /// @param suffix5U The uppercased version of the fifth character of the suffix.
        /// @param suffix6L The lowercased version of the sixth character of the suffix.
        /// @param suffix6U The uppercased version of the sixth character of the suffix.
        /// @param suffix7L The lowercased version of the seventh character of the suffix.
        /// @param suffix7U The uppercased version of the seventh character of the suffix.
        /// @returns @c true if suffix is in R2.
        [[nodiscard]]
        inline bool is_suffix_in_r2(const string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U,
                    const wchar_t suffix4L, const wchar_t suffix4U,
                    const wchar_t suffix5L, const wchar_t suffix5U,
                    const wchar_t suffix6L, const wchar_t suffix6U,
                    const wchar_t suffix7L, const wchar_t suffix7U) noexcept
            {
            if (text.length() < 7)
                { return false; }
            return ((stem<string_typeT>::is_either(text[text.length()-7], suffix1L, suffix1U) &&
                    stem<string_typeT>::is_either(text[text.length()-6], suffix2L, suffix2U) &&
                    stem<string_typeT>::is_either(text[text.length()-5], suffix3L, suffix3U) &&
                    stem<string_typeT>::is_either(text[text.length()-4], suffix4L, suffix4U) &&
                    stem<string_typeT>::is_either(text[text.length()-3], suffix5L, suffix5U) &&
                    stem<string_typeT>::is_either(text[text.length()-2], suffix6L, suffix6U) &&
                    stem<string_typeT>::is_either(text[text.length()-1], suffix7L, suffix7U) ) &&
                    (get_r2() <= text.length()-7) );
            }

        // Suffix removal functions
        //---------------------------
        /// @brief R1 deletion for one character suffix
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param success_on_find Return true if found, but not deleted.
        /// @returns @c true if characters match suffix and are deleted.
        inline bool delete_if_is_in_r1(string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const bool success_on_find = true)
            {
            assert(suffix1L == stem<string_typeT>::tolower_western(suffix1U) );
            if (text.length() < 1)
                {
                return false;
                }
            if (stem<string_typeT>::is_either(text[text.length()-1], suffix1L, suffix1U))
                {
                if (get_r1() <= text.length()-1)
                    {
                    text.pop_back();
                    update_r_sections(text);
                    return true;
                    }
                return success_on_find;
                }
            else
                {
                return false;
                }
            }
        /// @brief R1 deletion for two character suffix.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param success_on_find Return true if found, but not deleted.
        /// @returns @c true if characters match suffix and are deleted.
        inline bool delete_if_is_in_r1(string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const bool success_on_find = true)
            {
            if (text.length() < 2)
                {
                return false;
                }
            if (stem<string_typeT>::is_either(text[text.length()-2], suffix1L, suffix1U) &&
                stem<string_typeT>::is_either(text[text.length()-1], suffix2L, suffix2U))
                {
                if (get_r1() <= text.length()-2)
                    {
                    text.erase(text.length()-2);
                    update_r_sections(text);
                    return true;
                    }
                return success_on_find;
                }
            else
                {
                return false;
                }
            }
        /// @brief R1 deletion for three character suffix.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @param success_on_find Return true if found, but not deleted.
        /// @returns @c true if characters match suffix and are deleted.
        inline bool delete_if_is_in_r1(string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U,
                    const bool success_on_find = true)
            {
            if (text.length() < 3)
                {
                return false;
                }
            if (stem<string_typeT>::is_either(text[text.length()-3], suffix1L, suffix1U) &&
                stem<string_typeT>::is_either(text[text.length()-2], suffix2L, suffix2U) &&
                stem<string_typeT>::is_either(text[text.length()-1], suffix3L, suffix3U) )
                {
                if (get_r1() <= text.length()-3)
                    {
                    text.erase(text.length()-3);
                    update_r_sections(text);
                    return true;
                    }
                return success_on_find;
                }
            else
                {
                return false;
                }
            }
        /// @brief R1 deletion for four character suffix.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @param suffix4L The lowercased version of the fourth character of the suffix.
        /// @param suffix4U The uppercased version of the fourth character of the suffix.
        /// @param success_on_find Return true if found, but not deleted.
        /// @returns @c true if characters match suffix and are deleted.
        inline bool delete_if_is_in_r1(string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U,
                    const wchar_t suffix4L, const wchar_t suffix4U,
                    const bool success_on_find = true)
            {
            if (text.length() < 4)
                {
                return false;
                }
            if (stem<string_typeT>::is_either(text[text.length()-4], suffix1L, suffix1U) &&
                stem<string_typeT>::is_either(text[text.length()-3], suffix2L, suffix2U) &&
                stem<string_typeT>::is_either(text[text.length()-2], suffix3L, suffix3U) &&
                stem<string_typeT>::is_either(text[text.length()-1], suffix4L, suffix4U) )
                {
                if (get_r1() <= text.length()-4)
                    {
                    text.erase(text.length()-4);
                    update_r_sections(text);
                    return true;
                    }
                return success_on_find;
                }
            else
                {
                return false;
                }
            }
        /// @brief R1 deletion for five character suffix.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @param suffix4L The lowercased version of the fourth character of the suffix.
        /// @param suffix4U The uppercased version of the fourth character of the suffix.
        /// @param suffix5L The lowercased version of the fifth character of the suffix.
        /// @param suffix5U The uppercased version of the fifth character of the suffix.
        /// @param success_on_find Return true if found, but not deleted.
        /// @returns @c true if characters match suffix and are deleted.
        inline bool delete_if_is_in_r1(string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U,
                    const wchar_t suffix4L, const wchar_t suffix4U,
                    const wchar_t suffix5L, const wchar_t suffix5U,
                    const bool success_on_find = true)
            {
            if (text.length() < 5)
                {
                return false;
                }
            if (stem<string_typeT>::is_either(text[text.length()-5], suffix1L, suffix1U) &&
                stem<string_typeT>::is_either(text[text.length()-4], suffix2L, suffix2U) &&
                stem<string_typeT>::is_either(text[text.length()-3], suffix3L, suffix3U) &&
                stem<string_typeT>::is_either(text[text.length()-2], suffix4L, suffix4U) &&
                stem<string_typeT>::is_either(text[text.length()-1], suffix5L, suffix5U) )
                {
                if (get_r1() <= text.length()-5)
                    {
                    text.erase(text.length()-5);
                    update_r_sections(text);
                    return true;
                    }
                return success_on_find;
                }
            else
                {
                return false;
                }
            }
        /// @brief R1 deletion for six character suffix.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @param suffix4L The lowercased version of the fourth character of the suffix.
        /// @param suffix4U The uppercased version of the fourth character of the suffix.
        /// @param suffix5L The lowercased version of the fifth character of the suffix.
        /// @param suffix5U The uppercased version of the fifth character of the suffix.
        /// @param suffix6L The lowercased version of the sixth character of the suffix.
        /// @param suffix6U The uppercased version of the sixth character of the suffix.
        /// @param success_on_find Return true if found, but not deleted.
        /// @returns @c true if characters match suffix and are deleted.
        inline bool delete_if_is_in_r1(string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U,
                    const wchar_t suffix4L, const wchar_t suffix4U,
                    const wchar_t suffix5L, const wchar_t suffix5U,
                    const wchar_t suffix6L, const wchar_t suffix6U,
                    const bool success_on_find = true)
            {
            if (text.length() < 6)
                {
                return false;
                }
            if (stem<string_typeT>::is_either(text[text.length()-6], suffix1L, suffix1U) &&
                stem<string_typeT>::is_either(text[text.length()-5], suffix2L, suffix2U) &&
                stem<string_typeT>::is_either(text[text.length()-4], suffix3L, suffix3U) &&
                stem<string_typeT>::is_either(text[text.length()-3], suffix4L, suffix4U) &&
                stem<string_typeT>::is_either(text[text.length()-2], suffix5L, suffix5U) &&
                stem<string_typeT>::is_either(text[text.length()-1], suffix6L, suffix6U) )
                {
                if (get_r1() <= text.length()-6)
                    {
                    text.erase(text.length()-6);
                    update_r_sections(text);
                    return true;
                    }
                return success_on_find;
                }
            else
                {
                return false;
                }
            }
        /// @brief R1 deletion for seven character suffix.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @param suffix4L The lowercased version of the fourth character of the suffix.
        /// @param suffix4U The uppercased version of the fourth character of the suffix.
        /// @param suffix5L The lowercased version of the fifth character of the suffix.
        /// @param suffix5U The uppercased version of the fifth character of the suffix.
        /// @param suffix6L The lowercased version of the sixth character of the suffix.
        /// @param suffix6U The uppercased version of the sixth character of the suffix.
        /// @param suffix7L The lowercased version of the seventh character of the suffix.
        /// @param suffix7U The uppercased version of the seventh character of the suffix.
        /// @param success_on_find Return true if found, but not deleted.
        /// @returns @c true if characters match suffix and are deleted.
        inline bool delete_if_is_in_r1(string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U,
                    const wchar_t suffix4L, const wchar_t suffix4U,
                    const wchar_t suffix5L, const wchar_t suffix5U,
                    const wchar_t suffix6L, const wchar_t suffix6U,
                    const wchar_t suffix7L, const wchar_t suffix7U,
                    const bool success_on_find = true)
            {
            if (text.length() < 7)
                {
                return false;
                }
            if (stem<string_typeT>::is_either(text[text.length()-7], suffix1L, suffix1U) &&
                stem<string_typeT>::is_either(text[text.length()-6], suffix2L, suffix2U) &&
                stem<string_typeT>::is_either(text[text.length()-5], suffix3L, suffix3U) &&
                stem<string_typeT>::is_either(text[text.length()-4], suffix4L, suffix4U) &&
                stem<string_typeT>::is_either(text[text.length()-3], suffix5L, suffix5U) &&
                stem<string_typeT>::is_either(text[text.length()-2], suffix6L, suffix6U) &&
                stem<string_typeT>::is_either(text[text.length()-1], suffix7L, suffix7U) )
                {
                if (get_r1() <= text.length()-7)
                    {
                    text.erase(text.length()-7);
                    update_r_sections(text);
                    return true;
                    }
                return success_on_find;
                }
            else
                {
                return false;
                }
            }

        // R2 deletion functions
        //------------------------
        /// @brief R2 deletion for one character suffix.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param success_on_find Return true if found, but not deleted.
        /// @returns @c true if characters match suffix and are deleted.
        inline bool delete_if_is_in_r2(string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const bool success_on_find = true)
            {
            if (text.length() < 1)
                { return false; }
            else if (stem<string_typeT>::is_either(text[text.length()-1], suffix1L, suffix1U))
                {
                if (get_r2() <= text.length()-1)
                    {
                    text.pop_back();
                    update_r_sections(text);
                    return true;
                    }
                return success_on_find;
                }
            else
                { return false; }
            }
        /// @brief R2 deletion for two character suffix.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param success_on_find Return true if found, but not deleted.
        /// @returns @c true if characters match suffix and are deleted.
        inline bool delete_if_is_in_r2(string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const bool success_on_find = true)
            {
            if (text.length() < 2)
                {
                return false;
                }
            if (stem<string_typeT>::is_either(text[text.length()-2], suffix1L, suffix1U) &&
                stem<string_typeT>::is_either(text[text.length()-1], suffix2L, suffix2U))
                {
                if (get_r2() <= text.length()-2)
                    {
                    text.erase(text.length()-2);
                    update_r_sections(text);
                    return true;
                    }
                return success_on_find;
                }
            else
                {
                return false;
                }
            }
        /// @brief R2 deletion for three character suffix.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @param success_on_find Return true if found, but not deleted.
        /// @returns @c true if characters match suffix and are deleted.
        inline bool delete_if_is_in_r2(string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U,
                    const bool success_on_find = true)
            {
            if (text.length() < 3)
                {
                return false;
                }
            if (stem<string_typeT>::is_either(text[text.length()-3], suffix1L, suffix1U) &&
                stem<string_typeT>::is_either(text[text.length()-2], suffix2L, suffix2U) &&
                stem<string_typeT>::is_either(text[text.length()-1], suffix3L, suffix3U) )
                {
                if (get_r2() <= text.length()-3)
                    {
                    text.erase(text.length()-3);
                    update_r_sections(text);
                    return true;
                    }
                return success_on_find;
                }
            else
                {
                return false;
                }
            }
        /// @brief R2 deletion for four character suffix.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @param suffix4L The lowercased version of the fourth character of the suffix.
        /// @param suffix4U The uppercased version of the fourth character of the suffix.
        /// @param success_on_find Return true if found, but not deleted.
        /// @returns @c true if characters match suffix and are deleted.
        inline bool delete_if_is_in_r2(string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U,
                    const wchar_t suffix4L, const wchar_t suffix4U,
                    const bool success_on_find = true)
            {
            if (text.length() < 4)
                {
                return false;
                }
            if (stem<string_typeT>::is_either(text[text.length()-4], suffix1L, suffix1U) &&
                stem<string_typeT>::is_either(text[text.length()-3], suffix2L, suffix2U) &&
                stem<string_typeT>::is_either(text[text.length()-2], suffix3L, suffix3U) &&
                stem<string_typeT>::is_either(text[text.length()-1], suffix4L, suffix4U) )
                {
                if (get_r2() <= text.length()-4)
                    {
                    text.erase(text.length()-4);
                    update_r_sections(text);
                    return true;
                    }
                return success_on_find;
                }
            else
                {
                return false;
                }
            }
        /// @brief R2 deletion for five character suffix.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @param suffix4L The lowercased version of the fourth character of the suffix.
        /// @param suffix4U The uppercased version of the fourth character of the suffix.
        /// @param suffix5L The lowercased version of the fifth character of the suffix.
        /// @param suffix5U The uppercased version of the fifth character of the suffix.
        /// @param success_on_find Return true if found, but not deleted.
        /// @returns @c true if characters match suffix and are deleted.
        inline bool delete_if_is_in_r2(string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U,
                    const wchar_t suffix4L, const wchar_t suffix4U,
                    const wchar_t suffix5L, const wchar_t suffix5U,
                    const bool success_on_find = true)
            {
            if (text.length() < 5)
                {
                return false;
                }
            if (stem<string_typeT>::is_either(text[text.length()-5], suffix1L, suffix1U) &&
                stem<string_typeT>::is_either(text[text.length()-4], suffix2L, suffix2U) &&
                stem<string_typeT>::is_either(text[text.length()-3], suffix3L, suffix3U) &&
                stem<string_typeT>::is_either(text[text.length()-2], suffix4L, suffix4U) &&
                stem<string_typeT>::is_either(text[text.length()-1], suffix5L, suffix5U) )
                {
                if (get_r2() <= text.length()-5)
                    {
                    text.erase(text.length()-5);
                    update_r_sections(text);
                    return true;
                    }
                return success_on_find;
                }
            else
                {
                return false;
                }
            }
        /// @brief R2 deletion for six character suffix.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @param suffix4L The lowercased version of the fourth character of the suffix.
        /// @param suffix4U The uppercased version of the fourth character of the suffix.
        /// @param suffix5L The lowercased version of the fifth character of the suffix.
        /// @param suffix5U The uppercased version of the fifth character of the suffix.
        /// @param suffix6L The lowercased version of the sixth character of the suffix.
        /// @param suffix6U The uppercased version of the sixth character of the suffix.
        /// @param success_on_find Return true if found, but not deleted.
        /// @returns @c true if characters match suffix and are deleted.
        inline bool delete_if_is_in_r2(string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U,
                    const wchar_t suffix4L, const wchar_t suffix4U,
                    const wchar_t suffix5L, const wchar_t suffix5U,
                    const wchar_t suffix6L, const wchar_t suffix6U,
                    const bool success_on_find = true)
            {
            if (text.length() < 6)
                {
                return false;
                }
            if (stem<string_typeT>::is_either(text[text.length()-6], suffix1L, suffix1U) &&
                stem<string_typeT>::is_either(text[text.length()-5], suffix2L, suffix2U) &&
                stem<string_typeT>::is_either(text[text.length()-4], suffix3L, suffix3U) &&
                stem<string_typeT>::is_either(text[text.length()-3], suffix4L, suffix4U) &&
                stem<string_typeT>::is_either(text[text.length()-2], suffix5L, suffix5U) &&
                stem<string_typeT>::is_either(text[text.length()-1], suffix6L, suffix6U) )
                {
                if (get_r2() <= text.length()-6)
                    {
                    text.erase(text.length()-6);
                    update_r_sections(text);
                    return true;
                    }
                return success_on_find;
                }
            else
                {
                return false;
                }
            }
        /// @brief R2 deletion for seven character suffix.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @param suffix4L The lowercased version of the fourth character of the suffix.
        /// @param suffix4U The uppercased version of the fourth character of the suffix.
        /// @param suffix5L The lowercased version of the fifth character of the suffix.
        /// @param suffix5U The uppercased version of the fifth character of the suffix.
        /// @param suffix6L The lowercased version of the sixth character of the suffix.
        /// @param suffix6U The uppercased version of the sixth character of the suffix.
        /// @param suffix7L The lowercased version of the seventh character of the suffix.
        /// @param suffix7U The uppercased version of the seventh character of the suffix.
        /// @param success_on_find Return true if found, but not deleted.
        /// @returns @c true if characters match suffix and are deleted.
        inline bool delete_if_is_in_r2(string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U,
                    const wchar_t suffix4L, const wchar_t suffix4U,
                    const wchar_t suffix5L, const wchar_t suffix5U,
                    const wchar_t suffix6L, const wchar_t suffix6U,
                    const wchar_t suffix7L, const wchar_t suffix7U,
                    const bool success_on_find = true)
            {
            if (text.length() < 7)
                {
                return false;
                }
            if (stem<string_typeT>::is_either(text[text.length()-7], suffix1L, suffix1U) &&
                stem<string_typeT>::is_either(text[text.length()-6], suffix2L, suffix2U) &&
                stem<string_typeT>::is_either(text[text.length()-5], suffix3L, suffix3U) &&
                stem<string_typeT>::is_either(text[text.length()-4], suffix4L, suffix4U) &&
                stem<string_typeT>::is_either(text[text.length()-3], suffix5L, suffix5U) &&
                stem<string_typeT>::is_either(text[text.length()-2], suffix6L, suffix6U) &&
                stem<string_typeT>::is_either(text[text.length()-1], suffix7L, suffix7U) )
                {
                if (get_r2() <= text.length()-7)
                    {
                    text.erase(text.length()-7);
                    update_r_sections(text);
                    return true;
                    }
                return success_on_find;
                }
            else
                {
                return false;
                }
            }
        /// @brief R2 deletion for eight character suffix.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @param suffix4L The lowercased version of the fourth character of the suffix.
        /// @param suffix4U The uppercased version of the fourth character of the suffix.
        /// @param suffix5L The lowercased version of the fifth character of the suffix.
        /// @param suffix5U The uppercased version of the fifth character of the suffix.
        /// @param suffix6L The lowercased version of the sixth character of the suffix.
        /// @param suffix6U The uppercased version of the sixth character of the suffix.
        /// @param suffix7L The lowercased version of the seventh character of the suffix.
        /// @param suffix7U The uppercased version of the seventh character of the suffix.
        /// @param suffix8L The lowercased version of the eighth character of the suffix.
        /// @param suffix8U The uppercased version of the eighth character of the suffix.
        /// @param success_on_find Return true if found, but not deleted.
        /// @returns @c true if characters match suffix and are deleted.
        inline bool delete_if_is_in_r2(string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U,
                    const wchar_t suffix4L, const wchar_t suffix4U,
                    const wchar_t suffix5L, const wchar_t suffix5U,
                    const wchar_t suffix6L, const wchar_t suffix6U,
                    const wchar_t suffix7L, const wchar_t suffix7U,
                    const wchar_t suffix8L, const wchar_t suffix8U,
                    const bool success_on_find = true)
            {
            if (text.length() < 8)
                {
                return false;
                }
            if (stem<string_typeT>::is_either(text[text.length()-8], suffix1L, suffix1U) &&
                stem<string_typeT>::is_either(text[text.length()-7], suffix2L, suffix2U) &&
                stem<string_typeT>::is_either(text[text.length()-6], suffix3L, suffix3U) &&
                stem<string_typeT>::is_either(text[text.length()-5], suffix4L, suffix4U) &&
                stem<string_typeT>::is_either(text[text.length()-4], suffix5L, suffix5U) &&
                stem<string_typeT>::is_either(text[text.length()-3], suffix6L, suffix6U) &&
                stem<string_typeT>::is_either(text[text.length()-2], suffix7L, suffix7U) &&
                stem<string_typeT>::is_either(text[text.length()-1], suffix8L, suffix8U) )
                {
                if (get_r2() <= text.length()-8)
                    {
                    text.erase(text.length()-8);
                    update_r_sections(text);
                    return true;
                    }
                return success_on_find;
                }
            else
                {
                return false;
                }
            }

        // RV deletion functions
        //---------------------------
        /// @brief RV deletion for one character suffix.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param success_on_find Return true if found, but not deleted.
        /// @returns @c true if characters match suffix and are deleted.
        inline bool delete_if_is_in_rv(string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const bool success_on_find = true)
            {
            if (text.length() < 1)
                {
                return false;
                }
            if (stem<string_typeT>::is_either(text[text.length()-1], suffix1L, suffix1U))
                {
                if (get_rv() <= text.length()-1)
                    {
                    text.pop_back();
                    update_r_sections(text);
                    return true;
                    }
                return success_on_find;
                }
            else
                {
                return false;
                }
            }
        /// @brief RV deletion for two character suffix.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param success_on_find Return true if found, but not deleted.
        /// @returns @c true if characters match suffix and are deleted.
        inline bool delete_if_is_in_rv(string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const bool success_on_find = true)
            {
            if (text.length() < 2)
                {
                return false;
                }
            if (stem<string_typeT>::is_either(text[text.length()-2], suffix1L, suffix1U) &&
                stem<string_typeT>::is_either(text[text.length()-1], suffix2L, suffix2U))
                {
                if (get_rv() <= text.length()-2)
                    {
                    text.erase(text.length()-2);
                    update_r_sections(text);
                    return true;
                    }
                return success_on_find;
                }
            else
                {
                return false;
                }
            }
        /// @brief RV deletion for three character suffix.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @param success_on_find Return true if found, but not deleted.
        /// @returns @c true if characters match suffix and are deleted.
        inline bool delete_if_is_in_rv(string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U,
                    const bool success_on_find = true)
            {
            if (text.length() < 3)
                {
                return false;
                }
            if (stem<string_typeT>::is_either(text[text.length()-3], suffix1L, suffix1U) &&
                stem<string_typeT>::is_either(text[text.length()-2], suffix2L, suffix2U) &&
                stem<string_typeT>::is_either(text[text.length()-1], suffix3L, suffix3U) )
                {
                if (get_rv() <= text.length()-3)
                    {
                    text.erase(text.length()-3);
                    update_r_sections(text);
                    return true;
                    }
                return success_on_find;
                }
            else
                {
                return false;
                }
            }
        /// @brief RV deletion for four character suffix.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @param suffix4L The lowercased version of the fourth character of the suffix.
        /// @param suffix4U The uppercased version of the fourth character of the suffix.
        /// @param success_on_find Return true if found, but not deleted.
        /// @returns @c true if characters match suffix and are deleted.
        inline bool delete_if_is_in_rv(string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U,
                    const wchar_t suffix4L, const wchar_t suffix4U,
                    const bool success_on_find = true)
            {
            if (text.length() < 4)
                {
                return false;
                }
            if (stem<string_typeT>::is_either(text[text.length()-4], suffix1L, suffix1U) &&
                stem<string_typeT>::is_either(text[text.length()-3], suffix2L, suffix2U) &&
                stem<string_typeT>::is_either(text[text.length()-2], suffix3L, suffix3U) &&
                stem<string_typeT>::is_either(text[text.length()-1], suffix4L, suffix4U) )
                {
                if (get_rv() <= text.length()-4)
                    {
                    text.erase(text.length()-4);
                    update_r_sections(text);
                    return true;
                    }
                return success_on_find;
                }
            else
                {
                return false;
                }
            }
        /// @brief RV deletion for five character suffix.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @param suffix4L The lowercased version of the fourth character of the suffix.
        /// @param suffix4U The uppercased version of the fourth character of the suffix.
        /// @param suffix5L The lowercased version of the fifth character of the suffix.
        /// @param suffix5U The uppercased version of the fifth character of the suffix.
        /// @param success_on_find Return true if found, but not deleted.
        /// @returns @c true if characters match suffix and are deleted.
        inline bool delete_if_is_in_rv(string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U,
                    const wchar_t suffix4L, const wchar_t suffix4U,
                    const wchar_t suffix5L, const wchar_t suffix5U,
                    const bool success_on_find = true)
            {
            if (text.length() < 5)
                {
                return false;
                }
            if (stem<string_typeT>::is_either(text[text.length()-5], suffix1L, suffix1U) &&
                stem<string_typeT>::is_either(text[text.length()-4], suffix2L, suffix2U) &&
                stem<string_typeT>::is_either(text[text.length()-3], suffix3L, suffix3U) &&
                stem<string_typeT>::is_either(text[text.length()-2], suffix4L, suffix4U) &&
                stem<string_typeT>::is_either(text[text.length()-1], suffix5L, suffix5U) )
                {
                if (get_rv() <= text.length()-5)
                    {
                    text.erase(text.length()-5);
                    update_r_sections(text);
                    return true;
                    }
                return success_on_find;
                }
            else
                {
                return false;
                }
            }
        /// @brief RV deletion for six character suffix.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @param suffix4L The lowercased version of the fourth character of the suffix.
        /// @param suffix4U The uppercased version of the fourth character of the suffix.
        /// @param suffix5L The lowercased version of the fifth character of the suffix.
        /// @param suffix5U The uppercased version of the fifth character of the suffix.
        /// @param suffix6L The lowercased version of the sixth character of the suffix.
        /// @param suffix6U The uppercased version of the sixth character of the suffix.
        /// @param success_on_find Return true if found, but not deleted.
        /// @returns @c true if characters match suffix and are deleted.
        inline bool delete_if_is_in_rv(string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U,
                    const wchar_t suffix4L, const wchar_t suffix4U,
                    const wchar_t suffix5L, const wchar_t suffix5U,
                    const wchar_t suffix6L, const wchar_t suffix6U,
                    const bool success_on_find = true)
            {
            if (text.length() < 6)
                {
                return false;
                }
            if (stem<string_typeT>::is_either(text[text.length()-6], suffix1L, suffix1U) &&
                stem<string_typeT>::is_either(text[text.length()-5], suffix2L, suffix2U) &&
                stem<string_typeT>::is_either(text[text.length()-4], suffix3L, suffix3U) &&
                stem<string_typeT>::is_either(text[text.length()-3], suffix4L, suffix4U) &&
                stem<string_typeT>::is_either(text[text.length()-2], suffix5L, suffix5U) &&
                stem<string_typeT>::is_either(text[text.length()-1], suffix6L, suffix6U) )
                {
                if (get_rv() <= text.length()-6)
                    {
                    text.erase(text.length()-6);
                    update_r_sections(text);
                    return true;
                    }
                return success_on_find;
                }
            else
                {
                return false;
                }
            }
        /// @brief RV deletion for seven character suffix.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @param suffix4L The lowercased version of the fourth character of the suffix.
        /// @param suffix4U The uppercased version of the fourth character of the suffix.
        /// @param suffix5L The lowercased version of the fifth character of the suffix.
        /// @param suffix5U The uppercased version of the fifth character of the suffix.
        /// @param suffix6L The lowercased version of the sixth character of the suffix.
        /// @param suffix6U The uppercased version of the sixth character of the suffix.
        /// @param suffix7L The lowercased version of the seventh character of the suffix.
        /// @param suffix7U The uppercased version of the seventh character of the suffix.
        /// @param success_on_find Return true if found, but not deleted.
        /// @returns @c true if characters match suffix and are deleted.
        inline bool delete_if_is_in_rv(string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U,
                    const wchar_t suffix4L, const wchar_t suffix4U,
                    const wchar_t suffix5L, const wchar_t suffix5U,
                    const wchar_t suffix6L, const wchar_t suffix6U,
                    const wchar_t suffix7L, const wchar_t suffix7U,
                    const bool success_on_find = true)
            {
            if (text.length() < 7)
                {
                return false;
                }
            if (stem<string_typeT>::is_either(text[text.length()-7], suffix1L, suffix1U) &&
                stem<string_typeT>::is_either(text[text.length()-6], suffix2L, suffix2U) &&
                stem<string_typeT>::is_either(text[text.length()-5], suffix3L, suffix3U) &&
                stem<string_typeT>::is_either(text[text.length()-4], suffix4L, suffix4U) &&
                stem<string_typeT>::is_either(text[text.length()-3], suffix5L, suffix5U) &&
                stem<string_typeT>::is_either(text[text.length()-2], suffix6L, suffix6U) &&
                stem<string_typeT>::is_either(text[text.length()-1], suffix7L, suffix7U) )
                {
                if (get_rv() <= text.length()-7)
                    {
                    text.erase(text.length()-7);
                    update_r_sections(text);
                    return true;
                    }
                return success_on_find;
                }
            else
                {
                return false;
                }
            }
        /// @brief RV deletion for eight character suffix.
        /// @param text The string being reviewed.
        /// @param suffix1L The lowercased version of the first character of the suffix.
        /// @param suffix1U The uppercased version of the first character of the suffix.
        /// @param suffix2L The lowercased version of the second character of the suffix.
        /// @param suffix2U The uppercased version of the second character of the suffix.
        /// @param suffix3L The lowercased version of the third character of the suffix.
        /// @param suffix3U The uppercased version of the third character of the suffix.
        /// @param suffix4L The lowercased version of the fourth character of the suffix.
        /// @param suffix4U The uppercased version of the fourth character of the suffix.
        /// @param suffix5L The lowercased version of the fifth character of the suffix.
        /// @param suffix5U The uppercased version of the fifth character of the suffix.
        /// @param suffix6L The lowercased version of the sixth character of the suffix.
        /// @param suffix6U The uppercased version of the sixth character of the suffix.
        /// @param suffix7L The lowercased version of the seventh character of the suffix.
        /// @param suffix7U The uppercased version of the seventh character of the suffix.
        /// @param suffix8L The lowercased version of the eighth character of the suffix.
        /// @param suffix8U The uppercased version of the eighth character of the suffix.
        /// @param success_on_find Return true if found, but not deleted.
        /// @returns @c true if characters match suffix and are deleted.
        inline bool delete_if_is_in_rv(string_typeT& text,
                    const wchar_t suffix1L, const wchar_t suffix1U,
                    const wchar_t suffix2L, const wchar_t suffix2U,
                    const wchar_t suffix3L, const wchar_t suffix3U,
                    const wchar_t suffix4L, const wchar_t suffix4U,
                    const wchar_t suffix5L, const wchar_t suffix5U,
                    const wchar_t suffix6L, const wchar_t suffix6U,
                    const wchar_t suffix7L, const wchar_t suffix7U,
                    const wchar_t suffix8L, const wchar_t suffix8U,
                    const bool success_on_find = true)
            {
            if (text.length() < 8)
                {
                return false;
                }
            if (stem<string_typeT>::is_either(text[text.length()-8], suffix1L, suffix1U) &&
                stem<string_typeT>::is_either(text[text.length()-7], suffix2L, suffix2U) &&
                stem<string_typeT>::is_either(text[text.length()-6], suffix3L, suffix3U) &&
                stem<string_typeT>::is_either(text[text.length()-5], suffix4L, suffix4U) &&
                stem<string_typeT>::is_either(text[text.length()-4], suffix5L, suffix5U) &&
                stem<string_typeT>::is_either(text[text.length()-3], suffix6L, suffix6U) &&
                stem<string_typeT>::is_either(text[text.length()-2], suffix7L, suffix7U) &&
                stem<string_typeT>::is_either(text[text.length()-1], suffix8L, suffix8U) )
                {
                if (get_rv() <= text.length()-8)
                    {
                    text.erase(text.length()-8);
                    update_r_sections(text);
                    return true;
                    }
                return success_on_find;
                }
            else
                {
                return false;
                }
            }

        /// @brief Removes umlauts from string.
        /// @param text The string to update.
        void remove_german_umlauts(string_typeT& text)
            {
            for (size_t i = 0; i < text.length(); ++i)
                {
                if (text[i] == 0xC4)
                    {
                    text[i] = common_lang_constants::UPPER_A;
                    }
                else if (text[i] == 0xD6)
                    {
                    text[i] = common_lang_constants::UPPER_O;
                    }
                else if (text[i] == 0xDC)
                    {
                    text[i] = common_lang_constants::UPPER_U;
                    }
                else if (text[i] == 0xE4 )
                    {
                    text[i] = common_lang_constants::LOWER_A;
                    }
                else if (text[i] == 0xF6)
                    {
                    text[i] = common_lang_constants::LOWER_O;
                    }
                else if (text[i] == 0xFC)
                    {
                    text[i] = common_lang_constants::LOWER_U;
                    }
                }
            }
        /// @brief Encodes acutes to graves.
        /// @param[in,out] text The string to update.
        void italian_acutes_to_graves(string_typeT& text) noexcept
            {
            std::transform(text.cbegin(), text.cend(), text.begin(),
                [](const auto& ch) noexcept
                {
                return (ch == common_lang_constants::UPPER_A_ACUTE) ?
                    common_lang_constants::UPPER_A_GRAVE :
                    (ch == common_lang_constants::UPPER_E_ACUTE) ?
                    common_lang_constants::UPPER_E_GRAVE :
                    (ch == common_lang_constants::UPPER_I_ACUTE) ?
                    common_lang_constants::UPPER_I_GRAVE :
                    (ch == common_lang_constants::UPPER_O_ACUTE) ?
                    common_lang_constants::UPPER_O_GRAVE :
                    (ch == common_lang_constants::UPPER_U_ACUTE) ?
                    0xD9 :
                    (ch == common_lang_constants::LOWER_A_ACUTE) ?
                    common_lang_constants::LOWER_A_GRAVE :
                    (ch == common_lang_constants::LOWER_E_ACUTE) ?
                    common_lang_constants::LOWER_E_GRAVE :
                    (ch == common_lang_constants::LOWER_I_ACUTE) ?
                    common_lang_constants::LOWER_I_GRAVE :
                    (ch == common_lang_constants::LOWER_O_ACUTE) ?
                    common_lang_constants::LOWER_O_GRAVE :
                    (ch == common_lang_constants::LOWER_U_ACUTE) ?
                    0xF9 :
                    ch;
                });
            }

        /// @brief Hashes initial y, y after a vowel, and i between vowels into hashed character.
        /// @param text The string to update.
        /// @param vowel_string The list of vowels used by the stemmer's language.
        void hash_dutch_yi(string_typeT& text,
                    const wchar_t* vowel_string)
            {
            // need at least 2 letters for hashing
            if (text.length() < 2)
                { return; }

            if (text[0] == common_lang_constants::LOWER_Y)
                {
                text[0] = LOWER_Y_HASH;
                }
            else if (text[0] == common_lang_constants::UPPER_Y)
                {
                text[0] = UPPER_Y_HASH;
                }
            bool in_vowel_block = stem<string_typeT>::is_one_of(text[0], vowel_string);

            size_t i = 1;
            for (i = 1; i < text.length()-1; ++i)
                {
                if (in_vowel_block &&
                    text[i] == common_lang_constants::LOWER_I &&
                    stem<string_typeT>::is_one_of(text[i+1], vowel_string) )
                    {
                    text[i] = LOWER_I_HASH;
                    in_vowel_block = false;
                    }
                else if (in_vowel_block &&
                    text[i] == common_lang_constants::UPPER_I &&
                    stem<string_typeT>::is_one_of(text[i+1], vowel_string) )
                    {
                    text[i] = UPPER_I_HASH;
                    in_vowel_block = false;
                    }
                else if (in_vowel_block &&
                    text[i] == common_lang_constants::LOWER_Y)
                    {
                    text[i] = LOWER_Y_HASH;
                    in_vowel_block = false;
                    }
                else if (in_vowel_block &&
                    text[i] == common_lang_constants::UPPER_Y)
                    {
                    text[i] = UPPER_Y_HASH;
                    in_vowel_block = false;
                    }
                else if (stem<string_typeT>::is_one_of(text[i], vowel_string) )
                    {
                    in_vowel_block = true;
                    }
                else
                    {
                    in_vowel_block = false;
                    }
                }
            // check the last letter
            if (in_vowel_block &&
                text[i] == common_lang_constants::LOWER_Y)
                {
                text[i] = LOWER_Y_HASH;
                }
            else if (in_vowel_block &&
                text[i] == common_lang_constants::UPPER_Y)
                {
                text[i] = UPPER_Y_HASH;
                }
            }

        /// @brief Unhashes y and i in a string.
        /// @param text The string to update.
        inline void unhash_dutch_yi(string_typeT& text)
            {
            std::transform(text.cbegin(), text.cend(), text.begin(),
                [](const auto& ch) noexcept
                {
                return (ch == LOWER_Y_HASH) ?
                    common_lang_constants::LOWER_Y :
                    (ch == UPPER_Y_HASH) ?
                    common_lang_constants::UPPER_Y :
                    (ch == LOWER_I_HASH) ?
                    common_lang_constants::LOWER_I :
                    (ch == UPPER_I_HASH) ?
                    common_lang_constants::UPPER_I :
                    ch;
                });
            }

        /// @brief Hash 'u' and 'y' between vowels.
        /// @param text The string to update.
        /// @param vowel_string The list of vowels used by the stemmer's language.
        void hash_german_yu(string_typeT& text,
                    const wchar_t* vowel_string)
            {
            // need at least 2 letters for hashing
            if (text.length() < 2)
                { return; }

            bool in_vowel_block = stem<string_typeT>::is_one_of(text[0], vowel_string);

            for (size_t i = 1; i < text.length()-1; ++i)
                {
                if (in_vowel_block &&
                    stem<string_typeT>::is_one_of(text[i], vowel_string) &&
                    stem<string_typeT>::is_one_of(text[i+1], vowel_string) )
                    {
                    if (text[i] == common_lang_constants::LOWER_Y)
                        {
                        text[i] = LOWER_Y_HASH;
                        }
                    else if (text[i] == common_lang_constants::UPPER_Y)
                        {
                        text[i] = UPPER_Y_HASH;
                        }
                    else if (text[i] == common_lang_constants::LOWER_U)
                        {
                        text[i] = LOWER_U_HASH;
                        }
                    else if (text[i] == common_lang_constants::UPPER_U)
                        {
                        text[i] = UPPER_U_HASH;
                        }
                    }
                else if (stem<string_typeT>::is_one_of(text[i], vowel_string) )
                    {
                    in_vowel_block = true;
                    }
                else
                    {
                    in_vowel_block = false;
                    }
                }
            // hashable values must be between vowels, so don't bother looking at last letter
            }

        /// @brief Unhashes y and u in a string.
        /// @param text The string to update.
        inline void unhash_german_yu(string_typeT& text)
            {
            std::transform(text.cbegin(), text.cend(), text.begin(),
                [](const auto& ch) noexcept
                {
                return (ch == LOWER_Y_HASH) ?
                    common_lang_constants::LOWER_Y :
                    (ch == UPPER_Y_HASH) ?
                    common_lang_constants::UPPER_Y :
                    (ch == LOWER_U_HASH) ?
                    common_lang_constants::LOWER_U :
                    (ch == UPPER_U_HASH) ?
                    common_lang_constants::UPPER_U :
                    ch;
                });
            }

        /** @brief Hashes the following:\n
             ï -> [control character]i\n
             ë -> [control character]i
            @param[in,out] text The string to hash.*/
        void hash_french_ei_diaeresis(string_typeT& text)
            {
            for (size_t i = 0; i < text.length(); ++i)
                {
                if (text[i] == common_lang_constants::LOWER_I_UMLAUTS)
                    {
                    text[i] = common_lang_constants::LOWER_I;
                    text.insert(text.begin() + i, DIARESIS_HASH);
                    }
                else if (text[i] == common_lang_constants::UPPER_I_UMLAUTS)
                    {
                    text[i] = common_lang_constants::UPPER_I;
                    text.insert(text.begin() + i, DIARESIS_HASH);
                    }
                else if (text[i] == common_lang_constants::LOWER_E_UMLAUTS)
                    {
                    text[i] = common_lang_constants::LOWER_E;
                    text.insert(text.begin() + i, DIARESIS_HASH);
                    }
                else if (text[i] == common_lang_constants::UPPER_E_UMLAUTS)
                    {
                    text[i] = common_lang_constants::UPPER_E;
                    text.insert(text.begin() + i, DIARESIS_HASH);
                    }
                }
            }

        /** @brief Unhashes 'e' and 'i' with diareses back to 'ë' and 'ï'.
            @param[in,out] text The string to unhash.*/
        void unhash_french_ei_diaeresis(string_typeT& text)
            {
            for (size_t i = 0; i < text.length(); ++i)
                {
                if (text[i] == DIARESIS_HASH)
                    {
                    text.erase(i, 1);
                    if (text[i] == common_lang_constants::LOWER_I)
                        { text[i] = common_lang_constants::LOWER_I_UMLAUTS; }
                    else if (text[i] == common_lang_constants::UPPER_I)
                        { text[i] = common_lang_constants::UPPER_I_UMLAUTS; }
                    else if (text[i] == common_lang_constants::LOWER_E)
                        { text[i] = common_lang_constants::LOWER_E_UMLAUTS; }
                    else if (text[i] == common_lang_constants::UPPER_E)
                        { text[i] = common_lang_constants::UPPER_E_UMLAUTS; }
                    }
                }
            }

        /** Hash u or i preceded and followed by a vowel, and y preceded or followed by a vowel.
            u after q is also hashed. For example,\n
            jouer        ->         joUer
            ennuie       ->         ennuIe
            yeux         ->         Yeux
            quand        ->         qUand
        @param[in,out] text The string to update.
        @param vowel_string The list of vowels used by the stemmer's language.*/
        void hash_french_yui(string_typeT& text,
                    const wchar_t* vowel_string)
            {
            // need at least 2 letters for hashing
            if (text.length() < 2)
                { return; }

            bool in_vowel_block = false;

            // start loop at zero because 'y' at start of string can be hashed
            size_t i = 0;
            for (i = 0; i < text.length()-1; ++i)
                {
                if (in_vowel_block &&
                    stem<string_typeT>::is_one_of(text[i], vowel_string) &&
                    stem<string_typeT>::is_one_of(text[i+1], vowel_string) )
                    {
                    if (text[i] == common_lang_constants::LOWER_Y)
                        {
                        text[i] = LOWER_Y_HASH;
                        in_vowel_block = false;
                        }
                    else if (text[i] == common_lang_constants::UPPER_Y)
                        {
                        text[i] = UPPER_Y_HASH;
                        in_vowel_block = false;
                        }
                    else if (text[i] == common_lang_constants::LOWER_U)
                        {
                        text[i] = LOWER_U_HASH;
                        in_vowel_block = false;
                        }
                    else if (text[i] == common_lang_constants::UPPER_U)
                        {
                        text[i] = UPPER_U_HASH;
                        in_vowel_block = false;
                        }
                    else if (text[i] == common_lang_constants::LOWER_I)
                        {
                        text[i] = LOWER_I_HASH;
                        in_vowel_block = false;
                        }
                    else if (text[i] == common_lang_constants::UPPER_I)
                        {
                        text[i] = UPPER_I_HASH;
                        in_vowel_block = false;
                        }
                    }
                // if just previous letter is a vowel then examine for 'y'
                else if (in_vowel_block &&
                        text[i] == common_lang_constants::LOWER_Y)
                    {
                    text[i] = LOWER_Y_HASH;
                    in_vowel_block = false;
                    }
                else if (in_vowel_block &&
                        text[i] == common_lang_constants::UPPER_Y)
                    {
                    text[i] = UPPER_Y_HASH;
                    in_vowel_block = false;
                    }
                // if just following letter is a vowel then examine for 'y'
                else if (text[i] == common_lang_constants::LOWER_Y &&
                        stem<string_typeT>::is_one_of(text[i+1], vowel_string) &&
                        stem<string_typeT>::is_neither(text[i+1], common_lang_constants::LOWER_Y,
                                                       common_lang_constants::UPPER_Y) )
                    {
                    text[i] = LOWER_Y_HASH;
                    in_vowel_block = false;
                    }
                else if (text[i] == common_lang_constants::UPPER_Y &&
                        stem<string_typeT>::is_one_of(text[i+1], vowel_string) &&
                        stem<string_typeT>::is_neither(text[i+1], common_lang_constants::LOWER_Y,
                                                       common_lang_constants::UPPER_Y) )
                    {
                    text[i] = UPPER_Y_HASH;
                    in_vowel_block = false;
                    }
                else if (stem<string_typeT>::is_one_of(text[i], vowel_string) )
                    {
                    if (text[i] == common_lang_constants::LOWER_U &&
                        (i > 0) &&
                        stem<string_typeT>::is_either(text[i-1], common_lang_constants::LOWER_Q,
                                                      common_lang_constants::UPPER_Q) )
                        {
                        text[i] = LOWER_U_HASH;
                        in_vowel_block = false;
                        }
                    else if (text[i] == common_lang_constants::UPPER_U &&
                        (i > 0) &&
                        stem<string_typeT>::is_either(text[i-1], common_lang_constants::LOWER_Q,
                                                      common_lang_constants::UPPER_Q) )
                        {
                        text[i] = UPPER_U_HASH;
                        in_vowel_block = false;
                        }
                    else
                        {
                        in_vowel_block = true;
                        }
                    }
                else
                    {
                    in_vowel_block = false;
                    }
                }
            // verify that the last letter
            if (text[i] == common_lang_constants::LOWER_Y &&
                (i > 0) &&
                stem<string_typeT>::is_one_of(text[i-1], vowel_string)    )
                {
                text[i] = LOWER_Y_HASH;
                }
            else if (text[i] == common_lang_constants::UPPER_Y &&
                    (i > 0) &&
                    stem<string_typeT>::is_one_of(text[i-1], vowel_string) )
                {
                text[i] = UPPER_Y_HASH;
                }
            else if (text[i] == common_lang_constants::LOWER_U &&
                    (i > 0) &&
                    stem<string_typeT>::is_either(text[i-1], common_lang_constants::LOWER_Q,
                                                  common_lang_constants::UPPER_Q) )
                {
                text[i] = LOWER_U_HASH;
                }
            else if (text[i] == common_lang_constants::UPPER_U &&
                    (i > 0) &&
                    stem<string_typeT>::is_either(text[i-1], common_lang_constants::LOWER_Q,
                                                  common_lang_constants::UPPER_Q) )
                {
                text[i] = UPPER_U_HASH;
                }
            }

        /// @brief Unhashes y, u, and i in a string.
        /// @param text The string to update.
        void unhash_french_yui(string_typeT& text)
            {
            stem<string_typeT>::replace_all(text, LOWER_Y_HASH, common_lang_constants::LOWER_Y);
            stem<string_typeT>::replace_all(text, UPPER_Y_HASH, common_lang_constants::UPPER_Y);
            stem<string_typeT>::replace_all(text, LOWER_U_HASH, common_lang_constants::LOWER_U);
            stem<string_typeT>::replace_all(text, UPPER_U_HASH, common_lang_constants::UPPER_U);
            stem<string_typeT>::replace_all(text, LOWER_I_HASH, common_lang_constants::LOWER_I);
            stem<string_typeT>::replace_all(text, UPPER_I_HASH, common_lang_constants::UPPER_I);
            }

        /// @brief Hashes Y and y in a string.
        /// @param text The string to update.
        /// @param vowel_string The list of vowels used by the stemmer's language.
        void hash_y(string_typeT& text,
                    const wchar_t* vowel_string)
            {
            // need at least 2 letters for hashing
            if (text.length() < 2)
                { return; }

            // if first letter is a 'y', then it is likely not a vowel
            if (text[0] == common_lang_constants::LOWER_Y)
                {
                text[0] = LOWER_Y_HASH;
                }
            else if (text[0] == common_lang_constants::UPPER_Y)
                {
                text[0] = UPPER_Y_HASH;
                }

            bool in_vowel_block = stem<string_typeT>::is_one_of(text[0], vowel_string);

            for (size_t i = 1; i < text.length(); ++i)
                {
                // LOWER_Y after vowel is a consonant
                if (in_vowel_block &&
                    text[i] == common_lang_constants::LOWER_Y)
                    {
                    text[i] = LOWER_Y_HASH;
                    in_vowel_block = false;
                    }
                else if (in_vowel_block &&
                    text[i] == common_lang_constants::UPPER_Y)
                    {
                    text[i] = UPPER_Y_HASH;
                    in_vowel_block = false;
                    }
                else if (stem<string_typeT>::is_one_of(text[i], vowel_string) )
                    {
                    in_vowel_block = true;
                    }
                // we are on a consonant
                else
                    {
                    in_vowel_block = false;
                    }
                }
            }

        /// @brief Unhashes Y and y in a string.
        /// @param text The string to update.
        inline void unhash_y(string_typeT& text)
            {
            std::transform(text.cbegin(), text.cend(), text.begin(),
                [](const auto& ch) noexcept
                {
                return (ch == LOWER_Y_HASH) ?
                    common_lang_constants::LOWER_Y :
                    (ch == UPPER_Y_HASH) ?
                    common_lang_constants::UPPER_Y :
                    ch;
                });
            }

        /// @brief Hashes u after q, and u, i between vowels.
        /// @param text The string to update.
        /// @param vowel_string The list of vowels used by the stemmer's language.
        void hash_italian_ui(string_typeT& text,
                    const wchar_t* vowel_string)
            {
            // need at least 2 letters for hashing
            if (text.length() < 2)
                { return; }

            bool in_vowel_block = stem<string_typeT>::is_one_of(text[0], vowel_string);
            constexpr static std::array<wchar_t, 4> uiValues =
                {
                common_lang_constants::LOWER_U,
                common_lang_constants::UPPER_U,
                common_lang_constants::LOWER_I,
                common_lang_constants::UPPER_I
                };

            size_t i = 1;
            for (i = 1; i < text.length()-1; ++i)
                {
                // u or i in between vowels
                if (in_vowel_block &&
                    std::find(uiValues.cbegin(), uiValues.cend(), text[i]) != uiValues.cend() &&
                    stem<string_typeT>::is_one_of(text[i+1], vowel_string) )
                    {
                    if (text[i] == common_lang_constants::LOWER_I )
                        {
                        text[i] = LOWER_I_HASH;
                        }
                    else if (text[i] == common_lang_constants::UPPER_I )
                        {
                        text[i] = UPPER_I_HASH;
                        }
                    else if (text[i] == common_lang_constants::LOWER_U)
                        {
                        text[i] = LOWER_U_HASH;
                        }
                    else if (text[i] == common_lang_constants::UPPER_U)
                        {
                        text[i] = UPPER_U_HASH;
                        }
                    }
                else if (stem<string_typeT>::is_one_of(text[i], vowel_string) )
                    {
                    /* u after q should be encrypted and not be
                       treated as a vowel*/
                    if (text[i] == common_lang_constants::LOWER_U &&
                        (i > 0) &&
                        stem<string_typeT>::is_either(text[i-1], common_lang_constants::LOWER_Q,
                                                      common_lang_constants::UPPER_Q) )
                        {
                        text[i] = LOWER_U_HASH;
                        in_vowel_block = false;
                        }
                    else if (text[i] == common_lang_constants::UPPER_U &&
                        (i > 0) &&
                        stem<string_typeT>::is_either(text[i-1], common_lang_constants::LOWER_Q,
                                                      common_lang_constants::UPPER_Q) )
                        {
                        text[i] = UPPER_U_HASH;
                        in_vowel_block = false;
                        }
                    else
                        {
                        in_vowel_block = true;
                        }
                    }
                // we are on a consonant
                else
                    {
                    in_vowel_block = false;
                    }
                }
            // verify the last letter
            if (text[i] == common_lang_constants::LOWER_U &&
                (i > 0) &&
                stem<string_typeT>::is_either(text[i-1], common_lang_constants::LOWER_Q,
                                              common_lang_constants::UPPER_Q) )
                {
                text[i] = LOWER_U_HASH;
                }
            else if (text[i] == common_lang_constants::UPPER_U &&
                (i > 0) &&
                stem<string_typeT>::is_either(text[i-1], common_lang_constants::LOWER_Q,
                                              common_lang_constants::UPPER_Q) )
                {
                text[i] = UPPER_U_HASH;
                }
            }

        /// @brief Unhashes Italian UIs in a string.
        /// @param text The string to update.
        inline void unhash_italian_ui(string_typeT& text) noexcept
            {
            std::transform(text.cbegin(), text.cend(), text.begin(),
                [](const auto& ch) noexcept
                {
                return (ch == LOWER_I_HASH) ?
                    common_lang_constants::LOWER_I :
                    (ch == UPPER_I_HASH) ?
                    common_lang_constants::UPPER_I :
                    (ch == LOWER_U_HASH) ?
                    common_lang_constants::LOWER_U :
                    (ch == UPPER_U_HASH) ?
                    common_lang_constants::UPPER_U :
                    ch;
                });
            }

        /// @brief Encodes Dutch umlauts (diaerises) in a string.
        /// @param text The string to update.
        void remove_dutch_umlauts(string_typeT& text)
            {
            for (size_t i = 0; i < text.length(); ++i)
                {
                if (text[i] == 0xC4)
                    {
                    text[i] = common_lang_constants::UPPER_A;
                    }
                else if (text[i] == 0xCB)
                    {
                    text[i] = common_lang_constants::UPPER_E;
                    }
                else if (text[i] == 0xCF)
                    {
                    text[i] = common_lang_constants::UPPER_I;
                    }
                else if (text[i] == 0xD6)
                    {
                    text[i] = common_lang_constants::UPPER_O;
                    }
                else if (text[i] == 0xDC)
                    {
                    text[i] = common_lang_constants::UPPER_U;
                    }
                else if (text[i] == 0xE4)
                    {
                    text[i] = common_lang_constants::LOWER_A;
                    }
                else if (text[i] == 0xEB)
                    {
                    text[i] = common_lang_constants::LOWER_E;
                    }
                else if (text[i] == 0xEF)
                    {
                    text[i] = common_lang_constants::LOWER_I;
                    }
                else if (text[i] == 0xF6)
                    {
                    text[i] = common_lang_constants::LOWER_O;
                    }
                else if (text[i] == 0xFC)
                    {
                    text[i] = common_lang_constants::LOWER_U;
                    }
                }
            }

        /// @brief Encodes Dutch acutes in a string.
        /// @param text The string to update.
        void remove_dutch_acutes(string_typeT& text)
            {
            for (size_t i = 0; i < text.length(); ++i)
                {
                if (text[i] == 0xC1)
                    {
                    text[i] = common_lang_constants::UPPER_A;
                    }
                else if (text[i] == 0xC9)
                    {
                    text[i] = common_lang_constants::UPPER_E;
                    }
                else if (text[i] == 0xCD)
                    {
                    text[i] = common_lang_constants::UPPER_I;
                    }
                else if (text[i] == 0xD3)
                    {
                    text[i] = common_lang_constants::UPPER_O;
                    }
                else if (text[i] == 0xDA)
                    {
                    text[i] = common_lang_constants::UPPER_U;
                    }
                else if (text[i] == 0xE1)
                    {
                    text[i] = common_lang_constants::LOWER_A;
                    }
                else if (text[i] == 0xE9)
                    {
                    text[i] = common_lang_constants::LOWER_E;
                    }
                else if (text[i] == 0xED)
                    {
                    text[i] = common_lang_constants::LOWER_I;
                    }
                else if (text[i] == 0xF3)
                    {
                    text[i] = common_lang_constants::LOWER_O;
                    }
                else if (text[i] == 0xFA)
                    {
                    text[i] = common_lang_constants::LOWER_U;
                    }
                }
            }

        /// @brief Encodes Spanish acutes in a string.
        /// @param text The string to update.
        void remove_spanish_acutes(string_typeT& text)
            {
            for (size_t i = 0; i < text.length(); ++i)
                {
                if (text[i] == 0xC1)
                    {
                    text[i] = common_lang_constants::UPPER_A;
                    }
                else if (text[i] == 0xC9)
                    {
                    text[i] = common_lang_constants::UPPER_E;
                    }
                else if (text[i] == 0xCD)
                    {
                    text[i] = common_lang_constants::UPPER_I;
                    }
                else if (text[i] == 0xD3)
                    {
                    text[i] = common_lang_constants::UPPER_O;
                    }
                else if (text[i] == 0xDA)
                    {
                    text[i] = common_lang_constants::UPPER_U;
                    }
                else if (text[i] == 0xE1)
                    {
                    text[i] = common_lang_constants::LOWER_A;
                    }
                else if (text[i] == 0xE9)
                    {
                    text[i] = common_lang_constants::LOWER_E;
                    }
                else if (text[i] == 0xED)
                    {
                    text[i] = common_lang_constants::LOWER_I;
                    }
                else if (text[i] == 0xF3)
                    {
                    text[i] = common_lang_constants::LOWER_O;
                    }
                else if (text[i] == 0xFA)
                    {
                    text[i] = common_lang_constants::LOWER_U;
                    }
                }
            }

        /// @returns The position of R1.
        [[nodiscard]]
        inline size_t get_r1() const noexcept
            { return m_r1; }
        /// Sets the position of R1.
        /// @param pos The position.
        inline void set_r1(const size_t pos) noexcept
            { m_r1 = pos; }

        /// @returns The position of R2.
        [[nodiscard]]
        inline size_t get_r2() const noexcept
            { return m_r2; }
        /// @brief Sets the position of R2.
        /// @param pos The position.
        inline void set_r2(const size_t pos)
            { m_r2 = pos; }

        /// @returns The position of RV.
        [[nodiscard]]
        inline size_t get_rv() const noexcept
            { return m_rv; }
        /// @brief Sets the position of RV.
        /// @param pos The position.
        inline void set_rv(const size_t pos)
            { m_rv = pos; }

        /// @brief Resets the positions of R sections to 0.
        inline void reset_r_values() noexcept
            { m_r1 = m_r2 = m_rv = 0; }

        /// @brief lowercases any Western European alphabetic characters.
        /// @param c The character to lowercase.
        /// @returns The lowercased character.
        [[nodiscard]]
        inline static constexpr wchar_t tolower_western(const wchar_t c) noexcept
            {
            return ((c >= L'A') && (c <= L'Z')) ||
                ((c >= 0xC0) && (c <= 0xD6)) ||
                ((c >= 0xD8) && (c <= 0xDE))
                ? (c + 32) : c;
            }

        /// @brief Determines if a character is a Western European letter.
        /// @param ch The character to review.
        /// @returns @c true if character is a Western European letter.
        [[nodiscard]]
        inline static constexpr wchar_t is_western_letter(const wchar_t ch) noexcept
            {
            return (
                // A-Z
                (ch >= 0x41 && ch <= 0x5A) ||
                // uppercase extended ASCII set
                (ch >= 0xC0 && ch <= 0xD6) ||
                (ch >= 0xD8 && ch <= 0xDE) ||
                (ch == 0x0112) || // E with macron
                // Y with umlaut
                (ch == 0x0178) ||
                // a-z
                (ch >= 0x61 && ch <= 0x7A) ||
                // lowercase extended ASCII set
                (ch >= 0xE0 && ch <= 0xF6) ||
                (ch >= 0xF8 && ch <= 0xFF) ||
                (ch == 0x0113) || // e with macron
                // OE ligature
                (ch == 0x0153) ||
                // German eszett
                (ch == 0xDF));
            }

        /** @brief Determines if a character is one of a list of characters.
            @param character The character to review.
            @param char_string The list of characters to compare against.
            @returns @c true if the character of one of the list of characters.*/
        [[nodiscard]]
        inline static constexpr bool is_one_of(const wchar_t character,
                                               const wchar_t* char_string) noexcept
            {
            if (!char_string)
                { return false; }

            while (*char_string)
                {
                if (character == char_string[0])
                    { return true; }
                ++char_string;
                }
            return false;
            }

        /** @brief Replace all instances of a character in a string.
            @param text The text to replace items in.
            @param charToReplace The character to replace.
            @param replacementChar The character to replace @c charToReplace with.*/
        static void replace_all(string_typeT& text,
                     const typename string_typeT::value_type charToReplace,
                     const typename string_typeT::value_type replacementChar)
            {
            size_t start = 0;
            while (start != string_typeT::npos)
                {
                start = text.find(charToReplace, start);
                if (start == string_typeT::npos)
                    { return; }
                text[start++] = replacementChar;
                }
            }

        /** @brief Replace all instances of a substring in a string.
            @param text The text to replace items in.
            @param textToReplace The text to replace.
            @param replacementText The text to replace @c textToReplace with.*/
        static void replace_all(string_typeT& text, const string_typeT& textToReplace,
                                const string_typeT& replacementText)
            {
            size_t start = 0;
            while (start != string_typeT::npos)
                {
                start = text.find(textToReplace, start);
                if (start == string_typeT::npos)
                    { return; }
                text.replace(start, textToReplace.length(), replacementText);
                start += replacementText.length();
                }
            }

        /// @brief Determines if a given value is either of two other given values.
        /// @param value The value to compare with.
        /// @param first The first value to compare against.
        /// @param second The second value to compare against.
        /// @returns @c true if value is either of the other values.
        template<typename T>
        [[nodiscard]]
        static inline constexpr bool is_either(const T value, const T first, const T second) noexcept
            { return (value == first || value == second); }

        /// @brief Determines if a given value is neither of two other given values.
        /// @param value The value to compare with.
        /// @param first The first value to compare against.
        /// @param second The second value to compare against.
        /// @returns @c true if value is neither of the other values.
        template<typename T>
        [[nodiscard]]
        static inline constexpr bool is_neither(const T value, const T first, const T second) noexcept
            {
            assert(first != second);
            return (value != first && value != second);
            }
    private:
        size_t m_r1{ 0 };
        size_t m_r2{ 0 };
        // only used for Russian & romance languages
        size_t m_rv{ 0 };
        };

    //------------------------------------------------------
    /** A non-operational stemmer that is used in place of regular stemmers when
        you don't want the system to actually stem anything.*/
    template <typename string_typeT = std::wstring>
    class no_op_stem final : public stem<string_typeT>
        {
    public:
        /// @brief The string type that this class will accept.
        using string_type = string_typeT;
        /// @brief No-op stemming of declared string type.
        /// @param[in,out] text The text to stem.
        void operator()([[maybe_unused]] string_typeT&  text) final
            {}
        /// @returns The stemmer's language.
        [[nodiscard]]
        stemming_type get_language() const noexcept final
            { return stemming_type::no_stemming; }
        };
    }

/** @}*/

#endif // __STEM_H__
// clang-format on
