#pragma once

#include <kanji_tools/utils/Bitmask.h>
#include <kanji_tools/utils/EnumList.h>

namespace kanji_tools { /// \kana_group{KanaEnums}
/// #CharType and #ConvertFlags enums

/// used by Converter class for 'source' and 'target' types when converting as
/// well as Kana::get(), Kana::IterationMark::get() and other functions
enum class CharType : Enum::Size {
  Hiragana, ///< Japanese Hiragana (平仮名) syllable script
  Katakana, ///< Japanese Katakana (片仮名) syllable script
  Romaji    ///< Rōmaji (ローマ字), Japanese written in Latin script
};
/// enable #CharType to be used in an EnumList
template<> inline constexpr auto is_enumlist<CharType>{true};
/// create an EnumList for #CharType
inline const auto CharTypes{
    BaseEnumList<CharType>::create("Hiragana", "Katakana", "Romaji")};

/// used by Kana and Converter classes to control some aspects of conversion
///
/// \details Here are some examples of how these flags affect conversion:
/// \code
///   using enum ConvertFlags;
///   // Hepburn: only affects Rōmaji output
///   convert("つづき", CharType::Romaji);          // "tsuduki"
///   convert("つづき", CharType::Romaji, Hepburn); // "tsuzuki"
///   // Kunrei: only affects Rōmaji output
///   convert("しつ", CharType::Romaji);         // "shitsu"
///   convert("しつ", CharType::Romaji, Kunrei); // "situ"
///   // ProlongMark: only affects Hiragana output
///   convert("rāmen", CharType::Hiragana);                // "らーめん"
///   convert("rāmen", CharType::Hiragana, NoProlongMark); // "らあめん"
///   // RemoveSpaces: only applies when converting from Rōmaji
///   convert("akai kitsune", CharType::Hiragana); // output has a wide space
///   convert("akai kitsune", CharType::Hiragana, RemoveSpaces); // no spaces
/// \endcode
///
/// Prolonged sound marks in Hiragana are non-standard, but use by default in
/// order to support round-trip type conversions, otherwise "rāmen" would map to
/// "らあめん" which would map back to "raamen" (not the initial value).
///
/// ConvertFlags support bitwise operators so they can be combined, for example:
/// \code
///   convert("rāmen desu.", CharType::Hiragana, RemoveSpaces | NoProlongMark);
/// \endcode
/// results in "らあめんです。"
///
/// \note Enabling 'Hepburn' flag for get() and convert() functions results in
/// *more standard* Rōmaji, but the output is ambiguous and leads to different
/// Kana if converted back. This affects di (ぢ), dya (ぢゃ), dyo (ぢょ), dyu
/// (ぢゅ), du (づ) and wo (を) - these become ji, ja, ju, jo, zu and o instead.
/// There's also no support for trying to handle は and へ (which in standard
/// Hepburn should map to 'wa' and 'e' if they are used as particles) - instead
/// they always map to 'ha' and 'he'. If both Hepburn and Kunrei flags are set
/// then Hepburn is preferred, but will then try Kunrei before falling back to
/// the unique Kana::_romaji value.
enum class ConvertFlags : Enum::Size {
  None,              ///< no value (the default)
  Hepburn,           ///< use Hepburn style Rōmaji
  Kunrei,            ///< use Kunrei style Rōmaji
  NoProlongMark = 4, ///< don't use ProlongMark (ー) in Hiragana output
  RemoveSpaces = 8   ///< remove spaces in %Kana output
};
/// enable bitwise operators for #ConvertFlags
template<> inline constexpr auto is_bitmask<ConvertFlags>{true};

/// \end_group
} // namespace kanji_tools
