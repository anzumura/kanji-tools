#pragma once

#include <kt_kana/Converter.h>
#include <kt_kanji/Ucd.h>

#include <filesystem>

namespace kanji_tools { /// \kanji_group{UcdData}
/// UcdData class for loading data from 'ucd.txt'

/// load, store, find and print Ucd objects \kanji{UcdData}
class UcdData {
public:
  using Map = std::map<String, Ucd>;

  /// return 'meaning' from `u` if it's non-null, otherwise empty string
  [[nodiscard]] static Ucd::Meaning getMeaning(UcdPtr u);

  UcdData() = default; ///< default ctor

  UcdData(const UcdData&) = delete;        ///< deleted copy ctor
  auto operator=(const UcdData&) = delete; ///< deleted operator=

  /// return a (wide) comma separated string starting with 'onReading' converted
  /// to Katakana followed by 'kunReading' converted to Hiragana (spaces within
  /// the readings are also converted to wide commas)
  [[nodiscard]] String getReadingsAsKana(UcdPtr) const;

  /// return pointer to a Ucd instance if `name` is found, otherwise nullptr
  /// \details if `name` has a 'variation selector' then #_linkedJinmei then
  /// #_linkedOther maps are used to get a Ucd variant (variant returned is the
  /// same displayed character for Jinmei ones)
  [[nodiscard]] UcdPtr find(const String& name) const;

  [[nodiscard]] auto& map() const { return _map; }

  /// load Ucd data from `file`
  void load(const std::filesystem::path& file);

  /// print a summary of Ucd data loaded (like various counts and examples)
  void print(const class KanjiData& data) const;
private:
  [[nodiscard]] static Ucd::Links loadLinks(const class ColumnFile&, bool joyo);
  void processLinks(
      const ColumnFile&, const Ucd::Links&, const String& name, bool jinmei);

  void printVariationSelectorKanji(const KanjiData&) const;

  Map _map;

  /// '_linked...' are maps from standard Kanji to variant forms
  /// \details For example, FA67 (逸) is a variant of 9038 (逸) which can also
  /// be constructed by a variation selector, i.e., U"\u9038\uFE01" (逸︁).
  /// \note
  /// \li if a variant is marked as 'Jinmei' it will be put in '_linkedJinmei'
  /// \li otherwise it will be put in '_linkedOther' @{
  std::map<String, String> _linkedJinmei;
  std::map<String, std::vector<String>> _linkedOther; ///@}

  /// used by getReadingsAsKana() to convert Rōmaji readings loaded from UCD
  mutable Converter _converter;
};

/// \end_group
} // namespace kanji_tools
