#include <kanji_tools/kana/Converter.h>
#include <kanji_tools/utils/Args.h>
#include <kanji_tools/utils/Choice.h>

#include <filesystem>

namespace kanji_tools {

class KanaConvert {
public:
  KanaConvert(Args, std::istream& = std::cin, std::ostream& = std::cout);
private:
  // 'error' throws an exception (used during processing of command line args)
  static void error(const std::string&);

  [[nodiscard]] bool charTypeArgs(const std::string&);
  bool flagArgs(char);

  // 'usage' prints details about all command line args by default, but setting
  // 'showAllOptions' to false causes it to print a shorted message
  void usage(bool showAllOptions = true) const;

  void start();
  void getInput();
  void processOneLine(const std::string&);
  void setFlag(ConvertFlags);
  void printKanaChart(bool markdown = false) const;

  std::istream& _in;
  std::ostream& _out;
  bool _interactive{false}, _suppressNewLine{false};
  std::optional<CharType> _source{};
  std::vector<std::string> _strings;
  Converter _converter;
  const Choice _choice;
  const std::string _program;
};

} // namespace kanji_tools
