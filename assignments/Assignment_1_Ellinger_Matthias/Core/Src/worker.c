/*
 * worker.c
 *
 *  Created on: Apr 13, 2026
 *      Author: matth
 */


/**
* @brief Function recursively calculating the factorial
* @param argument: n Number to calculate factorial of
* @retval: factorial of n : int
*/
int recursive_factorial(int n)
{
	if(n == 1)
	{
		return 1;
	}

	return n*recursive_factorial(n-1);
}
