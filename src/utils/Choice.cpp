#include <kanji_tools/utils/Choice.h>

#include <cassert>

#include <termios.h>
#include <unistd.h>

namespace kanji_tools {

char Choice::getOneChar() {
  char result = 0;
  struct termios settings = {0};
  if (tcgetattr(0, &settings) < 0) perror("tcsetattr()");
  settings.c_lflag &= ~ICANON;
  settings.c_lflag &= ~ECHO;
  settings.c_cc[VMIN] = 1;
  settings.c_cc[VTIME] = 0;
  if (tcsetattr(0, TCSANOW, &settings) < 0) perror("tcsetattr() - turning on raw mode");
  if (read(0, &result, 1) < 0) perror("read()");
  settings.c_lflag |= ICANON;
  settings.c_lflag |= ECHO;
  if (tcsetattr(0, TCSADRAIN, &settings) < 0) perror("tcsetattr() - turning off raw mode");
  return result;
}

void Choice::add(std::string& prompt, const Choices& choices) {
  OptChar rangeStart = {};
  char prevChar;
  auto completeRange = [&prompt, &rangeStart, &prevChar]() {
    if (prevChar != *rangeStart) {
      prompt += '-';
      prompt += prevChar;
    }
  };
  for (const auto& i : choices) {
    if (i.second.empty()) {
      if (!rangeStart) {
        if (i != *choices.begin()) prompt += ", ";
        prompt += i.first;
        rangeStart = i.first;
      } else if (i.first - prevChar > 1) {
        // complete range if there was a jump of more than one value
        completeRange();
        prompt += ", ";
        prompt += i.first;
        rangeStart = i.first;
      }
    } else {
      // second value isn't empty so complete any ranges if needed
      if (rangeStart) {
        completeRange();
        rangeStart = std::nullopt;
      }
      if (i.first != choices.begin()->first) prompt += ", ";
      prompt += i.first;
      prompt += "=" + i.second;
    }
    prevChar = i.first;
  }
  if (rangeStart) completeRange();
}

char Choice::get(const std::string& msg, bool useQuit, const Choices& choicesIn, OptChar def) const {
  // if 'msg' is empty then don't leave a space before listing the choices in brackets.
  std::string line, prompt(msg + (msg.empty() ? "(" : " ("));
  Choices choices(choicesIn);
  if (useQuit && _quit && choices.find(*_quit) == choices.end()) choices[*_quit] = "quit";
  add(prompt, choices);
  if (def) {
    assert(choices.find(*def) != choices.end());
    prompt += ") def '";
    prompt += *def;
    prompt += "': ";
  } else
    prompt += "): ";
  do {
    _out << prompt;
    _out.flush();
    if (_in)
      std::getline(*_in, line);
    else {
      char choice = getOneChar();
      if (choice == '\n')
        line.clear();
      else
        line = choice;
      _out << '\n';
    }
    if (line.empty() && def) return *def;
  } while (line.length() != 1 || choices.find(line[0]) == choices.end());
  return line[0];
}

} // namespace kanji_tools
