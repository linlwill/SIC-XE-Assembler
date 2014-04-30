SIC-XE-Assembler
================

The final project of CSCI 300

CHANGELOG
---------

Main and ObjectCode have been rewritten for greater clarity, and in the process of doing so have been cleaned up considerably.  At the moment, only objectCode has been updated here on this site.

Finding length of instructions now occurs in a single function that is part of instructions.  This simplifies things greatly and will hopefully fix a problem I was seeing where address/length weren't quite what they should be.

Macros use linked lists instead of queues.  This is a philosophical choice, as well as a practical one.  Queues are single use, they're meant to be entered and left.  Lists are quasi-perminant, they're meant to be iterated through.  Additionally, using lists prevents any kind of queue cloning that would otherwise have been necessary.

Object code generation is a lot cleaner, flag bits are handled as a running integer that gets incremented by whatever power of two the bit in question would represent.  XBPE are handled as a single creature and NI are handled as an addition to opcode.  Conversion to hexidecimal happens only when all bit calculations are done.



