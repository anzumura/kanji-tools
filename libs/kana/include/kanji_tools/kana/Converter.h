#pragma once

#include <kanji_tools/kana/Kana.h>

#include <cassert>
#include <set>

namespace kanji_tools {

// 'Converter' supports converting between Rōmaji (ローマジ), Hiragana
// (平仮名) and Katakana (片仮名) - see comments at the bottom of this file more
// details
class Converter {
public:
  // 'Converter' constructor defaults the 'target' for conversion to Hiragana
  // and sets 'flags' to None (which means no special conversion flags). Calling
  // the below 'convert' functions can override these values.
  explicit Converter(CharType target = CharType::Hiragana,
      ConvertFlags flags = ConvertFlags::None);

  Converter(const Converter&) = delete;

  [[nodiscard]] CharType target() const { return _target; }
  void target(CharType target) { _target = target; }

  [[nodiscard]] auto flags() const { return _flags; }
  [[nodiscard]] String flagString() const; // return a | separated string
  void flags(ConvertFlags flags) { _flags = flags; }

  // 'convert' has 4 overloads. The first and second are 'const' and use current
  // '_target' and '_flags' values (the third and fourth overloads update them
  // before processing). If 'source' is specified then only input characters of
  // that type are converted. If 'source' == 'target' then the original string
  // is returned. See 'ConvertFlags' in Kana.h for more info about 'flags'.
  [[nodiscard]] String convert(const String& input) const;
  [[nodiscard]] String convert(CharType source, const String& input) const;
  [[nodiscard]] String convert(
      const String& input, CharType target, ConvertFlags = ConvertFlags::None);
  [[nodiscard]] String convert(CharType source, const String& input,
      CharType target, ConvertFlags = ConvertFlags::None);
private:
  using Set = std::set<String>;
  using NarrowDelims = std::map<char, String>;
  using WideDelims = std::map<String, char>;

  // For input, either 'Apostrophe' or 'Dash' can be used to separate 'n' in
  // the middle of Rōmaji words like gin'iro, kan'atsu, kan-i, etc.. For Rōmaji
  // output, 'Apostrophe' is used. Note, 'Dash' is used in 'Traditional Hepburn'
  // whereas apostrophe is used in 'Modern (revised) Hepburn'.
  static constexpr auto Apostrophe{'\''}, Dash{'-'};

  class Tokens {
  public:
    Tokens();

    [[nodiscard]] auto& repeatingConsonants() const {
      return _repeatingConsonants;
    }
    [[nodiscard]] auto& afterN(CharType t) const {
      return t == CharType::Hiragana ? _afterNHiragana : _afterNKatakana;
    }
    [[nodiscard]] auto& smallKana(CharType t) const {
      return t == CharType::Hiragana ? _smallHiragana : _smallKatakana;
    }
    [[nodiscard]] auto& narrowDelimList() const { return _narrowDelimList; }
    [[nodiscard]] auto& narrowDelims() const { return _narrowDelims; }
    [[nodiscard]] auto& wideDelims() const { return _wideDelims; }
  private:
    // 'insertUnique' performs an insert and ensures value was added by using
    // 'assert' (can't do on one line like 'assert(s.insert(x).second)' since
    // that would result in the code not getting executed when compiling with
    // asserts disabled, i.e., a 'Release' build.
    static void insertUnique(Set& s, const String& x) {
      [[maybe_unused]] const auto i{s.insert(x)};
      assert(i.second);
    }

    void populateDelimLists();

    // 'verifyData' is called by the constructor and performs various 'asserts'
    // on member data.
    void verifyData() const;

    // '_repeatingConsonants' is for processing small 'tsu' for sokuon output
    std::set<char> _repeatingConsonants;

    // '_afterN...' contain the 8 Kana (5 vowels and 3 y's) that should be
    // proceeded with 'Apostrophe' when producing Rōmaji if they follow 'n'.
    Set _afterNHiragana, _afterNKatakana;

    // '_small...' sets contain the 9 small Kana symbols (5 vowels, 3 y's, and
    // 'wa') that form the second parts of digraphs.
    Set _smallHiragana, _smallKatakana;

    // Support converting most non-alpha ascii from narrow to wide values. These
    // values are also used as delimiters when converting from Rōmaji to Kana.
    // Use a '*' for Katakana middle dot '・' to keep round-trip translations as
    // non-lossy as possible and '-' (dash) and apostrophe aren't included since
    // these could get mixed up with prolong mark 'ー' and handling after 'n' in
    // Rōmaji output. Backslash maps to ￥ as per usual keyboard input.
    String _narrowDelimList;
    NarrowDelims _narrowDelims;
    WideDelims _wideDelims;
  };

  // 'Tokens' constructor uses static maps from Kana class so wrap in a static
  // function to avoid order of static initialization problems
  static const Tokens& tokens();

  [[nodiscard]] static const Set& afterN(CharType);
  [[nodiscard]] static const Set& smallKana(CharType);
  [[nodiscard]] static const std::set<char>& repeatingConsonants();
  [[nodiscard]] static const NarrowDelims& narrowDelims();
  [[nodiscard]] static const WideDelims& wideDelims();

  [[nodiscard]] bool romajiTarget() const;
  [[nodiscard]] bool hiraganaTarget() const;
  [[nodiscard]] const String& get(const Kana&) const;
  [[nodiscard]] const String& getN() const;
  [[nodiscard]] const String& getSmallTsu() const;

  [[nodiscard]] static bool isN(const String&);

  // 'fromKana' takes a string of Kana (so 'source' is Hiragana or Katakana) and
  // retuns converted result based on '_target' and '_flags' (result can be
  // either Rōmaji or Kana, i.e., this function can convert Hiragana to Katakana
  // and vice versa).
  [[nodiscard]] String fromKana(const String&, CharType source) const;

  enum class State { New, SmallTsu, Done };
  enum class DoneType { NewGroup, NewEmptyGroup, Prolong };

  // helper functions used by 'fromKana'
  [[nodiscard]] String processKana(const String& kanaGroup, CharType source,
      const Kana*& prevKana, bool prolong = false) const;
  template<typename T>
  [[nodiscard]] bool processOneKana(const T&, CharType source,
      const String& kana, const String& kanaGroup, State&) const;
  [[nodiscard]] String processKanaMacron(bool prolong, const Kana*& prevKana,
      const Kana* kana, bool sokuon = false) const;

  // 'toKana' takes a string of Rōmaji and returns either Hiragana or Katakana
  // based on '_target' and '_flags'.
  [[nodiscard]] String toKana(const String&) const;

  // helper functions used by 'toKana'
  void processRomaji(String& romajiLetters, String& result) const;
  [[nodiscard]] bool processRomajiMacron(
      const String& letter, String& letters, String& result) const;

  // Members for the current conversion
  CharType _target;
  ConvertFlags _flags;
};

// More details on conversion: when Rōmaji is the output target, Revised Hepburn
// System (ヘボン式) is used, but for Rōmaji input many more letter combinations
// are supported such as:
// - Kunrei-shiki (訓令式) Rōmaji:
//   si -> し, sya -> しゃ, syu -> しゅ, syo -> しょ, ti -> ち, tu -> つ,
//   hu -> ふ, tya -> ちゃ, tyu -> ちゅ, tyo -> ちょ, ...
// - Nihon-shiki (日本式) Rōmaji: di -> ぢ,　du -> づ (plus Kunrei)
// - Wāpuro (ワープロ) Rōmaji combinations: ou -> おう, ...
// Letters with a macron (like ō, ā, ī) are supported for Rōmaji input, but when
// converting to Hiragana they are ambiguous, i.e., ō maps to either おお or
// おう so for simplicity the prolong mark (ー) is used (this can be overridden
// by a flag to produce the double vowel like おお). Note, when typing Kana
// 'macchi' and 'kocchi' produce "マッチ" and "こっち" respectively, but this is
// not standard Hepburn. Instead the standard is 'matchi' and 'kotchi', but
// either way is accepted as input to the 'convert' function (when converting
// from Kana to Rōmaji the standard form is used as output).
//
// Note: a number of delimiters are also supported and get converted from narrow
// to wide and vice versa (see Converter.cpp 'Delimiters'). When converting
// from Rōmaji, case is ignored so both 'Dare' and 'dARe' convert to 'だれ'.

} // namespace kanji_tools
