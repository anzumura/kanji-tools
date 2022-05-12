#include <kanji_tools/kana/MBChar.h>
#include <kanji_tools/kanji/Data.h>
#include <kanji_tools/utils/ColumnFile.h>

#include <sstream>

namespace kanji_tools {

namespace {

const ColumnFile::Column CodeCol{"Code"}, NameCol{"Name"}, BlockCol{"Block"},
    VersionCol{"Version"}, RadicalCol{"Radical"}, StrokesCol{"Strokes"},
    VStrokesCol{"VStrokes"}, PinyinCol{"Pinyin"}, MorohashiIdCol{"MorohashiId"},
    NelsonIdsCol{"NelsonIds"}, SourcesCol{"Sources"}, JSourceCol{"JSource"},
    JoyoCol{"Joyo"}, JinmeiCol{"Jinmei"}, LinkCodesCol{"LinkCodes"},
    LinkNamesCol{"LinkNames"}, LinkTypeCol{"LinkType"}, MeaningCol{"Meaning"},
    OnCol{"On"}, KunCol{"Kun"};

// 'PrintCount' is used for debug printing. Some combinations are prevented by
// 'load' function (like Joyo with a link or missing meaning), but count all
// cases for completeness.
class PrintCount {
public: // LCOV_EXCL_LINE
  PrintCount() noexcept = default;
  PrintCount(const PrintCount&) = delete; // LCOV_EXCL_LINE
  PrintCount& operator=(const PrintCount&) = delete;

  void add(const Ucd& k) {
    ++_count;
    if (k.hasLinks()) ++_link;
    if (k.strokes().hasVariant()) ++_variantStrokes;
    if (!k.meaning().empty()) ++_meaning;
    if (!k.onReading().empty()) ++_onReading;
    if (!k.kunReading().empty()) ++_kunReading;
    if (k.morohashiId()) ++_morohashi;
    if (!k.nelsonIds().empty()) ++_nelson;
  }

  [[nodiscard]] auto count() const { return _count; }
  [[nodiscard]] auto link() const { return _link; }
  [[nodiscard]] auto variantStrokes() const { return _variantStrokes; }
  [[nodiscard]] auto meaning() const { return _meaning; }
  [[nodiscard]] auto onReading() const { return _onReading; }
  [[nodiscard]] auto kunReading() const { return _kunReading; }
  [[nodiscard]] auto morohashi() const { return _morohashi; }
  [[nodiscard]] auto nelson() const { return _nelson; }
private:
  size_t _count{}, _link{}, _variantStrokes{}, _meaning{}, _onReading{},
      _kunReading{}, _morohashi{}, _nelson{};
};

} // namespace

Ucd::Meaning UcdData::getMeaning(UcdPtr u) {
  return u ? u->meaning() : EmptyString;
}

UcdPtr UcdData::find(const String& kanjiName) const {
  auto r{kanjiName};
  if (MBChar::isMBCharWithVariationSelector(kanjiName)) {
    const auto nonVariant{MBChar::noVariationSelector(kanjiName)};
    if (const auto i{_linkedJinmei.find(nonVariant)}; i != _linkedJinmei.end())
      r = i->second; // return jinmei variant
    else
      // could check _linkedOther, but so far this never happens so just return
      // nullptr, i.e., 'variant is not found in the data loaded from ucd.txt'
      return {};
  }
  const auto i{_map.find(r)};
  return i == _map.end() ? nullptr : &i->second;
}

String UcdData::getReadingsAsKana(UcdPtr u) const {
  if (u) {
    auto s{u->onReading()};
    std::replace(s.begin(), s.end(), ' ', ',');
    auto result{_converter.convert(CharType::Romaji, s, CharType::Katakana)};
    s = u->kunReading();
    if (!s.empty()) {
      std::replace(s.begin(), s.end(), ' ', ',');
      // if there are both 'on' and 'kun' readings then separate with a comma
      if (!result.empty()) s = ',' + s;
      result += _converter.convert(CharType::Romaji, s, CharType::Hiragana);
    }
    return result;
  }
  return EmptyString;
}

void UcdData::load(const Data::Path& file) {
  for (ColumnFile f{file,
           {CodeCol, NameCol, BlockCol, VersionCol, RadicalCol, StrokesCol,
               VStrokesCol, PinyinCol, MorohashiIdCol, NelsonIdsCol, SourcesCol,
               JSourceCol, JoyoCol, JinmeiCol, LinkCodesCol, LinkNamesCol,
               LinkTypeCol, MeaningCol, OnCol, KunCol}};
       f.nextRow();) {
    if (f.isEmpty(OnCol) && f.isEmpty(KunCol) && f.isEmpty(MorohashiIdCol) &&
        f.isEmpty(JSourceCol))
      f.error("one of 'On', 'Kun', 'Morohashi' or 'JSource' must be populated");
    auto& name{f.get(NameCol)};
    if (name.size() > 4) f.error("name more than 4 bytes");
    const auto radical{f.getULong(RadicalCol)};
    if (radical < 1 || radical > Radical::MaxRadicals)
      f.error("radical '" + std::to_string(radical) + "' out of range");
    const auto joyo{f.getBool(JoyoCol)}, jinmei{f.getBool(JinmeiCol)};
    if (joyo) {
      if (jinmei) f.error("can't be both joyo and jinmei");
      // meaning is empty for some entries like 乁, 乣, 乴, etc., but it
      // shouldn't be empty for Joyo
      if (f.isEmpty(MeaningCol)) f.error("meaning is empty for Jōyō Kanji");
    }
    auto links{loadLinks(f, joyo)};
    processLinks(f, links, name, jinmei);
    try {
      const auto strokes{f.isEmpty(VStrokesCol) ? Strokes{f.getU8(StrokesCol)}
                                                : Strokes{f.getU8(StrokesCol),
                                                      f.getU8(VStrokesCol)}};
      if (!_map.emplace(std::piecewise_construct, std::make_tuple(name),
                   std::make_tuple(UcdEntry{f.getChar32(CodeCol), name},
                       f.get(BlockCol), f.get(VersionCol), radical, strokes,
                       f.get(PinyinCol), f.get(MorohashiIdCol),
                       f.get(NelsonIdsCol), f.get(SourcesCol),
                       f.get(JSourceCol), joyo, jinmei, std::move(links),
                       AllUcdLinkTypes.fromStringAllowEmpty(f.get(LinkTypeCol)),
                       f.get(MeaningCol), f.get(OnCol), f.get(KunCol)))
               .second)
        throw std::domain_error{"duplicate entry '" + name + "'"};
    } catch (const std::exception& e) {
      f.error(e.what());
    }
  }
}

void UcdData::print(DataRef data) const {
  PrintCount joyo, jinmei, other;
  const auto print{[&](const char* s, auto f) {
    // NOLINTNEXTLINE: CallAndMessage
    const auto x{(joyo.*f)()}, y{(jinmei.*f)()}, z{(other.*f)()};
    data.log() << "  " << s << ": " << x + y + z << " (Jouyou " << x
               << ", Jinmei " << y << ", Other " << z << ")\n";
  }};
  data.log() << "Kanji Loaded from Unicode 'ucd' file:\n";
  for (auto& i : _map)
    if (auto& k{i.second}; k.joyo())
      joyo.add(k);
    else if (k.jinmei())
      jinmei.add(k);
    else
      other.add(k);
  print("Total", &PrintCount::count);
  print("Links", &PrintCount::link);
  print("VStrokes", &PrintCount::variantStrokes);
  print("Meanings", &PrintCount::meaning);
  print("On Readdings", &PrintCount::onReading);
  print("Kun Readings", &PrintCount::kunReading);
  print("Morohashi Ids", &PrintCount::morohashi);
  print("Nelson Ids", &PrintCount::nelson);
  const auto pLinks{[this, &data](const String& name, const auto& list) {
    const auto count{
        std::count_if(list.begin(), list.end(), [this](const auto& i) {
          const auto j{_map.find(i->name())};
          return j != _map.end() && j->second.hasLinks();
        })};
    data.log() << name << " Kanji with links " << count << ":\n";
    for (auto& i : list)
      if (const auto j{_map.find(i->name())}; j != _map.end()) {
        if (j->second.hasLinks())
          data.out() << "  " << j->second.codeAndName() << " -> "
                     << j->second.linkCodeAndNames() << ' '
                     << j->second.linkType() << '\n';
      } else
        data.out() << "  ERROR: " << i->name() << " not found in UCD\n";
  }};
  pLinks("Frequency", data.types(KanjiTypes::Frequency));
  pLinks("Extra", data.types(KanjiTypes::Extra));
  printVariationSelectorKanji(data);
}

Ucd::Links UcdData::loadLinks(const class ColumnFile& f, bool joyo) {
  Ucd::Links links;
  if (!f.isEmpty(LinkNamesCol)) {
    std::stringstream names{f.get(LinkNamesCol)}, codes{f.get(LinkCodesCol)};
    for (String linkName; std::getline(names, linkName, ',');)
      if (String linkCode; std::getline(codes, linkCode, ','))
        links.emplace_back(f.getChar32(LinkCodesCol, linkCode), linkName);
      else
        f.error("LinkNames has more values than LinkCodes");
    // Joyo are standard Kanji so they shouldn't have a link back to a
    // standard form. However, Some Jinmei do have links since they are
    // 'officially allowed variants/old forms'. There are links in raw XML
    // data for joyo, but the parse script ignores them.
    if (joyo) f.error("joyo shouldn't have links");
    if (f.isEmpty(LinkTypeCol))
      f.error("LinkNames has a value, but LinkType is empty");
  } else if (!f.isEmpty(LinkTypeCol))
    f.error("LinkType has a value, but LinkNames is empty");
  else if (!f.isEmpty(LinkCodesCol))
    f.error("LinkCodes has a value, but LinkNames is empty");
  return links;
}

void UcdData::processLinks(const ColumnFile& f, const Ucd::Links& links,
    const String& name, bool jinmei) {
  for (const auto& link : links)
    if (!jinmei)
      _linkedOther[link.name()].emplace_back(name);
    else if (const auto i{_linkedJinmei.emplace(link.name(), name)}; !i.second)
      f.error("jinmei entry '" + name + "' with link '" + link.name() +
              "' failed - link already points to '" + i.first->second + "'");
}

void UcdData::printVariationSelectorKanji(DataRef data) const {
  data.log()
      << "  Standard Kanji with 'Variation Selectors' vs UCD Variants:\n";
  data.log()
      << "    #      Standard Kanji with Selector    UCD Compatibility Kanji\n";
  data.log()
      << "    -      ----------------------------    -----------------------\n";
  for (size_t count{}; auto& i : data.kanjiNameMap())
    if (auto& k{*i.second}; k.variant()) {
      data.log() << "    " << std::left << std::setfill(' ') << std::setw(3)
                 << ++count << "    "
                 << toUnicode(k.name(), BracketType::Square) << ' ' << k.name()
                 << " variant of " << k.nonVariantName() << "    ";
      const auto u{find(k.name())};
      // ucd entry should always exist
      if (!u) Data::usage("UCD not found for '" + k.name() + "'");
      data.out() << u->codeAndName();
      if (u->hasLinks()) data.out() << " variant of " << u->linkCodeAndNames();
      data.out() << '\n';
    }
}

} // namespace kanji_tools
