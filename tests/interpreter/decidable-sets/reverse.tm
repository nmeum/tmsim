# Input: A binary number.
# Accepts the following input: { w + reverse(w) | w ∈ {0, 1}^+ }

start: q1;
accept: q9;

# Move to the end of the tape. As soon as we reached a blank character
# indicating the end of the tape move left and switch to state q2.
q1 {
	1 > 1 => q1;
	0 > 0 => q1;

	$ < $ => q2;
	a < a => q2;
	b < b => q2;
}

# The purpose of this state is to check the last input symbol on the
# tape. If it is a '0' the symbol is replaced with 'a' if it is a '1' it
# is replaced with 'b'. After the symbol has been replaced the machine
# moves back to the beginning of the tape and does the same thing with
# the first input symbol on the tape. If the first input symbol on the
# tape is not equal to the one encountered at the end the input is
# rejected.
#
# If the last character isn't an input symbol (meaning it's an 'a'
# or 'b') the machine switches to state q7 and verifies that the tape
# consists only of 'a' and 'b' characters.
q2 {
	a | a => q7;
	b | b => q7;

	0 < a => q3;
	1 < b => q5;
}

##
# Replacing a 0.
##

q3 {
	0 < 0 => q3;
	1 < 1 => q3;

	$ > $ => q4;
	a > a => q4;
	b > b => q4;
}

q4 {
	0 > a => q1;
}

##
# Replacing a 1.
##

q5 {
	0 < 0 => q5;
	1 < 1 => q5;

	$ > $ => q6;
	a > a => q6;
	b > b => q6;
}

q6 {
	1 > b => q1;
}

##
# Verifying the tape content.
##

# Move to the beginning of the tape and switch to state q8 as soon as
# the first blank character on the left hand side has been encountered.
q7 {
	0 < 0 => q7;
	1 < 1 => q7;
	a < a => q7;
	b < b => q7;
	$ > $ => q8;
}

# Move to the end of the tape again. If a character is encountered that
# is not an 'a' or a 'b', before the end of the tape is reached, the input
# is rejected.
q8 {
	a > a => q8;
	b > b => q8;
	$ | $ => q9;
}
