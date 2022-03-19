#pragma once

#include <iostream>
#include <map>
#include <optional>
#include <string>

namespace kanji_tools {

// 'Choice' is a class that supports presenting options and getting back a
// choice from user input. By default choices are read from terminal input
// (without requiring 'return'), but a istream can be passed in instead (good
// for testing). There is also support for 'default' choices (choice when just
// pressing 'return') and choice 'ranges' (see 'get' functions below).
class Choice {
public:
  inline static const std::string DefaultQuitDescription{"quit"};

  using OptChar = std::optional<char>;

  // 'Choices' should map 'char' choices to a description of the choice
  using Choices = std::map<char, std::string>;

  // There is no 'quit' option by default, but it can be specified in the
  // constructor or changed later via 'setQuit' and 'clearQuit' methods. An
  // 'istream' of '0' (the default) means read from stdin.
  Choice(std::ostream& out, OptChar quit = {},
      const std::string& d = DefaultQuitDescription)
      : Choice{out, nullptr, quit, d} {}
  Choice(std::ostream& out, std::istream* in, OptChar quit = {},
      const std::string& d = DefaultQuitDescription)
      : _out{out}, _in{in} {
    if (quit) setQuit(*quit, d);
  }

  Choice(const Choice&) = delete;
  Choice& operator=(const Choice&) = delete;

  // Provide support for a '_quitOption' choice instead of needing to specify it
  // every time when. If it has a value then it will be added to the 'choices'
  // provided to the below 'get' methods.
  void setQuit(char c, const std::string& d = DefaultQuitDescription) {
    checkPrintableAscii(c, "quit option");
    _quit = c;
    _quitDescription = d;
  }
  void clearQuit() { _quit = {}; }

  [[nodiscard]] auto isQuit(char c) const { return _quit == c; }
  [[nodiscard]] auto quit() const { return _quit; }
  [[nodiscard]] auto& quitDescription() const { return _quitDescription; }

  // 'get' prompts for one of the choices in the 'choices' structure. If a
  // default is provided it must correspond to an entry in 'choices', otherwise
  // an exception is thrown. If 'choices' contains two or more consecutive
  // values with empty descriptions then they will be displayed as a range,
  // i.e., 1-9, a-c, F-J, etc. - see ChoiceTest.cpp for examples of how to use
  // this class and expected output. 'useQuit' can be set to false to skip
  // providing '_quit' value (has no effect if '_quit' isn't set).
  char get(const std::string& msg, bool useQuit, const Choices& choices,
      OptChar def) const;
  auto get(const std::string& msg, bool useQuit, const Choices& choices) const {
    return get(msg, useQuit, choices, {});
  }
  auto get(const std::string& msg, const Choices& choices, OptChar def) const {
    return get(msg, true, choices, def);
  }
  auto get(const std::string& msg, const Choices& choices) const {
    return get(msg, choices, {});
  }

  // 'get' with ranges are convenience methods when there is a range (inclusive)
  // with no descriptions
  char get(const std::string& msg, bool useQuit, char first, char last,
      const Choices& choices, OptChar def) const;
  auto get(const std::string& msg, char first, char last,
      const Choices& choices, OptChar def) const {
    return get(msg, true, first, last, choices, def);
  }
  auto get(const std::string& msg, char first, char last,
      const Choices& choices) const {
    return get(msg, first, last, choices, {});
  }
  auto get(const std::string& msg, char first, char last) const {
    return get(msg, first, last, {}, {});
  }
  auto get(const std::string& msg, char first, char last, OptChar def) const {
    return get(msg, first, last, {}, def);
  }
private:
  static void add(std::string& prompt, const Choices& choices);
  [[nodiscard]] static char getOneChar();
  static void checkPrintableAscii(char x, const std::string& msg);
  static void error(const std::string& msg) { throw std::domain_error(msg); }

  std::ostream& _out;
  std::istream* _in;
  OptChar _quit{};
  std::string _quitDescription{DefaultQuitDescription};
};

} // namespace kanji_tools
