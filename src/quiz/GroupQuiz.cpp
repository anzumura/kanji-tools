#include <kanji_tools/quiz/GroupQuiz.h>
#include <kanji_tools/utils/DisplayLength.h>

#include <random>

namespace kanji_tools {

namespace {

std::random_device RandomDevice;
std::mt19937 RandomGen(RandomDevice());

const Choice::Choices
  PatternGroupChoices({{'1', "ア"}, {'2', "カ"}, {'3', "サ"}, {'4', "タ、ナ"}, {'5', "ハ、マ"}, {'6', "ヤ、ラ、ワ"}});

constexpr char DefaultPatternGroup = '1';

// Since there are over 1000 pattern groups, split them into 6 buckets based on reading. The first bucket starts
// at 'ア', the second bucket starts at 'カ' and so on (see 'PatternBucketChoices' above).
constexpr std::array PatternGroupBuckets{"：カ", "：サ", "：タ", "：ハ", "：ヤ"};

constexpr char RefreshOption = '\'', EditOption = '*';

const std::string AnswerToEdit("    Answer to edit: "), NewReadingForEntry("    New reading for Entry: ");

} // namespace

bool GroupQuiz::includeMember(const Entry& k, MemberType memberType) {
  return k->hasReading() &&
    (k->is(KanjiTypes::Jouyou) || memberType && k->hasLevel() || memberType > 1 && k->frequency() || memberType > 2);
}

GroupQuiz::GroupQuiz(const QuizLauncher& launcher, int question, bool showMeanings, const GroupData::List& list,
                     char otherGroup, MemberType memberType)
  : Quiz(launcher, question, showMeanings), _otherGroup(otherGroup) {
  int bucket = -1;
  // for 'pattern' groups, allow choosing a smaller subset based on the name reading
  if (otherGroup == 'm') {
    const char c = getChoice("Pattern name", PatternGroupChoices, DefaultPatternGroup);
    if (isQuit(c)) return;
    bucket = c - '1';
  }
  if (_launcher.questionOrder() == QuizLauncher::QuestionOrder::FromBeginning && memberType == All && bucket == -1)
    start(list, memberType);
  else {
    GroupData::List newList;
    const bool bucketHasEnd = bucket >= 0 && bucket < PatternGroupBuckets.size();
    for (bool startIncluding = bucket <= 0; const auto& i : list) {
      if (startIncluding) {
        if (bucketHasEnd && i->name().find(PatternGroupBuckets[bucket]) != std::string::npos) break;
      } else if (i->name().find(PatternGroupBuckets[bucket - 1]) != std::string::npos)
        startIncluding = true;
      if (int memberCount = 0; startIncluding) {
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

void GroupQuiz::start(const GroupData::List& list, MemberType memberType) {
  bool stopQuiz = false;
  for (bool firstTime = true; _question < list.size() && !stopQuiz; ++_question) {
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
    if (firstTime) {
      beginQuizMessage(list.size()) << i->type() << " groups\n";
      if (memberType) log() << "  " << Kanji::Legend << '\n';
      firstTime = false;
    }
    Answers answers;
    Choices choices = getDefaultChoices(list.size());
    bool repeatQuestion = false, skipGroup = false;
    do {
      beginQuestionMessage(list.size()) << *i << ", ";
      if (questions.size() == i->members().size())
        out() << questions.size();
      else
        out() << "showing " << questions.size() << " out of " << i->members().size();
      out() << " members\n";
      showGroup(questions, answers, readings, choices, repeatQuestion);
      if (getAnswers(answers, questions.size(), choices, skipGroup, stopQuiz)) {
        checkAnswers(answers, questions, readings, i->name());
        break;
      }
      repeatQuestion = true;
    } while (!stopQuiz && !skipGroup);
  }
  if (stopQuiz) --_question;
}

void GroupQuiz::addPinyin(const Entry& kanji, std::string& s) const {
  static const std::string NoPinyin(PinyinLength, ' ');
  if (kanji->pinyin()) {
    std::string p = "  (" + *kanji->pinyin() + ')';
    // need to use 'displayLength' since Pinyin can contain multi-byte chars (for the tones)
    s += p + std::string(PinyinLength - displayLength(p), ' ');
  } else
    s += NoPinyin;
}

void GroupQuiz::addOtherGroupName(const std::string& name, std::string& s) const {
  auto add = [this, &name, &s](const auto& map) {
    if (auto j = map.find(name); j != map.end()) {
      s += _otherGroup;
      s += ':';
      s += std::to_string(j->second->number());
    }
  };
  if (_otherGroup == 'm')
    add(_launcher.groupData().meaningMap());
  else
    add(_launcher.groupData().patternMap());
}

void GroupQuiz::showGroup(const List& questions, const Answers& answers, const List& readings, Choices& choices,
                          bool repeatQuestion) const {
  for (int count = 0; auto& i : questions) {
    const char choice = isTestMode() ? count < 26 ? 'a' + count : 'A' + (count - 26) : ' ';
    out() << std::right << std::setw(4) << count + 1 << ":  ";
    auto s = i->qualifiedName();
    addPinyin(i, s);
    if (!isTestMode()) addOtherGroupName(i->name(), s);
    out() << std::left << std::setw(wideSetw(s, 22)) << s;
    int j = 0;
    for (j = 0; j < answers.size(); ++j)
      if (answers[j] == choice) {
        out() << std::right << std::setw(2) << j + 1 << "->";
        break;
      }
    if (j == answers.size()) out() << "    ";
    out() << choice << ":  " << readings[count]->reading();
    printMeaning(readings[count]);
    if (!repeatQuestion && isTestMode()) choices[choice] = "";
    ++count;
  }
  out() << '\n';
}

bool GroupQuiz::getAnswers(Answers& answers, int totalQuestions, Choices& choices, bool& skipGroup, bool& stopQuiz) {
  for (int i = answers.size(); i < totalQuestions; ++i)
    if (bool refresh = false; !getAnswer(answers, choices, skipGroup, refresh)) {
      // set 'stopQuiz' to break out of top quiz loop if user quit in the middle of providing answers
      if (!refresh && !skipGroup) stopQuiz = true;
      return false;
    }
  return true;
}

bool GroupQuiz::getAnswer(Answers& answers, Choices& choices, bool& skipGroup, bool& refresh) {
  const static std::string ReviewMsg("  Select"), QuizMsg("  Reading for Entry: ");
  const std::string space = (answers.size() < 9 ? " " : "");
  do {
    if (!answers.empty()) {
      out() << "   ";
      for (int k = 0; k < answers.size(); ++k)
        out() << ' ' << k + 1 << "->" << answers[k];
      out() << '\n';
    }
    const char answer =
      getChoice(isTestMode() ? QuizMsg + space + std::to_string(answers.size() + 1) : ReviewMsg, choices);
    switch (answer) {
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
    case EditOption: editAnswer(answers, choices); break;
    default:
      if (isQuit(answer)) return false;
      answers.push_back(answer);
      choices.erase(answer);
      if (answers.size() == 1) {
        choices[EditOption] = "edit";
        choices[RefreshOption] = "refresh";
      }
      return true;
    }
  } while (!skipGroup && !refresh);
  return false;
}

void GroupQuiz::editAnswer(Answers& answers, Choices& choices) {
  auto getEntry = [this, &answers]() {
    std::map<char, std::string> answersToEdit;
    for (auto k : answers)
      answersToEdit[k] = "";
    const auto index = std::find(answers.begin(), answers.end(), getChoice(AnswerToEdit, answersToEdit, {}, false));
    assert(index != answers.end());
    return std::distance(answers.begin(), index);
  };
  const int entry = answers.size() == 1 ? 0 : getEntry();
  // put the answer back as a choice
  choices[answers[entry]] = "";
  auto newChoices = choices;
  newChoices.erase(EditOption);
  newChoices.erase(MeaningsOption);
  newChoices.erase(SkipOption);
  const char answer = getChoice(NewReadingForEntry + std::to_string(entry + 1), newChoices, answers[entry], false);
  answers[entry] = answer;
  choices.erase(answer);
}

void GroupQuiz::checkAnswers(const Answers& answers, const List& questions, const List& readings,
                             const std::string& name) {
  int count = 0;
  for (auto i : answers) {
    int index = (i <= 'z' ? i - 'a' : i - 'A' + 26);
    // Only match on readings (and meanings if '_showMeanings' is true) instead of making
    // sure the kanji is exactly the same since many kanjis have identical readings
    // especially in the 'patterns' groups (and the user has no way to distinguish).
    if (questions[count]->reading() == readings[index]->reading() &&
        (!showMeanings() || questions[count]->meaning() == readings[index]->meaning()))
      ++count;
  }
  if (count == answers.size())
    correctMessage();
  else
    incorrectMessage(name) << " (got " << count << " right out of " << answers.size() << ")\n";
}

} // namespace kanji_tools