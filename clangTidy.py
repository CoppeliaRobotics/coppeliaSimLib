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

# Configuration
SOURCE_DIR = './sourceCode'  # Your source root directory
COMPILE_COMMANDS_DIR = './build'  # Where compile_commands.json lives
FILE_PATTERNS = ['*.h', '*.cpp']  # File types to process

# Clang-tidy options
TIDY_OPTIONS = [
    '-checks=modernize-use-override',
    '-fix-errors',  # Auto-fix (remove for dry-run)
    '-p', COMPILE_COMMANDS_DIR  # Path to compile_commands.json
]

# Find all source files
all_files = find_files(SOURCE_DIR, FILE_PATTERNS)

# Process each file
for file in all_files:
    cmd = ['clang-tidy'] + TIDY_OPTIONS + [file]
    print(f"Processing: {file}")
    
    try:
        # Run clang-tidy (shell=False for security)
        result = subprocess.run(cmd, 
                              shell=False,
                              check=True,
                              text=True,
                              capture_output=True)
        
        # Print output if any
        if result.stdout:
            print(result.stdout)
        if result.stderr:
            print(result.stderr)
            
    except subprocess.CalledProcessError as e:
        print(f"Error processing {file}:")
        print(e.stderr)
