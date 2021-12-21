#include <kanji_tools/quiz/Group.h>
#include <kanji_tools/quiz/Quiz.h>
#include <kanji_tools/utils/DisplayLength.h>

#include <random>

namespace kanji_tools {

namespace {

std::random_device RandomDevice;
std::mt19937 RandomGen(RandomDevice());

// Below are some options used in for quiz questions. These are all ascii symbols that come
// before letters and numbers so that 'Choice::get' method displays them at the beginning
// of the list (assuming the other choices are just letters and/or numbers).
constexpr char RefreshOption = '\'';
constexpr char EditOption = '*';
constexpr char MeaningsOption = '-';
constexpr char PrevOption = ',';
constexpr char SkipOption = '.';
constexpr char QuitOption = '/';

constexpr auto ShowMeanings = "show meanings";
constexpr auto HideMeanings = "hide meanings";

constexpr auto HelpMessage = "\
kanjiQuiz [-hs] [-f[1-5] | -g[1-6s] | -k[1-9a-c] | -l[1-5] -m[1-4] | -p[1-4]]\n\
          [-r[num] | -t[num]] [kanji]\n\
    -h   show this help message for command-line options\n\
    -s   show English meanings by default (can be toggled on/off later)\n\n\
  The following options allow choosing the quiz/review type optionally followed\n\
  by question list type (grade, level, etc.) instead of being prompted:\n\
    -f   'frequency' (optional frequency group '1-5')\n\
    -g   'grade' (optional grade '1-6', 's' = Secondary School)\n\
    -k   'kyu' (optional Kentei Kyu '1-9', 'a' = 10, 'b' = 準１級, 'c' = 準２級)\n\
    -l   'level' (optional JLPT level number '1-5')\n\
    -m   'meaning' (optional kanji type '1-4')\n\
    -p   'pattern' (optional kanji type '1-4')\n\n\
  The following options can be followed by a 'num' to specify where to start in\n\
  the question list (use negative to start from the end or 0 for random order).\n\
    -r   review mode\n\
    -t   test mode\n\n\
  kanji  show details for a kanji instead of starting a review or test\n\n\
Examples:\n\
  kanjiQuiz -f        # start 'frequency' quiz (prompts for 'bucket' number)\n\
  kanjiQuiz -r40 -l1  # start 'JLPT N1' review beginning at the 40th entry\n\n\
Note: 'kanji' can be UTF-8, frequency (between 1 and 2501), 'm' followed by\n\
Morohashi ID (index in Dai Kan-Wa Jiten), 'n' followed by Classic Nelson ID\n\
or 'u' followed by Unicode. For example, theses all produce the same output:\n\
  kanjiQuiz 奉\n\
  kanjiQuiz 1624\n\
  kanjiQuiz m5894\n\
  kanjiQuiz n212\n\
  kanjiQuiz u5949\n\n";

static const Choice::Choices ProgramModeChoices({{'r', "review"}, {'t', "test"}}),
  QuizTypeChoices({{'f', "freq"}, {'g', "grade"}, {'k', "kyu"}, {'l', "JLPT"}, {'m', "meaning"}, {'p', "pattern"}}),
  FrequencyChoices({{'1', "1-500"}, {'2', "501-1000"}, {'3', "1001-1500"}, {'4', "1501-2000"}, {'5', "2001-2501"}}),
  GradeChoices({{'s', "Secondary School"}}), KyuChoices({{'a', "10"}, {'b', "準１級"}, {'c', "準２級"}}),
  LevelChoices({{'1', "N1"}, {'2', "N2"}, {'3', "N3"}, {'4', "N4"}, {'5', "N5"}}),
  ListOrderChoices({{'b', "from beginning"}, {'e', "from end"}, {'r', "random"}}),
  ListQuizStyleChoices({{'k', "kanji to reading"}, {'r', "reading to kanji"}}),
  GroupKanjiChoices({{'1', "Jōyō"}, {'2', "1+JLPT"}, {'3', "2+Freq."}, {'4', "all"}}),
  PatternGroupChoices({{'1', "ア"}, {'2', "カ"}, {'3', "サ"}, {'4', "タ、ナ"}, {'5', "ハ、マ"}, {'6', "ヤ、ラ、ワ"}});
constexpr char GradeStart = '1', GradeEnd = '6', KyuStart = '1', KyuEnd = '9';

// Default options are offered for some of the above 'Choices' (when prompting the user for input):
constexpr char DefaultProgramMode = 't', DefaultQuizType = 'g', DefaultGrade = '6', DefaultKyu = '2',
               DefaultQuestionOrder = 'r', DefaultListQuizAnswers = '4', DefaultListQuizStyle = 'k',
               DefaultGroupKanji = '2', DefaultPatternGroup = '1';

// Since there are over 1000 pattern groups, split them into 6 buckets based on reading. The first bucket starts
// at 'ア', the second bucket starts at 'カ' and so on (see 'PatternBucketChoices' above).
constexpr std::array PatternGroupBuckets{"：カ", "：サ", "：タ", "：ハ", "：ヤ"};

} // namespace

Quiz::Quiz(int argc, const char** argv, DataPtr data, std::istream* in)
  : _programMode(ProgramMode::NotAssigned), _questionOrder(QuestionOrder::NotAssigned), _question(0), _score(0),
    _showMeanings(false), _choice(data->out(), in), _groupData(data), _jukugoData(*data) {
  OptChar quizType, questionList;
  // checkQuizType is called to check f, g, l, k, m and p args (so ok to assume length is at least 2)
  auto checkQuizType = [&quizType, &questionList](const auto& arg, auto& choices, OptChar start = std::nullopt,
                                                  OptChar end = std::nullopt) {
    if (quizType) Data::usage("only one quiz type can be specified, use -h for help");
    quizType = arg[1];
    if (arg.length() > 2) {
      if (char c = arg[2]; arg.length() == 3 && (choices.contains(c) || (start && *start <= c && end.value_or(c) >= c)))
        questionList = c;
      else
        Data::usage("invalid format for " + arg.substr(0, 2) + ", use -h for help");
    }
  };

  bool endOptions = false;
  for (int i = Data::nextArg(argc, argv); i < argc; i = Data::nextArg(argc, argv, i))
    if (std::string arg = argv[i]; !endOptions && arg.starts_with("-") && arg.length() > 1) {
      if (arg == "-h") {
        out() << HelpMessage;
        return;
      }
      if (arg == "--")
        endOptions = true;
      else if (arg == "-s")
        _showMeanings = true;
      else
        switch (arg[1]) {
        case 'r': // intentional fallthrough
        case 't': parseProgramModeArg(arg); break;
        case 'f': checkQuizType(arg, FrequencyChoices); break;
        case 'g': checkQuizType(arg, GradeChoices, GradeStart, GradeEnd); break;
        case 'k': checkQuizType(arg, KyuChoices, KyuStart, KyuEnd); break;
        case 'l': checkQuizType(arg, LevelChoices); break;
        case 'm': // intentional fallthrough
        case 'p': checkQuizType(arg, GroupKanjiChoices); break;
        default: Data::usage("illegal option '" + arg + "' use -h for help");
        }
    } else {
      // show details for a 'kanji' (instead of running a test or review)
      _showMeanings = true;
      parseKanjiArg(arg);
      return;
    }
  if (!data->debug() && !in) start(quizType, questionList);
}

void Quiz::start(OptChar quizType, OptChar questionList) {
  _choice.setQuit(QuitOption);
  char c;
  if (_programMode == ProgramMode::NotAssigned) {
    c = _choice.get("Mode", ProgramModeChoices, DefaultProgramMode);
    if (c == QuitOption) return;
    _programMode = c == 'r' ? ProgramMode::Review : ProgramMode::Test;
  }
  if (!getQuestionOrder()) return;
  // can replace this with 'or_else' when C++23 is available
  c = quizType ? *quizType : _choice.get("Type", QuizTypeChoices, DefaultQuizType);
  switch (c) {
  case 'f':
    c = questionList ? *questionList : _choice.get("Choose list", FrequencyChoices);
    if (c == QuitOption) return;
    // suppress printing 'Freq' since this would work against showing the list in a random order.
    prepareListQuiz(data().frequencyList(c - '1'), Kanji::AllFields ^ Kanji::FreqField);
    break;
  case 'g':
    c = questionList ? *questionList : _choice.get("Choose grade", GradeStart, GradeEnd, GradeChoices, DefaultGrade);
    if (c == QuitOption) return;
    // suppress printing 'Grade' since it's the same for every kanji in the list
    prepareListQuiz(data().gradeList(AllKanjiGrades[c == 's' ? 6 : c - '1']), Kanji::AllFields ^ Kanji::GradeField);
    break;
  case 'k':
    c = questionList ? *questionList : _choice.get("Choose kyu", KyuStart, KyuEnd, KyuChoices, DefaultKyu);
    if (c == QuitOption) return;
    // suppress printing 'Kyu' since it's the same for every kanji in the list
    prepareListQuiz(data().kyuList(AllKenteiKyus[c == 'a'     ? 0
                                                   : c == 'c' ? 8
                                                   : c == '2' ? 9
                                                   : c == 'b' ? 10
                                                   : c == '1' ? 11
                                                              : 7 - (c - '3')]),
                    Kanji::AllFields ^ Kanji::KyuField);
    break;
  case 'l':
    c = questionList ? *questionList : _choice.get("Choose level", LevelChoices);
    if (c == QuitOption) return;
    // suppress printing 'Level' since it's the same for every kanji in the list
    prepareListQuiz(data().levelList(AllJlptLevels[4 - (c - '1')]), Kanji::AllFields ^ Kanji::LevelField);
    break;
  case 'm': prepareGroupQuiz(_groupData.meaningGroups(), _groupData.patternMap(), 'p', questionList); break;
  case 'p': prepareGroupQuiz(_groupData.patternGroups(), _groupData.meaningMap(), 'm', questionList); break;
  }
  if (isTestMode()) finalScore();
  reset();
}

void Quiz::parseProgramModeArg(const std::string& arg) {
  if (_programMode != ProgramMode::NotAssigned)
    Data::usage("only one mode (-r or -t) can be specified, use -h for help");
  _programMode = arg[1] == 'r' ? ProgramMode::Review : ProgramMode::Test;
  if (arg.length() > 2) {
    if (arg.length() == 3 && arg[2] == '0')
      _questionOrder = QuestionOrder::Random;
    else {
      int offset = 2;
      if (arg[2] == '-') {
        _questionOrder = QuestionOrder::FromEnd;
        offset = 3;
      } else {
        _questionOrder = QuestionOrder::FromBeginning;
        if (arg[2] == '+') offset = 3;
      }
      auto numArg = arg.substr(offset);
      if (!std::all_of(numArg.begin(), numArg.end(), ::isdigit))
        Data::usage("invalid format for " + arg.substr(0, 2) + ", use -h for help");
      _question = std::stoi(numArg);
    }
  }
}

void Quiz::parseKanjiArg(const std::string& arg) const {
  if (std::all_of(arg.begin(), arg.end(), ::isdigit)) {
    auto kanji = _groupData.data().findKanjiByFrequency(std::stoi(arg));
    if (!kanji) Data::usage("invalid frequency '" + arg + "'");
    printDetails((**kanji).name());
  } else if (arg.starts_with("m")) {
    auto id = arg.substr(1);
    // a valid Morohashi ID should be numeric followed by an optional 'P'
    if (std::string nonPrimeIndex = id.ends_with("P") ? id.substr(0, id.length() - 1) : id;
        id.empty() || !std::all_of(nonPrimeIndex.begin(), nonPrimeIndex.end(), ::isdigit))
      Data::usage("invalid Morohashi ID '" + id + "'");
    printDetails(_groupData.data().findKanjisByMorohashiId(id), "Morohashi", id);
  } else if (arg.starts_with("n")) {
    auto id = arg.substr(1);
    if (id.empty() || !std::all_of(id.begin(), id.end(), ::isdigit)) Data::usage("invalid Nelson ID '" + id + "'");
    printDetails(_groupData.data().findKanjisByNelsonId(std::stoi(id)), "Nelson", id);
  } else if (arg.starts_with("u")) {
    auto id = arg.substr(1);
    // must be a 4 or 5 digit hex value (and if 5 digits, then the first digit must be a 1 or 2)
    if (id.length() < 4 || id.length() > 5 || (id.length() == 5 && id[0] != '1' && id[0] != '2') ||
        !std::all_of(id.begin(), id.end(), ::ishexnumber))
      Data::usage("invalid Unicode value '" + id + "'");
    printDetails(toUtf8(std::strtol(id.c_str(), nullptr, 16)));
  } else if (isKanji(arg))
    printDetails(arg);
  else
    Data::usage("unrecognized 'kanji' value '" + arg + "' use -h for help");
}

void Quiz::printDetails(const Data::List& list, const std::string& name, const std::string& arg) const {
  if (list.size() != 1) {
    if (list.size() > 1) {
      printLegend();
      out() << '\n';
    }
    out() << "Found " << list.size() << " matches for " << name << " ID " << arg << (list.size() ? ":\n\n" : "\n");
  }
  for (auto& kanji : list)
    printDetails(kanji->name(), list.size() == 1);
}

void Quiz::printDetails(const std::string& arg, bool showLegend) const {
  if (showLegend) {
    printLegend();
    out() << '\n';
  }
  out() << "Showing details for " << arg << " [" << toUnicode(arg) << "]";
  if (auto ucd = data().ucd().find(arg); ucd) {
    out() << ", Block " << ucd->block() << ", Version " << ucd->version();
    if (auto k = data().findKanjiByName(arg); k) {
      printExtraTypeInfo(*k);
      out() << '\n' << (**k).info();
      printMeaning(*k, true);
      printReviewDetails(*k);
    } else // should never happen since all kanji in ucd.txt should be loaded
      out() << " --- Kanji not loaded'\n";
  } else // can happen for rare kanji that have aren't supported (see parseUcdAllFlat.sh for details)
    out() << " --- Not found in 'ucd.txt'\n";
}

// Helper functions for both List and Group quizzes

bool Quiz::getQuestionOrder() {
  if (_questionOrder == QuestionOrder::NotAssigned)
    switch (_choice.get("List order", ListOrderChoices, DefaultQuestionOrder)) {
    case 'b': _questionOrder = QuestionOrder::FromBeginning; break;
    case 'e': _questionOrder = QuestionOrder::FromEnd; break;
    case 'r': _questionOrder = QuestionOrder::Random; break;
    default: return false;
    }
  return true;
}

void Quiz::reset() {
  _programMode = ProgramMode::NotAssigned;
  _questionOrder = QuestionOrder::NotAssigned;
  _question = 0;
  _score = 0;
  _mistakes.clear();
}

std::ostream& Quiz::beginQuizMessage(int totalQuestions) {
  // _question can be set to non-zero from command line -r or -t options, report an error if it's out
  // of range, otherwise subtract one since 'question' list index starts at zero.
  if (_question) {
    if (_question > totalQuestions)
      Data::usage("entry num '" + std::to_string(_question) +
                  "' is larger than total questions: " + std::to_string(totalQuestions));
    --_question;
  }
  return log(true) << "Starting " << (isTestMode() ? "quiz" : "review") << " for " << totalQuestions << ' ';
}

std::ostream& Quiz::beginQuestionMessage(int totalQuestions) const {
  return out() << (isTestMode() ? "\nQuestion " : "\n") << _question + 1 << '/' << totalQuestions << ":  ";
}

void Quiz::finalScore() const {
  out() << "\nFinal score: " << _score << '/' << _question;
  if (!_question)
    out() << '\n';
  else if (_score == _question)
    out() << " - Perfect!\n";
  else {
    if (int skipped = _question - _score - _mistakes.size(); skipped) out() << ", skipped: " << skipped;
    if (!_mistakes.empty()) {
      out() << " - mistakes:";
      for (auto& i : _mistakes)
        out() << ' ' << i;
    }
    out() << '\n';
  }
}

Choice::Choices Quiz::getDefaultChoices(int totalQuestions) const {
  Choice::Choices c = {{MeaningsOption, _showMeanings ? HideMeanings : ShowMeanings},
                       {SkipOption,
                        _question + 1 == totalQuestions ? "finish"
                          : !isTestMode()               ? "next"
                                                        : "skip"},
                       {QuitOption, "quit"}};
  if (!isTestMode() && _question) c[PrevOption] = "prev";
  return c;
}

void Quiz::toggleMeanings(Choices& choices) {
  _showMeanings = !_showMeanings;
  choices[MeaningsOption] = _showMeanings ? HideMeanings : ShowMeanings;
}

void Quiz::printMeaning(const Entry& k, bool useNewLine) const {
  if (_showMeanings && k->hasMeaning()) out() << (useNewLine ? "\n    Meaning: " : " : ") << k->meaning();
  out() << '\n';
}

void Quiz::printLegend(int infoFields) const {
  std::string fields;
  if (infoFields & Kanji::LevelField) fields += " N[1-5]=JLPT Level";
  if (infoFields & Kanji::KyuField) {
    if (!fields.empty()) fields += ',';
    fields += " K[1-10]=Kentei Kyu";
  }
  if (infoFields & Kanji::GradeField) {
    if (!fields.empty()) fields += ',';
    fields += " G[1-6]=Grade (S=Secondary School)";
  }
  log() << "Legend:\nFields:" << fields << "\nSuffix: " << Kanji::Legend << '\n';
}

void Quiz::printExtraTypeInfo(const Entry& k) const {
  out() << ", " << k->type();
  if (auto i = k->extraTypeInfo(); i) out() << " (" << *i << ')';
}

void Quiz::printReviewDetails(const Entry& kanji) const {
  out() << "    Reading: " << kanji->reading() << '\n';
  // Similar Kanji
  if (auto i = _groupData.patternMap().find(kanji->name());
      i != _groupData.patternMap().end() && i->second->patternType() != Group::PatternType::Reading) {
    out() << "    Similar:";
    Data::List sorted(i->second->members());
    std::sort(sorted.begin(), sorted.end(), Data::orderByQualifiedName);
    for (auto& j : sorted)
      if (j != kanji) out() << ' ' << j->qualifiedName();
    out() << '\n';
  }
  // Morohashi and Nelson IDs
  if (kanji->morohashiId()) out() << "  Morohashi: " << *kanji->morohashiId() << '\n';
  if (kanji->hasNelsonIds()) {
    out() << (kanji->nelsonIds().size() == 1 ? "  Nelson ID:" : " Nelson IDs:");
    for (auto& i : kanji->nelsonIds())
      out() << ' ' << i;
    out() << '\n';
  }
  // Categories
  if (auto i = _groupData.meaningMap().equal_range(kanji->name()); i.first != i.second) {
    auto j = i.first;
    out() << (++j == i.second ? "   Category: " : " Categories: ");
    for (j = i.first; j != i.second; ++j) {
      if (j != i.first) out() << ", ";
      out() << '[' << j->second->name() << ']';
    }
    out() << '\n';
  }
  // Jukugo Lists
  static const std::string jukugo(" Jukugo"), sameGrade("Same Grade Jukugo"), otherGrade("Other Grade Jukugo");
  if (auto& list = _jukugoData.find(kanji->name()); !list.empty()) {
    // For kanji with a 'Grade' (so all Jouyou kanji) split Jukugo into two lists, one for the same
    // grade of the given kanji and one for other grades. For example, 一生（いっしょう） is a grade 1
    // Jukugo for '一', but 一縷（いちる） is a secondary school Jukugo (which also contains '一').
    if (JukugoData::List same, other; kanji->hasGrade() && list.size() > JukugoPerLine) {
      for (auto& i : list)
        (kanji->grade() == i->grade() ? same : other).push_back(i);
      if (other.empty())
        printJukugoList(jukugo, list);
      else {
        printJukugoList(sameGrade, same);
        printJukugoList(otherGrade, other);
      }
    } else
      printJukugoList(jukugo, list);
  }
  out() << '\n';
}

void Quiz::printJukugoList(const std::string& name, const JukugoData::List& list) const {
  out() << "    " << name << ':';
  if (list.size() <= JukugoPerLine)
    for (auto& i : list)
      out() << ' ' << i->nameAndReading();
  else {
    out() << ' ' << list.size();
    std::array<int, JukugoPerLine> colWidths;
    colWidths.fill(0);
    // make each column wide enough to hold the longest entry plus 2 spaces (upto MaxJukugoLength)
    for (int i = 0; i < list.size(); ++i)
      if (colWidths[i % JukugoPerLine] < MaxJukugoLength) {
        const int length = displayLength(list[i]->nameAndReading()) + 2;
        if (length > colWidths[i % JukugoPerLine])
          colWidths[i % JukugoPerLine] = length < MaxJukugoLength ? length : MaxJukugoLength;
      }
    for (int i = 0; i < list.size(); ++i) {
      if (i % JukugoPerLine == 0) out() << '\n';
      auto s = list[i]->nameAndReading();
      out() << std::left << std::setw(wideSetw(s, colWidths[i % JukugoPerLine])) << s;
    }
  }
  out() << '\n';
}

// List Based Quiz

void Quiz::prepareListQuiz(const List& list, int infoFields) {
  int numberOfChoicesPerQuestion = 1;
  char quizStyle = DefaultListQuizStyle;
  if (Choices choices; isTestMode()) {
    // in quiz mode, numberOfChoicesPerQuestion should be a value from 2 to 9
    for (int i = 2; i < 10; ++i)
      choices['0' + i] = "";
    const char c = _choice.get("Number of choices", choices, DefaultListQuizAnswers);
    if (c == QuitOption) return;
    numberOfChoicesPerQuestion = c - '0';
    quizStyle = _choice.get("Quiz style", ListQuizStyleChoices, quizStyle);
    if (quizStyle == QuitOption) return;
  }

  List questions;
  for (auto& i : list)
    if (i->hasReading()) questions.push_back(i);
  if (_questionOrder == QuestionOrder::FromEnd)
    std::reverse(questions.begin(), questions.end());
  else if (_questionOrder == QuestionOrder::Random)
    std::shuffle(questions.begin(), questions.end(), RandomGen);

  beginQuizMessage(questions.size()) << "kanji";
  if (questions.size() < list.size())
    out() << " (original list had " << list.size() << ", but not all entries have readings)";
  out() << "\n>>>\n";

  if (quizStyle == 'k') printLegend(infoFields);
  listQuiz(questions, infoFields, numberOfChoicesPerQuestion, quizStyle);
}

void Quiz::listQuiz(const List& questions, int infoFields, int numberOfChoicesPerQuestion, char quizStyle) {
  static const std::string reviewPrompt("  Select"), quizPrompt("  Select correct ");
  const std::string prompt(isTestMode() ? quizPrompt + (quizStyle == 'k' ? "reading" : "kanji") : reviewPrompt);

  std::uniform_int_distribution<> randomReading(0, questions.size() - 1);
  std::uniform_int_distribution<> randomCorrect(1, numberOfChoicesPerQuestion);

  bool stopQuiz = false;
  for (Choices choices; !stopQuiz && _question < questions.size(); ++_question) {
    const int correctChoice = randomCorrect(RandomGen);
    const Data::Entry& i = questions[_question];
    // 'sameReading' set is used to prevent more than one choice having the exact same reading
    DataFile::Set sameReading = {i->reading()};
    std::map<int, int> answers = {{correctChoice, _question}};
    for (int j = 1; j <= numberOfChoicesPerQuestion; ++j)
      if (j != correctChoice) do {
          if (int choice = randomReading(RandomGen); sameReading.insert(questions[choice]->reading()).second) {
            answers[j] = choice;
            break;
          }
        } while (true);
    do {
      beginQuestionMessage(questions.size());
      if (quizStyle == 'k') {
        out() << i->name();
        auto info = i->info(infoFields);
        if (!info.empty()) out() << "  " << info;
        if (!isTestMode()) printExtraTypeInfo(i);
      } else
        out() << "Reading:  " << i->reading();
      printMeaning(i, !isTestMode());
      choices = getDefaultChoices(questions.size());
      if (isTestMode()) {
        for (int i = 0; i < numberOfChoicesPerQuestion; ++i)
          choices['1' + i] = "";
        for (auto& j : answers)
          out() << "    " << j.first << ".  "
                << (quizStyle == 'k' ? questions[j.second]->reading() : questions[j.second]->name()) << '\n';
      } else
        printReviewDetails(i);
      const char answer = _choice.get(prompt, choices);
      if (answer == QuitOption)
        stopQuiz = true;
      else if (answer == MeaningsOption)
        toggleMeanings(choices);
      else {
        if (answer == PrevOption)
          _question -= 2;
        else if (answer - '0' == correctChoice)
          out() << "  Correct! (" << ++_score << '/' << _question << ")\n";
        else if (answer != SkipOption) {
          out() << "  The correct answer is " << correctChoice << '\n';
          _mistakes.push_back(i->name());
        }
        break;
      }
    } while (!stopQuiz);
  }
  // when quitting don't count the current question in the final score
  if (stopQuiz) --_question;
}

// Group Based Quiz

bool Quiz::includeMember(const Entry& k, MemberType type) {
  return k->hasReading() &&
    (k->is(KanjiTypes::Jouyou) || type && k->hasLevel() || type > 1 && k->frequency() || type > 2);
}

template<typename T>
void Quiz::prepareGroupQuiz(const GroupData::List& list, const T& otherMap, char otherGroup, OptChar questionList) {
  const char c = questionList ? *questionList : _choice.get("Kanji type", GroupKanjiChoices, DefaultGroupKanji);
  if (c == QuitOption) return;
  const MemberType type = static_cast<MemberType>(c - '1');
  int bucket = -1;
  // for 'pattern' groups, allow choosing a smaller subset based on the name reading
  if (otherGroup == 'm') {
    const char f = _choice.get("Pattern name", PatternGroupChoices, DefaultPatternGroup);
    if (f == QuitOption) return;
    bucket = f - '1';
  }
  if (_questionOrder == QuestionOrder::FromBeginning && type == All && bucket == -1)
    groupQuiz(list, type, otherMap, otherGroup);
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
          if (includeMember(j, type)) ++memberCount;
        // only include groups that have 2 or more members after applying the 'include member' filter
        if (memberCount > 1) newList.push_back(i);
      }
    }
    if (_questionOrder == QuestionOrder::FromEnd)
      std::reverse(newList.begin(), newList.end());
    else if (_questionOrder == QuestionOrder::Random)
      std::shuffle(newList.begin(), newList.end(), RandomGen);
    groupQuiz(newList, type, otherMap, otherGroup);
  }
}

template<typename T>
void Quiz::groupQuiz(const GroupData::List& list, MemberType type, const T& otherMap, char otherGroup) {
  bool stopQuiz = false;
  for (bool firstTime = true; _question < list.size() && !stopQuiz; ++_question) {
    auto& i = list[_question];
    List questions, readings;
    for (auto& j : i->members())
      if (includeMember(j, type)) {
        questions.push_back(j);
        readings.push_back(j);
      }
    if (isTestMode()) {
      std::shuffle(questions.begin(), questions.end(), RandomGen);
      std::shuffle(readings.begin(), readings.end(), RandomGen);
    }
    if (firstTime) {
      beginQuizMessage(list.size()) << i->type() << " groups\n";
      if (type) log() << "  " << Kanji::Legend << '\n';
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
      showGroup(questions, answers, readings, choices, repeatQuestion, otherMap, otherGroup);
      if (getAnswers(answers, questions.size(), choices, skipGroup, stopQuiz)) {
        checkAnswers(answers, questions, readings, i->name());
        break;
      }
      repeatQuestion = true;
    } while (!stopQuiz && !skipGroup);
  }
  if (stopQuiz) --_question;
}

template<typename T>
void Quiz::showGroup(const List& questions, const Answers& answers, const List& readings, Choices& choices,
                     bool repeatQuestion, const T& otherMap, char otherGroup) const {
  static const std::string NoPinyin(12, ' ');
  for (int count = 0; auto& i : questions) {
    const char choice = isTestMode() ? count < 26 ? 'a' + count : 'A' + (count - 26) : ' ';
    out() << std::right << std::setw(4) << count + 1 << ":  ";
    auto s = i->qualifiedName();
    if (i->pinyin()) {
      std::string p = "  (" + *i->pinyin() + ')';
      // need to use 'displayLength' since Pinyin can contain multi-byte chars (for the tones)
      s += p + std::string(NoPinyin.length() - displayLength(p), ' ');
    } else
      s += NoPinyin;
    if (!isTestMode()) {
      auto j = otherMap.find(i->name());
      if (j != otherMap.end()) {
        s += otherGroup;
        s += ':';
        s += std::to_string(j->second->number());
      }
    }
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

bool Quiz::getAnswers(Answers& answers, int totalQuestions, Choices& choices, bool& skipGroup, bool& stopQuiz) {
  for (int i = answers.size(); i < totalQuestions; ++i)
    if (bool refresh = false; !getAnswer(answers, choices, skipGroup, refresh)) {
      // set 'stopQuiz' to break out of top quiz loop if user quit in the middle of providing answers
      if (!refresh && !skipGroup) stopQuiz = true;
      return false;
    }
  return true;
}

bool Quiz::getAnswer(Answers& answers, Choices& choices, bool& skipGroup, bool& refresh) {
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
      _choice.get(isTestMode() ? QuizMsg + space + std::to_string(answers.size() + 1) : ReviewMsg, choices);
    switch (answer) {
    case QuitOption: return false;
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

void Quiz::editAnswer(Answers& answers, Choices& choices) {
  _choice.clearQuit();
  auto getEntry = [this, &answers]() {
    std::map<char, std::string> answersToEdit;
    for (auto k : answers)
      answersToEdit[k] = "";
    const auto index = std::find(answers.begin(), answers.end(), _choice.get("    Answer to edit: ", answersToEdit));
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
  const char answer =
    _choice.get("    New reading for Entry: " + std::to_string(entry + 1), newChoices, answers[entry]);
  answers[entry] = answer;
  choices.erase(answer);
  _choice.setQuit(QuitOption);
}

void Quiz::checkAnswers(const Answers& answers, const List& questions, const List& readings, const std::string& name) {
  int count = 0;
  for (auto i : answers) {
    int index = (i <= 'z' ? i - 'a' : i - 'A' + 26);
    // Only match on readings (and meanings if '_showMeanings' is true) instead of making
    // sure the kanji is exactly the same since many kanjis have identical readings
    // especially in the 'patterns' groups (and the user has no way to distinguish).
    if (questions[count]->reading() == readings[index]->reading() &&
        (!_showMeanings || questions[count]->meaning() == readings[index]->meaning()))
      ++count;
  }
  if (count == answers.size())
    out() << "  Correct! (" << ++_score << '/' << _question << ")\n";
  else {
    out() << "  Incorrect (got " << count << " right out of " << answers.size() << ")\n";
    _mistakes.push_back(name);
  }
}

} // namespace kanji_tools
