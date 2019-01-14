from conans import ConanFile

class Pxx11Conan(ConanFile):
    name = "pxx11"
    version = "0.0.1"
    exports_sources = "include/*"
    no_copy_source = True

    def package_id(self):
        self.info.header_only()

    def package(self):
        self.copy("*")