|int| i10 is initially 10;
|int| i20 is initially 20;
|int| b;
|bool| bool;
|string| s;
array of |int| ai is initially (array of |int| is 30, 40);
pointer to |int| pi10 is initially @i10;
pointer to |int| pi20 is initially @i20;

function main( ) returns |int|
{
    // Test write and arithmetic operations:
    write 42; write "\n";          // 42
    write i10; write "\n";         // 10
    write i20; write "\n";         // 20
    write (i10); write "\n";       // 10
    write (i20); write "\n";       // 20
    write *pi10; write "\n";       // 10
    write *pi20; write "\n";       // 20
    write (*pi10); write "\n";     // 10
    write *(pi20); write "\n";     // 20
    write i10 + i10; write "\n";   // 20
    write i20 + i10; write "\n";   // 30
    write i10 + i20; write "\n";   // 30
    write i20 + i20; write "\n";   // 40
    write i10 - i10; write "\n";   // 0
    write i20 - i10; write "\n";   // 10
    write i10 - i20; write "\n";   // -10
    write i20 - i20; write "\n";   // 0
    write i10 * i10; write "\n";   // 100
    write i20 * i10; write "\n";   // 200
    write i10 * i20; write "\n";   // 200
    write i20 * i20; write "\n";   // 400
    write i10 / i10; write "\n";   // 1
    write i20 / i10; write "\n";   // 2
    write i10 / i20; write "\n";   // 0
    write i20 / i20; write "\n";   // 1
    write i10 % i10; write "\n";   // 0
    write i20 % i10; write "\n";   // 0
    write i10 % i20; write "\n";   // 10
    write i20 % i20; write "\n";   // 0
    write i10 ^ 2; write "\n";     // 100
    write 2 ^ i10; write "\n";     // 1024
    write -i10; write "\n";        // -10
    write -i20; write "\n";        // -20
    
    // Test assignment and is operator for the primitives:
    b = 42;  write b; write "\n";            // 42
    bool = true; write bool; write "\n";     // true
    bool = false; write bool; write "\n";    // false
    s = "The quick brown fox\n"; write s;    // The quick brown fox
    s = "jumped\n"; write s;                 // jumped
 
    // Test reading of the primitive types:
    write "Type an integer: ";
    read b;
    write "You typed ";
    write b;
    write "\n";

    write "Type a boolean (true or false): ";
    read bool;
    write "You typed ";
    write bool;
    write "\n";

    write "Type a string: ";
    read s;
    write "You typed ";
    write s;
    write "\n";

    return 0;
}
