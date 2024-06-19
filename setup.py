from distutils.core import setup, Extension

def main():
    audio_module = Extension("audio", 
        sources=["audio.c"],
        libraries = ['m', 'gsl'],
        include_dirs=['/usr/include/gsl'],
        library_dirs = ['/usr/lib/x86_64-linux-gnu'],
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
