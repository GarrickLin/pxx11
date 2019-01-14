
if __name__ == "__main__":
    import sys
    from os.path import abspath
    import fire    
    sys.path.insert(0, abspath("./python"))
    from conan_toolkits import ConanToolkits
    fire.Fire(ConanToolkits)

