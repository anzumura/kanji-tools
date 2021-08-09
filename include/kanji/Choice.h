#ifndef KANJI_CHOICE_H
#define KANJI_CHOICE_H

#include <iostream>
#include <map>
#include <optional>
#include <string>

namespace kanji {

class Choice {
public:
  // 'Choices' should map 'char' choices to a description of the choice
  using Choices = std::map<char, std::string>;
  // By default choices are read from terminal input (without requiring 'return'), but a
  // istream can be passed in instead (good for testing).
  Choice(std::ostream& out, std::istream* in = 0) : _out(out), _in(in) {}

  // Provide special support for 'quitChoice'. If it has a value then it will be added to
  // the 'choices' provided to the below 'get' methods.
  std::optional<char> quit() const { return _quit; }
  void setQuit(char c) { _quit = c; }
  void clearQuit() { _quit = {}; }

  // 'get' will prompt the use to enter one of the choices in the 'choices' structure. If
  // an optional default choice is provided it must correspond to an entry in 'choices'.
  // If 'choices' contains two or more consecutive ascii values with empty descriptions
  // then they will be displayed as a range, i.e., 1-9, a-c, F-J, etc. - see ChoiceTest.cpp
  // for examples of how to use this class and expected output.
  char get(const std::string& msg, const Choices& choices) const { return get(msg, choices, {}); }
  char get(const std::string& msg, const Choices& choices, std::optional<char> def) const;
  // 'get' with ranges are convenience methods when there is a range with no descriptions
  char get(const std::string& msg, char first, char last) const { return get(msg, first, last, {}, {}); }
  char get(const std::string& msg, char first, char last, std::optional<char> def) const {
    return get(msg, first, last, {}, def);
  }
  char get(const std::string& msg, char first, char last, const Choices& choices) const {
    return get(msg, first, last, choices, {});
  }
  char get(const std::string& msg, char first, char last, const Choices& choices, std::optional<char> def) const {
    Choices c(choices);
    while (first <= last)
      c[first++] = "";
    return get(msg, c, def);
  }
private:
  static void add(std::string& prompt, const Choices& choices);
  static char getOneChar();
  std::ostream& _out;
  std::istream* _in;
  std::optional<char> _quit = {};
};

} // namespace kanji

#endif // KANJI_CHOICE_H
