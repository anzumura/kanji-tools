#pragma once

#include <kanji_tools/utils/String.h>

#include <iostream>
#include <map>
#include <optional>

namespace kanji_tools { /// \kana_group{Choice}
/// Choice class for getting choices from user input

/// class for getting choices from user input \kana{Choice}
///
/// By default choices are read from stdin (without requiring 'return'), but an
/// `istream` can be also be used (helps testing). There's also support for
/// 'default' choices (just pressing 'return') and choice 'ranges' (see 'get'
/// functions below as well as 'ChoiceTest.cpp' for examples of how to use this
/// class and expected output).
class Choice {
public:
  /// default description to be used if a 'quit option' has been specified
  inline static const String DefaultQuitDescription{"quit"};

  /// map `char` choices to a description of the choice
  using Choices = std::map<char, String>;

  /// optional `char` used for passing in default choices to 'get' functions
  using OptChar = std::optional<char>;

  /// used for passing in choice ranges to 'get' functions
  using Range = std::pair<char, char>;

  /// no `quit` option by default, but it can be specified here or changed later
  /// via setQuit() and clearQuit() methods
  /// \throw DomainError if `quit` is set and is not printable Ascii
  explicit Choice(
      std::ostream&, OptChar quit = {}, const String& = DefaultQuitDescription);

  /// setting `istream` to `0` means read from stdin
  /// \throw DomainError if `quit` is set and is not printable Ascii
  Choice(std::ostream&, std::istream*, OptChar quit = {},
      const String& = DefaultQuitDescription);

  Choice(const Choice&) = delete;         ///< deleted copy ctor
  auto operator=(const Choice&) = delete; ///< deleted operator=

  /// assign `c` to be the 'quit option' so that it doesn't need to be specified
  /// every time 'get' is called
  /// \throw DomainError if `c` is not printable Ascii
  void setQuit(char c, const String& = DefaultQuitDescription);

  /// clear any assigned 'quit option'
  void clearQuit();

  /// return true if `c` is the same as the assigned 'quit option'
  [[nodiscard]] auto isQuit(char c) const { return _quit == c; }

  /// return the currently assigned 'quit option' (can be std::nullopt)
  [[nodiscard]] auto quit() const { return _quit; }

  /// return the currently assigned 'quit description' string
  [[nodiscard]] auto& quitDescription() const { return _quitDescription; }

  /// read in a choice from `istream` provided in the ctor
  /// \param msg beginning part of prompt message written to `ostream` that was
  ///     specifed in the ctor, the rest of the prompt message shows the choices
  /// \param useQuit if false then 'quit option' is not included in the choices
  /// \param choices set of choices the user must choose from
  /// \param def optional default choice (for just pressing return)
  /// \return one of the choices from `choices` or possibly the 'quit option'
  /// \throw DomainError if `def` is provided, but it's not in `choicesIn`
  /// \throw DomainError if 'quit option' has been set and is also in `choices`
  /// \throw DomainError if any choice in `choices` is not printable Ascii
  [[nodiscard]] char get(const String& msg, bool useQuit,
      const Choices& choices, OptChar def) const;

  /// overloads that call the above get() function @{
  [[nodiscard]] char get(const String&, bool, const Choices&) const;
  [[nodiscard]] char get(const String&, const Choices&, OptChar) const;
  [[nodiscard]] char get(const String&, const Choices&) const; ///@}

  /// alternative get() function that also takes an (inclusive) range of values
  /// \param range pair of choices to use as a range like '1-9' or 'a-f'
  /// \details \copydetails get()
  /// \throw DomainError if `range.first` > `range.second` or if any choice in
  ///     `range` is not printable Ascii or is also included in `choices`
  [[nodiscard]] char get(Range range, const String& msg, bool useQuit,
      const Choices& choices, OptChar def) const;

  /// overloads that call the above get() function taking a range @{
  [[nodiscard]] char get(Range, const String&, const Choices&, OptChar) const;
  [[nodiscard]] char get(Range, const String&, const Choices&) const;
  [[nodiscard]] char get(Range, const String&) const;
  [[nodiscard]] char get(Range, const String&, OptChar) const; ///@}
private:
  [[nodiscard]] static char getOneChar();
  static void add(String& prompt, const Choices&);
  static void checkPrintableAscii(char x, const String& msg);
  static void error(const String&);

  std::ostream& _out;
  std::istream* _in;
  OptChar _quit{};
  String _quitDescription{DefaultQuitDescription};
};

/// \end_group
} // namespace kanji_tools
