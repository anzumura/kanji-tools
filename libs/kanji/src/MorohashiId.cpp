#include <kt_kanji/MorohashiId.h>
#include <kt_utils/Exception.h>

namespace kanji_tools {

namespace {

constexpr auto PrimeSuffix{'P'}, AltPrimeSuffix{'\''}, SupplementalPrefix{'H'};

const String DoublePrimeSuffix{"PP"}, AltDoublePrimeSuffix{"''"};

[[nodiscard]] auto isDoublePrime(const String& s) {
  return s.ends_with(DoublePrimeSuffix) || s.ends_with(AltDoublePrimeSuffix);
}

[[nodiscard]] auto isPrime(const String& s) {
  return s.ends_with(PrimeSuffix) || s.ends_with(AltPrimeSuffix);
}

[[nodiscard]] auto isSupplemental(const String& s) {
  return s.starts_with(SupplementalPrefix);
}

} // namespace

MorohashiId::MorohashiId(const String& s)
    : _id{getId(s)}, _idType{getIdType(s)} {}

MorohashiId::Id MorohashiId::getId(const String& s) {
  if (isDoublePrime(s)) return validate(s, 0, 2);
  if (isPrime(s)) return validate(s, 0, 1);
  if (isSupplemental(s)) return validate(s, 1);
  return validate(s);
}

MorohashiId::IdType MorohashiId::getIdType(const String& s) {
  if (isDoublePrime(s)) return IdType::DoublePrime;
  if (isPrime(s)) return IdType::Prime;
  if (isSupplemental(s)) return IdType::Supplemental;
  return IdType::Plain;
}

MorohashiId::Id MorohashiId::validate(
    const String& s, size_t prefixSize, size_t suffixSize) {
  static constexpr auto CharZero{'0'}, CharNine{'9'};
  static constexpr Id Ten{10};

  const auto error{[&s](const String& msg) {
    throw DomainError{"Morohashi ID '" + s + "' " + msg};
  }};

  Id result{};
  if (s.empty() && !prefixSize && !suffixSize) return result;
  if (s.size() - prefixSize - suffixSize == 0) error("is invalid");
  const auto nonPlain{prefixSize || suffixSize};
  // Id (ignoring any prefix or suffix) must be numeric and not exceed MaxId
  for (auto i = prefixSize; i < s.size() - suffixSize; ++i)
    if (i == prefixSize && s[i] == CharZero)
      ++prefixSize; // skip leading zeroes
    else if (s[i] < CharZero || s[i] > CharNine)
      error("is non-numeric");
    else if (const Id x{static_cast<Id>(s[i] - CharZero)};
             result > MaxId / Ten || (result *= Ten) > MaxId - x)
      error("exceeds max");
    else
      result += x;
  // Unicode currently has a few (bad) entries that consist of all zeroes so
  // allow them for now, but don't allow a non-plain Id to be zero.
  if (nonPlain && !result) error("can't be zero");
  return result;
}

String MorohashiId::toString() const {
  String result{_id ? std::to_string(_id) : EmptyString};
  switch (_idType) {
  case IdType::Prime: result += PrimeSuffix; break;
  case IdType::DoublePrime: result += DoublePrimeSuffix; break;
  case IdType::Supplemental: result = SupplementalPrefix + result; break;
  case IdType::Plain: break;
  }
  return result;
}

std::ostream& operator<<(std::ostream& os, const MorohashiId& x) {
  return os << x.toString();
}

} // namespace kanji_tools
