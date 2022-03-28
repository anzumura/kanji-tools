#include <kanji_tools/utils/Choice.h>
#include <kanji_tools/utils/MBUtils.h>
#include <termios.h>
#include <unistd.h>

namespace kanji_tools {

namespace {

const std::string AlreadyInChoices{"' already in choices"};

} // namespace

char Choice::getOneChar() {
  struct termios settings {};
  if (tcgetattr(0, &settings) < 0) perror("tcsetattr()");
  // turn raw mode on - allows getting single char without waiting for 'return'
  settings.c_lflag &= ~static_cast<tcflag_t>(ICANON);
  settings.c_lflag &= ~static_cast<tcflag_t>(ECHO);
  settings.c_cc[VMIN] = 1;
  settings.c_cc[VTIME] = 0;
  if (tcsetattr(0, TCSANOW, &settings) < 0)
    perror("tcsetattr() - turning on raw mode");
  // read a single char
  char result{};
  if (read(0, &result, 1) < 0) perror("read()");
  // turn raw mode off
  settings.c_lflag |= ICANON;
  settings.c_lflag |= ECHO;
  if (tcsetattr(0, TCSADRAIN, &settings) < 0)
    perror("tcsetattr() - turning off raw mode");
  return result;
}

void Choice::add(std::string& prompt, const Choices& choices) {
  static const std::string CommaSpace{", "}, Equals{"="}, Dash{"-"};

  OptChar rangeStart;
  char prevChar;
  const auto completeRange{[&prompt, &rangeStart, &prevChar]() {
    if (rangeStart != prevChar) prompt += Dash + prevChar;
  }};
  for (auto& i : choices) {
    checkPrintableAscii(i.first, "option");
    if (i.second.empty()) {
      if (!rangeStart) {
        if (i != *choices.begin()) prompt += CommaSpace;
        prompt += i.first;
        rangeStart = i.first;
      } else if (i.first - prevChar > 1) {
        // complete range if there was a jump of more than one value
        completeRange();
        prompt += CommaSpace + i.first;
        rangeStart = i.first;
      }
    } else {
      // second value isn't empty so complete any ranges if needed
      if (rangeStart) {
        completeRange();
        rangeStart = {};
      }
      if (i.first != choices.begin()->first) prompt += CommaSpace;
      prompt += i.first + Equals + i.second;
    }
    prevChar = i.first;
  }
  if (rangeStart) completeRange();
}

char Choice::get(const std::string& msg, bool useQuit, const Choices& choicesIn,
    OptChar def) const {
  static const std::string QuitError{"quit option '"},
      DefaultError{"default option '"}, DefaultPrompt{") def '"};

  auto choices{choicesIn};
  if (_quit && (useQuit ? !choices.emplace(*_quit, _quitDescription).second
                        : choices.contains(*_quit)))
    error(QuitError + *_quit + AlreadyInChoices);
  if (choices.empty()) error("must specify at least one choice");

  // if 'msg' is empty then don't leave space before listing choices in brackets
  std::string line, prompt{msg + (msg.empty() ? "(" : " (")};

  add(prompt, choices);
  if (def) {
    if (!choices.contains(*def))
      error(DefaultError + *def + "' not in choices");
    prompt += DefaultPrompt + *def + "': ";
  } else
    prompt += "): ";
  do {
    _out << prompt;
    _out.flush();
    if (_in)
      std::getline(*_in, line);
    else {
      if (const auto choice{getOneChar()}; choice == '\n')
        line.clear();
      else
        line = choice;
      _out << '\n';
    }
    if (line.empty() && def) return *def;
  } while (line.size() != 1 || choices.find(line[0]) == choices.end());
  return line[0];
}

char Choice::get(const std::string& msg, bool useQuit, const Range& range,
    const Choices& choicesIn, OptChar def) const {
  static const std::string RangeError{"range option"};
  static const std::string FirstError{"first " + RangeError},
      LastError{"last " + RangeError};

  checkPrintableAscii(range.first, FirstError);
  checkPrintableAscii(range.second, LastError);
  if (range.first > range.second)
    error(FirstError + " '" + range.first + "' is greater than last '" +
          range.second + "'");
  auto choices{choicesIn};
  for (auto i{range.first}; i <= range.second; ++i)
    if (!choices.emplace(i, EmptyString).second)
      error(RangeError + " '" + i + AlreadyInChoices);
  return get(msg, useQuit, choices, def);
}

void Choice::checkPrintableAscii(char x, const std::string& msg) {
  if (x < ' ' || x > '~') error(msg + " is non-printable: 0x" + toHex(x));
}

} // namespace kanji_tools
