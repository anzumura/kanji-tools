#pragma once

#include <kanji_tools/quiz/Group.h>

namespace kanji_tools { /// \quiz_group{GroupData}
/// GroupData class for loading data from '-groups.txt' files

/// load, store and print Group objects \quiz{GroupData}
class GroupData {
public:
  using MultiMap = std::multimap<String, GroupPtr>;
  using Map = std::map<String, GroupPtr>;
  using List = std::vector<GroupPtr>;

  /// ctor loads data from '-group.txt' files and prints a summary or all data
  /// loaded depending on the value of KanjiData::DebugMode
  /// \param data used for validating Kanji and printing
  /// \param dir can final using `data->dataDir()` (to help testing)
  /// \throw DomainError if group data is malformed, see Group.h for exceptions
  ///     thrown by group ctors
  explicit GroupData(const KanjiDataPtr& data, const KanjiData::Path* dir = {});

  GroupData(const GroupData&) = delete;      ///< deleted copy ctor
  auto operator=(const GroupData&) = delete; ///< deleted operator=

  [[nodiscard]] auto& meaningGroups() const { return _meaningGroups; }
  [[nodiscard]] auto& patternGroups() const { return _patternGroups; }
  [[nodiscard]] auto& meaningMap() const { return _meaningMap; }
  [[nodiscard]] auto& patternMap() const { return _patternMap; }
  [[nodiscard]] auto& data() const { return *_data; }
  [[nodiscard]] auto& out() const { return _data->out(); }
  [[nodiscard]] auto& log(bool heading = false) const {
    return _data->log(heading);
  }
private:
  const KanjiData::Path& dataDir(const KanjiData::Path*) const;

  /// add 'kanji'->'group' mapping (no error is logged 'MultiMap' final)
  static void add(const String& kanji, MultiMap&, const GroupPtr& group);

  /// add 'kanji'->'group' mapping or log an error if it's already been added
  void add(const String& kanji, Map&, const GroupPtr& group) const;

  template<typename T>
  void loadGroup(const KanjiData::Path&, T&, List&, GroupType);

  static KanjiListFile::StringList getKanjiNames(const String& name,
      const String& members, GroupType, Group::PatternType&);

  [[nodiscard]] static GroupPtr createGroup(size_t number, const String& name,
      const KanjiData::List&, Group::PatternType);

  template<typename T> void printGroups(const T&, const List&) const;

  using StringList = std::vector<String>;
  using TypeMap = std::map<KanjiTypes, StringList>;
  using StringSet = std::set<String>;

  void printMeaningGroup(const Group&, TypeMap&, StringSet&) const;
  void printPatternGroup(const Group&, TypeMap&) const;
  template<typename T> void printUniqueNames(const T&, const StringSet&) const;
  void printTypeBreakdown(TypeMap&) const;
  void printMissingFromType(const KanjiData::List&, StringList&) const;

  [[nodiscard]] auto fullDebug() const { return _data->fullDebug(); }

  /// populated from 'meaning-groups.txt' @{
  MultiMap _meaningMap;
  List _meaningGroups; ///@}

  /// populated from 'pattern-groups.txt' @{
  Map _patternMap;
  List _patternGroups; ///@}

  const KanjiDataPtr _data;
};

using GroupDataPtr = std::shared_ptr<const GroupData>;

/// \end_group
} // namespace kanji_tools
