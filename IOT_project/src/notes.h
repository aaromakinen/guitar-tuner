// Functions for lookup of notes by name and frequency

#include "string.h"
char* getNote(float freq);
float lookupNote(const char* name);
int sameName(const char* n1, char* n2, int len);

struct Notes {
	char name[4];
	float lolim; // Lower limit
	float rval; // Reference value
};

static struct Notes notestruct[] = {
	{ "? ", -1.0, 0.0},
	{ "? ", 0.0, 0.0},
	{ "C1", 31.8, 32.7 },
	{ "C#1", 33.7, 34.7 },
	{ "D1", 35.7, 36.7 },
	{ "D#1", 37.8, 38.9 },
	{ "E1", 40.0, 41.2 },
	{ "F1", 42.4, 43.7 },
	{ "F#1", 45.0, 46.3 },
	{ "G1", 47.6, 49.0 },
	{ "G#1", 50.5, 51.9 },
	{ "A1", 53.5, 55.0 },
	{ "A#1", 56.6, 58.3 },
	{ "B1", 60.0, 61.7 },
	{ "C2", 63.6, 65.4 },
	{ "C#2", 67.4, 69.3 },
	{ "D2", 71.4, 73.4 },
	{ "D#2", 75.6, 77.8 },
	{ "E2", 80.1, 82.4 },
	{ "F2", 84.9, 87.3 },
	{ "F#2", 89.9, 92.5 },
	{ "G2", 95.3, 98.0 },
	{ "G#2", 100.9, 103.8 },
	{ "A2", 106.9, 110.0 },
	{ "A#2", 113.3, 116.5 },
	{ "B2", 120.0, 123.5 },
	{ "C3", 127.1, 130.8 },
	{ "C#3", 134.7, 138.6 },
	{ "D3", 142.7, 146.8 },
	{ "D#3", 151.2, 155.6 },
	{ "E3", 160.2, 164.8 },
	{ "F3", 169.7, 174.6 },
	{ "F#3", 179.8, 185.0 },
	{ "G3", 190.5, 196.0 },
	{ "G#3", 201.8, 207.7 },
	{ "A3", 213.8, 220.0 },
	{ "A#3", 226.5, 233.1 },
	{ "B3", 240.0, 246.9 },
	{ "C4", 254.3, 261.6 },
	{ "C#4", 269.4, 277.2 },
	{ "D4", 285.4, 293.7 },
	{ "D#4", 302.4, 311.1 },
	{ "E4", 320.4, 329.6 },
	{ "F4", 339.4, 349.2 },
	{ "F#4", 359.6, 367.0 },
	{ "G4", 381.0, 392.0 },
	{ "G#4", 403.7, 415.3 },
	{ "A4", 427.7, 440.0  },
	{ "A#4", 453.1, 466.2 },
	{ "B4", 480.0, 493.9 },
	{ "C5", 508.6, 523.3 },
	{ "? ", 538.8, 0.0},
	{ "? ", 10000.0, 0.0}
};

char* getNote(float freq) { // Get note name from frequency
	int i = 0;
		while (freq >= notestruct[i].lolim) { // Search for first frequency lower limit that is higher than input freq.
			i++;
		}

		return notestruct[i - 1].name;
}

float lookupNote(const char* name) { // Get reference frequency from note name
	int i = 0;

	while (notestruct[i].lolim < 1000.0) {
		if (sameName(name, notestruct[i].name, 3) == 0) { // Only compare two chars, because first two are unique.
			return notestruct[i].rval;
		}

		i++;
	}

	return -1.0; // Indicates name not found in table.
}

int sameName(const char* n1, char* n2, int len) { // Compare C strings to specified length
	int i = 0;
	int r = 0;

	while (i <= (len -1)) {
		if (n1[i] != n2[i]) {
			r = 1;
		}

		i++;
	}

	return r;
}







