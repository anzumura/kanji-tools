#include <kanji_tools/kana/MBChar.h>
#include <kanji_tools/kanji/Data.h>
#include <kanji_tools/utils/ColumnFile.h>
#include <kanji_tools/utils/Utils.h>

#include <sstream>

namespace kanji_tools {

namespace {

// 'PrintCount' is used for debug printing. Some combinations are prevented by
// 'load' function (like Joyo with a link or missing meaning), but count all
// cases for completeness.
class PrintCount {
public:
  PrintCount() noexcept = default;
  PrintCount(const PrintCount&) = delete;
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

UcdPtr UcdData::find(const std::string& kanjiName) const {
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

std::string UcdData::getReadingsAsKana(UcdPtr u) const {
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
  const ColumnFile::Column codeCol{"Code"}, nameCol{"Name"}, blockCol{"Block"},
      versionCol{"Version"}, radicalCol{"Radical"}, strokesCol{"Strokes"},
      vStrokesCol{"VStrokes"}, pinyinCol{"Pinyin"}, morohashiCol{"MorohashiId"},
      nelsonIdsCol{"NelsonIds"}, sourcesCol{"Sources"}, jSourceCol{"JSource"},
      joyoCol{"Joyo"}, jinmeiCol{"Jinmei"}, linkCodesCol{"LinkCodes"},
      linkNamesCol{"LinkNames"}, linkTypeCol{"LinkType"}, meaningCol{"Meaning"},
      onCol{"On"}, kunCol{"Kun"};
  for (ColumnFile f{file,
           {codeCol, nameCol, blockCol, versionCol, radicalCol, strokesCol,
               vStrokesCol, pinyinCol, morohashiCol, nelsonIdsCol, sourcesCol,
               jSourceCol, joyoCol, jinmeiCol, linkCodesCol, linkNamesCol,
               linkTypeCol, meaningCol, onCol, kunCol}};
       f.nextRow();) {
    if (f.isEmpty(onCol) && f.isEmpty(kunCol) && f.isEmpty(morohashiCol) &&
        f.isEmpty(jSourceCol))
      f.error("one of 'On', 'Kun', 'Morohashi' or 'JSource' must be populated");
    auto& name{f.get(nameCol)};
    if (name.size() > 4) f.error("name more than 4 bytes");
    const auto radical{f.getULong(radicalCol)};
    if (radical < 1 || radical > Radical::MaxRadicals)
      f.error("radical '" + std::to_string(radical) + "' out of range");
    const auto joyo{f.getBool(joyoCol)}, jinmei{f.getBool(jinmeiCol)};
    if (joyo) {
      if (jinmei) f.error("can't be both joyo and jinmei");
      // meaning is empty for some entries like 乁, 乣, 乴, etc., but it
      // shouldn't be empty for Joyo
      if (f.isEmpty(meaningCol)) f.error("meaning is empty for Jōyō Kanji");
    }

    Ucd::Links links;
    if (!f.isEmpty(linkNamesCol)) {
      std::stringstream names{f.get(linkNamesCol)}, codes{f.get(linkCodesCol)};
      for (std::string linkName; std::getline(names, linkName, ',');)
        if (std::string linkCode; std::getline(codes, linkCode, ','))
          links.emplace_back(f.getChar32(linkCodesCol, linkCode), linkName);
        else
          f.error("LinkNames has more values than LinkCodes");
      // Joyo are standard Kanji so they shouldn't have a link back to a
      // standard form. However, Some Jinmei do have links since they are
      // 'officially allowed variants/old forms'. There are links in raw XML
      // data for joyo, but the parse script ignores them.
      if (joyo) f.error("joyo shouldn't have links");
      if (f.isEmpty(linkTypeCol))
        f.error("LinkNames has a value, but LinkType is empty");
    } else if (!f.isEmpty(linkTypeCol))
      f.error("LinkType has a value, but LinkNames is empty");
    else if (!f.isEmpty(linkCodesCol))
      f.error("LinkCodes has a value, but LinkNames is empty");
    try {
      const auto strokes{f.isEmpty(vStrokesCol) ? Strokes{f.getU8(strokesCol)}
                                                : Strokes{f.getU8(strokesCol),
                                                      f.getU8(vStrokesCol)}};
      if (!_map.emplace(std::piecewise_construct, std::make_tuple(name),
                   std::make_tuple(UcdEntry{f.getChar32(codeCol), name},
                       f.get(blockCol), f.get(versionCol), radical, strokes,
                       f.get(pinyinCol), f.get(morohashiCol),
                       f.get(nelsonIdsCol), f.get(sourcesCol),
                       f.get(jSourceCol), joyo, jinmei, links,
                       AllUcdLinkTypes.fromStringAllowEmpty(f.get(linkTypeCol)),
                       f.get(meaningCol), f.get(onCol), f.get(kunCol)))
               .second)
        throw std::domain_error{"duplicate entry '" + name + "'"};
    } catch (const std::exception& e) {
      f.error(e.what());
    }
    for (const auto& link : links)
      if (!jinmei)
        _linkedOther[link.name()].emplace_back(name);
      else if (const auto i{_linkedJinmei.emplace(link.name(), name)};
               !i.second)
        f.error("jinmei entry '" + name + "' with link '" + link.name() +
                "' failed - link already points to '" + i.first->second + "'");
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
  const auto pLinks{[this, &data](const std::string& name, const auto& list) {
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
