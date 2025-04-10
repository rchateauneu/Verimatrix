#include <string>
#include <iostream>
#include <vector>

using namespace std;

struct bit_stream : vector<bool> {
	void push_char(unsigned char ch) {
		for (int i = 0; i < 8; ++i) {
			push_back(ch & 1);
			ch >>= 1;
		}
	}

	unsigned char get_char(size_t offset) const {
		char ch = 0;;
		size_t sz = size();
		for (size_t index = offset + 8; index-- > offset;) {
			bool bit = index < sz ? (*this)[index] : false;
			ch <<= 1;
			ch |= bit;
		}
		return ch;
	}
};

bit_stream to_bit_stream(const string& s)
{
	bit_stream bs;
	for (auto c : s) {
		bs.push_char(c);
	}
	return bs;
}

static string from_bit_stream(const bit_stream& bs)
{
	cout << "from_bit_stream bs.size()=" << bs.size() << endl;
	string s;
	for (size_t ix = 0; ix < bs.size(); ix += 8) {
		char ch = bs.get_char(ix);
		s += ch;
	}
	return s;
}

static int forward(const string& code, int pc) {
	for (int counter = 1;;)
	{
		++pc;
		switch (code[pc]) {
		case '[': ++counter;
			break;
		case ']': --counter;
			if (counter == 0) return pc;
			break;
		}
	}
}

static int backward(const string& code, int pc) {
	for (int counter = 1;;)
	{
		--pc;
		switch (code[pc]) {
		case ']': ++counter;
			break;
		case '[': --counter;
			if (counter == 0) return pc;
			break;
		}
	}
}

/*
 * Take a program, "code", and the contents of the input stream, "input", and
 * execute the program. The contents of the output stream are returned.
 */
bit_stream interpret(const string& code, const bit_stream& input)
{
	size_t input_index = 0;
	int p = 0;
	int offset = 0;
	
	bit_stream output;
	vector<bool> tape(256, 0);

	int pc = 0;
	while(pc < code.size())
	{
		char ch = code[pc];
		switch (ch) {
		case '+': // Negate the bit on the tape under p (i.e. a 0 becomes a 1 and a 1 becomes a 0).
			tape[p + offset] = !tape[p + offset];
			++pc;
			break;
		case ',': // Read the next bit from the input stream, writing it to the tape under p.
			// If the end of the input stream (EOF) has been reached then the value read will just be 0.
			{
				bool next;
				if (input_index == input.size()) {
					next = 0;
				}
				else {
					next = input[input_index];
					++input_index;
				}
				tape[p + offset] = next;
			}
			++pc;
			break;
		case ';' : // Write the bit on the tape under p to the output stream.
			output.push_back(tape[p + offset]);
			++pc;
			break;
		case '<' : // Move p left by one cell.
			--p;
			if (p + offset < 0) {
				size_t sz = tape.size();
				size_t new_sz = 2 * sz;
				// This is not efficient but should not happen often.
				// It makes the normal operations very fast and simple.
				tape.resize(new_sz);
				copy(tape.begin(), tape.begin() + sz, tape.begin() + sz);
				fill(tape.begin(), tape.begin() + sz, false);
				offset += sz;
			}
			++pc;
			break;
		case '>': // Move p right by one cell.
			{
				size_t sz = tape.size();
				if (p + offset >= sz) {
					size_t new_sz = 2 * sz;
					// This is not efficient but should not happen often.
					// It makes the normal operations very fast and simple.
					tape.resize(new_sz, false);
				}
				++pc;
			}
			break;
		case '[' : // If the value on the tape under p is 0 then set p to the location of the next matching ].
			if (tape[p + offset] == false) {
				pc = forward(code, pc);
			}
			else {
				++pc;
			}
			break;
		case ']' : // If the value on the tape under p is 1 then set p to the location of the prior matching [.
			if (tape[p + offset] == true) {
				pc = backward(code, pc);
			}
			else {
				++pc;
			}
			break;
		default:
			++pc;
			break;
		}
	}
	return output;
}

// OK
const char * hw = 
";;;+;+;;+;+;" 
"+;+;+;+;;+;;+;" 
";;+;;+;+;;+;" 
";;+;;+;+;;+;"
"+;;;;+;+;;+;" 
";;+;;+;+;+;;" 
";;;;;+;+;;"
"+;;;+;+;;;+;" 
"+;;;;+;+;;+;" 
";+;+;;+;;;+;" 
";;+;;+;+;;+;" 
";;+;+;;+;;+;" 
"+;+;;;;+;+;;" 
";+;+;+;"
;


/*
 * Reverse the input. For example:
 *
 * $ ./interpret "Hello, world!"
 * !dlrow ,olleH
 */
int main(int argc, char* argv[])
{
	if (argc == 2)
	{
		auto program = ">,>,>,>,>,>,>,>,>+<<<<<<<<+[>+]<[<]>>>>>>>>>[+<<<<<<<<[>]+"
			"<[+<]>>>>>>>>>>,>,>,>,>,>,>,>,>+<<<<<<<<+[>+]<[<]>>>>>>>>>]<[+<]+<<<<<<<<+[>+]"
			"<[<]>>>>>>>>>[+<<<<<<<<[>]+<[+<]>;>;>;>;>;>;>;>;<<<<<<<<+<<<<<<<<+[>+]"
			"<[<]>>>>>>>>>]<[+<]";

		// OK
		// program = hw;
		cout << from_bit_stream(interpret(program, to_bit_stream(argv[1])));
	}
	return 0;
}