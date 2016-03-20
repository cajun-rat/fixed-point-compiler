#include "fixed_point_compiler.h"

using namespace std;

ostream& operator<<(ostream& os, const Scale scale) {
  os << "Q" << scale.shift_;
  return os;
}

Scale add_shifts(const Scale a, const Scale b) {
  return Scale(a.shift_ + b.shift_);
}