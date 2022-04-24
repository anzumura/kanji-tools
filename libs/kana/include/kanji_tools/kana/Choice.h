#pragma once

#include <iostream>
#include <map>
#include <optional>
#include <string>

namespace kanji_tools {

// 'Choice' supports getting a choice from user input. By default choices are
// read from stdin (without requiring 'return'), but an istream can be also be
// used (helps testing). There's also support for 'default' choices (when just
// pressing 'return') and choice 'ranges' (see 'get' functions below as well as
// 'ChoiceTest.cpp' for examples of how to use this class and expected output).
class Choice {
public:
  inline static const std::string DefaultQuitDescription{"quit"};

  using OptChar = std::optional<char>;

  // 'Choices' should map 'char' choices to a description of the choice
  using Choices = std::map<char, std::string>;

  using Range = std::pair<char, char>;

  // There is no 'quit' option by default, but it can be specified in the
  // constructor or changed later via 'setQuit' and 'clearQuit' methods. An
  // 'istream' of '0' (the default) means read from stdin.
  explicit Choice(std::ostream& out, OptChar quit = {},
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

  // 'get' prompts for a choice from 'choices' map. Set 'useQuit' to false to
  // skip showing '_quit' value (has no effect if '_quit' isn't set). If 'def'
  // is provided it must be in 'choices', otherwise an exception is thrown. If
  // 'choices' contains two or more consecutive values with empty descriptions
  // then they are displayed as a range, i.e., 1-9, a-c, F-J, etc..
  [[nodiscard]] char get(
      const std::string& msg, bool useQuit, const Choices&, OptChar) const;

  // overloads that call the above 'get' function
  [[nodiscard]] char get(const std::string& msg, bool, const Choices&) const;
  [[nodiscard]] char get(const std::string& msg, const Choices&, OptChar) const;
  [[nodiscard]] char get(const std::string& msg, const Choices&) const;

  // alternative 'get' functions that also take an (inclusive) range of value
  [[nodiscard]] char get(const std::string& msg, bool useQuit, const Range&,
      const Choices&, OptChar def) const;

  // overloads that call the above 'get' function taking a range
  [[nodiscard]] char get(
      const std::string& msg, const Range&, const Choices&, OptChar) const;
  [[nodiscard]] char get(
      const std::string& msg, const Range&, const Choices&) const;
  [[nodiscard]] char get(const std::string& msg, const Range&) const;
  [[nodiscard]] char get(const std::string& msg, const Range&, OptChar) const;
private:
  [[nodiscard]] static char getOneChar();
  static void add(std::string& prompt, const Choices&);
  static void checkPrintableAscii(char x, const std::string& msg);
  static void error(const std::string&);

  std::ostream& _out;
  std::istream* _in;
  OptChar _quit{};
  std::string _quitDescription{DefaultQuitDescription};
};

} // namespace kanji_tools
