#include <kanji_tools/quiz/GroupQuiz.h>
#include <kanji_tools/utils/DisplayLength.h>

#include <random>

namespace kanji_tools {

namespace {

std::random_device RandomDevice;
std::mt19937 RandomGen(RandomDevice());

const Choice::Choices PatternGroupChoices(
  {{'1', "ア"}, {'2', "カ"}, {'3', "サ"}, {'4', "タ、ナ"}, {'5', "ハ、マ"}, {'6', "ヤ、ラ、ワ"}});

constexpr auto DefaultPatternGroup = '1';

// Since there are over 1000 pattern groups, split them into 6 buckets based on reading. The first bucket starts
// at 'ア', the second bucket starts at 'カ' and so on (see 'PatternBucketChoices' above).
constexpr std::array PatternGroupBuckets{"：カ", "：サ", "：タ", "：ハ", "：ヤ"};

constexpr auto RefreshOption = '\'', EditOption = '*';

constexpr int TotalLetters = 'z' - 'a';

} // namespace

GroupQuiz::GroupQuiz(const QuizLauncher& launcher, int question, bool showMeanings, const GroupData::List& list,
                     MemberType memberType)
  : Quiz(launcher, question, showMeanings), _groupType(getGroupType(list)) {
  auto bucket = -1;
  // for 'pattern' groups, allow choosing a smaller subset based on the name reading
  if (_groupType == GroupType::Pattern) {
    const auto c = get("Pattern name", PatternGroupChoices, DefaultPatternGroup);
    if (isQuit(c)) return;
    bucket = c - '1';
  }
  if (_launcher.questionOrder() == QuizLauncher::QuestionOrder::FromBeginning && memberType == All && bucket == -1)
    start(list, memberType);
  else {
    GroupData::List newList;
    const auto bucketHasEnd = bucket >= 0 && bucket < PatternGroupBuckets.size();
    for (auto startIncluding = bucket <= 0; const auto& i : list) {
      if (startIncluding) {
        if (bucketHasEnd && i->name().find(PatternGroupBuckets[bucket]) != std::string::npos) break;
      } else if (i->name().find(PatternGroupBuckets[bucket - 1]) != std::string::npos)
        startIncluding = true;
      if (auto memberCount = 0; startIncluding) {
        for (auto& j : i->members())
          if (includeMember(j, memberType)) ++memberCount;
        // only include groups that have 2 or more members after applying the 'include member' filter
        if (memberCount > 1) newList.push_back(i);
      }
    }
    if (_launcher.questionOrder() == QuizLauncher::QuestionOrder::FromEnd)
      std::reverse(newList.begin(), newList.end());
    else if (_launcher.questionOrder() == QuizLauncher::QuestionOrder::Random)
      std::shuffle(newList.begin(), newList.end(), RandomGen);
    start(newList, memberType);
  }
}

GroupType GroupQuiz::getGroupType(const GroupData::List& list) {
  auto i = list.begin();
  return i != list.end() ? (**i).type() : throw std::domain_error("empty group list");
}

bool GroupQuiz::includeMember(const Entry& k, MemberType memberType) {
  return k->hasReading() &&
    (k->is(KanjiTypes::Jouyou) || memberType && k->hasLevel() || memberType > 1 && k->frequency() || memberType > 2);
}

void GroupQuiz::addPinyin(const Entry& kanji, std::string& s) {
  static const std::string NoPinyin(PinyinWidth, ' ');
  if (kanji->pinyin()) {
    std::string p = "  (" + *kanji->pinyin() + ')';
    // need to use 'displayLength' since Pinyin can contain multi-byte chars (for the tones)
    s += p + std::string(PinyinWidth - displayLength(p), ' ');
  } else
    s += NoPinyin;
}

void GroupQuiz::addOtherGroupName(const std::string& name, std::string& s) const {
  auto add = [this, &name, &s](const auto& map) {
    if (auto j = map.find(name); j != map.end()) {
      s += _groupType == GroupType::Meaning ? 'p' : 'm';
      s += ':';
      s += std::to_string(j->second->number());
    }
  };
  if (_groupType == GroupType::Meaning)
    add(_launcher.groupData().patternMap());
  else
    add(_launcher.groupData().meaningMap());
}

void GroupQuiz::start(const GroupData::List& list, MemberType memberType) {
  beginQuizMessage(list.size()) << _groupType << " groups\n";
  if (memberType) log() << "  " << Kanji::Legend << '\n';

  auto stopQuiz = false;
  for (; _question < list.size() && !stopQuiz; ++_question) {
    auto& i = list[_question];
    List questions, readings;
    for (auto& j : i->members())
      if (includeMember(j, memberType)) {
        questions.push_back(j);
        readings.push_back(j);
      }
    if (isTestMode()) {
      std::shuffle(questions.begin(), questions.end(), RandomGen);
      std::shuffle(readings.begin(), readings.end(), RandomGen);
    }
    _answers.clear();
    Choices choices = getDefaultChoices(list.size());
    auto repeatQuestion = false, skipGroup = false;
    do {
      beginQuestionMessage(list.size()) << *i << ", ";
      if (questions.size() == i->members().size())
        out() << questions.size();
      else
        out() << "showing " << questions.size() << " out of " << i->members().size();
      out() << " members\n";
      showGroup(questions, readings, choices, repeatQuestion);
      if (getAnswers(questions.size(), choices, skipGroup, stopQuiz)) {
        checkAnswers(questions, readings, i->name());
        break;
      }
      repeatQuestion = true;
    } while (!stopQuiz && !skipGroup);
  }
  if (stopQuiz) --_question;
}

void GroupQuiz::printAssignedAnswers() const {
  if (!_answers.empty()) {
    out() << "   ";
    for (auto i = 0; i < _answers.size(); ++i) out() << ' ' << i + 1 << "->" << _answers[i];
    out() << '\n';
  }
}

std::ostream& GroupQuiz::printAssignedAnswer(char choice) const {
  for (auto i = 0; i < _answers.size(); ++i)
    if (_answers[i] == choice) return out() << std::right << std::setw(2) << i + 1 << "->";
  return out() << "    ";
}

void GroupQuiz::showGroup(const List& questions, const List& readings, Choices& choices, bool repeatQuestion) const {
  for (auto count = 0; auto& i : questions) {
    const auto choice = isTestMode() ? count < TotalLetters ? 'a' + count : 'A' + (count - TotalLetters) : ' ';
    out() << std::right << std::setw(4) << count + 1 << ":  ";
    auto s = i->qualifiedName();
    addPinyin(i, s);
    if (!isTestMode()) addOtherGroupName(i->name(), s);
    out() << std::left << std::setw(wideSetw(s, GroupEntryWidth)) << s;
    printAssignedAnswer(choice) << choice << ":  " << readings[count]->reading();
    printMeaning(readings[count++]);
    if (!repeatQuestion && isTestMode()) choices[choice] = "";
  }
  out() << '\n';
}

bool GroupQuiz::getAnswers(int totalQuestions, Choices& choices, bool& skipGroup, bool& stopQuiz) {
  for (auto i = _answers.size(); i < totalQuestions; ++i)
    if (auto refresh = false; !getAnswer(choices, skipGroup, refresh)) {
      // set 'stopQuiz' to break out of top quiz loop if user quit in the middle of providing answers
      if (!refresh && !skipGroup) stopQuiz = true;
      return false;
    }
  return true;
}

bool GroupQuiz::getAnswer(Choices& choices, bool& skipGroup, bool& refresh) {
  const static std::string ReviewMsg("  Select"), QuizMsg("  Reading for: ");
  do {
    printAssignedAnswers();
    switch (const auto answer = get(isTestMode() ? QuizMsg + std::to_string(_answers.size() + 1) : ReviewMsg, choices);
            answer) {
    case RefreshOption: refresh = true; break;
    case MeaningsOption:
      refresh = true;
      toggleMeanings(choices);
      break;
    case PrevOption:
      _question -= 2;
      skipGroup = true;
      break;
    case SkipOption: skipGroup = true; break;
    case EditOption: editAnswer(choices); break;
    default:
      if (isQuit(answer)) return false;
      _answers.push_back(answer);
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

  const auto entry = getAnswerToEdit();
  choices[_answers[entry]] = ""; // put the answer back as a choice
  auto newChoices = choices;
  newChoices.erase(EditOption);
  newChoices.erase(MeaningsOption);
  newChoices.erase(RefreshOption);
  newChoices.erase(SkipOption);
  const auto answer = get(NewReadingForEntry + std::to_string(entry + 1), newChoices, _answers[entry], false);
  _answers[entry] = answer;
  choices.erase(answer);
}

int GroupQuiz::getAnswerToEdit() const {
  static const std::string AnswerToEdit("    Answer to edit: ");

  if (_answers.size() == 1) return 0;
  std::map<char, std::string> answersToEdit;
  for (auto k : _answers) answersToEdit[k] = "";
  const auto index = std::find(_answers.begin(), _answers.end(), get(AnswerToEdit, answersToEdit, {}, false));
  assert(index != _answers.end());
  return std::distance(_answers.begin(), index);
}

void GroupQuiz::checkAnswers(const List& questions, const List& readings, const std::string& name) {
  auto count = 0;
  for (auto i : _answers) {
    auto index = (i <= 'z' ? i - 'a' : i - 'A' + TotalLetters);
    // Only match on readings (and meanings if '_showMeanings' is true) instead of making
    // sure the kanji is exactly the same since many kanjis have identical readings
    // especially in the 'patterns' groups (and the user has no way to distinguish).
    if (questions[count]->reading() == readings[index]->reading() &&
        (!showMeanings() || questions[count]->meaning() == readings[index]->meaning()))
      ++count;
  }
  if (count == _answers.size())
    correctMessage();
  else
    incorrectMessage(name) << " (got " << count << " right out of " << _answers.size() << ")\n";
}

} // namespace kanji_tools
