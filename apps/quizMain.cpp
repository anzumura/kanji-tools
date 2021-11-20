#include <kanji_tools/kanji/KanjiData.h>
#include <kanji_tools/quiz/Quiz.h>

using namespace kanji_tools;

int main(int argc, const char** argv) {
  try {
    GroupData groupData(std::make_shared<KanjiData>(argc, argv));
    Quiz quiz(groupData);
    if (!groupData.data().debug()) quiz.quiz();
  } catch (const std::exception& err) {
    std::cerr << err.what() << '\n';
    return 1;
  }
  return 0;
}
