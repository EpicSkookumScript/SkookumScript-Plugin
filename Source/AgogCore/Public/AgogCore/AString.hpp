// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// Dynamic AString class declaration header
// Notes:          The AString class should be used in the place of standard C-String
//              character array pointers.
//
// ##### Function descriptions located at implementations rather than declarations. #####
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AConstructDestruct.hpp>
#include <AgogCore/ACompareBase.hpp>
#include <AgogCore/AMemory.hpp>
#include <AgogCore/APArray.hpp>

//=======================================================================================
// Global Defines / Macros
//=======================================================================================

//#define ASTR_ENABLE_WIDE_CHAR


//=======================================================================================
// Global Structures
//=======================================================================================

// Pre-declarations
struct AStringRef;       
class  AStringBM;
class  ASymbol;

// AString enumerated constants
enum
  {
  AString_indent_spaces_def     = 2,        // Default indentation amount in spaces
  AString_tab_stop_def          = 4,        // Default tab stop amount in spaces
  AString_def_alloc             = 8,        // Default C-String buffer allocation amount
  AString_real_extra_chars      = 10,       // Extra space above and beyond significant digits for sign, exponent, etc.
  AString_float_sig_digits_def  = FLT_DIG,  // Default significant digits for f32 (6)
  AString_double_sig_digits_def = DBL_DIG,  // Default significant digits for f64 (15)
  AString_int32_max_chars       = 40,
  AString_ansi_charset_length   = 256,
  AString_input_stream_max      = 256
  };


// Unsigned constants

const uint32_t AString_determine_base = 0u;  // Specifies that the base / radix expect for parsing should be determine via the first characters encountered
const uint32_t AString_def_base       = 10;  // Default numerical conversion base
const uint32_t AString_max_base       = 36u; // Maximum allowable base / radix for parsing /generating integral values


// Case sensitivity state
enum eAStrCase
  {
  AStrCase_sensitive,
  AStrCase_ignore
  };


// Match type - used by AString::match()
enum eAStrMatch
  {
  AStrMatch_subpart,
  AStrMatch_prefix,
  AStrMatch_suffix,
  AStrMatch_exact
  };


enum eALineBreak
  {
  ALineBreak_unix,  // \n    [Standard] C++, Unix, Mac-OSX
  ALineBreak_dos,   // \r\n  PC files, edit boxes
  ALineBreak_rich,  // \r    RichEdit boxes

  ALineBreak__default
  };


// Type of character match to use
// See Also: not_char_type(), init_match_table()
enum eACharMatch
  {
  ACharMatch_alphabetic,          // A-Z, a-z
  ACharMatch_alphanumeric,        // A-Z, a-z, 0-9
  ACharMatch_alphascore,          // A-Z, a-z, _
  ACharMatch_digit,               // 0-9
  ACharMatch_identifier,          // A-Z, a-z, _, 0-9
  ACharMatch_lowercase,           // a-z
  ACharMatch_punctuation,         // Punctuation characters
  ACharMatch_symbol,              // #$%^&* etc.
  ACharMatch_token,               // Punctuation or symbol character
  ACharMatch_uppercase,           // A-Z
  ACharMatch_white_space,         // Horizontal Tab, Line Feed, Vertical Tab, Form Feed, Carriage Return, Space
  ACharMatch__not_start,
  ACharMatch_not_alphabetic = ACharMatch__not_start, // not A-Z, a-z
  ACharMatch_not_alphanumeric,    // not A-Z, a-z, 0-9
  ACharMatch_not_alphascore,      // not A-Z, a-z, _
  ACharMatch_not_digit,           // not 0-9
  ACharMatch_not_identifier,      // not A-Z, a-z, _, 0-9
  ACharMatch_not_lowercase,       // not a-z
  ACharMatch_not_punctuation,     // not Punctuation characters including arithmetic
  ACharMatch_not_symbol,          // not #$%^&* etc.
  ACharMatch_not_token,           // not Punctuation or symbol character
  ACharMatch_not_uppercase,       // not A-Z
  ACharMatch_not_white_space,     // not Horizontal Tab, Line Feed, Vertical Tab, Form Feed, Carriage Return, Space
  ACharMatch_not_white_space_except_lf, // same as above except Line Feed is found
  ACharMatch__length
  };


//---------------------------------------------------------------------------------------
// Common argument data structure used in most string methods.  It specifies the range
// of the string to operate on and whether or not any character comparisons should be
// case sensitive or not.  
struct A_API AStrArgs
  {
  // Public Data Members

    // [In] One of: AStrCase_sensitive or AStrCase_ignore.  [Some methods do not use the case info.]
    eAStrCase m_case_check;

    // [In] Index position to start operation/search in string
    uint32_t m_start_pos;

    // [In] Index position to end operation/search in string.  If it is ALength_remainder,
    // the last index position of the string is used (length - 1).
    // -Or-
    // [Out] Index position that a parse stopped - either the first character that was
    // unexpected, or the first character that follows a successful parse.
    uint32_t m_end_pos;


  // Public Class Data Members

    // Default case sensitive
    static AStrArgs ms_sensitive;

    // Default case insensitive
    static AStrArgs ms_ignore;


  // Methods

    AStrArgs(eAStrCase case_check = AStrCase_sensitive);
    AStrArgs(uint32_t start_pos, uint32_t end_pos = ALength_remainder, eAStrCase case_check = AStrCase_sensitive);
    AStrArgs(const AStrArgs & args);

    static AStrArgs span(uint32_t pos, uint32_t char_count = ALength_remainder);

  };  // AStrArgs


//---------------------------------------------------------------------------------------
struct A_API AFindStrArgs : public AStrArgs
  {
  // Public Data Members

    // [In] Occurrence of match to find.  It may not be less than 1
    uint32_t m_instance;

    // [Out] Index position where n-instanced match was found.  If no match was found,
    // then the value is undefined.
    uint32_t m_find_start;

    // [Out] Index position at the end of the n-instanced found match.  For example, if
    // match was found at index position 3 and the match was 4 characters long then
    // m_find_end would be 7.  If no match was found, then the value is undefined.
    uint32_t m_find_end;

    // Inherited from AStrArgs:
      //eAStrCase m_case_check;
      //uint32_t  m_start_pos;
      //uint32_t  m_end_pos;


  // Public Class Data Members

    // Default case sensitive
    static AFindStrArgs ms_sensitive;

    // Default case insensitive
    static AFindStrArgs ms_ignore;


  // Methods

    AFindStrArgs(uint32_t instance = 1u, eAStrCase case_check = AStrCase_sensitive);
    AFindStrArgs(eAStrCase case_check, uint32_t instance = 1u);
    AFindStrArgs(const AStrArgs & args, uint32_t instance = 1u);
    AFindStrArgs(const AFindStrArgs & args);

    operator AFindStrArgs * () { return this; }

}; // AFindStrArgs


//---------------------------------------------------------------------------------------
// Notes      Dynamic AString class which should be used in the place of standard C-String
//            character array pointers.
//
//            This class also has many character classification methods.
//
//            Many AString methods take arguments with constant values that have a special
//            meaning.  Their meanings are described in AgogCore/AgogCore.hpp where they are defined.
//
//            Some special constant values for length arguments are:
//              ALength_calculate
//              ALength_remainder
//
//            Some special constant values for instance arguments are:
//              AMatch_last
//              AMatch_first_found
// Subclasses AStringBM
// See Also   ASymbol, AStringBM
// UsesLibs   
// InLibs     AgogCore/AgogCore.lib
// Examples:    
// Author(s)  Conan Reis
class A_API AString
  {
  friend class AStringBM;
  friend class ASymbol;
  friend class ASymbolTable;

  public:

  // Nested Structures

    // AEx<AString> exception id values
    enum
      {
      ErrId_null_cstr    = AErrId_last,  // Given nullptr instead of valid C-String
      ErrId_invalid_base               // Given invalid base / radix value for numerical conversion
      };

  // Public Class Data Members

    // For empty AString defaults (i.e. AString()) - use to cut down on unnecessary
    // internal C-String allocations and deallocations
    static const AString ms_empty;
    
    // For default tokenization
    static const AString ms_comma;
    
    // For line breaks to dos file format
    static const AString ms_dos_break;

    // Use these tables instead of the standard character classification functions like
    // isalpha(), isspace(), etc.
    // For example: ms_char_match_table[ACharMatch_white_space][uint8_t(ch)] == isspace(ch)
    // See method init_match_table() for more information.
    // $Revisit - Note that these are ASCII only! They will need to be revised if library converted to Unicode with UTF-8.
    static bool   ms_char_match_table[ACharMatch__length][AString_ansi_charset_length];
    static bool * ms_is_lowercase;  // = ms_char_match_table[ACharMatch_lowercase]
    static bool * ms_is_space;      // = ms_char_match_table[ACharMatch_white_space]
    static bool * ms_is_uppercase;  // = ms_char_match_table[ACharMatch_uppercase]
    static bool * ms_is_digit;      // = ms_char_match_table[ACharMatch_digit]

    // Converts to ASCII uppercase
    static const char ms_char2uppper[AString_ansi_charset_length];
    
    // Converts to ASCII lowercase
    static const char ms_char2lower[AString_ansi_charset_length];

  // Common Methods

    static void  initialize();
    static void  deinitialize();
    static bool  is_initialized();

    A_NEW_OPERATORS(AString);

    AString();
    AString(const AString & str);
    AString(const AString & str, eATerm term);
    AString(const AString & str, uint32_t extra_space);
    ~AString();
    AString & operator=(const AString & str);

  // Converter Methods

    AString(const char * cstr_p);
    AString(const char * cstr_p, bool persistent);
    AString(const char * cstr_p, uint32_t length, bool persistent = true);
    AString(const char * buffer_p, uint32_t size, uint32_t length, bool deallocate = false);
    AString(const wchar_t * wcstr_p);
    AString(const wchar_t * wcstr_p, uint32_t length);
    AString(AStringRef * str_ref_p);
    AString(char ch);
    AString(char ch, uint32_t char_count);
    AString(const APArrayLogical<AString> & strings, const AString & separator = ms_empty);
    AString(uint32_t max_size, const char * format_str_p, ...);

    static AString ctor_int(int integer, uint base = AString_def_base);
    static AString ctor_uint(uint natural, uint base = AString_def_base);
    static AString ctor_float(f32 real, uint significant = AString_float_sig_digits_def);
    static AString ctor_float64(f64 real, uint significant = AString_double_sig_digits_def);

    operator const AStringRef & () const { return *m_str_ref_p; }
    operator const char * () const;
    const char * as_cstr() const;
    char *       as_cstr_writable();
    f64          as_float64(uint32_t start_pos = 0u, uint32_t * stop_pos_p = nullptr) const;
    f32          as_float32(uint32_t start_pos = 0u, uint32_t * stop_pos_p = nullptr) const;
    int          as_int(uint32_t start_pos = 0u, uint32_t * stop_pos_p = nullptr, uint32_t base = AString_determine_base) const;
    uint         as_uint(uint32_t start_pos = 0u, uint32_t * stop_pos_p = nullptr, uint32_t base = AString_determine_base) const;
    uint32_t     as_crc32(uint32_t prev_crc = 0u) const;
    uint32_t     as_crc32_upper(uint32_t prev_crc = 0u) const;
     
    AString &    operator=(const char * cstr_p);

    // Binary Serialization Methods

      AString(const void ** source_stream_pp);
      uint32_t     as_binary_length() const;
      void         as_binary(void ** dest_stream_pp) const;
      AString &    assign_binary(const void ** source_stream_pp);


    // Future methods
      //explicit AString(strstream & strm);
      //AString & operator=(strstream & strm);
      //AString(uint32_t max_size, const char * format_str_p, va_list arg_list);

    //#ifdef ASTR_ENABLE_WIDE_CHAR
    //  const wchar_t * as_wchar_t() const;
    //#endif

  // Comparison Methods

    eAEquate compare(const AString & str) const;
    eAEquate compare(const AString & str, eAStrCase case_check) const;
    eAEquate compare(const char * cstr_p, eAStrCase case_check = AStrCase_sensitive) const;
    eAEquate compare_sub(const AString & substr, uint32_t index = 0u) const;
    eAEquate icompare_sub(const AString & substr, uint32_t index = 0u) const;
    bool     is_equal(const AString & str) const;
    bool     is_iequal(const AString & str) const;
    bool     is_match(const AString & str, eAStrMatch match_type) const;
    bool     is_match(const AStringBM & str, eAStrMatch match_type) const;
    bool     is_imatch(const AString & str, eAStrMatch match_type) const;
    bool     is_match_end(const AString & str) const;
    bool     is_imatch_end(const AString & str) const;
    bool     operator<(const AString & str) const;
    bool     operator>(const AString & str) const;
    bool     operator<=(const AString & str) const;
    bool     operator>=(const AString & str) const;
    bool     operator==(const AString & str) const;
    bool     operator!=(const AString & str) const;
    bool     operator==(const char * cstr_p) const;
    bool     operator!=(const char * cstr_p) const;

    // Future methods
      // bool operator<(const char * cstr_p) const;
      // bool operator>(const char * cstr_p) const;
      // bool operator<=(const char * cstr_p) const;
      // bool operator>=(const char * cstr_p) const;

      // bool is_equal(const AString & str, eAStrCase case_check = AStrCase_sensitive) const;
      // bool is_equal_wild(const AString & wild, eAStrCase case_check = AStrCase_sensitive) const;
      // bool is_equal_pattern(const AString & pattern, eAStrCase case_check = AStrCase_sensitive) const;
      // bool match_wild(const AString & wild, uint32_t start_pos = 0u, uint32_t * stop_pos_p = nullptr, eAStrCase case_check = AStrCase_sensitive) const;
      // bool match_pattern(const AString & pattern, uint32_t start_pos = 0u, uint32_t * stop_pos_p = nullptr, eAStrCase case_check = AStrCase_sensitive) const;

  // Accessor Methods

    char         get_at(uint32_t pos) const;
    char         get_first() const;
    char         get_last() const;
    uint32_t     get_length() const;
    uint32_t     get_size() const;
    AStringRef * get_str_ref() const { return m_str_ref_p; }
    bool         is_empty() const;
    bool         is_filled() const;

    char &  operator[](int pos);
    char &  operator[](uint32_t pos);
    char    operator()(int pos) const;
    char    operator()(uint32_t pos) const;

    // Future methods
      // void set_at(uint32_t pos);

  // Modifying Methods

    void      append(const AString & str);
    void      append(const char * cstr_p, uint32_t length = ALength_calculate);
    void      append(char ch);
    void      append(char ch, uint32_t char_count);
    void      append_format(const char * format_str_p, ...);
    void      compact();
    void      crop(uint32_t pos, uint32_t char_count = ALength_remainder);
    AString & crop(eACharMatch match_type = ACharMatch_white_space);
    void      crop_quick(uint32_t pos, uint32_t char_count);
    void      empty();
    void      ensure_size(uint32_t needed_chars);
    void      ensure_size_buffer(uint32_t needed_chars);
    bool      ensure_size_empty(uint32_t needed_chars);
    void      ensure_size_extra(uint32_t extra_chars);
    void      ensure_writable();
    void      format(const char * format_str_p, ...);
    uint32_t  increment(uint32_t increment_by = 1u, uint32_t min_digits = 4u);
    void      insert(char ch, uint32_t pos = 0u);
    void      insert(const AString & str, uint32_t pos = 0u);
    void      lowercase();
    void      rebuffer();
    bool      remove(const AString & str, uint32_t instance = 1u, uint32_t * find_pos_p = nullptr, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder, eAStrCase case_check = AStrCase_sensitive);
    bool      remove(const AStringBM & bm, uint32_t instance = 1u, uint32_t * find_pos_p = nullptr, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
    void      remove_all(uint32_t pos, uint32_t char_count = ALength_remainder);
    uint32_t  remove_all(char ch, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
    uint32_t  remove_all(eACharMatch match_type, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
    uint32_t  remove_all(const AString & str, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder, eAStrCase case_check = AStrCase_sensitive);
    uint32_t  remove_all(const AStringBM & bm, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
    void      remove_end(uint32_t char_count);
    void      replace(const AString & new_str, uint32_t pos = 0u, uint32_t char_count = ALength_remainder, uint32_t new_pos = 0u, uint32_t new_char_count = ALength_remainder);
    bool      replace(const AString & old_str, const AString & new_str, uint32_t instance = 1u, uint32_t * find_pos_p = nullptr, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder, eAStrCase case_check = AStrCase_sensitive);
    bool      replace(const AStringBM & bm, const AString & new_str, uint32_t instance = 1u, uint32_t * find_pos_p = nullptr, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
    uint32_t  replace_all(char old_ch, char new_ch, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
    uint32_t  replace_all(char old_ch, const AString & new_str, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
    uint32_t  replace_all(const AString & old_str, const AString & new_str, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder, eAStrCase case_check = AStrCase_sensitive);
    uint32_t  replace_all(const AStringBM & bm, const AString & new_str, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
    void      reverse(uint32_t pos = 0u, uint32_t char_count = ALength_remainder);
    void      set_buffer(const char * buffer_p, uint32_t size, uint32_t length = ALength_calculate, bool deallocate = false);
    void      set_cstr(const char * cstr_p, uint32_t length = ALength_calculate, bool persistent = true);
    void      set_length(uint32_t length = ALength_calculate);
    void      trim(eACharMatch match_type = ACharMatch_white_space);
    void      truncate(eACharMatch match_type = ACharMatch_white_space);
    void      uppercase();
    AString & operator+=(const AString & str);
    AString & operator+=(const char * cstr_p);
    AString & operator+=(char ch);

    // Row & Column Modifying Methods

      uint32_t line_break_dos2rich(uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
      uint32_t line_break_dos2unix(uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
      uint32_t line_break_rich2dos(uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
      uint32_t line_break_rich2unix(uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
      uint32_t line_break_unix2dos(uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
      uint32_t line_break_unix2rich(uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
      uint32_t line_indent(uint32_t space_count = AString_indent_spaces_def, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
      uint32_t line_indent_next(uint32_t space_count = AString_indent_spaces_def, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
      uint32_t line_unindent(uint32_t space_count = AString_indent_spaces_def, uint32_t tab_stops = AString_tab_stop_def, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);

    // Future methods

      // void format_vargs(const char * format_str_p, va_list arg_list);
      // void remove_reverse(-);
      // void replace_end(const AString & str, uint32_t char_count = ALength_remainder);
      // void replace_reverse(-);

      // bool pop_wild(const AString & wild, AFindStrArgs * args_p = &AFindStrArgs::ms_sensitive);
      // bool remove_wild(const AString & wild, AFindStrArgs * args_p = &AFindStrArgs::ms_sensitive);
      // bool remove_wild_all(const AString & wild, const AStrArgs & args = AStrArgs::ms_sensitive);
      // bool replace_wild(const AString & wild, const AString & new_str, AFindStrArgs * args_p = &AFindStrArgs::ms_sensitive);
      // bool replace_wild_all(const AString & wild, const AString & new_str, const AStrArgs & args = AStrArgs::ms_sensitive);

      // bool pop_pattern(const AString & pattern, AFindStrArgs * args_p = &AFindStrArgs::ms_sensitive);
      // bool remove_pattern(const AString & pattern, AFindStrArgs * args_p = &AFindStrArgs::ms_sensitive);
      // bool remove_pattern_all(const AString & pattern, const AStrArgs & args = AStrArgs::ms_sensitive);
      // bool replace_pattern(const AString & pattern, const AString & new_str, bool new_pattern = false, AFindStrArgs * args_p = &AFindStrArgs::ms_sensitive);
      // bool replace_pattern_all(const AString & pattern, const AString & new_str, bool new_pattern = false, const AStrArgs & args = AStrArgs::ms_sensitive);

      // void crop(const char * char_set_p, bool in_set = true, eAStrCase case_check = AStrCase_sensitive);
      // void remove_all(const char * char_set_p, bool in_set = true, const AStrArgs & args = AStrArgs::ms_sensitive);
      // void trim(const char * char_set_p, bool in_set = true, eAStrCase case_check = AStrCase_sensitive););
      // void truncate(const char * char_set_p, bool in_set = true, eAStrCase case_check = AStrCase_sensitive););


  // Non-Modifying Methods

    AString   add(const AString & str) const;
    AString   add(const char * cstr_p, uint32_t length = ALength_calculate) const;
    AString   add(char ch) const;
    AString   as_lowercase() const;
    void      as_reverse(AString * reversed_p, uint32_t pos = 0u, uint32_t char_count = ALength_remainder) const;
    AString   as_reverse(uint32_t pos = 0u, uint32_t char_count = ALength_remainder) const;
    AString   as_uppercase() const;
    uint32_t  count(char ch, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder, uint32_t * last_counted_p = nullptr) const;
    uint32_t  count(eACharMatch match_type, uint32_t  start_pos = 0u, uint32_t end_pos = ALength_remainder) const;
    uint32_t  count(const AString & str, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder, eAStrCase case_check = AStrCase_sensitive) const;
    uint32_t  count(const AStringBM & bm, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder) const;
    bool      find(char ch, uint32_t instance = 1u, uint32_t * find_pos_p = nullptr, uint32_t  start_pos = 0u, uint32_t end_pos = ALength_remainder) const;
    bool      find(eACharMatch match_type, uint32_t instance = 1u, uint32_t * find_pos_p = nullptr, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder) const;
    bool      find(const AString & str, uint32_t instance = 1u, uint32_t * find_pos_p = nullptr, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder, eAStrCase case_check = AStrCase_sensitive) const;
    bool      find(const AStringBM & bm, uint32_t instance = 1u, uint32_t * find_pos_p = nullptr, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder) const;
    bool      find_fuzzy(const AString & str, uint32_t instance = 1u, uint32_t * find_start_p = nullptr, uint32_t * find_end_p = nullptr, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder, eAStrCase case_check = AStrCase_sensitive) const;
    bool      find_fuzzy_reverse(const AString & str, uint32_t instance = 1u, uint32_t * find_start_p = nullptr, uint32_t * find_end_p = nullptr, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder, eAStrCase case_check = AStrCase_sensitive) const;
    bool      find_reverse(char ch, uint32_t instance = 1u, uint32_t * find_pos_p = nullptr, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder) const;
    bool      find_reverse(const AString & str, uint32_t instance = 1u, uint32_t * find_pos_p = nullptr, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder, eAStrCase case_check = AStrCase_sensitive) const;
    bool      find_reverse(eACharMatch match_type, uint32_t instance = 1u, uint32_t * find_pos_p = nullptr, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder) const;
    AString   get(uint32_t pos = 0u, uint32_t char_count = ALength_remainder) const;
    void      get(AString * str_p, uint32_t pos = 0u, uint32_t char_count = ALength_remainder) const;
    AString   get_end(uint32_t char_count) const;
    AString * get_new(uint32_t pos = 0u, uint32_t char_count = ALength_remainder) const;
    AString   get_token(uint32_t index = 0u, const AString & separator = ms_comma, uint32_t * find_pos_p = nullptr, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder, eAStrCase case_check = AStrCase_sensitive) const;
    AString   get_token(uint32_t index, const AStringBM & separator, uint32_t * find_pos_p = nullptr, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder) const;
    void      tokenize(APArrayLogical<AString> * collect_p, const AString & separator = ms_comma, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder, eAStrCase case_check = AStrCase_sensitive) const;
    void      tokenize(APArrayLogical<AString> * collect_p, const AStringBM & separator, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder) const;

    // Row & Column Methods

      uint32_t find_indent_column(uint32_t tab_stops = AString_tab_stop_def, uint32_t * indent_idx_p = nullptr, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder) const;
      uint32_t index_to_column(uint32_t idx, uint32_t tab_stops = AString_tab_stop_def) const;
      uint32_t index_to_row(uint32_t idx, uint32_t * row_idx_p = nullptr, uint32_t * column_p = nullptr, uint32_t tab_stops = AString_tab_stop_def) const;
      uint32_t row_to_index(uint32_t row, uint32_t column = 0u, uint32_t tab_stops = AString_tab_stop_def) const;

    // Future methods

      // void      check_integrity() const;
      // bool      find_reverse(const AStringBM & bm, uint32_t instance = 1u, uint32_t * find_pos_p = nullptr, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder) const;

      // uint32_t  count2(const AString & str, const AStrArgs & args = AStrArgs::ms_sensitive) const;
      // uint32_t  count_wild(const AString & wild, const AStrArgs & args = AStrArgs::ms_sensitive) const;
      // bool      find2(const AString & str, AFindStrArgs * args_p = &AFindStrArgs::ms_sensitive) const;
      // bool      find_wild(const AString & wild, AFindStrArgs * args_p = &AFindStrArgs::ms_sensitive) const;
      // bool      find_wild_reverse(const AString & wild, AFindStrArgs * args_p = &AFindStrArgs::ms_sensitive) const;
      // AString   get_wild(const AString & wild, AFindStrArgs * args_p = &AFindStrArgs::ms_sensitive) const;
      // AString   get_token_wild(uint32_t index = 0u, const AString & separator_wild, AFindStrArgs * args_p = &AFindStrArgs::ms_sensitive) const;
      // void      tokenize_wild(APArrayLogical<AString> * collect_p, const AString & separator_wild, const AStrArgs & args = AStrArgs::ms_sensitive) const;

      // uint32_t  count_pattern(const AString & pattern, const AStrArgs & args = AStrArgs::ms_sensitive) const;
      // bool      find_pattern(const AString & pattern, AFindStrArgs * args_p = &AFindStrArgs::ms_sensitive) const;
      // bool      find_pattern_reverse(const AString & pattern, AFindStrArgs * args_p = &AFindStrArgs::ms_sensitive) const;
      // AString   get_pattern(const AString & pattern, AFindStrArgs * args_p = &AFindStrArgs::ms_sensitive) const;
      // AString   get_pattern2(const AString & pattern, const AString & return_pattern, AFindStrArgs * args_p = &AFindStrArgs::ms_sensitive) const;
      // AString   get_token_pattern(uint32_t index = 0u, const AString & separator_pattern, AFindStrArgs * args_p = &AFindStrArgs::ms_sensitive) const;
      // void      tokenize_pattern(APArrayLogical<AString> * collect_p, const AString & separator_pattern, const AStrArgs & args = AStrArgs::ms_sensitive) const;

      // uint32_t  count(const char * char_set_p, bool in_set = true, uint32_t  start_pos = 0u, uint32_t end_pos = ALength_remainder) const;
      // bool      find(const char * char_set_p, bool in_set = true, uint32_t instance = 1u, uint32_t * find_pos_p = nullptr, uint32_t * find_end_p = nullptr, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder) const;
      // bool      find_reverse(const char * char_set_p, bool in_set = true, uint32_t instance = 1u, uint32_t * find_pos_p = nullptr, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder) const;

  // Class Methods

    // C-String Column Methods

    static const char * advance_to_indent(const char * cstr_p, const char * cstr_end_p, uint32_t tab_stops, uint32_t * column_p);
    static const char * advance_to_column(uint32_t column, const char * row_cstr_p, uint32_t tab_stops);
    static uint32_t     count_columns(const char * row_cstr_p, const char * column_cstr_p, uint32_t tab_stops);

    // Character functions

      static int         compare_insensitive(char lhs, char rhs);
      static eACharMatch not_char_type(eACharMatch match_type);
      static char        to_lowercase(char ch);
  
  // Friend Functions

    friend A_API AString   operator+(const AString & str1, const AString & str2);
    friend A_API AString   operator+(const AString & str, const char * cstr_p);
    friend A_API AString   operator+(const AString & str, char ch);
    friend A_API AString   operator+(const char * cstr_p, const AString & str);
    friend A_API AString   operator+(char ch, const AString & str);

    // Future methods
      //friend istream & operator>>(istream & strm, AString & str);
      //friend ostream & operator<<(ostream & ostrm, const AString & str);

  protected:
  // Internal Methods

    void make_writable();
    void set_size(uint32_t needed_chars);

    #ifdef A_BOUNDS_CHECK
      void bounds_check(uint32_t pos, const char * func_name_p) const;
      void bounds_check(uint32_t start_pos, uint32_t end_pos, const char * func_name_p) const;
      void bounds_check(uint32_t start_pos, uint32_t end_pos, uint32_t instance, const char * func_name_p) const;
      void span_check(uint32_t pos, uint32_t char_count, const char * func_name_p) const;
    #endif
    
  // Internal Class Methods

    static void init_match_table();

  protected:
  // Data Members

    AStringRef * m_str_ref_p;  // Pointer to AString reference
  
  // Class Data Members

    // Calls init_match_table() at global initialization
    static AConstructDestruct ms_construct_destruct;

  };

//---------------------------------------------------------------------------------------
// This is a specialization of the ACompareLogical template class found in AgogCore/ACompareBase.hpp
// and is passed as a second argument to various templates classes such as APArray and
// APSorted to provide a mechanism for logical case sensitive sorting of AString elements.
// For example: APSortedLogical<AString> == APSorted<AString, AString, ACompareLogical<AString> >
template<>
class ACompareLogical<AString>
  {
  public:
  // Class Methods

    // Returns true if elements are equal
    static bool equals(const AString & lhs, const AString & rhs) {return lhs == rhs;}

    // Returns 0 if true, < 0 if lhs is less than rhs, and > 0 if lhs is greater than rhs
    static ptrdiff_t comparison(const AString & lhs, const AString & rhs) {return lhs.compare(rhs);}
  };


//---------------------------------------------------------------------------------------
// This is passed as a second argument to various templates classes such as APArray and
// APSorted to provide a mechanism for logical case insensitive sorting of AString elements.
// For example: APSorted<AString, AString, ACompareStrInsensitive>
class ACompareStrInsensitive
  {
  public:
  // Class Methods

    // Returns true if elements are equal
    static bool equals(const AString & lhs, const AString & rhs) {return (lhs.compare(rhs, AStrCase_ignore) == AEquate_equal);}

    // Returns 0 if true, < 0 if lhs is less than rhs, and > 0 if lhs is greater than rhs
    static ptrdiff_t comparison(const AString & lhs, const AString & rhs) {return lhs.compare(rhs, AStrCase_ignore);}
  };


//---------------------------------------------------------------------------------------
// Class used for the speedy substring search based on the Boyer Moore search
// algorithm.
class A_API AStringBM : public AString
  {
  public:
    friend class AString;
  // Common Methods

    AStringBM(const AString & str = AString::ms_empty, eAStrCase case_check = AStrCase_sensitive);
    AStringBM(const AStringBM & bm);
    const AStringBM & operator=(const AStringBM & bm);

  // Converter Methods

    void convert(const AString & str, eAStrCase case_check = AStrCase_sensitive);

  // Accessor Methods

    uint32_t get_delta(char ch) const;
    char     operator[](int index) const;

  protected:
  // Data Members

    eAStrCase m_case;
    uint8_t   m_delta_p[AString_ansi_charset_length];  // delta table
  };


//=======================================================================================
// Inline Functions
//=======================================================================================

#ifndef A_INL_IN_CPP
  #include <AgogCore/AString.inl>
#endif
