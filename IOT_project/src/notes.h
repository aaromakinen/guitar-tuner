#include "string.h"
char* getNote(float freq);
double lookupNote(const char* name);
int sameName(const char* n1, char* n2, int len);

struct Notes {
	char name[4];
	double lolim; // Lower limit
	double rval; // Reference value
};

static struct Notes notestruct[] = {
	{ "? ", -1.0, 0.0},
	{ "? ", 0.0, 0.0},
	{ "C1", 31.785, 32.7 },
	{ "C#1", 33.675, 34.65 },
	{ "D1", 35.68, 36.71 },
	{ "D#1", 37.8, 38.89 },
	{ "E1", 40.045, 41.2 },
	{ "F1", 42.425, 43.65 },
	{ "F#1", 44.95, 46.25 },
	{ "G1", 47.625, 49.0 },
	{ "G#1", 50.455, 51.91 },
	{ "A1", 53.455, 55.0 },
	{ "A#1", 56.635, 58.27 },
	{ "B1", 60.005, 61.74 },
	{ "C2", 63.575, 65.41 },
	{ "C#2", 67.355, 69.30},
	{ "D2", 71.36, 73.42 },
	{ "D#2", 75.6, 77.78 },
	{ "E2", 80.095, 82.41 },
	{ "F2", 84.86, 87.31 },
	{ "F#2", 89.905, 92.5 },
	{ "G2", 95.25, 98.0 },
	{ "G#2", 100.915, 103.83 },
	{ "A2", 106.915, 110.0 },
	{ "A#2", 113.27, 116.54 },
	{ "B2", 120.005, 123.47 },
	{ "C3", 127.14, 130.81 },
	{ "C#3", 134.7, 138.59 },
	{ "D3", 142.71, 146.83 },
	{ "D#3", 151.195, 155.56 },
	{ "E3", 160.185, 164.81 },
	{ "F3", 169.71, 174.61 },
	{ "F#3", 179.805, 185.0 },
	{ "G3", 190.5, 196.0 },
	{ "G#3", 201.825, 207.65 },
	{ "A3", 213.825, 220.0 },
	{ "A#3", 226.54, 233.08 },
	{ "B3", 240.01, 246.94 },
	{ "C4", 254.285, 261.63 },
	{ "C#4", 269.405, 277.18 },
	{ "D4", 285.42, 293.66 },
	{ "D#4", 302.395, 311.13 },
	{ "E4", 320.38, 329.63 },
	{ "F4", 339.43, 349.23 },
	{ "F#4", 359.61, 369.99 },
	{ "G4", 380.995, 392.00 },
	{ "G#4", 403.65, 415.30 },
	{ "A4", 427.65, 440.0  },
	{ "A#4", 453.08, 466.16 },
	{ "B4", 480.02, 493.88 },
	{ "C5", 508.565, 523.25 },
	{ "? ", 538.81, 0.0},
	{ "? ", 10000.0, 0.0}
};

char* getNote(float freq) { // Get note name from frequency
	int i = 0;
		while (freq >= notestruct[i].lolim) {
			i++;
		}

		return notestruct[i - 1].name;
}

double lookupNote(const char* name) { // Get reference frequency from note name
	int i = 0;

	while (notestruct[i].lolim < 1000.0) {
		if (sameName(name, notestruct[i].name, 3) == 0) { // Only compare two chars, because first two are unique.
			return notestruct[i].rval;
		}

		i++;
	}

	return -1.0; // Indicates name not found in table.
}

int sameName(const char* n1, char* n2, int len) {
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







