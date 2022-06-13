#include <kt_kana/Kana.h>
#include <kt_utils/UnicodeBlock.h>

#include <cassert>

namespace kanji_tools {

namespace {

namespace kana_lists {

// define some short type aliases to help create arrays of Kana below
using K = Kana;
using V = Kana::RomajiVariants;

// 'KanaList' has mappings for all monographs (single Kana) with no 'dakuten' or
// 'han-dakuten' versions and regularly used digraphs (normal Kana followed by a
// small Kana 'vowel', 'y' or 'wa'). See comments for 'Kana' class for a
// description of the fields.
const std::array KanaList{// --- あ 行 ---
    K{"a", "あ", "ア"}, K{"na", "な", "ナ"}, K{"ma", "ま", "マ"},
    K{"ya", "や", "ヤ"}, K{"ra", "ら", "ラ"}, K{"wa", "わ", "ワ"},
    // あ Digraphs
    K{"qwa", "くゎ", "クヮ"}, K{"swa", "すぁ", "スァ"},
    K{"tsa", "つぁ", "ツァ"}, K{"nya", "にゃ", "ニャ"},
    K{"fa", "ふぁ", "ファ", V{"fwa", "hwa"}}, K{"fya", "ふゃ", "フャ"},
    K{"mya", "みゃ", "ミャ"}, K{"rya", "りゃ", "リャ"},
    // --- い 行 ---
    K{"i", "い", "イ"}, K{"ni", "に", "ニ"}, K{"mi", "み", "ミ"},
    K{"ri", "り", "リ"}, K{"wyi", "ゐ", "ヰ", "i", "i"},
    // い Digraphs
    K{"swi", "すぃ", "スィ"}, K{"tsi", "つぃ", "ツィ"},
    K{"nyi", "にぃ", "ニィ"}, K{"fi", "ふぃ", "フィ", V{"fyi", "fwi", "hwi"}},
    K{"myi", "みぃ", "ミィ"}, K{"ryi", "りぃ", "リィ"},
    // --- う 行 ---
    K{"nu", "ぬ", "ヌ"}, K{"mu", "む", "ム"}, K{"yu", "ゆ", "ユ"},
    K{"ru", "る", "ル"},
    //う Digraphs
    K{"swu", "すぅ", "スゥ"}, K{"nyu", "にゅ", "ニュ"},
    K{"fwu", "ふぅ", "フゥ"}, K{"fyu", "ふゅ", "フュ"},
    K{"myu", "みゅ", "ミュ"}, K{"ryu", "りゅ", "リュ"},
    // --- え 行 ---
    K{"e", "え", "エ"}, K{"ne", "ね", "ネ"}, K{"me", "め", "メ"},
    K{"re", "れ", "レ"}, K{"wye", "ゑ", "ヱ", "e", "e"},
    // え Digraphs
    K{"ye", "いぇ", "イェ"}, K{"swe", "すぇ", "スェ"}, K{"tse", "つぇ", "ツェ"},
    K{"nye", "にぇ", "ニェ"}, K{"fe", "ふぇ", "フェ", V{"fye", "fwe", "hwe"}},
    K{"mye", "みぇ", "ミェ"}, K{"rye", "りぇ", "リェ"},
    // --- お 行 ---
    K{"o", "お", "オ"}, K{"no", "の", "ノ"}, K{"mo", "も", "モ"},
    K{"yo", "よ", "ヨ"}, K{"ro", "ろ", "ロ"}, K{"wo", "を", "ヲ", "o", "o"},
    //お Digraphs
    K{"swo", "すぉ", "スォ"}, K{"tso", "つぉ", "ツォ"},
    K{"nyo", "にょ", "ニョ"}, K{"fo", "ふぉ", "フォ", V{"fwo", "hwo"}},
    K{"fyo", "ふょ", "フョ"}, K{"myo", "みょ", "ミョ"},
    K{"ryo", "りょ", "リョ"},
    // Digraphs that only have a dakuten version (all start with 'v')
    K{"va", "ゔぁ", "ヴァ"}, K{"vo", "ゔぉ", "ヴォ"}, K{"vya", "ゔゃ", "ヴャ"},
    K{"vyu", "ゔゅ", "ヴュ"}, K{"vyo", "ゔょ", "ヴョ"},
    // 12 Small letters (5 vowels, 2 k's, 3 y's, small 'wa' and small 'tsu') -
    // prefer 'l' versions for Rōmaji output, but 'x' version is also included
    K{"la", "ぁ", "ァ", V{"xa"}}, K{"li", "ぃ", "ィ", V{"xi"}},
    K{"lu", "ぅ", "ゥ", V{"xu"}}, K{"le", "ぇ", "ェ", V{"xe", "lye", "xye"}},
    K{"lo", "ぉ", "ォ", V{"xo"}}, K{"lka", "ゕ", "ヵ", V{"xka"}},
    K{"lke", "ゖ", "ヶ", V{"xke"}}, K{"lya", "ゃ", "ャ", V{"xya"}},
    K{"lyu", "ゅ", "ュ", V{"xyu"}}, K{"lyo", "ょ", "ョ", V{"xyo"}},
    K{"lwa", "ゎ", "ヮ", V{"xwa"}},
    // Keep 'small tsu' and 'n' at the end of the list. Some input methods allow
    // 'ltsu' and 'xtsu' for 'small tsu', but ignore these combinations for now.
    K{"ltu", "っ", "ッ", V{"xtu"}}, K{"n", "ん", "ン"}};

using D = DakutenKana;
// 'DakutenKanaList' holds Kana that have a 'dakuten' version (but not 'h' row)
const std::array DakutenKanaList{// --- あ 行 ---
    D{K{"ga", "が", "ガ"}, "ka", "か", "カ"},
    D{K{"za", "ざ", "ザ"}, "sa", "さ", "サ"},
    D{K{"da", "だ", "ダ"}, "ta", "た", "タ"},
    D{K{"gya", "ぎゃ", "ギャ"}, "kya", "きゃ", "キャ"},
    // あ Diagraphs
    D{K{"gwa", "ぐぁ", "グァ"}, "qa", "くぁ", "クァ", V{"kwa"}},
    D{K{"ja", "じゃ", "ジャ", V{"zya", "jya", true}}, "sha", "しゃ", "シャ",
        V{"sya", true}},
    D{K{"dya", "ぢゃ", "ヂャ", "ja", "zya"}, "cha", "ちゃ", "チャ",
        V{"tya", true}},
    D{K{"dha", "でゃ", "デャ"}, "tha", "てゃ", "テャ"},
    D{K{"dwa", "どぁ", "ドァ"}, "twa", "とぁ", "トァ"},
    // --- い 行 ---
    D{K{"gi", "ぎ", "ギ"}, "ki", "き", "キ"},
    D{K{"ji", "じ", "ジ", V{"zi", true}}, "shi", "し", "シ", V{"si", true}},
    D{K{"di", "ぢ", "ヂ", "ji", "zi"}, "chi", "ち", "チ", V{"ti", true}},
    // い Digraphs
    D{K{"vi", "ゔぃ", "ヴィ"}, "wi", "うぃ", "ウィ"},
    D{K{"gwi", "ぐぃ", "グィ"}, "qi", "くぃ", "クィ", V{"kwi", "qwi"}},
    D{K{"gyi", "ぎぃ", "ギィ"}, "kyi", "きぃ", "キィ"},
    D{K{"jyi", "じぃ", "ジィ", V{"zyi"}}, "syi", "しぃ", "シィ"},
    D{K{"dyi", "ぢぃ", "ヂィ"}, "tyi", "ちぃ", "チィ"},
    D{K{"dwi", "どぃ", "ドィ"}, "twi", "とぃ", "トィ"},
    D{K{"dhi", "でぃ", "ディ"}, "thi", "てぃ", "ティ"},
    // --- う 行 ---
    D{K{"vu", "ゔ", "ヴ"}, "u", "う", "ウ", V{"wu"}},
    D{K{"gu", "ぐ", "グ"}, "ku", "く", "ク"},
    D{K{"zu", "ず", "ズ"}, "su", "す", "ス"},
    D{K{"du", "づ", "ヅ", "zu", "zu"}, "tsu", "つ", "ツ", V{"tu", true}},
    // う Digraphs
    D{K{"gyu", "ぎゅ", "ギュ"}, "kyu", "きゅ", "キュ"},
    D{K{"gwu", "ぐぅ", "グゥ"}, "qu", "くぅ", "クゥ", V{"kwu", "qwu"}},
    D{K{"ju", "じゅ", "ジュ", V{"zyu", "jyu", true}}, "shu", "しゅ", "シュ",
        V{"syu", true}},
    D{K{"dyu", "ぢゅ", "ヂュ", "ju", "zyu"}, "chu", "ちゅ", "チュ",
        V{"tyu", true}},
    D{K{"dhu", "でゅ", "デュ"}, "thu", "てゅ", "テュ"},
    D{K{"dwu", "どぅ", "ドゥ"}, "twu", "とぅ", "トゥ"},
    // --- え 行 ---
    D{K{"ge", "げ", "ゲ"}, "ke", "け", "ケ"},
    D{K{"gye", "ぎぇ", "ギェ"}, "kye", "きぇ", "キェ"},
    D{K{"ze", "ぜ", "ゼ"}, "se", "せ", "セ"},
    D{K{"de", "で", "デ"}, "te", "て", "テ"},
    // え Digraphs
    D{K{"ve", "ゔぇ", "ヴェ"}, "we", "うぇ", "ウェ"},
    D{K{"gwe", "ぐぇ", "グェ"}, "qe", "くぇ", "クェ", V{"kwe", "qwe"}},
    D{K{"je", "じぇ", "ジェ", V{"zye", "jye"}}, "she", "しぇ", "シェ"},
    D{K{"dye", "ぢぇ", "ヂェ"}, "che", "ちぇ", "チェ", V{"tye"}},
    D{K{"dhe", "でぇ", "デェ"}, "the", "てぇ", "テェ"},
    D{K{"dwe", "どぇ", "ドェ"}, "twe", "とぇ", "トェ"},
    // --- お 行 ---
    D{K{"go", "ご", "ゴ"}, "ko", "こ", "コ"},
    D{K{"zo", "ぞ", "ゾ"}, "so", "そ", "ソ"},
    D{K{"do", "ど", "ド"}, "to", "と", "ト"},
    // お Digraphs
    D{K{"gyo", "ぎょ", "ギョ"}, "kyo", "きょ", "キョ"},
    D{K{"gwo", "ぐぉ", "グォ"}, "qo", "くぉ", "クォ", V{"kwo", "qwo"}},
    D{K{"jo", "じょ", "ジョ", V{"zyo", "jyo", true}}, "sho", "しょ", "ショ",
        V{"syo", true}},
    D{K{"dyo", "ぢょ", "ヂョ", "jo", "zyo"}, "cho", "ちょ", "チョ",
        V{"tyo", true}},
    D{K{"dho", "でょ", "デョ"}, "tho", "てょ", "テョ"},
    D{K{"dwo", "どぉ", "ドォ"}, "two", "とぉ", "トォ"}};

using H = HanDakutenKana;
// 'HanDakutenKanaList' has Kana that have both a 'dakuten' and a 'han-dakuten'
// version (so just the 'h' row)
const std::array HanDakutenKanaList{
    H{K{"pa", "ぱ", "パ"}, K{"ba", "ば", "バ"}, "ha", "は", "ハ"},
    H{K{"pi", "ぴ", "ピ"}, K{"bi", "び", "ビ"}, "hi", "ひ", "ヒ"},
    H{K{"pu", "ぷ", "プ"}, K{"bu", "ぶ", "ブ"}, "fu", "ふ", "フ",
        V{"hu", true}},
    H{K{"pe", "ぺ", "ペ"}, K{"be", "べ", "ベ"}, "he", "へ", "ヘ"},
    H{K{"po", "ぽ", "ポ"}, K{"bo", "ぼ", "ボ"}, "ho", "ほ", "ホ"},
    // Digraphs
    H{K{"pya", "ぴゃ", "ピャ"}, K{"bya", "びゃ", "ビャ"}, "hya", "ひゃ",
        "ヒャ"},
    H{K{"pyi", "ぴぃ", "ピィ"}, K{"byi", "びぃ", "ビィ"}, "hyi", "ひぃ",
        "ヒィ"},
    H{K{"pyu", "ぴゅ", "ピュ"}, K{"byu", "びゅ", "ビュ"}, "hyu", "ひゅ",
        "ヒュ"},
    H{K{"pye", "ぴぇ", "ピェ"}, K{"bye", "びぇ", "ビェ"}, "hye", "ひぇ",
        "ヒェ"},
    H{K{"pyo", "ぴょ", "ピョ"}, K{"byo", "びょ", "ビョ"}, "hyo", "ひょ",
        "ヒョ"}};

} // namespace kana_lists

using kana_lists::KanaList, kana_lists::DakutenKanaList,
    kana_lists::HanDakutenKanaList;

} // namespace

const Kana::Map Kana::RomajiMap{Kana::populate(CharType::Romaji)},
    Kana::HiraganaMap{Kana::populate(CharType::Hiragana)},
    Kana::KatakanaMap{Kana::populate(CharType::Katakana)};

const Kana::IterationMark Kana::RepeatPlain{"ゝ", "ヽ", false},
    Kana::RepeatAccented{"ゞ", "ヾ", true};

const Kana& Kana::SmallTsu{KanaList[KanaList.size() - 2]};
const Kana& Kana::N{KanaList[KanaList.size() - 1]};

bool Kana::IterationMark::matches(CharType t, const String& s) const {
  return t == CharType::Hiragana && _hiragana == s ||
         t == CharType::Katakana && _katakana == s;
}

const String& Kana::IterationMark::get(
    CharType target, ConvertFlags flags, const Kana* prevKana) const {
  switch (target) {
  case CharType::Hiragana: return _hiragana;
  case CharType::Katakana: return _katakana;
  case CharType::Romaji: break;
  }
  if (!prevKana) return emptyString();
  const Kana* k{prevKana};
  if (_dakuten) {
    if (const auto accented{prevKana->dakuten()}; accented) k = accented;
  } else if (const auto unaccented{prevKana->plain()}; unaccented)
    k = unaccented;
  return k->getRomaji(flags);
}

Kana::IterationMark::IterationMark(CharArray<OneKanaArraySize> hiragana,
    CharArray<OneKanaArraySize> katakana, bool dakuten)
    : _hiragana{hiragana}, _katakana{katakana}, _dakuten{dakuten} {
  validate();
}

void Kana::IterationMark::validate() const {
  assert(isAllHiragana(_hiragana));
  assert(isAllKatakana(_katakana));
}

const Kana::IterationMark* Kana::findIterationMark(
    CharType source, const String& kana) {
  if (RepeatPlain.matches(source, kana)) return &RepeatPlain;
  if (RepeatAccented.matches(source, kana)) return &RepeatAccented;
  return {};
}

const Kana::Map& Kana::getMap(CharType t) {
  switch (t) {
  case CharType::Romaji: return RomajiMap;
  case CharType::Hiragana: return HiraganaMap;
  case CharType::Katakana: return KatakanaMap;
  }
  __builtin_unreachable(); // stop gcc 'reaches end' warning XCOV_EXCL_LINE
}

Kana::OptString Kana::findDakuten(const String& s) {
  auto i{HiraganaMap.find(s)};
  if (i != HiraganaMap.end()) return i->second->dakuten(CharType::Hiragana);
  i = KatakanaMap.find(s);
  return i != KatakanaMap.end() ? i->second->dakuten(CharType::Katakana)
                                : EmptyOptString;
}

Kana::OptString Kana::findHanDakuten(const String& s) {
  auto i{HiraganaMap.find(s)};
  if (i != HiraganaMap.end()) return i->second->hanDakuten(CharType::Hiragana);
  i = KatakanaMap.find(s);
  return i != KatakanaMap.end() ? i->second->hanDakuten(CharType::Katakana)
                                : EmptyOptString;
}

const Kana* Kana::dakuten() const { return nullptr; }
const Kana* Kana::hanDakuten() const { return nullptr; }
const Kana* Kana::plain() const { return nullptr; }

Kana::OptString Kana::dakuten(CharType t) const {
  if (const auto i{dakuten()}; i) return i->get(t, ConvertFlags::None);
  return EmptyOptString;
}

Kana::OptString Kana::hanDakuten(CharType t) const {
  if (const auto i{hanDakuten()}; i) return i->get(t, ConvertFlags::None);
  return EmptyOptString;
}

bool Kana::isSmall() const { return _romaji.starts_with("l"); }
bool Kana::isMonograph() const { return _hiragana.size() == OneKanaSize; }
bool Kana::isDigraph() const { return _hiragana.size() == TwoKanaSize; }

bool Kana::isDakuten() const {
  if (auto* p{plain()}; p) return p->dakuten() == this;
  // special case for a few digraphs that start with 'v', but don't have an
  // unaccented version (see 'plain' method comments for more details)
  return _romaji.starts_with("v");
}

bool Kana::isHanDakuten() const {
  if (auto* p{plain()}; p) return p->hanDakuten() == this;
  return false;
}

const String& Kana::getRomaji(ConvertFlags flags) const {
  return hasValue(flags & ConvertFlags::Hepburn) && _hepburn ? *_hepburn
         : hasValue(flags & ConvertFlags::Kunrei) && kunreiVariant()
             ? romajiVariants()[0]
         : hasValue(flags & ConvertFlags::Kunrei) && _kunrei ? *_kunrei
                                                             : _romaji;
}

String Kana::getSokuonRomaji(ConvertFlags flags) const {
  auto& r{getRomaji(flags)};
  return (r[0] == 'c' ? 't' : r[0]) + r;
}

const String& Kana::get(CharType t, ConvertFlags flags) const {
  switch (t) {
  case CharType::Romaji: return getRomaji(flags);
  case CharType::Hiragana: return _hiragana;
  case CharType::Katakana: return _katakana;
  }
  __builtin_unreachable(); // stop gcc 'reaches end' warning XCOV_EXCL_LINE
}

bool Kana::containsKana(const String& s) const {
  return s == _hiragana || s == _katakana;
}

bool Kana::operator==(const Kana& rhs) const {
  // comparing _romaji is good enough since uniqueness is enforced by the rest
  // of the program
  return _romaji == rhs._romaji;
}

void Kana::validate() const {
  for ([[maybe_unused]] auto& i : romajiVariants()) assert(isAllSingleByte(i));
  assert(isAllSingleByte(_romaji));
  assert(isAllHiragana(_hiragana));
  assert(isAllKatakana(_katakana));
}

Kana::Map Kana::populate(CharType t) {
  Kana::Map result;
  size_t duplicates{};
  const auto insert{[&result, &duplicates, t](auto& k, auto& v) {
    if (const auto i{result.emplace(k, &v)}; !i.second) {
      // XCOV_EXCL_START: print all failures and then exit via an 'assert' below
      std::cerr << "key '" << k << "' already in " << toString(t)
                << " map: " << i.first->second << '\n';
      ++duplicates; // NOLINT
      // XCOV_EXCL_STOP
    }
  }};
  const auto processKana{[&insert, t](auto& k) {
    switch (t) {
    case CharType::Romaji:
      insert(k.romaji(), k);
      for (auto& i : k.romajiVariants()) insert(i, k);
      break;
    case CharType::Hiragana: insert(k.hiragana(), k); break;
    case CharType::Katakana: insert(k.katakana(), k); break;
    }
  }};
  // process lists (inserting into 'result')
  for (auto& i : KanaList) processKana(i);
  for (auto& i : DakutenKanaList) {
    processKana(i);
    processKana(*i.dakuten());
  }
  for (auto& i : HanDakutenKanaList) {
    processKana(i);
    processKana(*i.dakuten());
    processKana(*i.hanDakuten());
  }
  assert(duplicates == 0);
  return result;
}

const Kana* DakutenKana::dakuten() const { return &_dakuten; }

DakutenKana::AccentedKana::AccentedKana(Kana&& k, const Kana& p)
    : Kana{std::move(k)}, _plain{p} {}

const Kana* DakutenKana::AccentedKana::plain() const { return &_plain; }

const Kana* HanDakutenKana::hanDakuten() const { return &_hanDakuten; }

// Kana and related class ctor template definitions can be in the .cpp file
// since Kana objects are only created in this TU (Translation Unit)

template<size_t R>
Kana::RomajiVariants::RomajiVariants(CharArray<R> r, bool kunrei)
    : _list{r}, _kunrei{kunrei} {
  check<R>();
}

template<size_t R>
Kana::RomajiVariants::RomajiVariants(
    CharArray<R> r1, CharArray<R> r2, bool kunrei)
    : _list{r1, r2}, _kunrei{kunrei} {
  check<R>();
}

template<size_t R>
Kana::RomajiVariants::RomajiVariants(CharArray<R> r1, RMax r2, RMax r3)
    : _list{r1, r2, r3} {
  check<R>();
}

template<size_t R, size_t A>
Kana::Kana(CharArray<R> romaji, CharArray<A> hiragana, CharArray<A> katakana)
    : Kana{romaji, hiragana, katakana, {}, {}, {}} {}

template<size_t R, size_t A, size_t H, size_t K>
Kana::Kana(CharArray<R> romaji, CharArray<A> hiragana, CharArray<A> katakana,
    CharArray<H> hepburn, CharArray<K> kunrei)
    : Kana{romaji, hiragana, katakana, hepburn, kunrei, {}} {
  static_assert(H <= RomajiArrayMax && K <= RomajiArrayMax);
  if constexpr (A == OneKanaArraySize)
    static_assert(H >= RomajiArrayMin && K >= RomajiArrayMin);
  else {
    // all digraphs have Rōmaji of at least 2 characters
    static_assert(A == TwoKanaArraySize);
    static_assert(H > RomajiArrayMin && K > RomajiArrayMin);
  }
}

template<size_t R, size_t A>
Kana::Kana(CharArray<R> romaji, CharArray<A> hiragana, CharArray<A> katakana,
    RomajiVariants&& variants)
    : Kana{romaji, hiragana, katakana, {}, {}, std::move(variants)} {}

template<size_t R, size_t A>
Kana::Kana(CharArray<R> romaji, CharArray<A> hiragana, CharArray<A> katakana,
    const char* hepburn, const char* kunrei, RomajiVariants&& variants)
    : _romaji{romaji}, _hiragana{hiragana}, _katakana{katakana},
      _hepburn{hepburn ? OptString{hepburn} : std::nullopt},
      _kunrei{kunrei ? OptString{kunrei} : std::nullopt}, _variants{std::move(
                                                              variants)} {
  static_assert(R <= RomajiArrayMax);
  // Hiragana and Katakana must be the same size (3 or 6) and also check that
  // Rōmaji is at least 1 character for a monograph or 2 for a digraph
  static_assert(A == OneKanaArraySize && R >= RomajiArrayMin ||
                A == TwoKanaArraySize && R > RomajiArrayMin);
  validate();
}

template<typename... T>
DakutenKana::DakutenKana(Kana&& dakuten, T&&... t)
    : Kana{std::forward<T>(t)...}, _dakuten{std::move(dakuten), *this} {}

template<typename... T>
HanDakutenKana::HanDakutenKana(Kana&& hanDakuten, T&&... t)
    : DakutenKana{std::forward<T>(t)...}, _hanDakuten{
                                              std::move(hanDakuten), *this} {}

} // namespace kanji_tools
