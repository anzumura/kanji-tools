#include <kanji_tools/kanji/MorohashiId.h>

namespace kanji_tools {

namespace {

[[nodiscard]] auto isDoublePrime(const std::string& s) {
  return s.ends_with("PP") || s.ends_with("''");
}

[[nodiscard]] auto isPrime(const std::string& s) {
  return s.ends_with("P") || s.ends_with("'");
}

[[nodiscard]] auto isSupplemental(const std::string& s) {
  return s.starts_with("H");
}

} // namespace

MorohashiId::MorohashiId(const std::string& s)
    : _id{getId(s)}, _idType{getIdType(s)} {}

MorohashiId::Id MorohashiId::getId(const std::string& s) {
  if (isDoublePrime(s)) return validate(s, 0, 2);
  if (isPrime(s)) return validate(s, 0, 1);
  if (isSupplemental(s)) return validate(s, 1);
  return validate(s);
}

MorohashiId::IdType MorohashiId::getIdType(const std::string& s) {
  if (isDoublePrime(s)) return IdType::DoublePrime;
  if (isPrime(s)) return IdType::Prime;
  if (isSupplemental(s)) return IdType::Supplemental;
  return IdType::None;
}

MorohashiId::Id MorohashiId::validate(
    const std::string& s, size_t start, size_t end) {
  if (s.empty() && !start && !end) return Id{};
  if (s.size() - start - end == 0) throw std::domain_error{"invalid Id: " + s};
  Id result{};
  const auto typedId{start || end};
  for (auto i = start; i < s.size() - end; ++i)
    if (i == start && s[i] == '0')
      ++start;
    else if (s[i] < '0' || s[i] > '9')
      throw std::domain_error{"non-numeric Id: " + s};
    else if (const Id x{static_cast<Id>(s[i] - '0')};
             result > MaxId / 10 || (result *= 10) > MaxId - x)
      throw std::domain_error{"Id exceeds max: " + s};
    else
      result += x;
  if (typedId && !result)
    throw std::domain_error{"typed Id can't be zero: " + s};
  return result;
}

std::string MorohashiId::toString() const {
  std::string result;
  if (_id) {
    if (_idType == IdType::Supplemental) result = "H";
    result += std::to_string(_id);
    if (_idType == IdType::Prime)
      result += 'P';
    else if (_idType == IdType::DoublePrime)
      result += "PP";
  }
  return result;
}

std::ostream& operator<<(std::ostream& os, const MorohashiId& x) {
  return os << x.toString();
}

} // namespace kanji_tools
