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

  // 'get' will prompt the use to enter one of the choices in the 'choices' structure. If
  // an optional default choice is provided it must correspond to an entry in 'choices'.
  // If 'choices' contains two or more consecutive ascii values with empty descriptions
  // then they will be displayed as a range, i.e., 1-9, a-c, F-J, etc. - see ChoiceTest.cpp
  // for examples of how to use this class and expected output.
  char get(const std::string& msg, const Choices& choices) const { return get(msg, choices, {}); }
  char get(const std::string& msg, const Choices& choices, std::optional<char> def) const;
private:
  static void add(std::string& prompt, const Choices& choices);
  static char getOneChar();
  std::ostream& _out;
  std::istream* _in;
};

} // namespace kanji

#endif // KANJI_CHOICE_H
