#include <string>
#include "fixed_point_compiler.h"

using namespace std;

std::ostream& operator<<(std::ostream& o, const Var& v) {
  v.write_c_code(o);
  return o;
}

void write_shift(std::ostream& os, std::string& name, int shift);
int64_t do_shift(int64_t value, int shift);

Const::Const(string name, double value) : Var(name), double_value_(value) {
  int shift = 0;
  assert(std::numeric_limits<target_word>::min() <= value &&
         value <= std::numeric_limits<target_word>::max());
  while ((double)std::numeric_limits<target_word>::min() <= value * 2 &&
         value * 2 <= (double)numeric_limits<target_word>::max()) {
    value = value * 2;
    shift++;
  }
  value_ = (target_word)value;
  range_ = Range(min(0, (int)value_), value_);
  scale_ = Scale(shift);
}

void Const::write_c_code(ostream& o) const {
  o << name() << " = " << (int64_t)value_ << ";";
  o << " /* " << double_value_ << " " << scale_ << " " << range_ << " */";
}

void Load::write_c_code(ostream& o) const {
  o << name() << " = " << c_exp_ << ";";
  o << " /* " << scale_ << " " << range_ << " */";
}

void Store::write_c_code(ostream& o) const {
  o << name() << " = " << value_->name() << ";";
  o << " /* " << scale_ << " " << range_ << " */";
}

Add::Add(string name, VarR left, VarR right)
    : Var(name), left_(left), right_(right), lhs_shift_(0), rhs_shift_(0) {
  // line the binary points up
  Scale sl = left->scale();
  Range rl = left->range();
  Scale sr = right->scale();
  Range rr = right->range();
  while (sl < sr) {
    sl = sl << 1;
    rl = rl * 2;
    lhs_shift_++;
  }
  while (sr < sl) {
    sr = sr << 1;
    rr = rr * 2;
    rhs_shift_++;
  }
  assert(sr == sl);
  Scale result_scale = sr;
  Range result_range = rl + rr;
  while (!result_range.fits<target_word>()) {
    lhs_shift_--;
    rhs_shift_--;
    result_range = result_range / 2;
    result_scale = result_scale >> 1;
  }
  range_ = result_range;
  scale_ = result_scale;

  // The binary points line up
  assert(range_.fits<target_word>());
  assert(left->scale() << lhs_shift_ == right->scale() << rhs_shift_);
}

void Add::write_c_code(ostream& o) const {
  o << name() << " = ";
  write_shift(o, left_->name(), lhs_shift_);
  o << " + ";
  write_shift(o, right_->name(), rhs_shift_);
  o << " /* " << scale_ << " " << range_ << " */";
}

int64_t Add::eval(environment env) const {
  int64_t lhs = env[left_->name()];
  int64_t rhs = env[right_->name()];
  int64_t res = do_shift(lhs, lhs_shift_) + do_shift(rhs, rhs_shift_);
  env[name()] = res;
  return res;
}

Mul::Mul(string name, VarR left, VarR right)
    : Var(name), left_(left), right_(right), lhs_shift_(0), rhs_shift_(0) {
  Range range_result = left->range() * right->range();
  Scale scale_result = add_shifts(left->scale(), right->scale());
  bool left_next = false;
  for (bool left_next = false; !range_result.fits<target_word>();
       left_next = !left_next) {
    if (left_next) {
      lhs_shift_--;
    } else {
      rhs_shift_--;
    }
    range_result = range_result / 2;
    scale_result = scale_result >> 1;
  }
  range_ = range_result;
  scale_ = scale_result;
  assert(range_.fits<target_word>());
}
void Mul::write_c_code(ostream& o) const {
  o << name() << " = ";
  write_shift(o, left_->name(), lhs_shift_);
  o << " * ";
  write_shift(o, right_->name(), rhs_shift_);
  o << " /* " << scale_ << " " << range_ << " */";
}

int64_t Mul::eval(environment env) const {
  int64_t lhs = env[left_->name()];
  int64_t rhs = env[right_->name()];
  int64_t res = do_shift(lhs, lhs_shift_) * do_shift(rhs, rhs_shift_);
  env[name()] = res;
  return res;
}

void write_shift(ostream& os, string& name, int shift) {
  if (shift < 0) {
    os << "(" << name << " >> " << (-shift) << ")";
  } else if (shift == 0) {
    os << name;
  } else {
    os << "(" << name << " << " << (shift) << ")";
  }
}

int64_t do_shift(int64_t value, int shift) {
  if (shift < 0) {
    return value >> (-shift);
  } else if (shift == 0) {
    return value;
  } else {
    return value << (shift);
  }
}