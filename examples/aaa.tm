start: q0;
accept: q1;

q0 {
	a > a => q0;
	$ | $ => q1;
}
