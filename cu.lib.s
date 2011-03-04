# ...........................................................
# CU assembly functions.                                          
# Written by Michael Main (main@colorado.edu)
# Version: Feb 2, 2011
# Each function has a name that starts with "lib.".
# These functions get their arguments in the general purpose
# registers (eax, ebx, ecx, edx) or on the stack. Return values are
# sent back through the eax register.  The eax, ebx, ecx and edx
# register values might be destroyed, but the values of all other
# general purpose registers are preserved.
# External names use the Linux version (such as malloc).
# For windows, these should be rest (for example, to _malloc).
# ...........................................................

# ...........................................................
# Data Types:
# The implementations of our data types are listed here:
#
# 1. An integer is a 4-byte signed integer.
# 2. A boolean is a 4-byte value (0 for false, 1 for true).
# 3. Every pointer or reference is a 4-byte address.
# 4. A string-record is a record with 17 or more bytes containing these
#    components:
#    == (at start of record) An integer giving the total number of 
#       bytes for this record.  This will always equal 17 + the 
#       maximum number of characters that the string can hold without 
#       allocating more memory (allowing for the 16 bytes of
#       information that precede the first data character
#       and the one byte for the null termination).
#    == (at offset +4) An integer equal to -1 to indicate that 
#       this is a string record.  This is run time type information.
#    == (at offset +8) An integer giving the maximum number of 
#       characters that this string can grow to without allocating 
#       more memory.
#    == (at offset +12) An integer giving the current number of 
#       characters in the string, not counting the null terminator.
#    == (at offset +16 and beyond) The current chars of the string 
#       appear here, followed by a null terminator (a single zero byte).
# 5. A string is a pointer to byte number 16 of a string-record
#    (which is where the char data begins).
# 6. A ptr-to-string is a pointer to a string.  In other words,
#    a ptr-to-string is a pointer to a pointer to byte number 16
#    of a string record.
# 7. An array-record is a record with 16 or more bytes containing
#    components:
#    == (at start of record) An integer giving the total number of 
#       bytes for this record.  This will always equal 16 + 4 times the 
#       current number of elements in the array (allowing for the 16 bytes 
#       of information that precede the [0] element of the array).
#    == (at offset +4) An integer equal to 0 (for a simple array) or
#       1 (for an array of strings or an array of arrays).
#    == (at offset +8) reserved for future use.
#    == (at offset +12) An integer giving the current number of 
#       elements in the array..
#    == (at offset +16) The current elements of the array appear
#       here.  Each element requires exactly four bytes.
# 8. An array is a pointer to byte number 16 of an array-record
#    (which is where the element data begins).
# 9. A ptr-to-array is a pointer to an array.  In other words,
#    a ptr-to-array is a pointer to a pointer to byte number 16
#    of an array record.
# Some arguments to functions can be either an array or a string
# (which we call array-or-string).  Some of the arguments can be either a
# ptr-to-array or ptr-to-string (which we call ptr-to-array-or-string).
.section .text                                                         

	
# ...........................................................        
lib.copyrec:
# void lib.copyrec(ptr-to-array-or-string pa);
# Written by Michael Main (Feb 2, 2011)
# When this function is called, pa (passed in eax) contains a 
# pointer to an array or string called a.  This function 
# makes a deep copy of a.  On return, the eax register no 
# longer points to a.  At that point, the value stored in
# a is a deep copy of the original array or string.
  push  $0                           
  lib.copyrec.1:                  
  pushl %eax                         
  pushl (%eax)                       
  pushl (%esp)                       
  subl  $16, (%esp)             
  movl  (%esp), %ecx
  pushl (%ecx)	                     
  call  malloc
  pushl 4(%esp)                     
  pushl %eax                         
  call  memcpy                     
  addl  $16, %eax
  movl  20(%esp), %ecx
  movl  %eax, (%ecx)
  addl  $24, %esp
  cmpl  $0, -12(%eax)          
  jle   lib.copyrec.3             
  movl  -4(%eax), %ecx
  shll  $2, %ecx
  je    lib.copyrec.3
  subl  $4, %ecx
  lib.copyrec.2:
  pushl %eax
  addl  %ecx, (%esp)
  subl  $4, %ecx
  jge   lib.copyrec.2
  lib.copyrec.3:
  popl  %eax
  cmpl  $0, %eax
  jne   lib.copyrec.1
  ret
# ...........................................................        


# ...........................................................        
# ...........................................................        
lib.freerec:
# void lib.freerec(array-or-string a);
# Written by Michael Main (Feb 2, 2011)
# When this function is called, a (passed in the eax register)
# contains an array-or-string. This function frees the implicit 
# dynamic memory of that a is using, including any memory that 
# its components may use.
  pushl $0
  lib.freerec.1:
  cmpl  $0, -12(%eax)       
  jle   lib.freerec.2
  movl  -4(%eax), %ecx
  shll  $2, %ecx
  subl  %ecx, %esp
  pushl %ecx
  pushl %eax
  pushl %esp
  addl  $8, (%esp)
  call  memcpy
  movl  4(%esp), %eax
  addl  $12, %esp
  lib.freerec.2:
  subl  $16, %eax
  pushl %eax
  call  free
  addl  $4, %esp
  popl  %eax
  cmpl  $0, %eax
  jne   lib.freerec.1
  ret
# ...........................................................        


# ...........................................................        
lib.coercerec:
# void lib.coercerec(array a);
# Written by Michael Main (Feb 2, 2011)
# When this function is called, a (passed in the eax register)
# contains an array (maybe multidimensional) in which the
#  components are int. This function coerces each int to float.
  pushl $0
  lib.coercerec.1:
  movl  -4(%eax), %ecx
  cmpl  $0, -12(%eax)
  jle   lib.coercerec.2
  shll  $2, %ecx
  subl  %ecx, %esp
  pushl %ecx
  pushl %eax
  pushl %esp
  addl  $8, (%esp)
  call  memcpy
  addl  $12, %esp
  jmp   lib.coercerec.3
  lib.coercerec.2:
  cmpl  $0, %ecx
  jle   lib.coercerec.3
  fild  (%eax)
  fstp  (%eax)
  addl  $4, %eax
  decl  %ecx
  jmp   lib.coercerec.2
  lib.coercerec.3:
  popl  %eax
  cmpl  $0, %eax
  jne   lib.coercerec.1
  ret
# ...........................................................        


# ...........................................................        
lib.intpow:                               
# integer lib.intpow(integer base, integer exponent);
# Written by Michael Main (Feb 2, 2011)
# When this function is called, eax contains an integer 
# (the base), and ebx contains a non-negative integer 
# (the exponent). The function computes base raised to the
# exponent power, returning the answer in the eax register.
# Note: The function's behavior is not specified for
# any negative exponents, even when the base is 1 or -1.
# 
  # Create the new frame and get the parameters:                     
  pushl %ebp                   # save old ebase pointer          
  movl  %esp, %ebp             # set new base pointer               
  # Push base^1 base^2 base^4 base^8...                              
  movl  $1, %ecx               # ecx = exponent about to push   
  lib.intpow.1:               # top of the push loop           
  cmpl  $0, %ecx               # has ecx overflowed?            
  jle   lib.intpow.2          # if so, exit the loop           
  cmpl  %ebx, %ecx             # have we pushed enough powers?  
  jg    lib.intpow.2          # if so, exit the loop           
  pushl %eax                   # push this power onto the stack 
  imull %eax                   # eax = eax*eax (exp is doubled) 
  shll  $1, %ecx               # ecx = 2*ecx (double the exp)   
  jmp   lib.intpow.1          # to the top of the push loop    
  lib.intpow.2:               # exit of the push loop          
  shrl  $1, %ecx               # ecx = exponent of final push   
  # Compute eax = product of necessary powers of the base            
  movl  $1, %eax               # eax = base^0                   
  lib.intpow.3:               # top of the multiply loop       
  cmpl  $0, %ebx               # multiplied all needed powers?  
  jle   lib.intpow.5          # if so, exit the loop           
  cmpl  %ecx, %ebx             # need power on top of stack?    
  jl    lib.intpow.4          # if not, then skip it           
  imull (%esp)                 # eax = eax * needed power       
  subl  %ecx, %ebx             # subtract needed power          
  lib.intpow.4:               # after the multiplication       
  addl  $4, %esp               # pop the stack                  
  shrl  $1, %ecx               # ecx = ecx/2 (half the exponent)
  jmp   lib.intpow.3          # to top of the multiply loop    
  lib.intpow.5:               # exit of the multiply loop      
  movl  %ebp, %esp             # pop unneeded numbers from stack
  popl  %ebp                   # restore the old base pointer   
  ret                          # return from pow.lib           
# ...........................................................


# ...........................................................        
lib.readmore:
# bool lib.readmore( );
# Written by Michael Main (Feb 2, 2011)
# This function peeks ahead at the next character from
# standard input (without actually reading the character).
# If it finds a character (stdin is not exhausted), and that
# character is not a whitespace character, then this function
# returns true.  Otherwise, this function returns false. 
# The answer is returned in the eax register.
.section .data
  readmore.table:
  .long 1,1,1,1,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
  .long 0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
  .long 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
  .long 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
  .long 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
  .long 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
  .long 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
  .long 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
  .long 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
  .long 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
  .long 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
  .long 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
  .long 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
  .long 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
  .long 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
  .long 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
.section .text
  call   getchar                    # eax = next input character
  cmpl   $-1, %eax                  # is stdin at end-of-file?
  jne    lib.readmore.0             # if so, then...
  movl   $0, %eax                   # ...return value = false...
  ret                               # ...and return
  lib.readmore.0:                   # else
  subl   $8, %esp                   # the g++ compiler has 8 extra bytes here
  pushl  (stdin)                    # push ungetc's FILE* argument
  pushl  %eax                       # push ungetc's character argument
  call   ungetc                     # return that character back to stdin
  popl   %eax                       # pop that character back into eax
  addl   $12, %esp                  # pop the rest of ungetc's arguments
  movl   readmore.table(,%eax,4), %eax # eax = answer from readmore table
  ret
# ...........................................................        


# ...........................................................        
lib.readstr:
# void lib.readstr(pointerto :string: ps);
# Written by Michael Main (Feb 2, 2011)
# When this function is called, the stack top contains a 
# ptr-to-string (ps). The function reads characters from 
# standard input, putting the characters into the string 
# (increasing its size if needed). The reading stops 
# when !readmore( ).
# NOTE: The original string record might
# be replaced by a new larger string record (using realloc). 
.section .data
  readstr.psr:
  .long 0                     # Pointer to byte 0 of string record
  readstr.recsize:
  .long 0                     # Current record size
  readstr.format:
  .asciz "%128s"              # Format string for scanf
.section .text
  # Set the initial values for readstr.psr and readstr.size
  movl  4(%esp), %edx
  movl  (%edx), %eax
  movl  %eax, (readstr.psr)        
  subl  $16, (readstr.psr)
  movl  $17, (readstr.recsize)
  # Top of loop. Each iteration reads up to 127 characters
  lib.readstr.0:
  addl  $128, (readstr.recsize)
  pushl (readstr.recsize)
  pushl (readstr.psr)
  call  realloc
  movl  %eax, (readstr.psr)
  addl  $4, %esp
  addl  %eax, (%esp)
  subl  $129, (%esp)
  pushl $readstr.format
  call  scanf
  addl  $8, %esp
  call  lib.readmore
  cmpl  $0, %eax
  jne   lib.readstr.0
  # The loop has ended.  We have read the whole string, but
  # still need to set up its 16-byte header.
  movl  (readstr.recsize), %eax
  movl  (readstr.psr), %ecx
  movl  %eax, (%ecx)
  movl  $-1, 4(%ecx)
  subl  $17, %eax
  movl  %eax, 8(%ecx)
  pushl %ecx
  addl  $16, (%esp)
  call  strlen
  movl  (readstr.psr), %ecx
  movl  %eax, 12(%ecx)
  # The starting address of the string data is still on top of the
  # stack from the call to strlen.  It might be a new address, so
  # pop it into the original string.  This will leave just the
  # return address and the original ps argument on the stack.
  movl  8(%esp), %edx
  popl  (%edx)
  ret
  
  
