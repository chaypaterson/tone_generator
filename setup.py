import sys
#from distutils.core import setup, Extension
from setuptools import setup, Extension

def get_gsl_paths():
    if sys.platform == 'darwin':
        # macOS
        gsl_include_dir = '/opt/homebrew/opt/gsl/include'
        gsl_lib_dir = '/opt/homebrew/opt/gsl/lib'
    elif sys.platform.startswith('linux'):
        # Linux
        gsl_include_dir = '/usr/include'
        gsl_lib_dir = '/usr/lib/x86_64-linux-gnu'
    else:
        raise RuntimeError(f"Unsupported platform: {sys.platform}")
    return gsl_include_dir, gsl_lib_dir

def main():
    gsl_include_dir, gsl_lib_dir = get_gsl_paths()

    audio_module = Extension("audio", 
        sources=["audio.c"],
        libraries = ['m', 'gsl'],
        include_dirs=[gsl_include_dir],
        library_dirs = [gsl_lib_dir],
        extra_objects=[],
        extra_link_args=[]
        )

    setup(name="audio",
          version="0.0.1",
          description="Python interface for tone_generator.",
          author="<R and c>",
          author_email="your_email@gmail.com",
          ext_modules=[audio_module])

if __name__ == "__main__":
    main()
