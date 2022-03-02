#include <kanji_tools/quiz/QuizLauncher.h>

using namespace kanji_tools;

int main(int argc, const char** argv) {
  try {
    QuizLauncher::run(static_cast<size_t>(argc), argv);
  } catch (const std::exception& err) {
    std::cerr << err.what() << '\n';
    return 1;
  }
  return 0;
}
