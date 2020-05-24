/*
 * main.cpp -- part of random
 *
 * Copyright (c) 2015-2020 Michael Banack <bob5972@banack.net>
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

bool isNumber(const MBString &str)
{
	int x = 0;
	int len = str.length();
	
	if (len == 0) {
		return FALSE;
	}
	
	if (str.getChar(0) == '-' && len > 1) {
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
} DiceRoll;

int main(int argc, char *argv[])
{
	int min, max;
	int value;
	int count = 1;
	MBString str;
	MBVector<DiceRoll> dice;
	MBVector<MBString> strings;
	bool error = FALSE;
	
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
				error = TRUE;
				break;
			}
			i++;
			count = atoi(argv[i]);
		} else if (str == "-s") {
		    //Overloading loop variable!
            for (i = i+1; i < argc; i++) {
                strings.push(argv[i]);
            }
            useStrings = TRUE;
		} else if (str == "-h") {
		    // Print help text.
		    error = TRUE;
		} else if (str.find('d') != -1) {
			DiceRoll curDie;
			int d = str.find('d');
			MBString one = str.substr(0, d);
			MBString two = str.substr(d + 1, str.length() - d - 1);
			curDie.numDice = atoi(one.CStr());
			curDie.diceMax = atoi(two.CStr());
			useDice = TRUE;
			
			dice.push(curDie);
		} else {
			if (useDice || useStrings) {
			    TRACE();
				error = TRUE;
			}
			
			if (bUsed >= (int) ARRAYSIZE(bounds)) {
    			TRACE();
				error = TRUE;
				break;
			}
			
			if (isNumber(str)) {
				bounds[bUsed] = atoi(str.CStr());
				bUsed++;
			} else {
			    TRACE();
				error = TRUE;
				break;
			}
		}
	}
	
	if (bUsed != 2 && !(useDice || useStrings)) {
	    TRACE();
	    error = TRUE;
    }
    
    if (useDice && useStrings) {
        TRACE();
        error = TRUE;
    }
    
    if (count <= 0) {
	    TRACE();
		error = TRUE;
	}
	
	if (useStrings && strings.size() == 0) {
	    TRACE();
	    error = TRUE;
    }
	
	if (error) {
		printf("Usage: random [options] min max\n");
		printf("   or  random [options] diceSpec (e.g. 2d6)\n");
		printf("   or  random [options] -s str1 str2 ...\n");		
		printf("  -c count    run \"count\" times\n");
		return 1;
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
	    if (useStrings) {
	        int r = Random_Int(0, strings.size() - 1);
	        printf("%s\n", strings.get(r).CStr());
	    } else if (useDice) {
	        for (int d = 0; d < dice.size(); d++) {
			    value = Random_DiceSum(dice[d].numDice, dice[d].diceMax);
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

