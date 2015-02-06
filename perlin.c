#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

/*
 * Pseudocode of this file taken from: http://freespace.virgin.net/hugo.elias/models/m_perlin.htm
 */

/*
 * Random generator that returns the same floating point number between -1.0 and 1.0
 * from three integers given in parameters. i is the number of iterations.
 */
float Noise(int x, int y, int i){
	int n;
	
	switch (i) {
		case 0:
			n = x + y * 43;
			n = (n<<5) ^ n;
			return ( 1.0 - ( (n * (n * n * 11423 + 266681) + 1076314511) & 2147483647) / 1073741824.0);
		case 1:
			n = x + y * 47;
			n = (n<<7) ^ n;
			return ( 1.0 - ( (n * (n * n * 13327 + 666013) + 1376312589) & 2447486777) / 1073741824.0);	
		case 2:
			n = x + y * 53;
			n = (n<<11) ^ n;
			return ( 1.0 - ( (n * (n * n * 14011 + 789221) + 1666316363) & 2647483999) / 1073741824.0);
		case 3:
			n = x + y * 57;
			n = (n<<13) ^ n;
			return ( 1.0 - ( (n * (n * n * 15731 + 816667) + 1999312589) & 2847483647) / 1073741824.0);
		default:
			return 0.0;
	}
}

float SmoothNoise(int x, int y, int i) {
	float corners = (Noise(x-1, y-1, i) + Noise(x+1, y-1, i) + Noise(x-1, y+1, i) + Noise(x+1, y+1, i)) / 16;
	float sides   = (Noise(x-1, y, i) + Noise(x+1, y, i) + Noise(x, y-1, i) + Noise(x, y+1, i)) /  8;
	float center  =  Noise(x, y, i) / 4;
	return corners + sides + center;
}

float Interpolate(float a, float b, float x) {
	float ft = x * 3.1415927;
	float f = (1 - cos(ft)) * .5;
	return  a*(1-f) + b*f;
}

float InterpolatedNoise(float x, float y, int i) {
	int integer_X = (int) x;
	float fractional_X = x - integer_X;
	int integer_Y = (int) y;
	float fractional_Y = y - integer_Y;

	float v1 = SmoothNoise(integer_X, integer_Y, i);
	float v2 = SmoothNoise(integer_X + 1, integer_Y, i);
	float v3 = SmoothNoise(integer_X,     integer_Y + 1, i);
	float v4 = SmoothNoise(integer_X + 1, integer_Y + 1, i);

	float i1 = Interpolate(v1 , v2 , fractional_X);
	float i2 = Interpolate(v3 , v4 , fractional_X);

	return Interpolate(i1 , i2 , fractional_Y);
}

float PerlinNoise_2D(float x, float y, float persistence, int Number_Of_Octaves) {
	int i;
	float frequency, amplitude;	
	float total = 0.0;
	float f = 0.0;

	for(i = 0; i < Number_Of_Octaves; ++i) {
		frequency = pow(2, i);
		amplitude = pow(persistence, i);
		f = InterpolatedNoise(x * frequency, y * frequency, i) * amplitude;
		//printf("f=%f\n", f);
		total += f;
	}

	return total;
}

float noise_generation(int x, int y) {
	float noise = PerlinNoise_2D(x, y, 0.1, 2);
	//printf("noise=%f\n", noise);
	return noise;
}
