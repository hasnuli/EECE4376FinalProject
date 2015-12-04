/*
 * Recording.c
 *
 *  Created on: Nov 30, 2015
 *      Author: PC admin
 */

#include <stdlib.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include "../../BBBIO/BBBio_lib/BBBiolib.h"

// define needed constants
#define BIT_DEPTH					16
#define RECORDING_SAMPLERATE		96000*BIT_DEPTH
#define RECORDING_PERIOD_NS			((1/RECORDING_SAMPLERATE)*BIT_DEPTH*1000000000)
#define TUNER_SAMPLERATE			22225*BIT_DEPTH
#define TUNER_PERIOD_NS				((1/TUNER_SAMPLERATE)*BIT_DEPTH*1000000000)
#define AUDIOCAPE_BITRATE			96000*BIT_DEPTH
#define AUDIOCAPE_PERIOD_NS			(1/AUDIOCAPE_BITRATE)*1000000000
#define RECORDING_PERIOD_DIFFERENCE	RECORDING_PERIOD_NS-AUDIOCAPE_PERIOD_NS
#define TUNER_PERIOD_DIFFERENCE		TUNER_PERIOD_NS-AUDIOCAPE_PERIOD_NS
#define TUNER_UPDATE				40
#define TUNER_UPDATE_PERIOD_NS		(1/TUNER_UPDATE)*1000000000
#define	TUNER_ARRAY_SIZE			(TUNER_SAMPLERATE/BIT_DEPTH)/TUNER_UPDATE
#define FILE_NAME 					"file.bin"

static const uint64_t RECORDING_ARRAY_SIZE=((uint64_t)RECORDING_SAMPLERATE*BIT_DEPTH*60*3)+2;
static char tunerArrayAChar[TUNER_ARRAY_SIZE];
static char tunerArrayBChar[TUNER_ARRAY_SIZE];
static int tunerArrayA[TUNER_ARRAY_SIZE/BIT_DEPTH];
static int tunerArrayB[TUNER_ARRAY_SIZE/BIT_DEPTH];
static pthread_mutex_t mutexA;
static pthread_mutex_t mutexB;
static char currentArray;

void readAudioData(int mArraySize, char mArray[], int bool){

	struct timespec currentTime;
	struct timespec lastChange;
	char cur;
	char prev;
	int curPos=0;
	int bitsRead=0;
	int numOfPeriods;
	int i;
	uint64_t timeDifferenceNS;
	uint64_t audioPeriod;
	uint64_t difference;
	if(bool==0){
		audioPeriod=RECORDING_PERIOD_NS;
		difference=RECORDING_PERIOD_DIFFERENCE;
	}
	else{
		audioPeriod=TUNER_PERIOD_NS;
		difference=TUNER_PERIOD_DIFFERENCE;
	}
	iolib_init();
	iolib_setdir(9,25,BBBIO_DIR_IN);
	// Set any pin needed to boot up and set Audio Cape as outwards direction
	system("");		// Put in any terminal commands needed to boot up and set Audio Cape

	if(is_high(9,25)){
		cur='1';
		mArray[curPos]='1';
		prev='1';
	}
	else{
		cur='0';
		mArray[curPos]='0';
		prev='0';
	}
	++curPos;
	++bitsRead;
	clock_gettime(CLOCK_REALTIME,&currentTime);
	lastChange=currentTime;

	while(curPos<mArraySize-2 /*&& no buttons are pressed*/){
		if(is_high(9,25)){
			cur = '1';
		}
		else{
			cur='0';
		}
		if(cur!=prev){
			clock_gettime(CLOCK_REALTIME,&currentTime);
			if(currentTime.tv_nsec<lastChange.tv_nsec){
				timeDifferenceNS=((1000000000)*(currentTime.tv_sec-1-lastChange.tv_sec))+((1000000000+currentTime.tv_nsec)-lastChange.tv_nsec);
			}
			else{
				timeDifferenceNS=((1000000000)*(currentTime.tv_sec-lastChange.tv_sec))+(currentTime.tv_nsec-lastChange.tv_nsec);
			}
			numOfPeriods=timeDifferenceNS/audioPeriod;
			for(i=0;(i<numOfPeriods)&&(bitsRead<BIT_DEPTH);++i){
				mArray[curPos]=cur;
				++curPos;
				++bitsRead;
			}
			prev=cur;
			if(bitsRead>BIT_DEPTH){
				bitsRead=0;
				currentTime.tv_nsec += difference;
				clock_nanosleep(CLOCK_REALTIME,TIMER_ABSTIME,&currentTime,NULL);
			}
		}
	}
	mArray[curPos]='f';
	for(i=curPos+1;i<mArraySize-1;++i){
		mArray[i]=0;
	}
	mArray[mArraySize-1]='\0';
}

void *recordingState(void *arg){
	char recordingArray[RECORDING_ARRAY_SIZE];
	FILE* outfile;
	readAudioData(RECORDING_ARRAY_SIZE,recordingArray,0);
	outfile=fopen(FILE_NAME,"wb");
	if(outfile==NULL){
		printf("Error opening file");
		return NULL;
	}
	fwrite(&recordingArray,1,RECORDING_ARRAY_SIZE,outfile);
	fclose(outfile);
	return NULL;
}

void *TunerStateReader(void *arg){
	int temp;
	int i;
	int j;
	pthread_mutexattr_t mymutexattr;
	pthread_mutexattr_init(&mymutexattr);			// initiate mutex
	pthread_mutex_init(&mutexA,&mymutexattr);
	pthread_mutex_init(&mutexB,&mymutexattr);
	pthread_mutexattr_destroy(&mymutexattr);
	currentArray='A';
	// while none of the buttons are pressed
	if(currentArray=='A'){
		pthread_mutex_lock(&mutexA);
		readAudioData(TUNER_ARRAY_SIZE,tunerArrayAChar,1);
		for (i=0; i<TUNER_ARRAY_SIZE;i+=BIT_DEPTH){
			if(tunerArrayAChar[i]=='f'){
				break;
			}
			temp=0;
			for(j=0; j<BIT_DEPTH;++j){
				if(tunerArrayAChar[i+j]==1){
					temp=temp+(2^(BIT_DEPTH-j-1));
				}
				++i;
			}
			tunerArrayA[i/BIT_DEPTH]=temp;
		}
		pthread_mutex_unlock(&mutexA);
		currentArray='B';
	}
	else{
		pthread_mutex_lock(&mutexB);
		readAudioData(TUNER_ARRAY_SIZE,tunerArrayBChar,1);
		for (i=0; i<TUNER_ARRAY_SIZE;i+=BIT_DEPTH){
			if(tunerArrayBChar[i]=='f'){
				break;
			}
			temp=0;
			for(j=0; j<BIT_DEPTH;++j){
				if(tunerArrayBChar[i+j]==1){
					temp=temp+(2^(BIT_DEPTH-j-1));
				}
			}
			tunerArrayB[i/BIT_DEPTH]=temp;
			pthread_mutex_unlock(&mutexB);
		}
		currentArray='A';
	}
	return NULL;
}
