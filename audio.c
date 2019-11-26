#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <gsl/gsl_sf.h>

/* audio.c
 * a simple PCM signal generator
 * compiles with: gcc audio.c -lm -lgsl
 * depends on libgsl-dev (debian)
 */

// define some constants

const double A440 = 2*M_PI*440; // base tone: A 440Hz
const double Cs5  = 2*M_PI*554.365;
const double E5   = 2*M_PI*659.255;
const unsigned int DefaultSampleRate = 44100;

// define a header for a wav file

typedef struct FileHeader {
};

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
    return sin(theta)/cosh(decay*theta);
}

double besselj(int n, double theta) {
    return jn(n, theta);
}

double JSnNorm(double m, double theta) {
    // Jacobi elliptic sn function, with the frequency normalised
    double result=0;
    double foo,bar;
    // TODO what if |m|>1?
    // try to use sn(u,m) = m^-0.5 sn(sqrt(m) u, 1/m) ?
    // did not work.
    // compute period
    double K = gsl_sf_hyperg_2F1(0.5,0.5,1,m)*M_PI/2;
    theta = theta *(2*K)/M_PI;
    // compute result
    gsl_sf_elljac_e(theta, m, &result, &foo, &bar);
    return result;
}

double legendre(double n, double theta) {
    // legendre polynomials
    theta = theta/M_PI; //normalise to range 0,1
    theta = fmod(theta,2.0)-1.0;
    // output value of polynomial:
    return gsl_sf_legendre_Pl(n,theta);
}

// TODO mathieu
double mathieu() {
    return 0;
}

// define signal
double signal(double t) {
    double theta = A440 * t;
    // TODO pulse width modulation: tune dwell in one region?
    // TODO 
    return JSnNorm(0.95,theta);
}

int main() {
    // TODO the header: WAV magic numbers?

    // Sampling rate:
    unsigned int SampleRate = 16000; // this needs to be included in the
    // header

    // bits to output
    int16_t v;  // v is a 16-bit integer
    char* p = (char*) (&v); // this is a character that points to the value of
                            // v
    double start = 0; // sample offset
    double length = 2; // length of sample
    double tmax = start+length;
    double t = start; // time variable

    // On-the-fly bitrate conversion:
    // Downsampling should use a low-pass filter
    double CutOffFrequency = 0.5 * SampleRate; // Nyquist frequency
    double alpha = 1-CutOffFrequency/DefaultSampleRate; //remanence for low-pass filter

    while (t<tmax) {
        v = (int16_t) ( signal(t) * (2<<12) );
        // 2^12 = 4096, the range of int16_t

        if ((SampleRate < DefaultSampleRate)&&(t>0)) {
            // downsample the signal
            // we know that this signal is signal(t), and the last time was
            // t-dt: so we can say that
            double dt = 1./SampleRate;
            int16_t lastValue = (int16_t) ( (alpha*signal(t))*(2<<12) +
            ((1-alpha)*signal(t-dt))*(2<<12));
        }

        // write sound:
        putchar(p[0]);  // left channel
        putchar(p[1]);
        putchar(p[0]);  // right channel
        putchar(p[1]);

        t+=1./SampleRate; // default sampling rate: 44.1 kHz
                          // but we could be sampling at something else
    }

    return 0;
}
