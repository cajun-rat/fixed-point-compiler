#include "fixed_point_compiler.h"

using namespace std;

ostream& operator<<(ostream& os, const Range& range) {
  os << "[" << range.min_ << ".." << range.max_ << "]";
  return os;
}

Range Range::operator*(const Range other) const {
  int64_t min_val, max_val;
  min_val = std::min({
      min_ * other.min_, min_ * other.max_, max_ * other.min_,
      max_ * other.max_,
  });
  max_val = std::max({
      min_ * other.min_, min_ * other.max_, max_ * other.min_,
      max_ * other.max_,
  });
  return Range(min_val, max_val);
}