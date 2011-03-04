|float| x;
array of |float| data;
|int| i is initially 42;

function main( ) returns |int|
{
    // Some tests for coerce_stack_top_to_float_if_needed
    x = 4.2;              // No coercion needed here
    write x; write "\n";  // Prints 4.200000
    x = 42;               // Coercion needed here
    write x; write "\n";  // Prints 42.000000

    // Repeat the same tests, but with explicit floatcasts:
    x = floatcast(4.2);   // No coercion needed here
    write x; write "\n";  // Prints 4.200000
    x = floatcast(42);    // Coercion needed here
    write x; write "\n";  // Prints 42.000000

    // One of these needs a coercion, but the other does not:
    data = (array of |float| is 4.2, 42); 
    write data[0]; write "\n";  // Prints 4.200000
    write data[1]; write "\n";  // Prints 42.000000

    // Test the computation of various R-values:
    write --i; write "\n";               // Prints 41
    write ++i; write "\n";               // Prints 42
    write i--; write "\n";               // Prints 42
    write i++; write "\n";               // Prints 41
    write false and false; write "\n";   // Prints false
    write true and false; write "\n";    // Prints false
    write false and true; write "\n";    // Prints false
    write true and true; write "\n";     // Prints true
    write 1000 + 3200; write "\n";       // Prints 4200
    write round(3.3); write "\n";        // Prints 3
    write round(3); write "\n";          // Prints 3

    // Test the computation of the L-value of --i.
    // This illustrates the concept of "order of operand evaluation."
    // In particular, does i get the value 15 and then get decremented
    // to 14? Or does i get decremented to -1 and then get 15 assigned?
    // Your HW5 should do the assignment after the --.
    i = 0;                
    --i = 15;
    write i; write "\n"; // Prints 15

    return 0;
}
