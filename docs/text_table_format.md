Text table file format.
=======================

The table file in the text format consists of a header that contains table information, and the table itself, with the footer following it.

The table format also supports C/C++ single- and multi-line style comments.

Table header consists of following fields:
- signature field ("HTTF" - (H)uffman code (T)ext (T)able (F)file);
- version field ("%d.%d" format);

Table footer consists of the single signature string "END". The footer is required as it allows to detect damaged table files.

Code table consists of series of lines, each of which is a sequence of fields separated with semicolons (':'). Order of fields is as follows:
- C: code bit sequence in binary format (for example, "00110101");
- N: count of elements in any numeric format;
- V: unpacked element code in binary format.
Each line defines a code C that is used to represent N successive copies of value V. For example, the following lines
	`00111 : 4: 01`
	`00111 : 2: 0101`
	`00111 : 1: 01010101`
all define the Huffman code of "00111" that is used to represent unpacked value of "01010101".

There are special codes that can represent some specific actions that the decoder can take into account. Such lines use action name instead of the C field and can optionally have additional parameter.
Clients receive such fields as messages (see libhuffman_message).
For example, a image decoder can recognize following fields:
- "EOL": end of line (the client fills end of the line with black pixels or with value specified as the parameter);
- "EOF": end of file (the client fills end of the image rectangle with black pixels and aborts parsing);
They can be represented in following manner:
000000000001: EOL		// end of line, fill the rest of the line with the default pixel value
000000011111: EOL: 0101	// end of line, fill the rest of the line with the value of "0101"

Example file (all codes are given only as an example, without any meaningful values):
-------------------------------------------------------------------------------------
```
HHTF
1.0
00111 : 14: 1		// the "00111" Huffman code represents the "11 1111 1111 1111" bit sequence
1100 : 1: 010001100	// the "1100" Huffman code represents the "0 1000 1100" bit sequence
000000000001: END	// send the message to the client with the "END" string as a message name
END					// file footer
```