#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <gsl/gsl_sf.h>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

/* audio.c
 * a simple PCM signal generator
 * compiles with: gcc audio.c -lm -lgsl
 * depends on libgsl-dev (debian)
 */

// define some constants

const double A440 = 2 * M_PI * 440; // base tone: A 440Hz
const double Cs5  = 2 * M_PI * 554.365;
const double E5   = 2 * M_PI * 659.255;
const unsigned int DefaultSampleRate = 44100;

// define some interesting functions

double sinc(double theta) {
    double A;
    if (theta == 0) {
        A=1;
    } else {
        A = sin(theta)/theta;
    }
    return A;
}

double sinlet(double theta, double decay) {
    return sin(theta) / cosh(decay * theta);
}

double besselj(int n, double theta) {
    // the function jn lacks a prototype in the header so the ISO 17 standard
    // complains: however, gsl_sf_bessel_Jn is fine.
    return gsl_sf_bessel_Jn(n, theta);
}

double JSnNorm(double m, double theta) {
    // Jacobi elliptic sn function, with the frequency normalised
    double result = 0;
    double foo,bar;
    // compute period
    double K = gsl_sf_hyperg_2F1(0.5, 0.5, 1, m) * M_PI / 2;
    theta = theta * (2 * K) / M_PI;
    // compute result
    gsl_sf_elljac_e(theta, m, &result, &foo, &bar);
    return result;
}

double legendre(double n, double theta) {
    // legendre polynomials
    theta = theta / M_PI; //normalise to range 0,1
    theta = fmod(theta, 2.0) - 1.0;
    // output value of polynomial:
    return gsl_sf_legendre_Pl(n, theta);
}

// define signal
double signal(double t) {
    double theta = A440 * t;
    return JSnNorm(0.95, theta);
}

int putint(int var, char bytes) {
    // writes "bytes" bytes from the start of an integer
    char* p = (char*) &var;
    for (char n = 0; n < bytes; ++n)
        putchar(*p++);

    return var;
}

int filesize(double length, int SampleRate) {
    // return the overall filesize of a WAV file in bytes
    int header = 44;
    int samples = length * SampleRate;

    // each sample is 16 bit and two channels, so 32 bits = 4 bytes
    return header + 4 * samples;
}

void WavHeader(double length, int SampleRate) {
    char BytePerBloc = 4;
    // Master chunk:
    printf("RIFF");
    putint(filesize(length, SampleRate) - 8, 4);
    printf("WAVE");

    // Format chunk:
    printf("fmt ");
    putint(16, 4); // chunk size
    putint(1, 2); // 1: PCM format, 3: IIEE float
    putint(2, 2); // 2 channels
    putint(SampleRate, 4); // sample rate in Hz
    putint(SampleRate * BytePerBloc, 4); // BytePerSec
    putint(BytePerBloc, 2); // BytePerBloc
    putint(16, 2); // BitsPerSample

    // Data chunk:
    printf("data");
    putint((int)(SampleRate * length) * BytePerBloc, 4);
}

// TODO a WAV struct that we populate in memory on the heap and can then write
// out with a helper function?

int main() {
    // Sampling rate:
    unsigned int SampleRate = DefaultSampleRate; // this needs to be included in the
    // header

    int maxlevel = 2<<12;
    double length = 10; // length of sample in s
    double dt = 1. / SampleRate;

    int16_t v;  // v is a 16-bit integer

    // write the header: WAV magic numbers
    WavHeader(length, SampleRate);

    for (double t = 0; t < length; t += dt) {
        v = (int16_t) ( signal(t) * maxlevel );

        // write sound:
        putint(v, 2); // left channel
        putint(v, 2);  // right channel
    }

    return 0;
}
