import os
import sys

child = sys.argv[1];
outchild = sys.argv[2];
shadersDirectory = child + "/shaders";
outputfilename = outchild + "/include/shaders.hpp";
print(f"shadersDirectory = {shadersDirectory}")
print(f"outputfilename = {outputfilename}")

try:
    os.mkdir(outchild)
except OSError as error:
    pass

try:
    os.mkdir(outchild + "/include")
except OSError as error:
    pass

output = "#pragma once\n"
for shaderfilename in os.listdir(shadersDirectory):
    varname = shaderfilename.replace(".", "_")
    output += "const char* " + varname + "[] = {"
    lens = []
    with open(shadersDirectory + "/" + shaderfilename) as shaderfile:
        code = shaderfile.read();
        output += "\"" + code.replace("\r\n", "\\n").replace("\n", "\\n") + "\""

    output += "\n};\n\n"
    output += "int " + varname + "_lens[] = {" + str(len(code)) + "};\n\n"
    output += "int " + varname + "_count = 1;\n\n"

with open(outchild + "/include/shaders.hpp", "w") as outputfile:
    outputfile.write(output)