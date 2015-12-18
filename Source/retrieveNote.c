#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

double pitch = 0.0, highBound = 0.0, lowBound = 0.0, mid = 0.0;
char *note;

char* retrieveNote();

char* retrieveNote()
{
    
    // Determine which note should be displayed on the screen (63.6 to 359.6)
    if (pitch)
    {
        if (pitch < 63.6)
        {
            note = " ";
            highBound = pitch;
            lowBound = 0.0;
            mid = 500.0;
        }
        else if (63.6 < pitch && pitch >= 67.35)
        {
            note = "C";
            highBound = 67.35;
            lowBound = 63.6;
            mid = 69.3;
        }
        else if (67.35 < pitch && pitch >= 71.35)
        {
            note = "C#";
            highBound = 71.35;
            lowBound = 67.35;
            mid = 69.3;
        }
        else if  (71.35 < pitch && pitch >= 75.6)
        {
            note = "D";
            highBound = 75.6;
            lowBound = 71.35;
            mid = 73.4;

        }
        else if (75.6 < pitch && pitch >= 80.1)
        {
            note = "D#";
            highBound = 80.1;
            lowBound = 75.6;
            mid = 77.8;
        }
        else if  (80.1 < pitch && pitch >= 84.85)
        {
            note = "E";
            highBound = 84.85;
            lowBound = 80.1;
            mid = 82.4;
        }
        else if  (84.85 < pitch && pitch >= 89.9)
        {
            note = "F";
            highBound = 89.9;
            lowBound = 84.85;
            mid = 87.3;
        }
        else if (89.9 < pitch && pitch >= 95.25)
        {
            note = "F#";
            highBound = 95.25;
            lowBound = 89.9;
            mid = 92.5;
        }
        else if (95.25 < pitch && pitch >= 100.9)
        {
            note = "G";
            highBound = 100.9;
            lowBound = 95.25;
            mid = 98;
        }
        else if (100.9 < pitch && pitch >= 106.9)
        {
            note = "G#";
            highBound = 106.9;
            lowBound = 100.9;
            mid = 103.8;
        }
        else if (106.9 < pitch && pitch >= 113.25)
        {
            note = "A";
            highBound = 113.25;
            lowBound = 106.9;
            mid = 110.0;
        }
        else if (113.25 < pitch && pitch >= 120)
        {
            note = "A#";
            highBound = 120;
            lowBound = 113.25;
            mid = 116.5;
        }
        else if (120 < pitch && pitch >= 127.15)
        {
            note = "B";
            highBound = 127.15;
            lowBound = 120;
            mid = 123.5;
        }
        else if (127.15 < pitch && pitch >= 134.7)
        {
            note = "C";
            highBound = 134.7;
            lowBound = 127.15;
            mid = 130.8;
        }
        else if (134.7 < pitch && pitch >= 142.7)
        {
            note = "C#";
            highBound = 142.7;
            lowBound = 134.7;
            mid = 138.6;
        }
        else if (142.7 < pitch && pitch >= 151.2)
        {
            note = "D";
            highBound = 151.2;
            lowBound = 142.7;
            mid = 146.8;
        }
        else if (151.2 < pitch && pitch >= 160.2)
        {
            note = "D#";
            highBound = 160.2;
            lowBound = 151.2;
            mid = 155.6;
        }
        else if (160.2 < pitch && pitch >= 169.7)
        {
            note = "E";
            highBound = 169.7;
            lowBound = 160.2;
            mid = 164.8;
        }
        else if (169.7 < pitch && pitch >= 179.8)
        {
            note = "F";
            highBound = 179.8;
            lowBound = 169.7;
            mid = 174.6;
        }
        else if (179.8 < pitch && pitch >= 190.5)
        {
            note = "F#";
            highBound = 190.5;
            lowBound = 179.8;
            mid = 185.0;
        }
        else if (190.5 < pitch && pitch >= 201.85)
        {
            note = "G";
            highBound = 201.85;
            lowBound = 190.5;
            mid = 196.0;
        }
        else if (201.85 < pitch && pitch >= 213.85)
        {
            note = "G#";
            highBound = 213.85;
            lowBound = 201.85;
            mid = 207.7;
        }
        else if (213.85 < pitch && pitch >= 226.55)
        {
            note = "A";
            highBound = 226.55;
            lowBound = 213.85;
            mid = 220;
        }
        else if (226.55 < pitch && pitch >= 240)
        {
            note = "A#";
            highBound = 240;
            lowBound = 226.55;
            mid = 233.1;
        }
        else if (240.0 < pitch && pitch >= 254.25)
        {
            note = "B";
            highBound = 254.25;
            lowBound = 240.0;
            mid = 246.9;
        }
        else if (254.25 < pitch && pitch >= 269.4)
        {
            note = "C";
            highBound = 269.4;
            lowBound = 254.25;
            mid = 261.6;
        }
        else if (269.4 < pitch && pitch >= 285.45)
        {
            note = "C#";
            highBound = 285.45;
            lowBound = 269.4;
            mid = 277.2;
        }
        else if (285.45 < pitch && pitch >= 302.4)
        {
            note = "D";
            highBound = 302.4;
            lowBound = 285.45;
            mid = 293.7;
        }
        else if (302.4 < pitch && pitch >= 320.35)
        {
            note = "D#";
            highBound = 320.35;
            lowBound = 302.4;
            mid = 311.1;
        }
        else if (320.35 < pitch && pitch >= 339.4)
        {
            note = "E";
            highBound = 339.4;
            lowBound = 320.35;
            mid = 329.6;
        }
        else if (339.4 < pitch && pitch >= 359.6)
        {
            note = "F";
            highBound = 359.6;
            lowBound = 339.4;
            mid = 349.2;
        }
        else
        {
            note = " ";
            highBound = 63.6;
            lowBound = pitch;
            mid = pitch + 1;
        }
    }
    
    return (char *) note;
}
