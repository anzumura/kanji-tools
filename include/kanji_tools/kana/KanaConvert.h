#pragma once

#include <kanji_tools/kana/Kana.h>

#include <set>

namespace kanji_tools {

// 'KanaConvert' supports converting between Rōmaji (ローマジ), Hiragana
// (平仮名) and Katakana (片仮名). When Rōmaji is the output target, Revised
// Hepburn System (ヘボン式) is used, but for Rōmaji input many more letter
// combinations are supported such as:
// - Kunrei-shiki (訓令式) Rōmaji:
//   si -> し, sya -> しゃ, syu -> しゅ, syo -> しょ, ti -> ち, tu -> つ,
//   hu -> ふ, tya -> ちゃ, tyu -> ちゅ, tyo -> ちょ, ...
// - Nihon-shiki (日本式) Rōmaji: di -> ぢ,　du -> づ (plus Kunrei)
// - Wāpuro (ワープロ) Rōmaji combinations: ou -> おう, ...
// Letters with a macron (like ō, ā, ī) are supported for Rōmaji input, but when
// converting to Hiragana they are ambiguous, i.e., ō maps to either おお or
// おう so for simplicity the prolong mark (ー) is used (this can be overridded
// by a flag to produce the double vowel like おお). Note, when typing Kana
// 'macchi' and 'kocchi' produce "マッチ" and "こっち" respectively, but this is
// not standard Hepburn. Instead the standard is 'matchi' and 'kotchi', but
// either way is accepted as input to the 'convert' function (when converting
// from Kana to Rōmaji the standard form is used as output).
class KanaConvert {
public:
  // 'KanaConvert' constructor defaults the 'target' for conversion to Hiragana
  // and sets 'flags' to None (which means no special conversion flags). Calling
  // the below 'convert' functions can override these values.
  KanaConvert(CharType target = CharType::Hiragana,
              ConvertFlags flags = ConvertFlags::None);

  KanaConvert(const KanaConvert&) = delete;
  // operator= is not generated since there are const members

  [[nodiscard]] CharType target() const { return _target; }
  void target(CharType target) { _target = target; }

  [[nodiscard]] auto flags() const { return _flags; }
  [[nodiscard]] std::string flagString() const; // return a | separated string
  void flags(ConvertFlags flags) { _flags = flags; }

  // Support converting most non-alphanumeric ascii from narrow to wide values.
  // These values are also used as delimiters for splitting up input strings
  // when converting from Rōmaji to Kana. Use a '*' for Katakana middle dot '・'
  // to keep round-trip translations as non-lossy as possible. For now, don't
  // include '-' (minus) or apostrophe since these could get mixed up with
  // prolong mark 'ー' and special separation handling after 'n' in Rōmaji
  // output. Backslash maps to ￥ as per usual keyboard input.
  using NarrowDelims = std::map<char, std::string>;
  using WideDelims = std::map<std::string, char>;

  [[nodiscard]] auto& narrowDelims() const { return _narrowDelims; }
  [[nodiscard]] auto& wideDelims() const { return _wideDelims; }

  // 'convert' has 4 overloads. The first and second use the current values of
  // '_target' and '_flags'. The first and third convert characters of any
  // source type whereas the second and fourth restrict the source type to be
  // converted. If 'source' == 'target' then the original string is returned.
  //
  // Note: a number of delimiters are also supported and get converted from
  // narrow to wide and vice versa (see KanaConvert.cpp 'Delimiters'). Also,
  // when converting from Rōmaji, case is ignored so both 'Dare' and 'dARe'
  // would convert to 'だれ'. See 'ConvertFlags' in Kana.h for an explanation of
  // available flags that can be used. The third and fourth overloads update
  // '_target' and '_flags'.
  [[nodiscard]] std::string convert(const std::string& input) const;
  [[nodiscard]] std::string convert(CharType source,
                                    const std::string& input) const;
  [[nodiscard]] std::string convert(const std::string& input, CharType target,
                                    ConvertFlags = ConvertFlags::None);
  [[nodiscard]] std::string convert(CharType source, const std::string& input,
                                    CharType target,
                                    ConvertFlags = ConvertFlags::None);
private:
  // For input, either 'Apostrophe' or 'Dash' can be used to separate 'n' in
  // the middle of Rōmaji words like gin'iro, kan'atsu, kan-i, etc.. For Rōmaji
  // output, 'Apostrophe' is used. Note, 'Dash' is used in 'Traditional Hepburn'
  // whereas apostrophe is used in 'Modern (revised) Hepburn'.
  static constexpr auto Apostrophe{'\''}, Dash{'-'};

  // 'verifyData' is called by the constructor and performs various 'asserts' on
  // member data.
  void verifyData() const;

  [[nodiscard]] auto romajiTarget() const {
    return _target == CharType::Romaji;
  }
  [[nodiscard]] auto hiraganaTarget() const {
    return _target == CharType::Hiragana;
  }
  [[nodiscard]] auto& get(const Kana& k) const {
    return k.get(_target, _flags);
  }
  [[nodiscard]] auto& getN() const { return get(Kana::N); }
  [[nodiscard]] auto& getSmallTsu() const { return get(Kana::SmallTsu); }

  using Set = std::set<std::string>;

  // 'insertUnique' performs an insert and ensures the value was added by using
  // 'assert' (can't do on one line like 'assert(s.insert(x).second)' since that
  // would result in the code not getting executed when compiling with asserts
  // disabled, i.e., a 'Release' build.
  static void insertUnique(Set& s, const std::string& x) {
    [[maybe_unused]] const auto i{s.insert(x)};
    assert(i.second);
  }

  [[nodiscard]] std::string convertFromKana(const std::string& input,
                                            CharType source, const Set& afterN,
                                            const Set& smallKana) const;
  [[nodiscard]] std::string kanaLetters(const std::string& letterGroup,
                                        CharType source, u_int8_t count,
                                        const Kana*& prevKana,
                                        bool prolong = false) const;
  [[nodiscard]] std::string convertFromRomaji(const std::string& input) const;
  void romajiLetters(std::string& letterGroup, std::string& result) const;
  [[nodiscard]] bool romajiMacronLetter(const std::string& letter,
                                        std::string& letterGroup,
                                        std::string& result) const;

  // '_repeatingConsonents' is used for processing small 'tsu' for sokuon output
  std::set<char> _repeatingConsonents;

  // '_markAfterN...' contain the 8 Kana (5 vowels and 3 y's) that should be
  // proceedeed with 'Apostrophe' when producing Rōmaji if they follow 'n'.
  Set _markAfterNHiragana;
  Set _markAfterNKatakana;

  // '_digraphSecond...' sets contain the 9 small Kana symbols (5 vowels, 3 y's,
  // and 'wa') that form the second parts of digraphs.
  Set _digraphSecondHiragana;
  Set _digraphSecondKatakana;

  // Punctuation and word delimiter handling
  std::string _narrowDelimList;
  NarrowDelims _narrowDelims;
  WideDelims _wideDelims;

  // Members for the current conversion
  CharType _target;
  ConvertFlags _flags;
};

} // namespace kanji_tools
