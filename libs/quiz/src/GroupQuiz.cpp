#include <kt_kana/DisplaySize.h>
#include <kt_quiz/GroupQuiz.h>

#include <algorithm>
#include <optional>
#include <random>

namespace kanji_tools {

namespace {

std::mt19937 RandomGen(std::random_device{}());

const Choice::Choices PatternGroupChoices{{'1', "ア"}, {'2', "カ"}, {'3', "サ"},
    {'4', "タ、ナ"}, {'5', "ハ、マ"}, {'6', "ヤ、ラ、ワ"}};

// Since there are over 1000 pattern groups, split them into 6 buckets based on
// reading. The first bucket starts at 'ア', the second bucket starts at 'カ'
// and so on (see 'PatternBucketChoices' above). LCOV_EXCL_START
constexpr std::array PatternGroups{"：カ", "：サ", "：タ", "：ハ", "：ヤ"};
// LCOV_EXCL_STOP
constexpr auto RefreshOption{'\''}, EditOption{'*'};

constexpr auto TotalLetters{'z' - 'a' + 1};

} // namespace

GroupQuiz::GroupQuiz(const QuizLauncher& launcher, Question question,
    bool showMeanings, const GroupData::List& list, MemberType memberType)
    : Quiz{launcher, question, showMeanings}, _groupType{getGroupType(list)},
      _memberType{memberType} {
  if (_groupType == GroupType::Pattern) {
    // for 'pattern' groups, a subset (based on 'reading') needs to be choosen
    const auto c{get("Pattern name", PatternGroupChoices, '1')};
    if (isQuit(c)) return;
    start(prepareList(list, c - '1'));
  } else if (launcher.questionOrder() == QuestionOrder::FromBeginning &&
             _memberType == MemberType::All)
    start(list); // can use the original list in this case
  else
    start(prepareList(list));
}

GroupType GroupQuiz::getGroupType(const GroupData::List& list) {
  const auto i{list.begin()};
  return i != list.end() ? (**i).type() : throw DomainError{"empty group list"};
}

void GroupQuiz::addPinyin(const Kanji& kanji, String& s) {
  static const String NoPinyin(PinyinWidth, ' ');
  if (kanji.pinyin()) {
    const auto p{"  (" + kanji.pinyin().name() + ')'};
    // use 'displaySize' since Pinyin can contain multi-byte chars (for tones)
    s += p + String(PinyinWidth - displaySize(p), ' ');
  } else
    s += NoPinyin;
}

GroupData::List GroupQuiz::prepareList(
    const GroupData::List& list, Bucket bucket) const { // LCOV_EXCL_LINE
  GroupData::List result;
  const auto bucketHasEnd{bucket && *bucket < PatternGroups.size()};
  for (auto startIncluding{!bucket.value_or(0)}; const auto& i : list) {
    // NOLINTBEGIN(bugprone-unchecked-optional-access)
    if (startIncluding) {
      if (bucketHasEnd &&
          i->name().find(PatternGroups[*bucket]) != String::npos)
        break;
    } else if (i->name().find(PatternGroups[*bucket - 1]) != String::npos)
      startIncluding = true;
    // NOLINTEND(bugprone-unchecked-optional-access)
    if (size_t memberCount{}; startIncluding) {
      for (auto& j : i->members())
        if (includeMember(*j)) ++memberCount;
      // only include groups that have 2 or more members after applying the
      // 'include member' filter
      if (memberCount > 1) result.emplace_back(i);
    }
  }
  if (launcher().questionOrder() == QuestionOrder::FromEnd)
    std::reverse(result.begin(), result.end());
  else if (launcher().questionOrder() == QuestionOrder::Random)
    std::shuffle(result.begin(), result.end(), RandomGen);
  return result;
}

bool GroupQuiz::includeMember(const Kanji& k) const {
  using enum MemberType;
  return k.hasReading() && (k.is(KanjiTypes::Jouyou) ||
                               (_memberType > Jouyou && k.hasLevel() ||
                                   (_memberType > JLPT && k.frequency() ||
                                       _memberType > Frequency)));
}

void GroupQuiz::addOtherGroupName(const String& name, String& s) const {
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

void GroupQuiz::start(const GroupData::List& list) {
  beginQuizMessage(list.size()) << _groupType << " groups\n";
  if (_memberType > MemberType::Jouyou) log() << "  " << Kanji::Legend << '\n';

  auto stopQuiz{false};
  for (; currentQuestion() < list.size() && !stopQuiz; ++currentQuestion()) {
    auto& i{list[currentQuestion()]};
    List questions, readings;
    for (auto& j : i->members())
      if (includeMember(*j)) {
        questions.emplace_back(j);
        readings.emplace_back(j);
      }
    if (isQuizMode() && launcher().randomizeAnswers()) {
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
        isQuizMode() ? toChar(count < TotalLetters ? 'a' + count
                                                   : 'A' + count - TotalLetters)
                     : ' '};
    out() << std::right << std::setw(4) << count + 1 << ":  ";
    auto s{i->qualifiedName()};
    addPinyin(*i, s);
    if (!isQuizMode()) addOtherGroupName(i->name(), s);
    out() << std::left << std::setw(wideSetw(s, GroupEntryWidth)) << s;
    printAssignedAnswer(choice)
        << choice << ":  " << readings[count]->reading();
    printMeaning(*readings[count++]);
    if (!repeatQuestion && isQuizMode()) choices[choice] = {};
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
  const static String ReviewMsg{"  Select"}, QuizMsg{"  Reading for: "};
  const auto msg{
      isQuizMode() ? QuizMsg + std::to_string(_answers.size() + 1) : ReviewMsg};
  do {
    printAssignedAnswers();
    switch (const auto answer{get(msg, choices)}; answer) {
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
  } while (!skipGroup && !refresh); // only 'edit' case needs to loop again
  return false;
}

void GroupQuiz::editAnswer(Choices& choices) {
  static const String NewReadingForEntry("    New reading for Entry: ");

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
  static const String AnswerToEdit{"    Answer to edit: "};

  if (_answers.size() == 1) return 0;
  std::map<char, String> answersToEdit;
  for (auto k : _answers) answersToEdit[k] = {};
  const auto index{std::find(_answers.begin(), _answers.end(),
      get(AnswerToEdit, answersToEdit, {}, false))};
  assert(index != _answers.end());
  return static_cast<size_t>(std::distance(_answers.begin(), index));
}

void GroupQuiz::checkAnswers(
    const List& questions, const List& readings, const String& kanjiName) {
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
    incorrectMessage(kanjiName) << " (got " << correctCount << " right out of "
                                << _answers.size() << ")\n";
}

} // namespace kanji_tools
