function f (ref |int| x)
{
    x = 19;
}

function main ( ) returns |int|
{
    |int| y is initially 42;
    f(y);
    write y;
}
