#ifndef FIXED_POINT_COMPILER_FIXED_POINT_COMPILER_H_
#define FIXED_POINT_COMPILER_FIXED_POINT_COMPILER_H_

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

typedef int32_t target_word;

class Range {
 public:
  Range(int64_t min, int64_t max) : min_(min), max_(max) {
    assert(min <= max);
    assert(min <= 0);
  }
  template <class T>
  bool fits() const {
    static_assert(numeric_limits<T>::is_specialized, "T is not a numeric type");
    static_assert(numeric_limits<T>::is_integer, "T is not an integer type");
    return (numeric_limits<T>::min() <= min_ &&
            max_ <= numeric_limits<T>::max());
  }

  Range operator/(int denom) const { return Range(min_ / denom, max_ / denom); }

  Range operator*(int factor) const { return Range(min_ * 2, max_ * 2); }

  Range operator*(const Range other) const;

  Range operator+(const Range other) const {
    return Range(min_ + other.min_, max_ + other.max_);
  }
  friend std::ostream& operator<<(std::ostream&, const Range&);

 private:
  int64_t min_;
  int64_t max_;
};

/**
* Resprents the scaling factor of a fixed point number
*/
class Scale {
 public:
  explicit Scale(int shift) : shift_(shift) {}

  double value_of(int64_t fixed_point) {
    return ((double)fixed_point) * pow(2, -shift_);
  }

  Scale operator<<(int shift) const { return Scale(shift_ + shift); }

  Scale operator>>(int shift) const { return Scale(shift_ - shift); }

  bool operator==(Scale& other) const { return other.shift_ == shift_; }

  bool operator<(Scale& other) const { return shift_ < other.shift_; }
  friend Scale add_shifts(const Scale, const Scale);
  friend std::ostream& operator<<(std::ostream&, const Scale);

 private:
  int shift_;
};

typedef std::map<std::string, int64_t> environment;
/**
* Superclass of all expressions.
* min/max value are the range of machine word values that it can possibly have
* scale is the ratio of the machine word to the real value
*/
class Var {
 public:
  explicit Var(std::string name) : name_(name), range_(0, 0), scale_(0) {}

  virtual ~Var() {}
  virtual void write_c_code(std::ostream& o) const = 0;
  virtual int64_t eval(environment env) const = 0;
  friend std::ostream& operator<<(std::ostream& o, const Var& v);
  std::string name() const { return name_; }

  Scale scale() { return scale_; }
  Range range() { return range_; }

 private:
  std::string name_;

 protected:
  int min_value_;
  int max_value_;
  Range range_;
  Scale scale_;
};

typedef std::shared_ptr<Var> VarR;

class Const : public Var {
 public:
  Const(std::string name, double value);
  virtual void write_c_code(std::ostream& o) const;
  virtual int64_t eval(environment env) const { return env[name()]; }

 private:
  target_word value_;
  double double_value_;
};

class Load : public Var {
 public:
  Load(std::string name, std::string c_exp, Range range, Scale scale)
      : Var(name), c_exp_(c_exp) {
    range_ = range;
    scale_ = scale;
  }
  virtual void write_c_code(std::ostream& o) const;

  virtual int64_t eval(environment env) const { return env[name()]; }

 private:
  std::string c_exp_;
};

class Store : public Var {
 public:
  Store(std::string c_exp, VarR value) : Var(c_exp), value_(value) {
    range_ = value->range();
    scale_ = value->scale();
  }
  virtual void write_c_code(std::ostream& o) const;

  virtual int64_t eval(environment env) const { return env[name()]; }

 private:
  VarR value_;
};

class Add : public Var {
 public:
  Add(std::string name, VarR left, VarR right);
  virtual void write_c_code(std::ostream& o) const;
  virtual int64_t eval(environment env) const;

 private:
  VarR left_;
  VarR right_;
  int lhs_shift_;
  int rhs_shift_;
};

class Mul : public Var {
 public:
  Mul(std::string name, VarR left, VarR right);
  virtual void write_c_code(std::ostream& o) const;

  virtual int64_t eval(environment env) const;

 private:
  VarR left_;
  VarR right_;
  int lhs_shift_;
  int rhs_shift_;
};

class Program {
 public:
  void push_back(VarR var) { instructions_.push_back(var); }

  VarR lit(double value, std::string name_hint);
  VarR load(std::string c_exp, Range range, Scale scale, std::string name_hint);
  VarR store(std::string c_exp, VarR value);
  VarR add(VarR lhs, VarR rhs, std::string name_hint);
  VarR mul(VarR lhs, VarR rhs, std::string name_hint);

 private:
  std::vector<VarR> instructions_;
  std::set<std::string> local_temps_;
  friend std::ostream& operator<<(std::ostream& o, const Program& p);

  std::string get_tmp_local(std::string name);
};

#endif