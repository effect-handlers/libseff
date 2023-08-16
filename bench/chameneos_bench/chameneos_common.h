#include <stdio.h>
#include <math.h>

typedef enum {Blue, Red, Yellow} colour;

colour allColours[] = {Blue, Red, Yellow};
char* colourNames[] = {"Blue", "Red", "Yellow"};
size_t colours = 3;

colour firstCreatures[] = {Blue, Red, Yellow};
size_t firstCreaturesSize = 3;
colour secondCreatures[] = {Blue, Red, Yellow, Red, Yellow, Blue, Red, Yellow, Red, Blue};
size_t secondCreaturesSize = 10;

colour complement(colour a, colour b) {
    if (a == b) {
        return a;
    } else {
        return (colour)(3 ^ (a | b)); // 3 == 0b11
    }
}

void print_complements(){
    for(int i = 0; i < colours; ++i){
        for (int j = 0; j < colours; ++j){
            printf(
                "%s + %s -> %s\n",
                colourNames[allColours[i]],
                colourNames[allColours[j]],
                colourNames[complement(allColours[i], allColours[j])]
            );
        }
    }
}

int int_pow(int base, int exp) {
    if (exp == 0){
        return 1;
    } else if (exp == 1) {
        return base;
    } else {
        int prev = int_pow(base, exp/2);
        prev = prev * prev;
        if (exp % 2) prev *= base;
        return prev;
    }
}

void spell_int(int a) {
    if (a == 0){
        printf("zero\n");
        return;
    }
    for (int base = (int)floor(log10(a)); base >= 0; --base ){
        int mod = a / int_pow(10, base);
        a = a % int_pow(10, base);

        switch (mod)
        {
        #define CASE(n, s) \
        case n: \
            printf(s " "); \
            break;
        CASE(0, "zero")
        CASE(1, "one")
        CASE(2, "two")
        CASE(3, "three")
        CASE(4, "four")
        CASE(5, "five")
        CASE(6, "six")
        CASE(7, "seven")
        CASE(8, "eight")
        CASE(9, "nine")
        #undef CASE
        default:
            printf("Big Error");
            abort();
        }
    }
    printf("\n");
}
