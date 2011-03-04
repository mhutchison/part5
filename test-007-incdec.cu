function main( ) returns |int|
{
    |int| x is initially 10;
    |int| y;
    pointer to |int| p;

    y = x++;    // x=11 y=10
    write x; write " "; write y; write "\n";

    y = x--;    // x=10 y=11
    write x; write " "; write y; write "\n";

    y = ++x;    // x=11 y=11
    write x; write " "; write y; write "\n";

    y = --x;    // x=10 y=10
    write x; write " "; write y; write "\n";

    p = @(--x); // x=9 and p points to x
    write x; write " "; write *p; write "\n";

    p = @(++x); // x=10 and p points to x
    write x; write " "; write *p; write "\n";
}

