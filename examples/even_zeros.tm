# Input: A binary number.
# Accepts if the given number has an even amout of 0s
# 	or doesn't contain any zeros at all.

start: q1;
accept: q0;

q1 {
	1 > 1 => q1;
	0 > 0 => q2;

	$ | $ => q0;
}

q2 {
	1 > 1 => q2;
	0 > 0 => q1;
}
