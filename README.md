tmsim
=====

A fast turing machine simulator with graphviz export functionality.

Dependencies
============

In order to build tmsim run the following command:

	$ make

To test if tmsim works as expected you can run the test suite using:

	$ make test

Format
======

The following EBNF describes valid tmsim input:

	turingmachine = metadata, states;
	metadata = "start:", statename, ";",
		"accept:", statenames, ";";

	statename = "q", digit, { digit };
	statenames = statename, { ",", statename };

	states = { state };
	state = statename, "{", [ transitions ], "}";

	transitions = transition, ";", { transition, ";" };
	transition = symbol, direction, symbol, "=>", statename;

	symbol = digit | letter | "$";
	direction = ">" | "|" | "<";

Besides a comment can be added anywhere in the tmsim input file.
Comments begin with the ASCII character '#' and end with a newline.

Documentation
=============

The entire code base is documented with comments that can be
parsed by Doxygen. A Doxyfile is not provided but if you are into that
sort of thing you can use Doxygen to generate some source code
documentation.

License
=======

This program is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <http://www.gnu.org/licenses/>.
