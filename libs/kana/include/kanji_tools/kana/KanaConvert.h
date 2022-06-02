#include <kanji_tools/kana/Choice.h>
#include <kanji_tools/kana/Converter.h>
#include <kanji_tools/utils/Args.h>

#include <filesystem>
#include <vector>

namespace kanji_tools {

// 'KanaConvert' provides a command line interface as well as an interactive
// mode for converting between Hiragana, Katakana and Rōmaji.
class KanaConvert {
public:
  // allow overriding in and out streams for testing
  explicit KanaConvert(
      const Args&, std::ostream& = std::cout, std::istream* = {});

  KanaConvert(const KanaConvert&) = delete;
private:
  using List = std::vector<String>;

  // 'error' throws an exception (used during processing of command line args)
  static void error(const String&);

  // 'processArg' returns true if 'arg' is recognized and processed, otherwise
  // it returns false meaning 'arg' should be treated as a string to convert
  [[nodiscard]] bool processArg(
      const String& arg, bool& printKana, bool& printMarkdown);

  [[nodiscard]] bool charTypeArgs(const String&);
  bool flagArgs(char);

  // 'usage' prints details about all command line args by default, but setting
  // 'showAllOptions' to false causes it to print a shorted message
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

} // namespace kanji_tools
