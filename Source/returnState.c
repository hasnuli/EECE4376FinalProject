/*
 * returnState.c
 *
 *  Created on: 	Dec 7, 2015
 *  Last Edited:	Dec 18 2015
 *  Author:			Ilham Hasnul
 *  Description:	Defines a function to be used by the Python library.
 */

#include <main.c>

// Returns the state variable for the use of the Python library
int returnState() {
	int temp;
	pthread_mutex_lock(&stateMutex);
	temp = state;
	pthread_mutex_unlock(&stateMutex);
	return temp;
}
