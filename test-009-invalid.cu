// This file contains various type errors in the CU language.
// It is not a complete test.
function f(ref |float| x) { }
function g(|float| x) { }
function h(|float| x) returns |int| { return round x; } 

function main( ) returns |int|
{
    |int| a;      
    array of |int| data;  
    pointer to |int| p;

    |int| a;              // Duplicate identifier      
    |double| x;           // Unknown type name

    y = 42;               // Unknown identifier
    f(4.1);               // L-Value expected for ref param
    f(a);                 // Invalid argument type
    g(4.1);               // OK
    g(a);                 // OK
    g("Hello");           // Invalid argument type
    g();                  // Wrong number of arguments
    g(4.2, 2.2);          // Wrong number of arguments
    a = h;                // Function name used incorrectly
    a = g(4.1);           // OK
    data = (array of |int| is 1, 2.2 );// Wrong type in array constant
    p = (new |int| is 2.2);            // Wrong type in new pointer
    if (a and true)then write 0; fi    // Expected boolean
    if (a > "Hello") then write 1; fi  // Expected same type
    a = 4 - "Hello";                   // Type error
    a = *a;                            // Type error
    read 42;                           // Must be addressable
    write data;                        // Must be basic type
    if (a) then write 2; fi            // Expected boolean
    return 4.2;                        // Wrong return type
}
