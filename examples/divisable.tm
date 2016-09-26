# Input: A string conisting of 'a' and 'b' symbols.
# Accepts the following input: { a^{n}b^{m} | n > 1, n divides m }

start: q0;
accept: q7;

q0 {
	a > $ => q1;
	c | c => q3;
}

q1 {
	a > a => q1;
	c > c => q1;

	b < c => q2;
}

q2 {
	a < a => q2;
	c < c => q2;

	$ > $ => q0;
}

q3 {
	c > c => q3;
	$ | $ => q7;
	b < b => q4;
}

q4 {
	c < c => q4;
	$ > $ => q5;
}

q5 {
	c > a => q5;
	b < b => q6;
}

q6 {
	a < a => q6;
	$ > $ => q0;
}
