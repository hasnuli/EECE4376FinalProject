/*
 * main.c
 *
 *  Created on: Dec 2, 2015
 *      Author: PC admin
 */

#include <stdlib.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include "../../BBBIO/BBBio_lib/BBBiolib.h"

// define needed constants
#define BIT_DEPTH					16
#define RECORDING_SAMPLERATE		96000*BIT_DEPTH
#define RECORDING_PERIOD_NS			((1/RECORDING_SAMPLERATE)*BIT_DEPTH*1000000000)
#define TUNER_SAMPLERATE			22050*BIT_DEPTH
#define TUNER_PERIOD_NS				((1/TUNER_SAMPLERATE)*BIT_DEPTH*1000000000)
#define AUDIOCAPE_BITRATE			96000*BIT_DEPTH
#define AUDIOCAPE_PERIOD_NS			(1/AUDIOCAPE_BITRATE)*1000000000
#define RECORDING_PERIOD_DIFFERENCE	RECORDING_PERIOD_NS-AUDIOCAPE_PERIOD_NS
#define TUNER_PERIOD_DIFFERENCE		TUNER_PERIOD_NS-AUDIOCAPE_PERIOD_NS
#define TUNER_UPDATE				40
#define TUNER_UPDATE_PERIOD_NS		(1/TUNER_UPDATE)*1000000000
#define	TUNER_ARRAY_SIZE			((TUNER_SAMPLERATE*BIT_DEPTH)/TUNER_UPDATE)+1
#define FILE_NAME 					"file.bin"

//Macros needed for Pitch Detection Algorithm
#define SAMPLERATE 					22050
#define UPDATERATE 					40
#define PI 							3.14159256
#define MAXFREQ 					360.0	//Highest note freq (F above E4)
#define MINFREQ 					63.0	//Lowest note (B below E2)
#define MAXP 						360 	//(22,050/63) - Number of samples in the largest signal period (lowest freq)
#define MINP 						55 		//(22,050/360) - Number of samples in the smallest signal period (highest freq)
#define SUBMULTTHRESH 				0.90	//Threshold to determine harmonics
#define numSamples 					SAMPLERATE/UPDATERATE	//Number of samples used in one algorithm run through

//Arrays used for pitch algorithm and note reading
static const uint64_t RECORDING_ARRAY_SIZE = ((uint64_t) RECORDING_SAMPLERATE* BIT_DEPTH * 60 * 3) + 2;
static char tunerArrayAChar[TUNER_ARRAY_SIZE];
static char tunerArrayBChar[TUNER_ARRAY_SIZE];
static int tunerArrayA[TUNER_ARRAY_SIZE / BIT_DEPTH];
static int tunerArrayB[TUNER_ARRAY_SIZE / BIT_DEPTH];
static pthread_mutex_t mutexA = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutexB = PTHREAD_MUTEX_INITIALIZER;
static char currentArray;


static int lagArray[numSamples];	//Working array for algorithm
static double pitch = 0;			//Calculated frequency of pitch

//Initial button states
static int stopButton = 0;
static int modeButton = 0;
static int playButton = 0;
static int onButton = 1;

//Mutexes and conditionals for each state
static int state = 0;
static pthread_mutex_t stateMutex = PTHREAD_MUTEX_INITIALIZER;
//static pthread_mutex_t tunerStateMutex = PTHREAD_MUTEX_INITIALIZER;
//static pthread_mutex_t recordingStateMutex = PTHREAD_MUTEX_INITIALIZER;
//static pthread_mutex_t playbackStateMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t tunerStateCond = PTHREAD_COND_INITIALIZER;
static pthread_cond_t recordingStateCond = PTHREAD_COND_INITIALIZER;
static pthread_cond_t playbackStateCond = PTHREAD_COND_INITIALIZER;

void readAudioData(int mArraySize, char mArray[]) {

	struct timespec currentTime;
	struct timespec lastChange;
	char cur;
	char prev;
	int curPos = 0;
	int bitsRead = 0;
	int numOfPeriods;
	int i;
	uint64_t timeDifferenceNS;
	uint64_t audioPeriod;
	uint64_t difference;
	if (state == 1) {
		audioPeriod = RECORDING_PERIOD_NS;
		difference = RECORDING_PERIOD_DIFFERENCE;
	} else {
		audioPeriod = TUNER_PERIOD_NS;
		difference = TUNER_PERIOD_DIFFERENCE;
	}
	// Set any pin needed to boot up and set Audio Cape as outwards direction
	system("");	// Put in any terminal commands needed to boot up and set Audio Cape

	if (is_high(9, 25)) {
		cur = '1';
		mArray[curPos] = '1';
		prev = '1';
	} else {
		cur = '0';
		mArray[curPos] = '0';
		prev = '0';
	}
	++curPos;
	++bitsRead;
	clock_gettime(CLOCK_REALTIME, &currentTime);
	lastChange = currentTime;

	while (curPos < mArraySize - 1 && (state == 1 && stopButton == 0)/*&& no buttons are pressed*/) {
		if (is_high(9, 25)) {
			cur = '1';
		} else {
			cur = '0';
		}
		if (cur != prev) {
			clock_gettime(CLOCK_REALTIME, &currentTime);
			if (currentTime.tv_nsec < lastChange.tv_nsec) {
				timeDifferenceNS = ((1000000000)
						* (currentTime.tv_sec - 1 - lastChange.tv_sec))
								+ ((1000000000 + currentTime.tv_nsec)
										- lastChange.tv_nsec);
			} else {
				timeDifferenceNS = ((1000000000)
						* (currentTime.tv_sec - lastChange.tv_sec))
								+ (currentTime.tv_nsec - lastChange.tv_nsec);
			}
			numOfPeriods = timeDifferenceNS / AUDIOCAPE_PERIOD_NS;
			for (i = 0; (i < numOfPeriods) && (bitsRead < BIT_DEPTH); ++i) {
				mArray[curPos] = cur;
				++curPos;
				++bitsRead;
			}
			prev = cur;
			if (bitsRead > BIT_DEPTH) {
				bitsRead = 0;
				if(difference!=0){
					currentTime.tv_nsec += difference;
					clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &currentTime,
							NULL);
				}
			}
		}
	}
	mArray[curPos] = 'f';
	for (i = curPos + 1; i < mArraySize - 1; ++i) {
		mArray[i] = 0;
	}
}

void *recordingThreadBody(void *arg) {
	int oldstate;
	pthread_setcanceltype(PTHREAD_CANCEL_ENABLE, &oldstate);

	while (1) {
		pthread_mutex_lock(&stateMutex);
		while (state != 1) {
			pthread_cond_wait(&recordingStateCond, &stateMutex);
		}
		pthread_mutex_unlock(&stateMutex);
		char recordingArray[RECORDING_ARRAY_SIZE];
		FILE* outfile;
		readAudioData(RECORDING_ARRAY_SIZE, recordingArray);
		outfile = fopen(FILE_NAME, "wb");
		if (outfile == NULL) {
			printf("Error opening file");
			return NULL;
		}
		fwrite(&recordingArray, 1, RECORDING_ARRAY_SIZE, outfile);
		fclose(outfile);
		pthread_mutex_lock(&stateMutex);
		state = 3;
		pthread_mutex_unlock(&stateMutex);
	}
	return NULL;
}

void *readerThreadBody(void *arg) {
	int oldstate;
	pthread_setcanceltype(PTHREAD_CANCEL_ENABLE, &oldstate);

	while (1) {
		pthread_mutex_lock(&stateMutex);
		while (state != 0) {
			pthread_cond_wait(&tunerStateCond, &stateMutex);
		}
		pthread_mutex_unlock(&stateMutex);
		int temp;
		int i;
		int j;
		currentArray = 'A';
		if (currentArray == 'A') {
			pthread_mutex_lock(&mutexA);
			readAudioData(TUNER_ARRAY_SIZE, tunerArrayAChar);
			for (i = 0; i < TUNER_ARRAY_SIZE; i += BIT_DEPTH) {
				if (tunerArrayAChar[i] == 'f') {
					break;
				}
				temp = 0;
				for (j = 0; j < BIT_DEPTH; ++j) {
					if (tunerArrayAChar[i + j] == 1) {
						temp = temp + (2 ^ (BIT_DEPTH - j - 1));
					}
					++i;
				}
				tunerArrayA[i / BIT_DEPTH] = temp;
			}
			pthread_mutex_unlock(&mutexA);
			currentArray = 'B';
		} else {
			pthread_mutex_lock(&mutexB);
			readAudioData(TUNER_ARRAY_SIZE, tunerArrayBChar);
			for (i = 0; i < TUNER_ARRAY_SIZE; i += BIT_DEPTH) {
				if (tunerArrayBChar[i] == 'f') {
					break;
				}
				temp = 0;
				for (j = 0; j < BIT_DEPTH; ++j) {
					if (tunerArrayBChar[i + j] == 1) {
						temp = temp + (2 ^ (BIT_DEPTH - j - 1));
					}
				}
				tunerArrayB[i / BIT_DEPTH] = temp;
				pthread_mutex_unlock(&mutexB);
			}
			currentArray = 'A';
		}
	}
	return NULL;
}

double autoCorrelation() {
	int bestP = MINP;
	int p;
	for (p = MINP - 1; p <= MAXP + 1; p++) {
		double ac = 0.0;        // Standard auto-correlation
		double sumSqBeg = 0.0;  // Sum of squares of beginning part
		double sumSqEnd = 0.0;  // Sum of squares of ending part

		int i;
		for (i = 0; i < (numSamples - p); i++) {
			ac += lagArray[i] * lagArray[i + p];
			sumSqBeg += lagArray[i] * lagArray[i];
			sumSqEnd += lagArray[i + p] * lagArray[i + p];
		}

		lagArray[p - MINP] = ac / sqrt(sumSqBeg * sumSqEnd);
		//Period/num of Samples long is array's max + MINP

		//Find peak

		int j;
		for (j = MINP; j <= MAXP; j++) {
			if (lagArray[j] > lagArray[bestP])
				bestP = j;
		}

		//Could do some interpolating, not going to...

		//Check if freq found is a harmonic by looking at other peaks
		int maxMul = bestP / MINP;
		int found = 0;
		int mul;
		for (mul = maxMul; found == 0 && mul >= 1; mul--) {
			int subsAllStrong = 1;

			//For each multiple...
			int k;
			for (k = 1; k < mul; k++) {
				int subMulP = (int) (k * bestP / mul + 0.5);
				if (lagArray[subMulP] < SUBMULTTHRESH * lagArray[bestP])
					subsAllStrong = 0;
			}

			if (subsAllStrong == 1) {
				found = 1;
				bestP = bestP / mul;
				break;
			}
		}
	}
	printf("Best phase lag is %d", bestP);
	//return bestP;
	return SAMPLERATE / bestP;
}

void *pdaThreadBody(void *arg) {
	int oldstate;
	pthread_setcanceltype(PTHREAD_CANCEL_ENABLE, &oldstate);

	while (1) {
		pthread_mutex_lock(&stateMutex);
		while (state != 0) {
			pthread_cond_wait(&tunerStateCond, &stateMutex);
		}
		pthread_mutex_unlock(&stateMutex);
		if (currentArray == 'A') {
			pthread_mutex_lock(&mutexB);
			pitch = autoCorrelation(tunerArrayB);
			pthread_mutex_unlock(&mutexB);
		} else if (currentArray == 'B') {
			pthread_mutex_lock(&mutexA);
			pitch = autoCorrelation(tunerArrayA);
			pthread_mutex_unlock(&mutexA);
		}
	}
	return NULL;
}



void *playbackThreadBody(void *arg) {
	int oldstate;
	pthread_setcanceltype(PTHREAD_CANCEL_ENABLE, &oldstate);

	while (1) {
		pthread_mutex_lock(&stateMutex);
		while (state != 2) {
			pthread_cond_wait(&playbackStateCond, &stateMutex);
		}
		pthread_mutex_unlock(&stateMutex);
		// Declare variables needed for
		unsigned char* buffer;
		FILE *fp;
		unsigned long fileLen;
		int i;

		if ((fp = fopen(FILE_NAME, "rb")) == NULL) {
			printf("Error opening file\n");
		}

		// Get the file length for playback
		fseek(fp, 0, SEEK_CUR);
		fileLen = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		// Allocate memory into the buffer
		buffer = (unsigned char *) malloc(fileLen + 1);
		if (!buffer) {
			fprintf(stderr, "Memory error!");
			fclose(fp);
			return NULL;
		}

		// Read file contents into buffer
		fread(buffer, fileLen, 1, fp);
		fclose(fp);

		// Run through the file and play the music

		for (i = 0; (i < fileLen) && (state == 2); ++i) {
			if (buffer[i] == '1')
				pin_high(9, 28);
			else
				pin_low(9, 28);

			nanosleep((const struct timespec[] ) { {0, 651.041667L}}, NULL);
			if (playButton == 1) {// while play button is held down, it will replay the recorded file purposefully
				i = 0;
			}
		}
		pthread_mutex_lock(&stateMutex);
		state = 3;
		pthread_mutex_unlock(&stateMutex);
	}
	return NULL;
}

int main(void) {

//	iolib_init();
//			iolib_setdir(9, 25, BBBIO_DIR_IN);
	//		iolib_setdir(9, 28, BBBIO_DIR_OUT);
	//		iolib_setdir(8, 13, BBBIO_DIR_IN);	// Play Button
	//		iolib_setdir(8, 15, BBBIO_DIR_IN);	// Mode Button
	//		iolib_setdir(8, 17, BBBIO_DIR_IN);	// Stop Button
	//		iolib_setdir(8, 7, BBBIO_DIR_OUT);
	//		iolib_setdir(8, 8, BBBIO_DIR_OUT);
	//		iolib_setdir(8, 9, BBBIO_DIR_OUT);
	//		iolib_setdir(8, 10, BBBIO_DIR_OUT);
	//		iolib_setdir(8, 11, BBBIO_DIR_OUT);
	//		iolib_setdir(8, 12, BBBIO_DIR_OUT);
	//		iolib_setdir(8, 14, BBBIO_DIR_OUT);
	//		iolib_setdir(8, 16, BBBIO_DIR_OUT);
	//		iolib_setdir(8, 18, BBBIO_DIR_OUT);



	pthread_t mainThread, recordingThread, playbackThread, pdaThread,
	readerThread, screenThread;
	struct sched_param mainParam, recorderParam, playbackParam, pdaParam,
	readerParam, screenParam;
	pthread_attr_t recorderAttr, playbackAttr, pdaAttr, readerAttr, screenAttr;
	struct timespec currentTime, firstPushed;

	mainParam.__sched_priority = 4;
	recorderParam.__sched_priority = 2;
	playbackParam.__sched_priority = 2;
	pdaParam.__sched_priority = 3;
	readerParam.__sched_priority = 3;
	screenParam.__sched_priority = 1;

	while (onButton == 1) {

		stopButton=0;
		modeButton=0;
		playButton=0;
		state=1;

		mainThread = pthread_self();
		pthread_setschedparam(mainThread, SCHED_OTHER, &mainParam);

		pthread_attr_init(&recorderAttr);
		pthread_attr_setinheritsched(&recorderAttr, PTHREAD_EXPLICIT_SCHED);
		pthread_attr_setschedpolicy(&recorderAttr, SCHED_OTHER);
		pthread_attr_setschedparam(&recorderAttr, &recorderParam);

		pthread_attr_init(&playbackAttr);
		pthread_attr_setinheritsched(&playbackAttr, PTHREAD_EXPLICIT_SCHED);
		pthread_attr_setschedpolicy(&playbackAttr, SCHED_OTHER);
		pthread_attr_setschedparam(&playbackAttr, &playbackParam);

		pthread_attr_init(&pdaAttr);
		pthread_attr_setinheritsched(&pdaAttr, PTHREAD_EXPLICIT_SCHED);
		pthread_attr_setschedpolicy(&pdaAttr, SCHED_OTHER);
		pthread_attr_setschedparam(&pdaAttr, &pdaParam);

		pthread_attr_init(&readerAttr);
		pthread_attr_setinheritsched(&readerAttr, PTHREAD_EXPLICIT_SCHED);
		pthread_attr_setschedpolicy(&readerAttr, SCHED_OTHER);
		pthread_attr_setschedparam(&readerAttr, &readerParam);

		pthread_attr_init(&screenAttr);
		pthread_attr_setinheritsched(&screenAttr, PTHREAD_EXPLICIT_SCHED);
		pthread_attr_setschedpolicy(&screenAttr, SCHED_OTHER);
		pthread_attr_setschedparam(&screenAttr, &screenParam);

		iolib_init();
		iolib_setdir(9, 25, BBBIO_DIR_IN);
		iolib_setdir(9, 28, BBBIO_DIR_OUT);
//		iolib_setdir(8, 13, BBBIO_DIR_IN);	// Play Button
//		iolib_setdir(8, 15, BBBIO_DIR_IN);	// Mode Button
//		iolib_setdir(8, 17, BBBIO_DIR_IN);	// Stop Button
//		iolib_setdir(8, 7, BBBIO_DIR_OUT);
//		iolib_setdir(8, 8, BBBIO_DIR_OUT);
//		iolib_setdir(8, 9, BBBIO_DIR_OUT);
//		iolib_setdir(8, 10, BBBIO_DIR_OUT);
//		iolib_setdir(8, 11, BBBIO_DIR_OUT);
//		iolib_setdir(8, 12, BBBIO_DIR_OUT);
//		iolib_setdir(8, 14, BBBIO_DIR_OUT);
//		iolib_setdir(8, 16, BBBIO_DIR_OUT);
//		iolib_setdir(8, 18, BBBIO_DIR_OUT);


		pthread_create(&recordingThread, &recorderAttr, recordingThreadBody,
				NULL);
		pthread_create(&playbackThread, &playbackAttr, playbackThreadBody,
				NULL);
		pthread_create(&pdaThread, &pdaAttr, pdaThreadBody, NULL);
		pthread_create(&readerThread, &readerAttr, readerThreadBody, NULL);
		//pthread_create(&screenThread,&screenAttr,screenThreadBody,NULL);

		pthread_attr_destroy(&recorderAttr);
		pthread_attr_destroy(&playbackAttr);
		pthread_attr_destroy(&pdaAttr);
		pthread_attr_destroy(&readerAttr);
		pthread_attr_destroy(&screenAttr);

		// Only expected to press one button at a time
		while (onButton == 1) {
			int prevModeButton = 0;
			int curModeButton = 0;
			int prevPlayButton = 0;
			int curPlayButton = 0;

			if (is_high(8, 17)) {
				clock_gettime(CLOCK_REALTIME, &firstPushed);
				while (is_high(8, 17)) {
					// Only expected to press one button at a time
				}
				clock_gettime(CLOCK_REALTIME, &currentTime);
				if (currentTime.tv_sec - firstPushed.tv_sec > 2) {
					onButton = 0;
				} else if (currentTime.tv_nsec - firstPushed.tv_nsec
						> 1000000) {
					stopButton = 1;
				} else {
					stopButton = 0;
				}
			}

			if (stopButton == 1) {
				pthread_mutex_lock(&stateMutex);
				if (state == 1) {
					state = 3;
				} else if (state == 3) {
					state = 1;
					pthread_cond_broadcast(&recordingStateCond);
				} else if (state == 2) {
					state = 3;
				}
				pthread_mutex_unlock(&stateMutex);
			}

			if (is_high(8, 15) && prevModeButton == 1) { //needs to detect if pressed or released
				modeButton = 1;
				prevModeButton = curModeButton;
				curModeButton = 1;
			} else {
				prevModeButton = curModeButton;
				if (is_high(8, 15)) {
					curModeButton = 1;
				} else {
					curModeButton = 0;
				}
				modeButton = 0;
			}

			if (modeButton == 1) {
				pthread_mutex_lock(&stateMutex);
				if (state == 0) {
					state = 3;
				} else if (state == 3) {
					state = 0;
					pthread_cond_broadcast(&tunerStateCond);
				}
				pthread_mutex_unlock(&stateMutex);
			}

			if (is_high(8, 13) && prevPlayButton == 1) { //needs to detect if pressed or released
				playButton = 1;
				prevPlayButton = curPlayButton;
				curPlayButton = 1;
			} else {
				prevPlayButton = curPlayButton;
				if (is_high(8, 13)) {
					curPlayButton = 1;
				} else {
					curPlayButton = 0;
				}
				playButton = 0;
			}

			if (playButton == 1) {
				pthread_mutex_lock(&stateMutex);
				if (state == 3) {
					state = 2;
					pthread_cond_broadcast(&playbackStateCond);
				}
				pthread_mutex_unlock(&stateMutex);
			}

		}

		pthread_cancel(recordingThread);
		pthread_cancel(playbackThread);
		pthread_cancel(pdaThread);
		pthread_cancel(readerThread);
		pthread_cancel(screenThread);

		pthread_join(recordingThread, NULL);
		pthread_join(playbackThread, NULL);
		pthread_join(pdaThread, NULL);
		pthread_join(readerThread, NULL);
		pthread_join(screenThread, NULL);
	}

	if (is_high(8, 17)) {
		clock_gettime(CLOCK_REALTIME, &firstPushed);
		while (is_high(8, 17)) {
			// Only expected to press one button at a time
		}
		clock_gettime(CLOCK_REALTIME, &currentTime);
		if (currentTime.tv_sec - firstPushed.tv_sec > 2) {
			onButton = 1;
		}
	}
	iolib_free();
}

