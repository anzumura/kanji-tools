#include <kanji_tools/quiz/GroupQuiz.h>
#include <kanji_tools/utils/DisplaySize.h>
#include <kanji_tools/utils/Utils.h>

#include <optional>
#include <random>

namespace kanji_tools {

namespace {

std::random_device RandomDevice;
std::mt19937 RandomGen(RandomDevice());

const Choice::Choices PatternGroupChoices{{'1', "ア"}, {'2', "カ"}, {'3', "サ"},
    {'4', "タ、ナ"}, {'5', "ハ、マ"}, {'6', "ヤ、ラ、ワ"}};

constexpr auto DefaultPatternGroup{'1'};

// Since there are over 1000 pattern groups, split them into 6 buckets based on
// reading. The first bucket starts at 'ア', the second bucket starts at 'カ'
// and so on (see 'PatternBucketChoices' above).
constexpr std::array PatternGroups{"：カ", "：サ", "：タ", "：ハ", "：ヤ"};

constexpr auto RefreshOption{'\''}, EditOption{'*'};

constexpr auto TotalLetters{'z' - 'a'}; // LCOV_EXCL_LINE: gcov bug

} // namespace

GroupQuiz::GroupQuiz(const QuizLauncher& launcher, Question question,
    // LCOV_EXCL_START: gcov bug
    bool showMeanings, const GroupData::List& list, MemberType memberType)
    // LCOV_EXCL_STOP
    : Quiz{launcher, question, showMeanings}, _groupType{getGroupType(list)} {
  std::optional<size_t> bucket;
  // for 'pattern' groups, allow choosing a smaller subset (based on 'reading')
  if (_groupType == GroupType::Pattern) {
    const auto c{get("Pattern name", PatternGroupChoices, DefaultPatternGroup)};
    if (isQuit(c)) return;
    bucket = c - '1';
  }
  if (launcher.questionOrder() == QuestionOrder::FromBeginning &&
      memberType == All && !bucket)
    start(list, memberType);
  else {
    GroupData::List newList;
    const auto bucketHasEnd{bucket && *bucket < PatternGroups.size()};
    for (auto startIncluding{!bucket.value_or(0)}; const auto& i : list) {
      if (startIncluding) {
        if (bucketHasEnd &&
            i->name().find(PatternGroups[*bucket]) != std::string::npos)
          break;
      } else if (i->name().find(PatternGroups[*bucket - 1]) !=
                 std::string::npos)
        startIncluding = true;
      if (size_t memberCount{}; startIncluding) {
        for (auto& j : i->members())
          if (includeMember(*j, memberType)) ++memberCount;
        // only include groups that have 2 or more members after applying the
        // 'include member' filter
        if (memberCount > 1) newList.push_back(i);
      }
    }
    if (launcher.questionOrder() == QuestionOrder::FromEnd)
      std::reverse(newList.begin(), newList.end());
    else if (launcher.questionOrder() == QuestionOrder::Random)
      std::shuffle(newList.begin(), newList.end(), RandomGen);
    start(newList, memberType);
  }
}

GroupType GroupQuiz::getGroupType(const GroupData::List& list) {
  const auto i{list.begin()};
  return i != list.end() ? (**i).type()
                         : throw std::domain_error{"empty group list"};
}

bool GroupQuiz::includeMember(const Kanji& k, MemberType memberType) {
  return k.hasReading() &&
         (k.is(KanjiTypes::Jouyou) || memberType && k.hasLevel() ||
             memberType > 1 && k.frequency() || memberType > 2);
}

void GroupQuiz::addPinyin(const Kanji& kanji, std::string& s) {
  static const std::string NoPinyin(PinyinWidth, ' ');
  if (kanji.pinyin()) {
    const auto p{"  (" + *kanji.pinyin() + ')'};
    // use 'displaySize' since Pinyin can contain multi-byte chars (for tones)
    s += p + std::string(PinyinWidth - displaySize(p), ' ');
  } else
    s += NoPinyin;
}

void GroupQuiz::addOtherGroupName(
    const std::string& name, std::string& s) const {
  const auto add{[this, &name, &s](const auto& map) {
    if (const auto j{map.find(name)}; j != map.end()) {
      s += _groupType == GroupType::Meaning ? 'p' : 'm';
      s += ':';
      s += std::to_string(j->second->number());
    }
  }};
  if (_groupType == GroupType::Meaning)
    add(launcher().groupData()->patternMap());
  else
    add(launcher().groupData()->meaningMap());
}

void GroupQuiz::start(const GroupData::List& list, MemberType memberType) {
  beginQuizMessage(list.size()) << _groupType << " groups\n";
  if (memberType) log() << "  " << Kanji::Legend << '\n';

  auto stopQuiz{false};
  for (; currentQuestion() < list.size() && !stopQuiz; ++currentQuestion()) {
    auto& i{list[currentQuestion()]};
    List questions, readings;
    for (auto& j : i->members())
      if (includeMember(*j, memberType)) {
        questions.emplace_back(j);
        readings.emplace_back(j);
      }
    if (isTestMode() && launcher().randomizeAnswers()) {
      std::shuffle(questions.begin(), questions.end(), RandomGen);
      std::shuffle(readings.begin(), readings.end(), RandomGen);
    }
    _answers.clear();
    auto choices{getDefaultChoices(list.size())};
    auto repeatQuestion{false}, skipGroup{false};
    do {
      beginQuestionMessage(list.size()) << *i << ", ";
      if (questions.size() == i->members().size())
        out() << questions.size();
      else
        out() << "showing " << questions.size() << " out of "
              << i->members().size();
      out() << " members\n";
      showGroup(questions, readings, choices, repeatQuestion);
      if (getAnswers(questions.size(), choices, skipGroup, stopQuiz)) {
        checkAnswers(questions, readings, i->name());
        break;
      }
      repeatQuestion = true;
    } while (!stopQuiz && !skipGroup);
  }
  if (stopQuiz) --currentQuestion();
}

void GroupQuiz::printAssignedAnswers() const {
  if (!_answers.empty()) {
    out() << "   ";
    for (size_t i{}; i < _answers.size(); ++i)
      out() << ' ' << i + 1 << "->" << _answers[i];
    out() << '\n';
  }
}

std::ostream& GroupQuiz::printAssignedAnswer(char choice) const {
  for (size_t i{}; i < _answers.size(); ++i)
    if (_answers[i] == choice)
      return out() << std::right << std::setw(2) << i + 1 << "->";
  return out() << "    ";
}

void GroupQuiz::showGroup(const List& questions, const List& readings,
    Choices& choices, bool repeatQuestion) const {
  for (size_t count{}; auto& i : questions) {
    const char choice{
        isTestMode() ? toChar(count < TotalLetters ? 'a' + count
                                                   : 'A' + count - TotalLetters)
                     : ' '};
    out() << std::right << std::setw(4) << count + 1 << ":  ";
    auto s{i->qualifiedName()};
    addPinyin(*i, s);
    if (!isTestMode()) addOtherGroupName(i->name(), s);
    out() << std::left << std::setw(wideSetw(s, GroupEntryWidth)) << s;
    printAssignedAnswer(choice)
        << choice << ":  " << readings[count]->reading();
    printMeaning(*readings[count++]);
    if (!repeatQuestion && isTestMode()) choices[choice] = {};
  }
  out() << '\n';
}

bool GroupQuiz::getAnswers(
    size_t totalQuestions, Choices& choices, bool& skipGroup, bool& stopQuiz) {
  for (size_t i{_answers.size()}; i < totalQuestions; ++i)
    if (auto refresh{false}; !getAnswer(choices, skipGroup, refresh)) {
      // set 'stopQuiz' to break out of top quiz loop if user quit in the middle
      // of providing answers
      if (!refresh && !skipGroup) stopQuiz = true;
      return false;
    }
  return true;
}

bool GroupQuiz::getAnswer(Choices& choices, bool& skipGroup, bool& refresh) {
  const static std::string ReviewMsg{"  Select"}, QuizMsg{"  Reading for: "};
  do {
    printAssignedAnswers();
    switch (const auto answer{
        get(isTestMode() ? QuizMsg + std::to_string(_answers.size() + 1)
                         : ReviewMsg,
            choices)};
            answer) {
    case RefreshOption: refresh = true; break;
    case MeaningsOption:
      refresh = true;
      toggleMeanings(choices);
      break;
    case PrevOption:
      currentQuestion() -= 2;
      skipGroup = true;
      break;
    case SkipOption: skipGroup = true; break;
    case EditOption: editAnswer(choices); break;
    default:
      if (isQuit(answer)) return false;
      _answers.emplace_back(answer);
      choices.erase(answer);
      if (_answers.size() == 1) {
        choices[EditOption] = "edit";
        choices[RefreshOption] = "refresh";
      }
      return true;
    }
  } while (!skipGroup && !refresh);
  return false;
}

void GroupQuiz::editAnswer(Choices& choices) {
  static const std::string NewReadingForEntry("    New reading for Entry: ");

  const auto entry{getAnswerToEdit()};
  choices[_answers[entry]] = {}; // put the answer back as a choice
  auto newChoices{choices};
  newChoices.erase(EditOption);
  newChoices.erase(MeaningsOption);
  newChoices.erase(RefreshOption);
  newChoices.erase(SkipOption);
  const auto answer{get(NewReadingForEntry + std::to_string(entry + 1),
      newChoices, _answers[entry], false)};
  _answers[entry] = answer;
  choices.erase(answer);
}

size_t GroupQuiz::getAnswerToEdit() const {
  static const std::string AnswerToEdit{"    Answer to edit: "};

  if (_answers.size() == 1) return 0;
  std::map<char, std::string> answersToEdit;
  for (auto k : _answers) answersToEdit[k] = {};
  const auto index{std::find(_answers.begin(), _answers.end(),
      get(AnswerToEdit, answersToEdit, {}, false))};
  assert(index != _answers.end());
  return static_cast<size_t>(std::distance(_answers.begin(), index));
}

void GroupQuiz::checkAnswers(
    const List& questions, const List& readings, const std::string& name) {
  size_t correctCount{};
  for (size_t count{}; auto i : _answers) {
    const auto answer{
        static_cast<size_t>(i <= 'z' ? i - 'a' : i - 'A' + TotalLetters)};
    // always look at reading for a match and only additionally require matching
    // meanings if '_showMeanings' is true
    if (questions[count]->reading() == readings[answer]->reading() &&
        (!showMeanings() ||
            questions[count]->meaning() == readings[answer]->meaning()))
      ++correctCount;
    ++count;
  }
  if (correctCount == _answers.size())
    correctMessage();
  else
    incorrectMessage(name) << " (got " << correctCount << " right out of "
                           << _answers.size() << ")\n";
}

} // namespace kanji_tools
