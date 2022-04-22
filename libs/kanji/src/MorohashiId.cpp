#include <kanji_tools/kanji/MorohashiId.h>

namespace kanji_tools {

namespace {

constexpr auto PrimeSuffix{'P'}, AltPrimeSuffix{'\''}, SupplementalPrefix{'H'};

const std::string DoublePrimeSuffix{"PP"}, AltDoublePrimeSuffix{"''"};

[[nodiscard]] auto isDoublePrime(const std::string& s) {
  return s.ends_with(DoublePrimeSuffix) || s.ends_with(AltDoublePrimeSuffix);
}

[[nodiscard]] auto isPrime(const std::string& s) {
  return s.ends_with(PrimeSuffix) || s.ends_with(AltPrimeSuffix);
}

[[nodiscard]] auto isSupplemental(const std::string& s) {
  return s.starts_with(SupplementalPrefix);
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
  return IdType::Regular;
}

MorohashiId::Id MorohashiId::validate(
    const std::string& s, size_t start, size_t end) {
  static constexpr auto CharZero{'0'}, CharNine{'9'};
  static constexpr MorohashiId::Id Ten{10};

  if (s.empty() && !start && !end) return Id{};
  if (s.size() - start - end == 0) throw std::domain_error{"invalid Id: " + s};
  Id result{};
  const auto typedId{start || end};
  for (auto i = start; i < s.size() - end; ++i)
    if (i == start && s[i] == CharZero)
      ++start;
    else if (s[i] < CharZero || s[i] > CharNine)
      throw std::domain_error{"non-numeric Id: " + s};
    else if (const Id x{static_cast<Id>(s[i] - CharZero)};
             result > MaxId / Ten || (result *= Ten) > MaxId - x)
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
    if (_idType == IdType::Supplemental) result = SupplementalPrefix;
    result += std::to_string(_id);
    if (_idType == IdType::Prime)
      result += PrimeSuffix;
    else if (_idType == IdType::DoublePrime)
      result += DoublePrimeSuffix;
  }
  return result;
}

std::ostream& operator<<(std::ostream& os, const MorohashiId& x) {
  return os << x.toString();
}

} // namespace kanji_tools
