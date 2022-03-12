#include <kanji_tools/kanji/KanjiData.h>
#include <kanji_tools/quiz/GroupQuiz.h>
#include <kanji_tools/quiz/ListQuiz.h>
#include <kanji_tools/quiz/QuizLauncher.h>
#include <kanji_tools/utils/DisplaySize.h>

namespace kanji_tools {

namespace {

constexpr auto HelpMessage =
  R"(kanjiQuiz [-hs] [-f[1-5] | -g[1-6s] | -k[1-9a-c] | -l[1-5] -m[1-4] | -p[1-4]]
          [-r[num] | -t[num]] [kanji]
    -h   show this help message for command-line options
    -s   show English meanings by default (can be toggled on/off later)

  The following options allow choosing the quiz/review type optionally followed
  by question list type (grade, level, etc.) instead of being prompted:
    -f   'frequency' (optional frequency group '1-5')
    -g   'grade' (optional grade '1-6', 's' = Secondary School)
    -k   'kyu' (optional Kentei Kyu '1-9', 'a' = 10, 'b' = 準１級, 'c' = 準２級)
    -l   'level' (optional JLPT level number '1-5')
    -m   'meaning' (optional kanji type '1-4')
    -p   'pattern' (optional kanji type '1-4')

  The following options can be followed by a 'num' to specify where to start in
  the question list (use negative to start from the end or 0 for random order).
    -r   review mode
    -t   test mode

  kanji  show details for a kanji instead of starting a review or test

Examples:
  kanjiQuiz -f        # start 'frequency' quiz (prompts for 'bucket' number)
  kanjiQuiz -r40 -l1  # start 'JLPT N1' review beginning at the 40th entry

Note: 'kanji' can be UTF-8, frequency (between 1 and 2501), 'm' followed by
Morohashi ID (index in Dai Kan-Wa Jiten), 'n' followed by Classic Nelson ID
or 'u' followed by Unicode. For example, theses all produce the same output:
  kanjiQuiz 奉
  kanjiQuiz 1624
  kanjiQuiz m5894
  kanjiQuiz n212
  kanjiQuiz u5949
)";

const Choice::Choices ProgramModeChoices({{'r', "review"}, {'t', "test"}}),
  ListOrderChoices({{'b', "from beginning"},
                    {'e', "from end"},
                    {'r', "random"}}),
  QuizTypeChoices({{'f', "freq"},
                   {'g', "grade"},
                   {'k', "kyu"},
                   {'l', "JLPT"},
                   {'m', "meaning"},
                   {'p', "pattern"}}),
  FrequencyChoices({{'1', "1-500"},
                    {'2', "501-1000"},
                    {'3', "1001-1500"},
                    {'4', "1501-2000"},
                    {'5', "2001-2501"}}),
  GradeChoices({{'s', "Secondary School"}}),
  KyuChoices({{'a', "10"}, {'b', "準１級"}, {'c', "準２級"}}),
  LevelChoices(
    {{'1', "N1"}, {'2', "N2"}, {'3', "N3"}, {'4', "N4"}, {'5', "N5"}}),
  ListStyleChoices({{'k', "kanji to reading"}, {'r', "reading to kanji"}}),
  GroupKanjiChoices(
    {{'1', "Jōyō"}, {'2', "1+JLPT"}, {'3', "2+Freq."}, {'4', "all"}});
constexpr auto GradeStart = '1', GradeEnd = '6', KyuStart = '1', KyuEnd = '9',
               ListChoiceCountStart = '2', ListChoiceCountEnd = '9';

// Default options are offered for some of the above 'Choices' (when prompting
// the user for input):
constexpr auto DefaultProgramMode = 't', DefaultQuestionOrder = 'r',
               DefaultQuizType = 'g', DefaultGrade = '6', DefaultKyu = '2',
               DefaultListChoiceCount = '4', DefaultListStyle = 'k',
               DefaultGroupKanji = '2';

} // namespace

void QuizLauncher::run(size_t argc, const char** argv) {
  auto data = std::make_shared<KanjiData>(argc, argv);
  QuizLauncher(argc, argv, data, std::make_shared<GroupData>(data),
               std::make_shared<JukugoData>(data));
}

QuizLauncher::QuizLauncher(size_t argc, const char** argv, DataPtr data,
                           GroupDataPtr groupData, JukugoDataPtr jukugoData,
                           std::istream* in)
    : _programMode(ProgramMode::NotAssigned),
      _questionOrder(QuestionOrder::NotAssigned), _choice(data->out(), in, '/'),
      _groupData(groupData), _jukugoData(jukugoData) {
  OptChar quizType, questionList;
  // checkQuizType is called to check f, g, l, k, m and p args (so ok to assume
  // size is at least 2)
  const auto checkQuizType = [&quizType,
                              &questionList](const auto& arg, auto& choices,
                                             OptChar start = std::nullopt,
                                             OptChar end = std::nullopt) {
    if (quizType)
      Data::usage("only one quiz type can be specified, use -h for help");
    quizType = arg[1];
    if (arg.size() > 2) {
      if (auto c = arg[2];
          arg.size() == 3 && (choices.contains(c) ||
                              (start && *start <= c && end.value_or(c) >= c)))
        questionList = c;
      else
        Data::usage("invalid format for " + arg.substr(0, 2) +
                    ", use -h for help");
    }
  };

  size_t question{};
  auto endOptions = false, showMeanings = false;
  for (auto i = Data::nextArg(argc, argv); i < argc;
       i = Data::nextArg(argc, argv, i))
    if (std::string arg = argv[i];
        !endOptions && arg.starts_with("-") && arg.size() > 1) {
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
  if (!data->debug() && !in)
    start(quizType, questionList, question, showMeanings);
}

void QuizLauncher::start(OptChar quizType, OptChar qList, size_t question,
                         bool meanings) {
  if (_programMode == ProgramMode::NotAssigned) {
    const auto c = _choice.get("Mode", ProgramModeChoices, DefaultProgramMode);
    if (isQuit(c)) return;
    _programMode = c == 'r' ? ProgramMode::Review : ProgramMode::Test;
  }
  if (!getQuestionOrder()) return;

  const auto listQuiz = [this, question, meanings](auto f, auto& l) {
    startListQuiz(question, meanings, f, l);
  };
  const auto groupQuiz = [this, question, meanings, qList](auto& l) {
    startGroupQuiz(question, meanings, qList, l);
  };

  // replace 'quizType' turnary operator with 'or_else' when C++23 is available
  switch (quizType ? *quizType
                   : _choice.get("Type", QuizTypeChoices, DefaultQuizType)) {
  case 'f':
    // suppress printing 'Freq' since this would work against showing the list
    // in a random order.
    if (const auto c =
          qList ? *qList : _choice.get("Choose list", FrequencyChoices);
        !isQuit(c))
      listQuiz(KanjiInfo::Freq,
               data().frequencyList(static_cast<size_t>(c - '1')));
    break;
  case 'g':
    // suppress printing 'Grade' since it's the same for every kanji in the list
    if (const auto c = qList ? *qList
                             : _choice.get("Choose grade", GradeStart, GradeEnd,
                                           GradeChoices, DefaultGrade);
        !isQuit(c))
      listQuiz(KanjiInfo::Grade,
               data().gradeList(AllKanjiGrades[c == 's' ? 6 : c - '1']));
    break;
  case 'k':
    // suppress printing 'Kyu' since it's the same for every kanji in the list
    if (const auto c = qList ? *qList
                             : _choice.get("Choose kyu", KyuStart, KyuEnd,
                                           KyuChoices, DefaultKyu);
        !isQuit(c))
      listQuiz(KanjiInfo::Kyu,
               data().kyuList(AllKenteiKyus[c == 'a'   ? 0
                                            : c == 'c' ? 8
                                            : c == '2' ? 9
                                            : c == 'b' ? 10
                                            : c == '1' ? 11
                                                       : 7 - (c - '3')]));
    break;
  case 'l':
    // suppress printing 'Level' since it's the same for every kanji in the list
    if (const char c =
          qList ? *qList : _choice.get("Choose level", LevelChoices);
        !isQuit(c))
      listQuiz(KanjiInfo::Level,
               data().levelList(AllJlptLevels[4 - (c - '1')]));
    break;
  case 'm': groupQuiz(_groupData->meaningGroups()); break;
  case 'p': groupQuiz(_groupData->patternGroups()); break;
  }
  // reset mode and question order in case 'start' is called again
  _programMode = ProgramMode::NotAssigned;
  _questionOrder = QuestionOrder::NotAssigned;
}

void QuizLauncher::printExtraTypeInfo(const Entry& k) const {
  out() << ", " << k->type();
  if (const auto i = k->extraTypeInfo(); i) out() << " (" << *i << ')';
}

void QuizLauncher::printLegend(KanjiInfo fields) const {
  std::string s;
  if (hasValue(fields & KanjiInfo::Level)) s += " N[1-5]=JLPT Level";
  if (hasValue(fields & KanjiInfo::Kyu)) {
    if (!s.empty()) s += ',';
    s += " K[1-10]=Kentei Kyu";
  }
  if (hasValue(fields & KanjiInfo::Grade)) {
    if (!s.empty()) s += ',';
    s += " G[1-6]=Grade (S=Secondary School)";
  }
  log() << "Legend:\nFields:" << s << "\nSuffix: " << Kanji::Legend << '\n';
}

void QuizLauncher::printMeaning(const Entry& k, bool useNewLine,
                                bool showMeaning) const {
  if (showMeaning && k->hasMeaning())
    out() << (useNewLine ? "\n    Meaning: " : " : ") << k->meaning();
  out() << '\n';
}

void QuizLauncher::printReviewDetails(const Entry& kanji) const {
  out() << "    Reading: " << kanji->reading() << '\n';
  // Similar Kanji
  if (const auto i = _groupData->patternMap().find(kanji->name());
      i != _groupData->patternMap().end() &&
      i->second->patternType() != Group::PatternType::Reading) {
    out() << "    Similar:";
    Data::List sorted(i->second->members());
    std::sort(sorted.begin(), sorted.end(), Data::orderByQualifiedName);
    for (auto& j : sorted)
      if (j != kanji) out() << ' ' << j->qualifiedName();
    out() << '\n';
  }
  // Morohashi and Nelson IDs
  if (kanji->morohashiId())
    out() << "  Morohashi: " << *kanji->morohashiId() << '\n';
  if (kanji->hasNelsonIds()) {
    out() << (kanji->nelsonIds().size() == 1 ? "  Nelson ID:" : " Nelson IDs:");
    for (auto& i : kanji->nelsonIds()) out() << ' ' << i;
    out() << '\n';
  }
  // Categories
  if (const auto i = _groupData->meaningMap().equal_range(kanji->name());
      i.first != i.second) {
    auto j = i.first;
    out() << (++j == i.second ? "   Category: " : " Categories: ");
    for (j = i.first; j != i.second; ++j) {
      if (j != i.first) out() << ", ";
      out() << '[' << j->second->name() << ']';
    }
    out() << '\n';
  }
  // Jukugo Lists
  static const std::string jukugo(" Jukugo"), sameGrade("Same Grade Jukugo"),
    otherGrade("Other Grade Jukugo");
  if (auto& list = _jukugoData->find(kanji->name()); !list.empty()) {
    // For kanji with a 'Grade' (so all Jouyou kanji) split Jukugo into two
    // lists, one for the same grade of the given kanji and one for other
    // grades. For example, 一生（いっしょう） is a grade 1 Jukugo for '一', but
    // 一縷（いちる） is a secondary school Jukugo (which also contains '一').
    if (JukugoData::List same, other;
        kanji->hasGrade() && list.size() > JukugoPerLine) {
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

void QuizLauncher::startListQuiz(size_t question, bool showMeanings,
                                 KanjiInfo excludeField,
                                 const List& list) const {
  size_t choiceCount = 1;
  auto quizStyle = DefaultListStyle;
  if (isTestMode()) {
    const auto c = _choice.get("Number of choices", ListChoiceCountStart,
                               ListChoiceCountEnd, DefaultListChoiceCount);
    if (isQuit(c)) return;
    choiceCount = static_cast<size_t>(c - '0');
    quizStyle = _choice.get("Quiz style", ListStyleChoices, quizStyle);
    if (isQuit(quizStyle)) return;
  }
  ListQuiz(*this, question, showMeanings, list, KanjiInfo::All ^ excludeField,
           choiceCount, ListQuiz::toQuizStyle(quizStyle));
}

void QuizLauncher::startGroupQuiz(size_t question, bool showMeanings,
                                  OptChar qList,
                                  const GroupData::List& list) const {
  if (const auto c =
        qList ? *qList
              : _choice.get("Kanji type", GroupKanjiChoices, DefaultGroupKanji);
      !isQuit(c))
    GroupQuiz(*this, question, showMeanings, list,
              static_cast<GroupQuiz::MemberType>(c - '1'));
}

size_t QuizLauncher::processProgramModeArg(const std::string& arg) {
  if (_programMode != ProgramMode::NotAssigned)
    Data::usage("only one mode (-r or -t) can be specified, use -h for help");
  _programMode = arg[1] == 'r' ? ProgramMode::Review : ProgramMode::Test;
  if (arg.size() > 2) {
    if (arg.size() == 3 && arg[2] == '0')
      _questionOrder = QuestionOrder::Random;
    else {
      size_t offset = 2;
      if (arg[2] == '-') {
        _questionOrder = QuestionOrder::FromEnd;
        offset = 3;
      } else {
        _questionOrder = QuestionOrder::FromBeginning;
        if (arg[2] == '+') offset = 3;
      }
      const auto numArg = arg.substr(offset);
      if (!std::all_of(numArg.begin(), numArg.end(), ::isdigit))
        Data::usage("invalid format for " + arg.substr(0, 2) +
                    ", use -h for help");
      return std::stoul(numArg);
    }
  }
  return 0;
}

void QuizLauncher::processKanjiArg(const std::string& arg) const {
  if (std::all_of(arg.begin(), arg.end(), ::isdigit)) {
    const auto kanji = _groupData->data().findKanjiByFrequency(std::stoul(arg));
    if (!kanji) Data::usage("invalid frequency '" + arg + "'");
    printDetails((**kanji).name());
  } else if (arg.starts_with("m")) {
    const auto id = arg.substr(1);
    // a valid Morohashi ID should be numeric followed by an optional 'P'
    if (auto nonPrime = id.ends_with("P") ? id.substr(0, id.size() - 1) : id;
        id.empty() || !std::all_of(nonPrime.begin(), nonPrime.end(), ::isdigit))
      Data::usage("invalid Morohashi ID '" + id + "'");
    printDetails(_groupData->data().findKanjisByMorohashiId(id), "Morohashi",
                 id);
  } else if (arg.starts_with("n")) {
    const auto id = arg.substr(1);
    if (id.empty() || !std::all_of(id.begin(), id.end(), ::isdigit))
      Data::usage("invalid Nelson ID '" + id + "'");
    printDetails(_groupData->data().findKanjisByNelsonId(std::stoi(id)),
                 "Nelson", id);
  } else if (arg.starts_with("u")) {
    const auto id = arg.substr(1);
    // must be a 4 or 5 digit hex value (and if 5 digits, then the first digit
    // must be a 1 or 2)
    if (id.size() < 4 || id.size() > 5 ||
        (id.size() == 5 && id[0] != '1' && id[0] != '2') ||
        !std::all_of(id.begin(), id.end(), ::ishexnumber))
      Data::usage("invalid Unicode value '" + id + "'");
    printDetails(toUtf8(std::strtol(id.c_str(), nullptr, 16)));
  } else if (isKanji(arg))
    printDetails(arg);
  else
    Data::usage("unrecognized 'kanji' value '" + arg + "' use -h for help");
}

void QuizLauncher::printDetails(const Data::List& list, const std::string& name,
                                const std::string& arg) const {
  if (list.size() != 1) {
    if (list.size() > 1) {
      printLegend();
      out() << '\n';
    }
    out() << "Found " << list.size() << " matches for " << name << " ID " << arg
          << (list.size() ? ":\n\n" : "\n");
  }
  for (auto& kanji : list) printDetails(kanji->name(), list.size() == 1);
}

void QuizLauncher::printDetails(const std::string& arg, bool showLegend) const {
  if (showLegend) {
    printLegend();
    out() << "Sources: G=China / Singapore, H=Hong Kong, J=Japan, K=Korea, "
             "T=Taiwan, V=Vietnam\n\n";
  }
  out() << "Details for " << arg << ' ' << toUnicode(arg, BracketType::Square);
  if (const auto ucd = data().ucd().find(arg); ucd) {
    out() << ", Blk " << ucd->block() << ", Ver " << ucd->version();
    if (!ucd->sources().empty()) {
      out() << ", Sources " << ucd->sources();
      if (!ucd->jSource().empty()) out() << " (" << ucd->jSource() << ')';
    }
    if (const auto k = data().findKanjiByName(arg); k) {
      printExtraTypeInfo(*k);
      out() << '\n' << (**k).info();
      printMeaning(*k, true);
      printReviewDetails(*k);
    } else // should never happen since all kanji in ucd.txt should be loaded
      out() << " --- Kanji not loaded'\n";
  } else
    // can happen for non=supported rare kanji (see parseUcdAllFlat.sh)
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

void QuizLauncher::printJukugoList(const std::string& name,
                                   const JukugoData::List& list) const {
  out() << "    " << name << ':';
  if (list.size() <= JukugoPerLine)
    for (auto& i : list) out() << ' ' << i->nameAndReading();
  else {
    out() << ' ' << list.size();
    std::array<size_t, JukugoPerLine> colWidths;
    colWidths.fill(0);
    // make each column wide enough to hold the longest entry plus 2 spaces
    // (upto MaxJukugoSize)
    for (size_t i{}; i < list.size(); ++i)
      if (colWidths[i % JukugoPerLine] < MaxJukugoSize)
        if (const size_t size = displaySize(list[i]->nameAndReading()) + 2;
            size > colWidths[i % JukugoPerLine])
          colWidths[i % JukugoPerLine] =
            size < MaxJukugoSize ? size : MaxJukugoSize;
    for (size_t i{}; i < list.size(); ++i) {
      if (i % JukugoPerLine == 0) out() << '\n';
      const auto s = list[i]->nameAndReading();
      out() << std::left << std::setw(wideSetw(s, colWidths[i % JukugoPerLine]))
            << s;
    }
  }
  out() << '\n';
}

} // namespace kanji_tools
