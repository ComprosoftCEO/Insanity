# Insanity
An esoteric programming language that consists of single symbol commands.

### Abstract
Insanity is a programming language designed by Bryan McClain. Similar to assembly, Insanity uses single-symbol commands such as < + and $ to represent simple operations. All text characters that aren’t commands are ignored, unless part of a label or jump command. Whitespace (spacebar, tab, etc.) is ignored, even when inside a label. 

### System Architecture

#### Registers
The Insanity programming language has 2 registers, the Accumulator and the Backup. The Accumulator is used for basic mathematical operations (addition and subtraction) as well as all of the compare operations (greater than, less than, etc.). The Backup register cannot be written to or read from directly, but can be accessed via the save and swap commands ($ and ~). Registers can hold values from -999 to 999 (inclusive).

#### Memory
Within the Insanity programming language are 1000 memory slots that can be written to and read from using the Accumulator. Like registers, memory slots can hold values between -999 and 999 (inclusive). Memory is navigated using the memory cursor (detailed below).

#### The Stack
One common feature of modern programming languages is the concept of subroutines. Insanity is no exception, and contains an internal stack for storing subroutines. The Insanity stack holds the last 100 calls to a subroutine, with a stack pointer indicating the current position in the stack. However, the values in the stack cannot be directly accessed within an Insanity program, and are handled by the implementation of Insanity. (The Insanity program itself only controls when subroutine or a return from subroutine is called). Errors should be thrown for stack overflow or underflow.

#### Status Flags
In addition to registers and memory, Insanity has two status flags that are used during execution. These flags are the Overflow Flag and the Compare Flag. 
The Overflow Flag is set to true when the result of an addition or subtraction operation results in a value greater than 999 or less than -999. If the operation is successful, then the Overflow Flag is set to false.
The Compare Flag is set to true when the result of a compare operation is true, or false when the result of a compare operation is false.

#### Cursors
Finally, the Insanity programming language has two cursors used during execution. These cursors are called the Memory Cursor and the Digit Cursor. The cursors do not loop around.
The Memory Cursor represents the currently “selected” memory slot. This is the memory slot that will be written to and read from using the Accumulator.
The Digit Cursor represents the current digits spot. This cursor can be in one of three states: 1’s, 10’s, or 100’s. When performing addition or subtraction operations, the computer will add the respective value. (I.E. if the Digit Cursor is in the 10’s position, the computer will add and subtract the number 10) The digit cursor also indicates how many memory slots to move the memory cursor with the < and > commands.
