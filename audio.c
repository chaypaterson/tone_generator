#include <Python.h>
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

//define the wav structure 
struct WavFile {
    unsigned int SampleRate;
    int BytePerBloc;
    int maxlevel;
    double length;
    int dataSize;
    int16_t *data;
};

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

// define signals
double gensignal(double t) {
    return JSnNorm(0.95, t);
}

double gensignal2(double t) {
    return besselj(0, t);
}

int putint(FILE *file, int var, char bytes) {
    // writes "bytes" bytes from the start of an integer
    char* p = (char*) &var;
    for (char n = 0; n < bytes; ++n)
        fputc(*p++, file);

    return var;
}

int filesize(double length, int SampleRate) {
    // return the overall filesize of a WAV file in bytes
    int header = 44;
    int samples = length * SampleRate;

    // each sample is 16 bit and two channels, so 32 bits = 4 bytes
    return header + 4 * samples;
}

void WavHeader(struct WavFile *wav, FILE *file) {

    char BytePerBloc = 4;
    // Master chunk:
    fprintf(file, "RIFF");
    putint(file, filesize(wav->length, wav->SampleRate) - 8, 4);
    fprintf(file, "WAVE");

    // Format chunk:
    fprintf(file, "fmt ");
    putint(file, 16, 4); // chunk size
    putint(file, 1, 2); // 1: PCM format, 3: IIEE float
    putint(file, 2, 2); // 2 channels
    putint(file, wav->SampleRate, 4); // sample rate in Hz
    putint(file, wav->SampleRate * wav->BytePerBloc, 4); // BytePerSec
    putint(file, wav->BytePerBloc, 2); // BytePerBloc
    putint(file, 16, 2); // BitsPerSample

    // Data chunk:
    fprintf(file, "data");
    putint(file, wav->dataSize, 4);
}

// TODO a WAV struct that we populate in memory on the heap and can then write
// out with a helper function?

void beep(double (*waveform)(double), double freq) {
    struct WavFile wav;
    wav.SampleRate = DefaultSampleRate;
    wav.BytePerBloc = 4; // Assuming 2 bytes per channel for stereo
    wav.maxlevel = 2 << 12;
    wav.length = 10; // Length of sample in seconds

    int numSamples = (int)(wav.length * wav.SampleRate);
    wav.dataSize = numSamples * wav.BytePerBloc;
    wav.data = (int16_t *)malloc(wav.dataSize);

    double dt = 1. / wav.SampleRate;
    int16_t v;  // v is a 16-bit integer
    
    FILE *file = fopen("output.wav", "wb");
    if (!file) {
        printf("Error opening output file.\n");
        return;
    }

    // write the header: WAV magic numbers
    WavHeader(&wav, file);

    for (double t = 0; t < wav.length; t += dt) {
        v = (int16_t) ( waveform(freq * t) * wav.maxlevel );

        // write sound:
        putint(file, v,  2); // left channel
        putint(file, v,  2);  // right channel
    }
    // Write data array to file
    fwrite(wav.data, 1, wav.dataSize, file);

    fclose(file);
    free(wav.data);
}

//Python Binding for signal
static PyObject* py_gensignal(PyObject* self, PyObject* args) {
    double t;
    if (!PyArg_ParseTuple(args, "d", &t)) {
        return NULL;
    }
    double result = gensignal(t);
    return Py_BuildValue("d", result);
}

//Python Binding for signal 2
static PyObject* py_gensignal2(PyObject* self, PyObject* args) {
    double t;
    if (!PyArg_ParseTuple(args, "d", &t)) {
        return NULL;
    }
    double result = gensignal2(t);
    return Py_BuildValue("d", result);
}
//Python Binding for beep

static PyObject* py_beep(PyObject* self, PyObject* args) {
    double freq = 440.0; // Default frequency
    if (!PyArg_ParseTuple(args, "|d", &freq)) {
        return NULL;
    }

    // Generate beep
    beep(gensignal, freq);

    Py_RETURN_NONE;
}

/* Module method table */
static PyMethodDef AudioMethods[] = {
    {"signal", py_gensignal, METH_VARARGS, "Calculate signal value."},
    {"signal2", py_gensignal2, METH_VARARGS, "Calculate signal 2 value."},
    {"beep", py_beep, METH_VARARGS, "Generate a beep (sine wave) and save it to a WAV file."},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

static struct PyModuleDef audiomodule = {
    PyModuleDef_HEAD_INIT,
    "audio",   /* name of module */
    NULL, // spam_doc, /* module documentation, may be NULL */
    -1,       /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
    AudioMethods
};

/* Module initialization function */
PyMODINIT_FUNC PyInit_audio(void) {
    return PyModule_Create(&audiomodule);
}

int
main(int argc, char *argv[])
{
    wchar_t *program = Py_DecodeLocale(argv[0], NULL);
    if (program == NULL) {
        fprintf(stderr, "Fatal error: cannot decode argv[0]\n");
        exit(1);
    }

    /* Add a built-in module, before Py_Initialize */
    if (PyImport_AppendInittab("audio", PyInit_audio) == -1) {
        fprintf(stderr, "Error: could not extend in-built modules table\n");
        exit(1);
    }

    /* Pass argv[0] to the Python interpreter */
    Py_SetProgramName(program);

    /* Initialize the Python interpreter.  Required.
       If this step fails, it will be a fatal error. */
    Py_Initialize();

    /* Optionally import the module; alternatively,
       import can be deferred until the embedded script
       imports it. */
    PyObject *pmodule = PyImport_ImportModule("audio");
    if (!pmodule) {
        PyErr_Print();
        fprintf(stderr, "Error: could not import module 'audio'\n");
    }

    PyMem_RawFree(program);
    return 0;
}

