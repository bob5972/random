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
#include <iostream>

#include "Random.h"
#include "MBString.hpp"
#include "MBTypes.h"
#include "MBUtil.h"
#include "MBVector.hpp"

NORETURN void PrintUsageAndExit()
{
    printf("\n");
    printf("random version 1.2\n");
    printf("Usage: random [options] min max\n");
    printf("   or  random [options] diceSpec (e.g. 2d6 1d20+4 1d20a 1d20+3d)\n");
    printf("   or  random [options] -s str1 str2 ...\n");
    printf("  -c count    run \"count\" times\n");
    printf("  -i          read diceSpec from stdIn\n");
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

void DoBounds(int min, int max)
{
    int value;
    if (min > max) {
        int temp = min;
        min = max;
        max = temp;
    }

    value = Random_Int(min, max);
    printf("%d\n", value);
}

void DoStrings(MBVector<MBString> &strings)
{
    int r = Random_Int(0, strings.size() - 1);
    printf("%s\n", strings.get(r).CStr());
}

void DoDice(MBVector<DiceRoll> &dice)
{
    int value;
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
}

DiceRoll ParseDice(const MBString str)
{
    DiceRoll curDie;
    MBUtil_Zero(&curDie, sizeof(curDie));

    int d = str.find('d');
    if (d == -1) {
        printf("Error %d: Malformed dice roll (eg 1d6): %s\n", __LINE__, str.CStr());
        PrintUsageAndExit();
    }

    MBString nDiceStr = str.substr(0, d);

    if (d + 1 == str.length()) {
        printf("Error %d: Malformed dice roll (eg 1d6): %s\n", __LINE__, str.CStr());
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
                printf("Error %d: Dice modifier not a number (eg 1d6+3): %s\n", __LINE__, restStr.CStr());
                PrintUsageAndExit();
            }
            curDie.mod = atoi(restStr.CStr());
        } else {
            printf("Error %d: Malformed dice modifier (eg 1d6+3): expected +/-, got %d, str=%s\n",
                   __LINE__, restStr.getChar(0), restStr.CStr());
            PrintUsageAndExit();
        }
    }

    return curDie;
}

int main(int argc, char *argv[])
{
	int count = 1;
	MBString str;
	MBVector<DiceRoll> dice;
	MBVector<MBString> strings;
	int bUsed = 0;
	int bounds[2];

	bool useDice = FALSE;
	bool useStrings = FALSE;
        bool useStdIn = FALSE;

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
		} else if (str == "-i") {
            useStdIn = TRUE;
        } else if (str == "-h" ||
                   str == "--help") {
		    // Print help text.
		    PrintUsageAndExit();
		} else if (str.find('d') != -1) {
			dice.push(ParseDice(str));
            useDice = TRUE;
		} else {
			if (useDice || useStrings || useStdIn) {
                printf("Error: Can't mix min/max, dice, strings, or stdIn\n");
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

	if (useStdIn) {
        if (bUsed != 0) {
            printf("Error: Can't mix min/max and stdIn\n");
            PrintUsageAndExit();
        }
        if (useDice || useStrings) {
            printf("Error: Can't mix stdIn and other modes\n");
            PrintUsageAndExit();
        }
        if (count != 1) {
            printf("Error: Can't mix count and stdIn\n");
            PrintUsageAndExit();
        }
    } else {
        if (bUsed != 2 && !(useDice || useStrings)) {
            printf("Error: Not enough bounds\n");
            PrintUsageAndExit();
        }

        if (useDice && useStrings) {
            printf("Error: Can't mix dice and strings\n");
            PrintUsageAndExit();
        }

        if (useStdIn) {
            printf("Error: Can't mix stdIn and other modes\n");
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
    }

	Random_Init();

    if (useStdIn) {
        MBString line;
        MBString str;
        MBVector<MBString> strings;
        MBVector<DiceRoll> dice;
        while (MBString_GetLine(std::cin, line)) {
            strings.makeEmpty();
            dice.makeEmpty();

            line = line.stripWS();

            str = "";
            for (int x = 0; x < line.length(); x++) {
                if (!MBUtil_IsWhitespace(line.getChar(x))) {
                    str += line.getChar(x);
                } else if (str.length() > 0) {
                    strings.push(str);
                    str = "";
                }
            }

            if (str.length() > 0) {
                strings.push(str);
                str = "";
            }

            for (int x = 0; x < strings.size(); x++) {
                dice.push(ParseDice(strings[x]));
            }

            DoDice(dice);
            printf("\n");
        }
    } else {
        for (int i = 0; i < count; i++) {
            if (useStrings) {
                DoStrings(strings);
            } else if (useDice) {
                DoDice(dice);
            } else {
                DoBounds(bounds[0], bounds[1]);
            }
        }
    }

	Random_Exit();
	return 0;
}
