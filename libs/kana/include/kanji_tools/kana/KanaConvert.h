#include <kanji_tools/kana/Choice.h>
#include <kanji_tools/kana/Converter.h>
#include <kanji_tools/utils/Args.h>

#include <filesystem>
#include <vector>

namespace kanji_tools { /// \kana_group{KanaConvert}
/// KanaConvert class used by 'kanaConvert' `main` function

/// provides a command line interface and interactive mode for converting
/// between Hiragana, Katakana and Rōmaji \kana{KanaConvert}
class KanaConvert {
public:
  /// allow overriding in and out streams for testing
  explicit KanaConvert(
      const Args&, std::ostream& = std::cout, std::istream* = {});

  KanaConvert(const KanaConvert&) = delete; ///< deleted copy ctor
private:
  using List = std::vector<String>;

  /// helper function for handling errors during processing of command line args
  /// \throw DomainError
  static void error(const String&);

  /// return true if `arg` is recognized and processed, otherwise returns false
  /// which means `arg` should be treated as a string to convert
  [[nodiscard]] bool processArg(
      const String& arg, bool& printKana, bool& printMarkdown);

  [[nodiscard]] bool charTypeArgs(const String&);
  bool flagArgs(char);

  /// print details about command line args
  /// \param showAllOptions set to false to print a shorter usage message which
  ///     is used during interactive mode
  void usage(bool showAllOptions = true) const;

  void start(const List& = {});

  void getInput();
  void printOptions() const;
  [[nodiscard]] bool processLine(const String&);
  void convert(const String&);
  void setFlag(ConvertFlags);

  void printKanaChart(bool markdown = false) const;

  std::ostream& _out;
  std::istream* _in;
  bool _interactive{false}, _suppressNewLine{false};
  std::optional<CharType> _source{};
  Converter _converter;
  const Choice _choice;
};

/// \end_group
} // namespace kanji_tools
