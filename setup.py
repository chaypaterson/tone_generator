from distutils.core import setup, Extension
gsl_path = '/opt/homebrew/opt/gsl'


def main():
    audio_module = Extension("audio", 
        sources=["audio.c"],
        libraries = ['m', 'gsl'],
        include_dirs=[f'{gsl_path}/include'],
        library_dirs = [f'{gsl_path}/lib/'],
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
