import os
import fnmatch
import subprocess

def find_files(directory, patterns):
    matches = []
    for root, dirnames, filenames in os.walk(directory):
        for pattern in patterns:
            for filename in fnmatch.filter(filenames, pattern):
                absolute_path = os.path.abspath(os.path.join(root, filename))
                matches.append(absolute_path)
    return matches

allFiles = find_files('./sourceCode', ['*.h', '*.cpp'])
for file in allFiles:
    opts = ''
    opts += '-i '
    #opts += '--dry-run '
    cmd = 'clang-format ' + opts + '--style=file ' + file
    subprocess.run(cmd, shell=True)
