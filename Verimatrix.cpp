#include <string>
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

// This is not efficient when resizing but should not happen often.
// It makes the normal operations very fast and simple, and is memory-efficient.
struct bit_stream : vector<bool> {
	void push_char(unsigned char ch) {
		for (int i = 0; i < 8; ++i) {
			push_back(ch & 1);
			ch >>= 1;
		}
	}

	unsigned char get_char(size_t offset) const {
		char ch = 0;
		size_t sz = size();
		for (size_t index = offset + 8; index-- > offset;) {
			int bit = index < sz ? (*this)[index] : 0;
			ch <<= 1;
			ch |= bit;
		}
		return ch;
	}
};

static bit_stream to_bit_stream(const string& s)
{
	bit_stream bs;
	for (auto c : s) {
		bs.push_char(c);
	}
	return bs;
}

static string from_bit_stream(const bit_stream& bs)
{
	string s;
	for (size_t ix = 0; ix < bs.size(); ix += 8) {
		s += bs.get_char(ix);
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
static bit_stream interpret(const string& code, const bit_stream& input)
{
	size_t input_index = 0;
	int p = 0;
	ptrdiff_t offset = 0;
	
	bit_stream output;
	// Arbitrary initial size.
	vector<bool> tape(256, 0);

	int pc = 0;
	while(pc < code.size())
	{
		const char ch = code[pc];
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
				const size_t sz = tape.size();
				tape.insert(tape.begin(), sz, false);
				offset += sz;
			}
			++pc;
			break;
		case '>': // Move p right by one cell.
			{
				++p;
				const ptrdiff_t sz = tape.size();
				if (p + offset >= sz) {
					tape.resize(2 * sz, false);
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

static void test_one(const char* program, const char* input, const char* expected)
{
	ostringstream sstrm;
	sstrm << from_bit_stream(interpret(program, to_bit_stream(input)));
	string actual = sstrm.str();
	cout << program << " " << input << " : ";
	if (actual != expected) {
		cout << "Error:" << actual << " instead of " << expected;
	}
	else {
		cout << "OK:" << actual;
	}
	cout << endl;
}

static void test()
{
	const char* hw =
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
	test_one(hw, "", "Hello, world!\n");
	test_one(",;,;,;,;,;,;,;,;", "A", "A");
	test_one(",;,;,;,;,;,;,;,; ,;,;,;,;,;,;,;,;", "AB", "AB");
	test_one(",>,>,>,>,>,>,>,> <<<<<<<< ;>;>;>;>;>;>;>;>", "Z", "Z");
	auto reverter = ">,>,>,>,>,>,>,>,>+<<<<<<<<+[>+]<[<]>>>>>>>>>[+<<<<<<<<[>]+"
		"<[+<]>>>>>>>>>>,>,>,>,>,>,>,>,>+<<<<<<<<+[>+]<[<]>>>>>>>>>]<[+<]+<<<<<<<<+[>+]"
		"<[<]>>>>>>>>>[+<<<<<<<<[>]+<[+<]>;>;>;>;>;>;>;>;<<<<<<<<+<<<<<<<<+[>+]"
		"<[<]>>>>>>>>>]<[+<]";
	test_one(reverter, "1", "1");
	test_one(reverter, "12", "21");
	test_one(reverter, "123", "321");
	test_one(reverter, "abcdefghijklmnopqrstuvwxyz", "zyxwvutsrqponmlkjihgfedcba");
}

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
		cout << from_bit_stream(interpret(program, to_bit_stream(argv[1]))) << endl;
	}
	else
	{
		// If no command-line argument.
		test();
	}
	return 0;
}