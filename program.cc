#include "fixed_point_compiler.h"

using namespace std;

VarR Program::lit(double value, string name_hint) {
  string local_name = get_tmp_local(name_hint);
  VarR res = make_shared<Const>(local_name, value);
  push_back(res);
  return res;
}

VarR Program::load(string c_exp, Range range, Scale scale, string name_hint) {
  string local_name = get_tmp_local(name_hint);
  VarR res = make_shared<Load>(local_name, c_exp, range, scale);
  push_back(res);
  return res;
}

VarR Program::store(string c_exp, VarR value) {
  VarR res = make_shared<Store>(c_exp, value);
  push_back(res);
  return res;
}
VarR Program::add(VarR lhs, VarR rhs, string name_hint) {
  string local_name = get_tmp_local(name_hint);
  VarR res = make_shared<Add>(local_name, lhs, rhs);
  push_back(res);
  return res;
}
VarR Program::mul(VarR lhs, VarR rhs, string name_hint) {
  string local_name = get_tmp_local(name_hint);
  VarR res = make_shared<Mul>(local_name, lhs, rhs);
  push_back(res);
  return res;
}

int64_t Program::eval(environment& env) const {
  int64_t res;
  for (auto pc = instructions_.cbegin(); pc != instructions_.cend(); ++pc) {
    res = (*pc)->eval(env);
  }
  return res;
}

string Program::get_tmp_local(
    string name) {  // TODO: make this create fresh temps
  assert(local_temps_.count(name) == 0);
  local_temps_.insert(name);
  return name;
}

ostream& operator<<(ostream& o, const Program& p) {
  int locals_on_this_line = 0;
  if (p.local_temps_.size() > 0) {
    o << "int32_t ";
    bool first = true;
    for (auto local = p.local_temps_.cbegin(); local != p.local_temps_.cend();
         ++local) {
      if (!first) {
        o << ", ";
      }
      first = false;
      o << *local;
    }
    o << ";\n";
  }

  for (auto i = p.instructions_.cbegin(); i != p.instructions_.cend(); ++i) {
    o << **i << "\n";
  }
  return o;
}