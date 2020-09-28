/*
 * main.cpp -- part of random
 *
 * Copyright (c) 2015-2020 Michael Banack <github@banack.net>
 *
 * MIT License
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>

#include "random.h"
#include "MBString.hpp"
#include "mbtypes.h"
#include "mbutil.h"
#include "MBVector.hpp"

NORETURN void PrintUsageAndExit()
{
    printf("\n");
    printf("random version 1.1\n");
    printf("Usage: random [options] min max\n");
    printf("   or  random [options] diceSpec (e.g. 2d6)\n");
    printf("   or  random [options] -s str1 str2 ...\n");
    printf("  -c count    run \"count\" times\n");
    exit(1);
}

bool isNumber(const MBString &str)
{
	int x = 0;
	int len = str.length();
	
	if (len == 0) {
		return FALSE;
	}
	
	if ((str.getChar(0) == '-' || str.getChar(0) == '+') && len > 1) {
		x++;
	}
	
	while (x < len) {
		if (str.getChar(x) < '0' || str.getChar(x) > '9') {
			return FALSE;
		}
		x++;
	}
	
	return TRUE;
}

typedef struct {
	int numDice;
	int diceMax;
    int mod;
    char type;
} DiceRoll;

int main(int argc, char *argv[])
{
	int min, max;
	int count = 1;
	MBString str;
	MBVector<DiceRoll> dice;
	MBVector<MBString> strings;
	int bUsed = 0;
	int bounds[2];
	
	bool useDice = FALSE;
	bool useStrings = FALSE;
	
	/*
	 * Parse arguments.
	 
	 * Skip argv[0] (program name).
	 */
	for (int i = 1; i < argc; i++) {
		str = argv[i];
		
		if (str == "-c") {
			if (i + 1 >= argc) {
                printf("Error: -c requires an argument\n");
				PrintUsageAndExit();
			}
			i++;
			count = atoi(argv[i]);
		} else if (str == "-s") {
		    //Overloading loop variable!
            for (i = i+1; i < argc; i++) {
                strings.push(argv[i]);
            }
            useStrings = TRUE;
		} else if (str == "-h" ||
                   str == "--help") {
		    // Print help text.
		    PrintUsageAndExit();
		} else if (str.find('d') != -1) {
			DiceRoll curDie;
            MBUtil_Zero(&curDie, sizeof(curDie));

			int d = str.find('d');
			MBString nDiceStr = str.substr(0, d);

            if (d + 1 == str.length()) {
                printf("Error: Malformed dice roll (eg 1d6)\n");
                PrintUsageAndExit();
            }

            int len = 1;
            while (d + len < str.length() &&
                   str.getChar(d + len) >= '0' &&
                   str.getChar(d + len) <= '9') {
                len++;
            }
            len--;
            ASSERT(len >= 1);

			MBString maxRollStr = str.substr(d + 1, len);
            MBString restStr = str.substr(d + 1 + len, str.length() - d - 1 - len);

			curDie.numDice = atoi(nDiceStr.CStr());
			curDie.diceMax = atoi(maxRollStr.CStr());

            if (restStr.length() > 0) {
                if (restStr.lastChar() == 'a' ||
                    restStr.lastChar() == 'A') {
                    curDie.type = 'a';
                    restStr = restStr.substr(0, restStr.length() - 1);
                } else if (restStr.lastChar() == 'd' ||
                           restStr.lastChar() == 'D') {
                    curDie.type = 'd';
                    restStr = restStr.substr(0, restStr.length() - 1);
                }
            }

            if (restStr.length() > 0) {
                if (restStr.getChar(0) == '+' ||
                    restStr.getChar(0) == '-') {
                    if (!isNumber(restStr)) {
                        printf("Error: Dice modifier not a number (eg 1d6+3)\n");
                        PrintUsageAndExit();
                    }
                    curDie.mod = atoi(restStr.CStr());
                } else {
                    printf("Error: Malformed dice modifier (eg 1d6+3)\n");
                    PrintUsageAndExit();
                }
            }

			useDice = TRUE;
			
			dice.push(curDie);
		} else {
			if (useDice || useStrings) {
                printf("Error: Can't mix min/max, dice, and strings\n");
				PrintUsageAndExit();
			}
			
			if (bUsed >= (int) ARRAYSIZE(bounds)) {
    			printf("Error: Too many bounds?\n");
				PrintUsageAndExit();
			}
			
			if (isNumber(str)) {
				bounds[bUsed] = atoi(str.CStr());
				bUsed++;
			} else {
			    printf("Error: Bound is not a number?\n");
				PrintUsageAndExit();
			}
		}
	}
	
	if (bUsed != 2 && !(useDice || useStrings)) {
	    printf("Error: Not enough bounds\n");
        PrintUsageAndExit();
    }
    
    if (useDice && useStrings) {
        printf("Error: Can't mix dice and strings\n");
        PrintUsageAndExit();
    }
    
    if (count <= 0) {
	    printf("Error: Count must be >= 0\n");
        PrintUsageAndExit();
	}
	
	if (useStrings && strings.size() == 0) {
	    printf("Error: String mode selected, but no strings found\n");
        PrintUsageAndExit();
    }
	
	min = bounds[0];
	max = bounds[1];
	
	if (min > max) {
		int temp = min;
		min = max;
		max = temp;
	}
	
	//printf("min = %d\n", min);
	//printf("max = %d\n", max);
	
	Random_Init();
	
	for (int i = 0; i < count; i++) {
        int value;

	    if (useStrings) {
	        int r = Random_Int(0, strings.size() - 1);
	        printf("%s\n", strings.get(r).CStr());
	    } else if (useDice) {
	        for (int d = 0; d < dice.size(); d++) {

                value = Random_DiceSum(dice[d].numDice, dice[d].diceMax);
                if (dice[d].type == 'a') {
                    int reroll = Random_DiceSum(dice[d].numDice, dice[d].diceMax);
                    value = MAX(value, reroll);
                } else if (dice[d].type == 'd') {
                    int reroll = Random_DiceSum(dice[d].numDice, dice[d].diceMax);
                    value = MIN(value, reroll);
                }
                value += dice[d].mod;
			    printf("%d\n", value);
		    }
	    } else {
		    value = Random_Int(min, max);
		    printf("%d\n", value);
	    }
    }
	
	Random_Exit();	
	return 0;
}

