#include <kanji/Quiz.h>
#include <kanji/KanjiData.h>

using namespace kanji;

int main(int argc, const char** argv) {
  try {
    GroupData groupData(std::make_shared<KanjiData>(argc, argv));
    Quiz quiz(groupData);
    quiz.quiz();
  } catch(const std::exception& err) {
    std::cerr << err.what() << '\n';
    return 1;
  }
  return 0;
}
