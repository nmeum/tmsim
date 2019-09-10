start: q0;
accept: q5;

# Remove one zero from left hand side of the operation.
# If at operation symbol: terminate.
q0 {
	0 > X => q1;
	P | P => q5;
}

# Go to the right hand side of the operation.
q1 {
	0 > 0 => q1;
	P > P => q2;
}

# Add a zero to the right hand side of the operation.
q2 {
	0 > 0 => q2;
	$ < 0 => q3;
}

# Go to the left hand side of the operation.
q3 {
	0 < 0 => q3;
	P < P => q4;
}

# Find the first non-X character.
# If none: stay add operation symbol.
q4 {
	0 < 0 => q4;
	X > X => q0;
}

# Cleanup, remove X and P characters.
q5 {
	X < $ => q5;
	P < $ => q5;
}
