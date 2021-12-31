#ifndef KANJI_TOOLS_UTILS_CHOICE_H
#define KANJI_TOOLS_UTILS_CHOICE_H

#include <iostream>
#include <map>
#include <optional>
#include <string>

namespace kanji_tools {

class Choice {
public:
  using OptChar = std::optional<char>;

  // 'Choices' should map 'char' choices to a description of the choice
  using Choices = std::map<char, std::string>;

  // By default choices are read from terminal input (without requiring 'return'), but a
  // istream can be passed in instead (good for testing).
  Choice(std::ostream& out, std::istream* in = 0, OptChar quit = std::nullopt) : _out(out), _in(in) {
    if (quit) setQuit(*quit);
  }

  // Provide special support for '_quit' choice. If it has a value then it will be added to
  // the 'choices' provided to the below 'get' methods with a description of "quit".
  OptChar quit() const { return _quit; }
  void setQuit(char c) {
    checkPrintableAscii(c, "quit option");
    _quit = c;
  }
  void clearQuit() { _quit = {}; }
  bool isQuit(char c) const { return _quit == c; }

  // 'get' prompts for one of the choices in the 'choices' structure. If a default choice is provided
  // it must correspond to an entry in 'choices', otherwise an exception is thrown. If 'choices' contains
  // two or more consecutive ascii values with empty descriptions then they will be displayed as a range,
  // i.e., 1-9, a-c, F-J, etc. - see ChoiceTest.cpp for examples of how to use this class and expected output.
  // 'useQuit' can be set to false to skip providing '_quit' value (has no effect if '_quit' isn't set).
  char get(const std::string& msg, const Choices& choices) const { return get(msg, choices, {}); }
  char get(const std::string& msg, const Choices& choices, OptChar def) const { return get(msg, true, choices, def); }
  char get(const std::string& msg, bool useQuit, const Choices& choices) const {
    return get(msg, useQuit, choices, {});
  }
  char get(const std::string& msg, bool useQuit, const Choices& choices, OptChar def) const;

  // 'get' with ranges are convenience methods when there is a range (inclusive) with no descriptions
  char get(const std::string& msg, char first, char last) const { return get(msg, first, last, {}, {}); }
  char get(const std::string& msg, char first, char last, OptChar def) const { return get(msg, first, last, {}, def); }
  char get(const std::string& msg, char first, char last, const Choices& choices) const {
    return get(msg, first, last, choices, {});
  }
  char get(const std::string& msg, char first, char last, const Choices& choices, OptChar def) const {
    return get(msg, true, first, last, choices, def);
  }
  char get(const std::string& msg, bool useQuit, char first, char last, const Choices& choices, OptChar def) const;
private:
  static void add(std::string& prompt, const Choices& choices);
  static char getOneChar();
  static void checkPrintableAscii(char x, const std::string& msg);
  static void error(const std::string& msg) { throw std::domain_error(msg); }

  std::ostream& _out;
  std::istream* _in;
  OptChar _quit = {};
};

} // namespace kanji_tools

#endif // KANJI_TOOLS_UTILS_CHOICE_H
