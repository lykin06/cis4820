/*
 * Pseudocode of this file taken from: http://freespace.virgin.net/hugo.elias/models/m_perlin.htm
 */

/*
 * Random generator that a floating point number between -1.0 and 1.0
 */
float Noise(int x, int y, int i);
float SmoothNoise(int x, int y, int i);
float Interpolate(float a, float b, float x);
float InterpolatedNoise(float x, float y, int i);
float PerlinNoise_2D(float x, float y, float persistence, int Number_Of_Octaves);

/*
 * Generates perlin noise
 */
float noise_generation(int x, int y);
