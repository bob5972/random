#include <stdio.h>
#include <stdlib.h>

#include "random.h"
#include "MBString.h"
#include "mbtypes.h"
#include "mbutil.h"
#include "MBVector.h"

bool isNumber(const MBString &str)
{
	int x = 0;
	int len = str.length();
	
	if (len == 0) {
		return FALSE;
	}
	
	if (str[0] == '-' && len > 1) {
		x++;
	}
	
	while (x < len) {
		if (str[x] < '0' || str[x] > '9') {
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
	bool error = FALSE;
	
	int bUsed = 0;
	int bounds[2];
	
	bool useDice = FALSE;
	
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
		} else if (str.find('d') != -1) {
			DiceRoll curDie;
			int d = str.find('d');
			MBString one = str.substr(0, d);
			MBString two = str.substr(d + 1, str.length() - d - 1);
			curDie.numDice = atoi(one.cstr());
			curDie.diceMax = atoi(two.cstr());
			useDice = TRUE;
			
			dice.push(curDie);
		} else {
			if (useDice) {
				error = TRUE;
			}
			
			if (bUsed >= (int) ARRAYSIZE(bounds)) {
				error = TRUE;
				break;
			}
			
			if (isNumber(str)) {
				bounds[bUsed] = atoi(str.cstr());
				bUsed++;
			} else {
				error = TRUE;
				break;
			}
		}
	}
	
	if ((bUsed != 2 && !useDice) || count <= 0) {
		error = TRUE;
	}
	
	if (error) {
		printf("Usage: random [options] min max\n");
		printf("   or  random [options] diceSpec\n");
		printf("  -c count    roll count numbers\n");
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
	
	if (!useDice) {
		for (int i = 0; i < count; i++) {
			value = Random_Int(min, max);
			printf("%d\n", value);
		}
	} else {
		for (int i = 0; i < count; i++) {
			for (int d = 0; d < dice.length(); d++) {
				value = Random_DiceSum(dice[d].numDice, dice[d].diceMax);
				printf("%d\n", value);
			}
		}
	}
	
	Random_Exit();	
	return 0;
}

