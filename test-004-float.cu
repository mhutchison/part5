|float| f;
function main() returns |int|
{
    write 42.3; write "\n";
    f = 42;
    write f; write "\n";
    write 10.3 + 8.9; write "\n";
    write 10 + 8.9; write "\n";
    write 10.3 + 8; write "\n";
    write 10.3 - 8.9; write "\n";
    write 10 - 8.9; write "\n";
    write 10.3 - 8; write "\n";
    write 100.3 * 8.9; write "\n";
    write 10 * 8.9; write "\n";
    write 10.3 * 8; write "\n";
    write 100.3 / 8.9; write "\n";
    write 10 / 8.9; write "\n";
    write 10.3 / 8; write "\n";
    write 2.1 ^ 5.1; write "\n";

    write (4.3 < 2.4); write "\n";
    write (0.3 < 2.4); write "\n";
    write (4.3 < 4.3); write "\n\n";

    write (0.3 > 2.4); write "\n";
    write (4.3 > 2.4); write "\n";
    write (4.3 > 4.3); write "\n\n";

    write (4.3 <= 2.4); write "\n";
    write (0.3 <= 2.4); write "\n";
    write (4.3 <= 4.3); write "\n\n";

    write (0.3 >= 2.4); write "\n";
    write (4.3 >= 2.4); write "\n";
    write (4.3 >= 4.3); write "\n\n";

    write (0.3 == 4.3); write "\n";
    write (4.3 == 4.3); write "\n\n";

    write (4.3 != 4.3); write "\n";
    write (0.3 != 4.3); write "\n";

    write "Please type a float number: ";
    read f;
    write f; write "\n";
    write "Rounds to: ";
    write round f; write "\n";

    return 0;
}
