#!/usr/bin/env python

# Script for crawling shader directory and generating
# SPIR-V shader modules from GLSL shaders with glslc.
# It generates makefiles with shader interdependency.

import sys
import argparse
import subprocess
import glob
import os

class ShaderScript:
    USAGE = "DIRECTORY"
    DESCRIPTION = """
    """

    GLSLC = "glslc -O -Os -c "

    SHADER_TYPES = [ "*.vert",
                     "*.tesc",
                     "*.tese",
                     "*.geom",
                     "*.frag",
                     "*.comp" ]

    def __init__(self):
        parser = argparse.ArgumentParser(description=self.DESCRIPTION,
                                         usage="%(prog)s "+self.USAGE)

        option = parser.add_argument

        option("directories", metavar="DIRECTORIES", nargs="+",
               help="""path where the shaders are located.""")

        self.options = parser.parse_args()

    def __enter__(self):
        return self

    def __exit__(self, error, value, trace):
        pass

    def execute(self, location=sys.argv[0]):
        for directory in self.options.directories:

            os.chdir(directory)

            shader_files = [  ]

            contents = ""

            contents = contents + "all: "

            for shader_type in self.SHADER_TYPES:
                shader_files = shader_files + glob.glob(shader_type)

            for shader_file in shader_files:
                contents = contents + (shader_file + ".spv ")

            contents = contents[:-1]

            contents = contents + "\n\n"

            for shader_file in shader_files:
                glslc = subprocess.Popen(["glslc", "-M", shader_file],
                                          stdout=subprocess.PIPE)
                dependencies, error = glslc.communicate()

                command = ""
                command = command + dependencies.decode("utf-8")
                command = command + "\t"
                command = command + self.GLSLC
                command = command + shader_file

                contents = contents + command
                contents = contents + "\n\n"

            contents = contents[:-2]

            makefile = open("Makefile", "w")
            makefile.write(contents)
            makefile.close()

INIT_ERROR_STATUS = -1
if __name__ == "__main__":
    status = INIT_ERROR_STATUS
    with ShaderScript() as script:
        status = script.execute()
    sys.exit(status);
