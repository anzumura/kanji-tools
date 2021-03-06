#include <kt_kana/DisplaySize.h>
#include <kt_quiz/GroupQuiz.h>
#include <kt_quiz/ListQuiz.h>
#include <kt_quiz/QuizLauncher.h>

#include <sstream>

namespace kanji_tools {

namespace {

// Clang marks some lines in 'HelpMessage' as '0' coverage whereas GCC doesn't
// count them at all (which seems like the correct way to go for 'constexpr')
constexpr auto HelpMessage{// LCOV_EXCL_START
    R"(kanjiQuiz [-hs] [-f[1-5] | -g[1-6s] | -k[1-9a-c] | -l[1-5] -m[1-4] | -p[1-4]]
          [-r[num] | -t[num]] [kanji]
    -h   show this help message for command-line options
    -s   show English meanings by default (can be toggled on/off later)

  The following options allow choosing the quiz/review type optionally followed
  by question list type (grade, level, etc.) instead of being prompted:
    -f   'frequency' (optional frequency group '0-9')
    -g   'grade' (optional grade '1-6', 's' = Secondary School)
    -k   'kyu' (optional Kentei Kyu '1-9', 'a' = 10, 'b' = 準１級, 'c' = 準２級)
    -l   'level' (optional JLPT level number '1-5')
    -m   'meaning' (optional Kanji type '1-4')
    -p   'pattern' (optional Kanji type '1-4')

  The following options can be followed by a 'num' to specify where to start in
  the question list (use negative to start from the end or 0 for random order).
    -r   review mode
    -t   test mode

  kanji  show details for a Kanji instead of starting a review or test

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
)"};                       // LCOV_EXCL_STOP

const Choice::Choices ProgramModeChoices{{'r', "review"}, {'t', "test"}},
    ListOrderChoices{
        {'b', "from beginning"}, {'e', "from end"}, {'r', "random"}},
    QuizTypeChoices{{'f', "freq"}, {'g', "grade"}, {'k', "kyu"}, {'l', "JLPT"},
        {'m', "meaning"}, {'p', "pattern"}},
    FrequencyChoices{{'0', "top 250 Kanji"}},
    GradeChoices{{'s', "Secondary School"}},
    KyuChoices{{'a', "10"}, {'b', "準１級"}, {'c', "準２級"}},
    LevelChoices{
        {'1', "N1"}, {'2', "N2"}, {'3', "N3"}, {'4', "N4"}, {'5', "N5"}},
    ListStyleChoices{{'k', "kanji to reading"}, {'r', "reading to kanji"}},
    GroupKanjiChoices{
        {'1', "Jōyō"}, {'2', "1+JLPT"}, {'3', "2+Freq."}, {'4', "all"}};

constexpr Choice::Range FrequencyRange{'1', '9'}, GradeRange{'1', '6'},
    KyuRange{'1', '9'}, ChoiceCountRange{'2', '9'};

// Default options are offered for some of the above 'Choices' (when prompting
// the user for input):
constexpr auto DefaultProgramMode{'t'}, DefaultQuestionOrder{'r'},
    DefaultChoiceCount{'4'}, DefaultListStyle{'k'}, DefaultGroupKanji{'2'};

} // namespace

QuizLauncher::QuizLauncher(const Args& args, const KanjiDataPtr& data,
    const GroupDataPtr& groupData, // LCOV_EXCL_LINE
    const JukugoDataPtr& jukugoData, std::istream* in)
    : _choice{data->out(), in, QuitOption}, _groupData{groupData},
      _jukugoData{jukugoData} {
  OptChar quizType, qList;
  Question question{};
  auto endOptions{false}, showMeanings{false};
  for (auto i{KanjiData::nextArg(args)}; i < args.size();
       i = KanjiData::nextArg(args, i))
    if (String arg{args[i]};
        endOptions || !arg.starts_with("-") || arg.size() < 2) {
      processKanjiArg(arg);
      return; // exit after showing info for a Kanji, i.e., don't start a quiz
    } else if (arg == "-h") {
      out() << HelpMessage;
      return; // exit after showing help message
    } else if (arg == "--")
      endOptions = true;
    else if (arg == "-s")
      showMeanings = true;
    else if (const auto c{processArg(question, quizType, arg)}; c)
      qList = c; // only set 'qList' if a non-empty value was returned
  if (!data->debug() && (!in || quizType))
    start(quizType, qList, question, showMeanings);
}

void QuizLauncher::start(OptChar quizType, OptChar qList, Question question,
    bool meanings, bool randomizeAnswers) {
  if (_programMode == ProgramMode::NotAssigned) {
    const auto c{_choice.get("Mode", ProgramModeChoices, DefaultProgramMode)};
    if (isQuit(c)) return;
    _programMode = c == 'r' ? ProgramMode::Review : ProgramMode::Quiz;
  }
  if (!getQuestionOrder()) return;
  _randomizeAnswers = randomizeAnswers;

  const auto listQuiz{[this, question, meanings](auto f, auto& l) {
    startListQuiz(question, meanings, f, l);
  }};
  const auto groupQuiz{[this, question, meanings, qList](auto& l) {
    startGroupQuiz(question, meanings, qList, l);
  }};

  switch (chooseQuizType(quizType)) {
  case 'f':
    if (const auto c{chooseFreq(qList)}; !isQuit(c))
      // suppress printing 'Freq' (by passing 'Kanji::Info::Freq') since this
      // would work against showing the list in a random order
      listQuiz(Kanji::Info::Freq,
          data().frequencyList(static_cast<size_t>(c - '0')));
    break;
  case 'g':
    if (const auto c{chooseGrade(qList)}; !isQuit(c))
      // suppress printing 'Grade' (it's the same for every kanji in the list)
      listQuiz(Kanji::Info::Grade,
          data().grades()[c == 's' ? KanjiGrades::S : AllKanjiGrades[c - '1']]);
    break;
  case 'k':
    if (const auto c{chooseKyu(qList)}; !isQuit(c))
      listQuiz(Kanji::Info::Kyu, getKyuList(c));
    break;
  case 'l':
    if (const char c{chooseLevel(qList)}; !isQuit(c))
      listQuiz(
          Kanji::Info::Level, data().levels()[AllJlptLevels[4 - (c - '1')]]);
    break;
  case 'm': groupQuiz(_groupData->meaningGroups()); break;
  case 'p': groupQuiz(_groupData->patternGroups()); break;
  }
  // reset mode and question order in case 'start' is called again
  _programMode = ProgramMode::NotAssigned;
  _questionOrder = QuestionOrder::NotAssigned;
}

bool QuizLauncher::isQuizMode() const {
  return _programMode == ProgramMode::Quiz;
}

std::ostream& QuizLauncher::log(bool heading) const {
  return data().log(heading);
}

void QuizLauncher::printExtraTypeInfo(const Kanji& k) const {
  out() << ", " << k.type();
  if (const auto i{k.extraTypeInfo()}; i) out() << " (" << *i << ')';
}

void QuizLauncher::printLegend(Kanji::Info fields) const {
  String s;
  if (hasValue(fields & Kanji::Info::Level)) s += " N[1-5]=JLPT Level";
  if (hasValue(fields & Kanji::Info::Kyu)) {
    if (!s.empty()) s += ',';
    s += " K[1-10]=Kentei Kyu";
  }
  if (hasValue(fields & Kanji::Info::Grade)) {
    if (!s.empty()) s += ',';
    s += " G[1-6]=Grade (S=Secondary School)";
  }
  log() << "Legend:\nFields:" << s << "\nSuffix: " << Kanji::Legend << '\n';
}

void QuizLauncher::printMeaning(
    const Kanji& k, bool useNewLine, bool showMeaning) const {
  if (showMeaning && k.hasMeaning())
    out() << (useNewLine ? "\n    Meaning: " : " : ") << k.meaning();
  out() << '\n';
}

void QuizLauncher::printReviewDetails(const Kanji& kanji) const {
  out() << "    Reading: " << kanji.reading() << '\n';
  // Similar Kanji
  if (const auto i{_groupData->patternMap().find(kanji.name())};
      i != _groupData->patternMap().end() &&
      i->second->patternType() != Group::PatternType::Reading) {
    out() << "    Similar:";
    KanjiData::List sorted(i->second->members());
    std::sort(sorted.begin(), sorted.end(), KanjiData::OrderByQualifiedName);
    for (auto& j : sorted)
      if (*j != kanji) out() << ' ' << j->qualifiedName();
    out() << '\n';
  }
  // Morohashi and Nelson IDs
  if (kanji.morohashiId())
    out() << "  Morohashi: " << kanji.morohashiId() << '\n';
  if (kanji.hasNelsonIds()) {
    out() << (kanji.nelsonIds().size() == 1 ? "  Nelson ID:" : " Nelson IDs:");
    for (auto& i : kanji.nelsonIds()) out() << ' ' << i;
    out() << '\n';
  }
  // Categories
  if (const auto i{_groupData->meaningMap().equal_range(kanji.name())};
      i.first != i.second) {
    auto j{i.first};
    out() << (++j == i.second ? "   Category: " : " Categories: ");
    for (j = i.first; j != i.second; ++j) {
      if (j != i.first) out() << ", ";
      out() << '[' << j->second->name() << ']';
    }
    out() << '\n';
  }
  printJukugo(kanji);
}

void QuizLauncher::startListQuiz(Question question, bool showMeanings,
    Kanji::Info excludeField, const List& list) const {
  ListQuiz::ChoiceCount choiceCount{1};
  auto quizStyle{DefaultListStyle};
  if (isQuizMode()) {
    const auto c{
        _choice.get(ChoiceCountRange, "Number of choices", DefaultChoiceCount)};
    if (isQuit(c)) return;
    choiceCount = static_cast<ListQuiz::ChoiceCount>(c - '0');
    quizStyle = _choice.get("Quiz style", ListStyleChoices, quizStyle);
    if (isQuit(quizStyle)) return;
  }
  ListQuiz(*this, question, showMeanings, list, Kanji::Info::All ^ excludeField,
      choiceCount, ListQuiz::toQuizStyle(quizStyle));
}

void QuizLauncher::startGroupQuiz(Question question, bool showMeanings,
    OptChar qList, const GroupData::List& list) const {
  if (const auto c{qList ? *qList
                         : _choice.get("Kanji type", GroupKanjiChoices,
                               DefaultGroupKanji)};
      !isQuit(c))
    GroupQuiz(*this, question, showMeanings, list,
        to_enum<GroupQuiz::MemberType>(c - '1'));
}

QuizLauncher::OptChar QuizLauncher::processArg(
    Question& question, OptChar& quizType, const String& arg) {
  switch (arg[1]) {
  case 'r': // intentional fallthrough
  case 't': question = processProgramModeArg(arg); break;
  case 'f': return setQuizType(quizType, arg, FrequencyChoices, FrequencyRange);
  case 'g': return setQuizType(quizType, arg, GradeChoices, GradeRange);
  case 'k': return setQuizType(quizType, arg, KyuChoices, KyuRange);
  case 'l': return setQuizType(quizType, arg, LevelChoices);
  case 'm': // intentional fallthrough
  case 'p': return setQuizType(quizType, arg, GroupKanjiChoices);
  default: KanjiData::usage("illegal option '" + arg + "', use -h for help");
  }
  return {};
}

QuizLauncher::Question QuizLauncher::processProgramModeArg(const String& arg) {
  if (_programMode != ProgramMode::NotAssigned)
    KanjiData::usage(
        "only one mode (-r or -t) can be specified, use -h for help");
  _programMode = arg[1] == 'r' ? ProgramMode::Review : ProgramMode::Quiz;
  if (arg.size() > 2) {
    if (arg.size() == 3 && arg[2] == '0')
      _questionOrder = QuestionOrder::Random;
    else {
      uint16_t offset{2};
      if (arg[2] == '-') {
        _questionOrder = QuestionOrder::FromEnd;
        offset = 3;
      } else {
        _questionOrder = QuestionOrder::FromBeginning;
        if (arg[2] == '+') offset = 3;
      }
      const auto s{arg.substr(offset)};
      if (!std::all_of(s.begin(), s.end(), ::isdigit))
        KanjiData::usage(
            "invalid format for '" + arg.substr(0, 2) + "', use -h for help");
      if (const auto i{std::stoul(s)};
          i <= std::numeric_limits<Question>::max())
        return static_cast<Question>(i);
      KanjiData::usage("value for '" + arg.substr(0, 2) + "' exceeds limit");
    }
  }
  return 0;
}

Kanji::NelsonId QuizLauncher::getId(const String& msg, const String& arg) {
  std::stringstream ss{arg};
  Kanji::NelsonId id{};
  if (!(ss >> id)) KanjiData::usage("invalid " + msg + " '" + arg + "'");
  return id;
}

QuizLauncher::OptChar QuizLauncher::setQuizType(OptChar& quizType,
    const String& arg, const Choices& choices,
    const std::optional<Choice::Range>& r) {
  if (quizType)
    KanjiData::usage("only one quiz type can be specified, use -h for help");
  quizType = arg[1];
  if (arg.size() > 2) {
    if (const auto c{arg[2]};
        arg.size() == 3 &&
        (choices.contains(c) || r && r->first <= c && r->second >= c))
      return c;
    KanjiData::usage(
        "invalid format for '" + arg.substr(0, 2) + "', use -h for help");
  }
  return {};
}

void QuizLauncher::processKanjiArg(const String& arg) const {
  if (std::all_of(arg.begin(), arg.end(), ::isdigit)) {
    const auto kanji{
        _groupData->data().findByFrequency(getId("frequency", arg))};
    if (!kanji) KanjiData::usage("Kanji not found for frequency '" + arg + "'");
    printDetails(kanji->name());
  } else if (arg.starts_with("m")) {
    const MorohashiId id{arg.substr(1)};
    printDetails(
        _groupData->data().findByMorohashiId(id), "Morohashi", id.toString());
  } else if (arg.starts_with("n")) {
    const auto id{arg.substr(1)};
    if (id.empty() || !std::all_of(id.begin(), id.end(), ::isdigit))
      KanjiData::usage("Nelson ID '" + id + "' is non-numeric");
    printDetails(_groupData->data().findByNelsonId(getId("Nelson ID", id)),
        "Nelson", id);
  } else if (arg.starts_with("u")) {
    const auto id{arg.substr(1)};
    // must be 4 or 5 digit hex (and if 5, then first digit must be 1 or 2)
    if (id.size() < UnicodeStringMinSize || id.size() > UnicodeStringMaxSize ||
        (id.size() == UnicodeStringMaxSize && id[0] != '1' && id[0] != '2') ||
        !std::all_of(id.begin(), id.end(), ::ishexnumber))
      KanjiData::usage("Unicode value '" + id + "' is invalid");
    printDetails(toUtf8(std::stoi(id, nullptr, HexDigits)));
  } else if (isKanji(arg))
    printDetails(arg);
  else
    KanjiData::usage(
        "unrecognized 'kanji' value '" + arg + "', use -h for help");
}

void QuizLauncher::printDetails(
    const KanjiData::List& list, const String& name, const String& arg) const {
  if (list.size() != 1) {
    if (list.size() > 1) {
      printLegend();
      out() << '\n';
    }
    out() << "Found " << list.size() << " matches for " << name << " ID " << arg
          << (list.empty() ? "\n" : ":\n\n");
  }
  for (auto& kanji : list) printDetails(kanji->name(), list.size() == 1);
}

void QuizLauncher::printDetails(const String& arg, bool showLegend) const {
  if (showLegend) {
    printLegend();
    out() << "Sources: G=China / Singapore, H=Hong Kong, J=Japan, K=Korea, "
             "T=Taiwan, V=Vietnam\n\n";
  }
  out() << arg << ' ' << toUnicode(arg, BracketType::Square);
  if (const auto ucd{data().ucd().find(arg)}; ucd) {
    out() << ", Blk " << ucd->block() << ", Ver " << ucd->version();
    if (!ucd->sources().empty()) {
      out() << ", Sources " << ucd->sources();
      if (!ucd->jSource().empty()) out() << " (" << ucd->jSource() << ')';
    }
    if (const auto k{data().findByName(arg)}; k) {
      printExtraTypeInfo(*k);
      out() << '\n' << k->info();
      printMeaning(*k, true);
      printReviewDetails(*k);
    } else // should never happen since all kanji in ucd.txt should be loaded
      out() << " --- Kanji not loaded'\n"; // LCOV_EXCL_LINE
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

void QuizLauncher::printJukugo(const Kanji& kanji) const {
  static const String Jukugo{" Jukugo"}, SameGrade{"Same Grade Jukugo"},
      OtherGrade{"Other Grade Jukugo"};
  if (auto& list{_jukugoData->find(kanji.name())}; !list.empty()) {
    // For Kanji with a 'Grade' split Jukugo into two lists, one for the same
    // grade of the given Kanji and one for the other grades. For example,
    // 一生（いっしょう） is a grade 1 Jukugo for '一', but 一縷（いちる） is a
    // secondary school Jukugo (which also contains '一').
    if (JukugoData::List same, other;
        kanji.hasGrade() && list.size() > JukugoPerLine) {
      for (auto& i : list)
        (kanji.grade() == i->grade() ? same : other).push_back(i);
      if (other.empty())
        printJukugoList(Jukugo, list);
      else {
        printJukugoList(SameGrade, same);
        printJukugoList(OtherGrade, other);
      }
    } else
      printJukugoList(Jukugo, list);
  }
  out() << '\n';
}

void QuizLauncher::printJukugoList(
    const String& name, const JukugoData::List& list) const {
  out() << "    " << name << ':';
  if (list.size() <= JukugoPerLine)
    for (auto& i : list) out() << ' ' << i->nameAndReading();
  else {
    out() << ' ' << list.size();
    std::array<size_t, JukugoPerLine> colWidths{};
    // make each column wide enough to hold the longest entry plus 2 spaces
    // (upto MaxJukugoSize)
    for (size_t i{}; i < list.size(); ++i)
      if (colWidths[i % JukugoPerLine] < MaxJukugoSize)
        if (const auto size{displaySize(list[i]->nameAndReading()) + 2};
            size > colWidths[i % JukugoPerLine])
          colWidths[i % JukugoPerLine] =
              size < MaxJukugoSize ? size : MaxJukugoSize;
    for (size_t i{}; i < list.size(); ++i) {
      if (i % JukugoPerLine == 0) out() << '\n';
      const auto s{list[i]->nameAndReading()};
      out() << std::left << std::setw(wideSetw(s, colWidths[i % JukugoPerLine]))
            << s;
    }
  }
  out() << '\n';
}

char QuizLauncher::chooseQuizType(OptChar quizType) const {
  return quizType ? *quizType : _choice.get("Type", QuizTypeChoices, 'g');
}

char QuizLauncher::chooseFreq(OptChar qList) const {
  return qList ? *qList
               : _choice.get(
                     FrequencyRange, "Choose frequency list", FrequencyChoices);
}

char QuizLauncher::chooseGrade(OptChar qList) const {
  return qList ? *qList
               : _choice.get(GradeRange, "Choose grade", GradeChoices, '6');
}

char QuizLauncher::chooseKyu(OptChar qList) const {
  return qList ? *qList : _choice.get(KyuRange, "Choose kyu", KyuChoices, '2');
}

char QuizLauncher::chooseLevel(OptChar qList) const {
  return qList ? *qList : _choice.get("Choose level", LevelChoices);
}

const KanjiData::List& QuizLauncher::getKyuList(char c) const {
  using enum KenteiKyus;
  return data()
      .kyus()[c == 'a'   ? K10
              : c == 'c' ? KJ2
              : c == '2' ? K2
              : c == 'b' ? KJ1
              : c == '1' ? K1
                         : AllKenteiKyus[to_underlying(K3) - (c - '3')]];
}

} // namespace kanji_tools
