#include <kanji_tools/kana/Converter.h>
#include <kanji_tools/utils/Args.h>
#include <kanji_tools/utils/Choice.h>

#include <filesystem>

namespace kanji_tools {

class KanaConvert {
public:
  KanaConvert(Args, std::ostream& = std::cout, std::istream& = std::cin);
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

  std::ostream& _out;
  std::istream& _in;
  bool _interactive{false}, _suppressNewLine{false};
  std::optional<CharType> _source{};
  std::vector<std::string> _strings;
  Converter _converter;
  const Choice _choice;
};

} // namespace kanji_tools
