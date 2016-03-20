#include <cassert>
#include <string>
#include <stdint.h>
#include <limits>
#include <iostream>

#include "fixed_point_compiler.h"

using namespace std;

void generate_cordic(ostream&s, double w, int samples) {
	Program prog;
	const int adc_bits = 12;
	int max_mag = samples * (1 << adc_bits);
	int scale = 0;
	while (max_mag < numeric_limits<target_word>::max()/2) {
		max_mag *= 2;
		scale++;
	}
	VarR cos_value = prog.lit(cos(w), "cosw");
	VarR sin_value = prog.lit(sin(w), "sinw");
	VarR msin_value = prog.lit(-sin(w), "msinw");
	VarR i = prog.load("acc_i", Range(-max_mag, max_mag), Scale(scale), "i");
	VarR q = prog.load("acc_q", Range(-max_mag, max_mag), Scale(scale), "q");
	VarR adc = prog.load("adc_reading", Range(0, 1 << (adc_bits+1)), Scale(0), "adc");
	VarR i2 = prog.mul(i, cos_value, "i2");
	VarR i3 = prog.mul(q, sin_value, "i3");
	VarR i4 = prog.add(i2, i3, "i4");
	VarR i5 = prog.add(i4, adc, "i5");
	VarR acc_i = prog.store("acc_i", i5);
	assert(acc_i->scale() == i->scale());
	VarR q2 = prog.mul(q, cos_value, "q2");
	VarR q3 = prog.mul(i, msin_value, "q3");
	VarR q4 = prog.add(q2, q3, "q4");
	VarR acc_q = prog.store("acc_q", q4);
	assert(acc_q->scale() == q->scale());
	s << prog;
}

int main()
{
	generate_cordic(cout, 2 * 3.141 / 10, 10);
	return 0;
}

