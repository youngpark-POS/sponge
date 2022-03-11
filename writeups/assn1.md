Assignment 1 Writeup
=============

My name: yeongjae Park

My POVIS ID: youngpark

My student ID (numeric): 20190980

This assignment took me about [15] hours to do (including the time on studying, designing, and writing the code).

Program Structure and Design of the StreamReassembler:
[
  segments vector can hold _capacity characters
  0. When push_substrings() called, Erase already written bytes, push back empty bytes.
  1. StreamReassembler decides where to put characters.
  2. StreamReassembler checks "EMPTY_BYTE" in byte_status vector, put characters, 
  alter that byte's status as "USED_BYTE".
  3. StreamReassembler loops over segments vector, put as many characters into ByteStream as possible.
  alter written bytes' status as "WRITTEN_BYTES"
  4. When got EOF, note EOF and last byte of the stream. 
  send EOF to ByteStream when segments vector is empty and all characters are transferred.
  5. unassembled_bytes() returns the number of "USED_BYTES", empty() returns unassembled_bytes == 0.
]

Implementation Challenges:
[
  "How can I maintain segments?"
  "How to put segments into auxiliary spaces?"
  "When does the EOF flag turn on?"
]

Remaining Bugs:
[
  None. Be careful to manipulate std::vector, some methods might throw errors.
]

- Optional: I had unexpected difficulty with: [understanding about capacity and EOF]

- Optional: I think you could make this assignment better by: [enhance its time efficiency]

- Optional: I was surprised by: []

- Optional: I'm not sure about: [whether my code works well or not]
