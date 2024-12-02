Huffman codec application library
==============================================

Command line
-------------------

HUFFMAN -?
HUFFMAN command { [{switch}] file }

Commands:
a	analyze file and output statistics.
d	decode file
e	encode file

Switches:
	[General]
-?				: display command line help.
-h	--help		: display advanced help.
-q	--quiet		: quiet mode.
-v	--verbose	: operate verbosely.
	--version	: display application version.

	[Compression]
	--blocksize N	: size of the stream block.
	--format NAME	: file format (RAW or STREAMED).
	--rle			: encode RLE sequences instead of just bit sequences.
-t	--table	FILE	: specify external huffman table file (binary or text).
-o	--output FILE	: specify output file.
	--store FILE	: store generated table to FILE.
	--streams N		: specify how many streams in the output file must be generated.

Huffman code table text file format
---------------------------------------

Comments are in C++ style and can appear everywhere where spaces are permitted:
// any text to end of line
/* any text */

  Code	   Count	Length	   Value
xx...xx : nn...nn : ll...ll	: vv...vv

Code: code value in binary form. Each binary digit is important, including leading zeroes.
Count: number of elements in any numeric format.
Length: length of the value, in bits. If empty, all bits of the value are used. This field
		can be 0.
Value: value bits in any format (numeric, string, char etc.).

Numeric values (except code field) can be in following forms:
[0 radix-spec] {digit}
Not that spaces are not permitted. To separate digits, underscore can be used. Radix specifications:
b/B		binary
o/O		octal
d/D		decimal (optional)
x/X		hexadecimal
C form of octal numbers is not permitted.
Example:
0b1111_0000		numeric value of F0 hex (in binary form with underscore separator)
0o40			numeric value of 20 hex (in octal form, 040 is decimal 40)
0128			numeric value of 80 hex (in decimal form)

Values, assigned to Huffman codes, can be in following formats:
- numeric value; length field specifies number of valid bits;
- enclosed in quotes, double quotes or backticks string value; optional length field specifies
  number of bytes. Extra bytes are omitted with warning, length longer than the actual string
  is rejected with error.

For example:
001 : 16 : 1 : 0		// code 001 -> 0000 0000 0000 0000 (16 1-bit 0es)
 10 :  4 : 3 : "abc"	// code 10 -> abcabcabcabc (4 3-byte strings concatenated one by one)


There additional forms of code encoding: event and repeat.

The event code generates an "event" which results in calling user-supplied callback function.
Format of event code is follows:
	code : E : event-code
for example
	000000000001 : E : 1	// code 000000000001 genertaes event with parameter = 1

----------- UNIMPLEMENTED
The repeat code repeats previously stored value specified number of times. For example:
	000000011111 : 1000 : R	// code 000000011111 repeats previously stored value 2560 times
-----------------------------


Huffman streamed output file format
---------------------------------

FILEHEADER
	SIGNATURE (4)
	N: NUMBER OF STREAMS (4)
DATA
	STREAM[0]
	...
	STREAM[N - 1]
STREAM DIRECTORY
	SIGNATURE (4)
	STREAM LENGTH[0] (4)
	...
	STREAM LENGTH[N - 1] (4)
	SIGNATURE (4)

Huffman multitable streamed output file format
---------------------------------

FILEHEADER
	SIGNATURE (4)
	N: NUMBER OF STREAMS (4)
DATA
	STREAM[0]
	...
	STREAM[N - 1]
TABLES
	TABLE[0]
	...
	TABLE[N - 1]
STREAM DIRECTORY
	SIGNATURE (4)
	STREAM LENGTH[0] (4)
	TABLE LENGTH[0] (4)
	...
	STREAM LENGTH[N - 1] (4)
	TABLE LENGTH[N - 1] (4)
	SIGNATURE (4)
