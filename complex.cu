|int| x;
|int| y is initially 42;

function f(|int| x) returns |int|
{
    return x*x*x;
}

function test_syntax 
(|string| s, |int| x, |float| y, |bool| z)
{
    pointer to |int| p;
    array of |int| n;
    |int| i;

    x = 42;
    y = 42.8e+9;
    s = "Hello, World!";
    z = true;
    z = false;
    n = (array of |int| is 1,2,3,42);
    p = (new |int| is 42);
    free p;
    x = (3 + 4) + 5 * 6 / 7 % 8 + -9 + +10 ^ 2;
    z = z or false and not z;
    if (x == 42) then x = 10; fi
    if (x != 42) then x = 10; fi
    if (x <= 42) then x = 10; fi
    if (x >= 42) then x = 10; fi
    if (x < 42) then x = 10; fi
    if (x > 42) then x = 10; fi
    p = @x;
    *p = 45;
    // y = ++x;
    // y = x++;
    // y = --x;
    // y = x--;
    write "About to write f(x): ";
    write f(x);
    f(x);
    write " Done.\n";
    write "About to write n[3]: ";
    write n[3];
    write " Done.\n";
    write "Please type a new value for n[3]: ";
    read n[3];
    if z then 
        write x; 
    else 
        write y; 
    fi
    while z do
        f(x);
    od
    do
        f(x);
    until z od
    return;
}

function main( ) returns |int|
{
    |string| main_s;
    |bool| main_z;

    test_syntax(main_s, x, y, main_z);
    return 0;
}
