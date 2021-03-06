#include <kt_kana/Choice.h>
#include <kt_utils/Exception.h>

#include <termios.h>
#include <unistd.h>

namespace kanji_tools {

namespace {

const String AlreadyInChoices{"' already in choices"};

} // namespace

Choice::Choice(std::ostream& out, OptChar quit, const String& d)
    : Choice{out, nullptr, quit, d} {}

Choice::Choice(
    std::ostream& out, std::istream* in, OptChar quit, const String& d)
    : _out{out}, _in{in} {
  if (quit) setQuit(*quit, d);
}

void Choice::setQuit(char c, const String& d) {
  checkPrintableAscii(c, "quit option");
  _quit = c;
  _quitDescription = d;
}

void Choice::clearQuit() { _quit = {}; }

char Choice::get(const String& msg, bool useQuit, const Choices& choices,
    OptChar def) const {
  static const String QuitError{"quit option '"},
      DefaultError{"default option '"}, DefaultPrompt{") def '"};

  auto choicesOut{choices};
  if (_quit && (useQuit ? !choicesOut.emplace(*_quit, _quitDescription).second
                        : choicesOut.contains(*_quit)))
    error(QuitError + *_quit + AlreadyInChoices);
  if (choicesOut.empty()) error("must specify at least one choice");

  // if 'msg' is empty then don't leave space before listing choices in brackets
  String line, prompt{msg + (msg.empty() ? "(" : " (")};

  add(prompt, choicesOut);
  if (def) {
    if (!choicesOut.contains(*def))
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
      // XCOV_EXCL_START - code only used in interactive mode
      if (const auto choice{getOneChar()}; choice == '\n')
        line.clear();
      else
        line = choice;
      _out << '\n';
      // XCOV_EXCL_STOP
    }
    if (line.empty() && def) return *def;
  } while (line.size() != 1 || choicesOut.find(line[0]) == choicesOut.end());
  return line[0];
}

char Choice::get(
    const String& msg, bool useQuit, const Choices& choices) const {
  return get(msg, useQuit, choices, {});
}

char Choice::get(const String& msg, const Choices& choices, OptChar def) const {
  return get(msg, true, choices, def);
}

char Choice::get(const String& msg, const Choices& choices) const {
  return get(msg, choices, {});
}

char Choice::get(Range range, const String& msg, bool useQuit,
    const Choices& choices, OptChar def) const {
  static const String RangeError{"range option"};
  static const String FirstError{"first " + RangeError},
      LastError{"last " + RangeError};

  checkPrintableAscii(range.first, FirstError);
  checkPrintableAscii(range.second, LastError);
  if (range.first > range.second)
    error(FirstError + " '" + range.first + "' is greater than last '" +
          range.second + "'");
  auto choicesOut{choices};
  for (auto i{range.first}; i <= range.second; ++i)
    if (!choicesOut.emplace(i, emptyString()).second)
      error((RangeError + " '") += (i + AlreadyInChoices));
  return get(msg, useQuit, choicesOut, def);
}

char Choice::get(
    Range range, const String& msg, const Choices& choices, OptChar def) const {
  return get(range, msg, true, choices, def);
}

char Choice::get(Range range, const String& msg, const Choices& choices) const {
  return get(range, msg, choices, {});
}

char Choice::get(Range range, const String& msg) const {
  return get(range, msg, {}, {});
}

char Choice::get(Range range, const String& msg, OptChar def) const {
  return get(range, msg, {}, def);
}

// XCOV_EXCL_START - this code requires user input (so not covered by tests)
char Choice::getOneChar() {
  static constexpr tcflag_t Icanon{ICANON}, Echo{ECHO};
  struct termios settings {};
  if (tcgetattr(0, &settings) < 0) perror("tcsetattr()");
  // turn raw mode on - allows getting single char without waiting for 'return'
  settings.c_lflag &= ~Icanon;
  settings.c_lflag &= ~Echo;
  settings.c_cc[VMIN] = 1;
  settings.c_cc[VTIME] = 0;
  if (tcsetattr(0, TCSANOW, &settings) < 0)
    perror("tcsetattr() - turning on raw mode");
  // read a single char
  char result{};
  if (read(0, &result, 1) < 0) perror("read()");
  // turn raw mode off
  settings.c_lflag |= Icanon;
  settings.c_lflag |= Echo;
  if (tcsetattr(0, TCSADRAIN, &settings) < 0)
    perror("tcsetattr() - turning off raw mode");
  return result;
}
// XCOV_EXCL_STOP

void Choice::add(String& prompt, const Choices& choices) {
  static const String CommaSpace{", "}, Equals{"="}, Dash{"-"};

  OptChar rangeStart;
  char prevChar{};
  const auto completeRange{[&prompt, &rangeStart, &prevChar] {
    if (rangeStart != prevChar) prompt += Dash + prevChar; // NOLINT
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

void Choice::checkPrintableAscii(char x, const String& msg) {
  if (x < ' ' || x > '~') error(msg + " is non-printable: 0x" + toHex(x));
}

void Choice::error(const String& msg) { throw DomainError(msg); }

} // namespace kanji_tools
