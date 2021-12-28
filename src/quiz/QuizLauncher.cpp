#include <kanji_tools/quiz/GroupQuiz.h>
#include <kanji_tools/quiz/ListQuiz.h>
#include <kanji_tools/quiz/QuizLauncher.h>
#include <kanji_tools/utils/DisplayLength.h>

namespace kanji_tools {

namespace {

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

const Choice::Choices ProgramModeChoices({{'r', "review"}, {'t', "test"}}),
  ListOrderChoices({{'b', "from beginning"}, {'e', "from end"}, {'r', "random"}}),
  QuizTypeChoices({{'f', "freq"}, {'g', "grade"}, {'k', "kyu"}, {'l', "JLPT"}, {'m', "meaning"}, {'p', "pattern"}}),
  FrequencyChoices({{'1', "1-500"}, {'2', "501-1000"}, {'3', "1001-1500"}, {'4', "1501-2000"}, {'5', "2001-2501"}}),
  GradeChoices({{'s', "Secondary School"}}), KyuChoices({{'a', "10"}, {'b', "準１級"}, {'c', "準２級"}}),
  LevelChoices({{'1', "N1"}, {'2', "N2"}, {'3', "N3"}, {'4', "N4"}, {'5', "N5"}}),
  ListStyleChoices({{'k', "kanji to reading"}, {'r', "reading to kanji"}}),
  GroupKanjiChoices({{'1', "Jōyō"}, {'2', "1+JLPT"}, {'3', "2+Freq."}, {'4', "all"}});
constexpr char GradeStart = '1', GradeEnd = '6', KyuStart = '1', KyuEnd = '9', ListChoiceCountStart = '2',
               ListChoiceCountEnd = '9';

// Default options are offered for some of the above 'Choices' (when prompting the user for input):
constexpr char DefaultProgramMode = 't', DefaultQuestionOrder = 'r', DefaultQuizType = 'g', DefaultGrade = '6',
               DefaultKyu = '2', DefaultListChoiceCount = '4', DefaultListStyle = 'k', DefaultGroupKanji = '2';

constexpr char QuitOption = '/';

} // namespace

QuizLauncher::QuizLauncher(int argc, const char** argv, DataPtr data, std::istream* in)
  : _programMode(ProgramMode::NotAssigned), _questionOrder(QuestionOrder::NotAssigned),
    _choice(data->out(), in, QuitOption), _groupData(data), _jukugoData(*data) {
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

  int question = 0;
  bool endOptions = false, showMeanings = false;
  for (int i = Data::nextArg(argc, argv); i < argc; i = Data::nextArg(argc, argv, i))
    if (std::string arg = argv[i]; !endOptions && arg.starts_with("-") && arg.length() > 1) {
      if (arg == "-h") {
        out() << HelpMessage;
        return;
      }
      if (arg == "--")
        endOptions = true;
      else if (arg == "-s")
        showMeanings = true;
      else
        switch (arg[1]) {
        case 'r': // intentional fallthrough
        case 't': question = processProgramModeArg(arg); break;
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
      processKanjiArg(arg);
      return;
    }
  if (!data->debug() && !in) start(quizType, questionList, question, showMeanings);
}

void QuizLauncher::start(OptChar quizType, OptChar qList, int question, bool meanings) {
  if (_programMode == ProgramMode::NotAssigned) {
    char c = _choice.get("Mode", ProgramModeChoices, DefaultProgramMode);
    if (isQuit(c)) return;
    _programMode = c == 'r' ? ProgramMode::Review : ProgramMode::Test;
  }
  if (!getQuestionOrder()) return;

  auto listQuiz = [this, question, meanings](int f, const auto& l) { startListQuiz(question, meanings, f, l); };
  auto groupQuiz = [this, question, meanings, qList](const auto& l) { startGroupQuiz(question, meanings, qList, l); };

  // can replace below 'quizType' turnary operator with 'or_else' when C++23 is available
  switch (quizType ? *quizType : _choice.get("Type", QuizTypeChoices, DefaultQuizType)) {
  case 'f':
    // suppress printing 'Freq' since this would work against showing the list in a random order.
    if (const char c = qList ? *qList : _choice.get("Choose list", FrequencyChoices); !isQuit(c))
      listQuiz(Kanji::FreqField, data().frequencyList(c - '1'));
    break;
  case 'g':
    // suppress printing 'Grade' since it's the same for every kanji in the list
    if (const char c = qList ? *qList : _choice.get("Choose grade", GradeStart, GradeEnd, GradeChoices, DefaultGrade);
        !isQuit(c))
      listQuiz(Kanji::GradeField, data().gradeList(AllKanjiGrades[c == 's' ? 6 : c - '1']));
    break;
  case 'k':
    // suppress printing 'Kyu' since it's the same for every kanji in the list
    if (const char c = qList ? *qList : _choice.get("Choose kyu", KyuStart, KyuEnd, KyuChoices, DefaultKyu); !isQuit(c))
      listQuiz(Kanji::KyuField,
               data().kyuList(AllKenteiKyus[c == 'a'     ? 0
                                              : c == 'c' ? 8
                                              : c == '2' ? 9
                                              : c == 'b' ? 10
                                              : c == '1' ? 11
                                                         : 7 - (c - '3')]));
    break;
  case 'l':
    // suppress printing 'Level' since it's the same for every kanji in the list
    if (const char c = qList ? *qList : _choice.get("Choose level", LevelChoices); !isQuit(c))
      listQuiz(Kanji::LevelField, data().levelList(AllJlptLevels[4 - (c - '1')]));
    break;
  case 'm': groupQuiz(_groupData.meaningGroups()); break;
  case 'p': groupQuiz(_groupData.patternGroups()); break;
  }
  // reset mode and question order in case 'start' is called again
  _programMode = ProgramMode::NotAssigned;
  _questionOrder = QuestionOrder::NotAssigned;
}

void QuizLauncher::printExtraTypeInfo(const Entry& k) const {
  out() << ", " << k->type();
  if (auto i = k->extraTypeInfo(); i) out() << " (" << *i << ')';
}

void QuizLauncher::printLegend(int infoFields) const {
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

void QuizLauncher::printMeaning(const Entry& k, bool useNewLine, bool showMeaning) const {
  if (showMeaning && k->hasMeaning()) out() << (useNewLine ? "\n    Meaning: " : " : ") << k->meaning();
  out() << '\n';
}

void QuizLauncher::printReviewDetails(const Entry& kanji) const {
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

void QuizLauncher::startListQuiz(int question, bool showMeanings, int excludeField, const List& list) const {
  int choiceCount = 1;
  char quizStyle = DefaultListStyle;
  if (isTestMode()) {
    const char c = _choice.get("Number of choices", ListChoiceCountStart, ListChoiceCountEnd, DefaultListChoiceCount);
    if (isQuit(c)) return;
    choiceCount = c - '0';
    quizStyle = _choice.get("Quiz style", ListStyleChoices, quizStyle);
    if (isQuit(quizStyle)) return;
  }
  ListQuiz(*this, question, showMeanings, list, Kanji::AllFields ^ excludeField, choiceCount,
           ListQuiz::toQuizStyle(quizStyle));
}

void QuizLauncher::startGroupQuiz(int question, bool showMeanings, OptChar qList, const GroupData::List& list) const {
  if (const char c = qList ? *qList : _choice.get("Kanji type", GroupKanjiChoices, DefaultGroupKanji); !isQuit(c))
    GroupQuiz(*this, question, showMeanings, list, static_cast<GroupQuiz::MemberType>(c - '1'));
}

int QuizLauncher::processProgramModeArg(const std::string& arg) {
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
      return std::stoi(numArg);
    }
  }
  return 0;
}

void QuizLauncher::processKanjiArg(const std::string& arg) const {
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

void QuizLauncher::printDetails(const Data::List& list, const std::string& name, const std::string& arg) const {
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

void QuizLauncher::printDetails(const std::string& arg, bool showLegend) const {
  if (showLegend) {
    printLegend();
    out() << '\n';
  }
  out() << "Showing details for " << arg << ' ' << toUnicode(arg, true);
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

bool QuizLauncher::getQuestionOrder() {
  if (_questionOrder == QuestionOrder::NotAssigned)
    switch (_choice.get("List order", ListOrderChoices, DefaultQuestionOrder)) {
    case 'b': _questionOrder = QuestionOrder::FromBeginning; break;
    case 'e': _questionOrder = QuestionOrder::FromEnd; break;
    case 'r': _questionOrder = QuestionOrder::Random; break;
    default: return false;
    }
  return true;
}

void QuizLauncher::printJukugoList(const std::string& name, const JukugoData::List& list) const {
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

} // namespace kanji_tools
