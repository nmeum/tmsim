start: q0;
accept: q1, q2, q3;

q0 {
	a > b => q1;
	b | b => q2;
}

q1 {
	a < b => q1;
	b | b => q3;
}
