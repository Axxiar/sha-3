#include <stdio.h>

int usage(char *program_name)
{
    printf("Usage: %s [file]\n", program_name);
    return 1;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Invalid number of parameters\n");
        return usage(argv[0]);
    }
    return 0;
}
