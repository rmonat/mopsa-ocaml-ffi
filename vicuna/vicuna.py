#******************************************************************************
#*                                  Vicuna                                    *
#*                  Simon Spies and Greta Yorsh, Jane Street                  *
#* -------------------------------------------------------------------------- *
#*                               MIT License                                  *
#*                                                                            *
#* Copyright (c) 2025 Jane Street Group LLC                                   *
#* opensource-contacts@janestreet.com                                         *
#*                                                                            *
#* Permission is hereby granted, free of charge, to any person obtaining a    *
#* copy of this software and associated documentation files (the "Software"), *
#* to deal in the Software without restriction, including without limitation  *
#* the rights to use, copy, modify, merge, publish, distribute, sublicense,   *
#* and/or sell copies of the Software, and to permit persons to whom the      *
#* Software is furnished to do so, subject to the following conditions:       *
#*                                                                            *
#* The above copyright notice and this permission notice shall be included    *
#* in all copies or substantial portions of the Software.                     *
#*                                                                            *
#* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR *
#* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   *
#* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    *
#* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER *
#* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    *
#* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER        *
#* DEALINGS IN THE SOFTWARE.                                                  *
#******************************************************************************


import argparse
import os
import subprocess
import signal
import re

# This script exits with status 0 even when the analysis fails at any stage.
# It exits with status 1 only if the arguments that were provided are malformed.

parser = argparse.ArgumentParser()
parser.add_argument("-externals", "-e", metavar="E", help="File containing the names of external functions.")
parser.add_argument("-include", "-I", metavar="incl", action='append', help="C include path")
parser.add_argument("-input-file", metavar="inp", action='append', help="C source file")
parser.add_argument("-mopsa-opt", metavar="mop", action='append', help="options to pass to MOPSA directly")
parser.add_argument("-dryrun", action="store_true", help="Print the command that will be executed")
parser.add_argument("-check-main", action="store_true", help="Check the main function instead of the functions supplied as externals")
parser.add_argument("-verbose", action="store_true", help="Output the full MOPSA output, not only the results")
parser.add_argument("-output-file", metavar="O", help="File for storing the output of the analysis.")
parser.add_argument("-timeout", metavar="T", type=float, default=60.0, help="Timout in seconds after which to abort the analysis. Default is 360s(= 6 min).")
parser.add_argument("-mopsa-exe", metavar="M", help="Path to MOPSA executable.")
parser.add_argument("-runtime", metavar="R", help="Path to the MOPSA runtime directory.")
parser.add_argument("-mopsa-config", metavar="C", help="Path to the MOPSA configuration folder.")

# we parse all known arguments (in particular includes);
# the remainder are arguments that should be forwarded to clang
args, clang_flags=parser.parse_known_args()

externals = args.externals
checker_timeout = args.timeout
includes = [] if args.include is None else args.include
output_file = args.output_file

check_externals = not (args.check_main)
c_files = [] if args.input_file is None else args.input_file
mopsa_args = [] if args.mopsa_opt is None else args.mopsa_opt
mopsa_path = args.mopsa_exe
runtime_path = args.runtime
mopsa_config_path = args.mopsa_config

if mopsa_path is None:
  print("Please provide the path to the MOPSA executable.")
  exit(1)

if not (os.path.exists(mopsa_path)) or not (os.access(mopsa_path, os.X_OK)):
  print(f"The MOPSA executable {mopsa_path} does not exist or is not executable.")
  exit(1)

if runtime_path is None:
  print("Please provide the path to the MOPSA runtime directory.")
  exit(1)

if not (os.path.exists(runtime_path)):
  print(f"The MOPSA runtime directory {runtime_path} does not exist.")
  exit(1)

if mopsa_config_path is None:
  print("Please provide the path to the MOPSA configuration directory.")
  exit(1)

if not (os.path.exists(mopsa_config_path)):
  print(f"The MOPSA configuration directory {mopsa_config_path} does not exist.")
  exit(1)

if check_externals and (externals is None):
  print("No externals file passed")
  exit(1)

# if the externals file does not exist, we fail loudly
if check_externals and (not (os.path.exists(externals))):
  print(f"The externals file {externals} does not exist.")
  exit(1)


# Paths for passing to MOPSA
vicuna_mopsa_config = mopsa_config_path
vicuna_config_file = "configs/c/ocaml-c-ffi.json"
ffi_include = os.path.join(runtime_path, "include/")
ffi_implementation = os.path.join(runtime_path, "src/ffi.c")



# we filter out paths containing "lib/ocaml", since they could point to a
# standard library with the original version of the runtime headers
def not_ocaml_include(x):
  return not (x.endswith("lib/ocaml"))

includes = list(filter(not_ocaml_include, includes))

# the mock includes and implementations
includes += [ffi_include]


# the mopsa flags
mopsa_flags = [
  "-share-dir " + vicuna_mopsa_config,
  "-config " + vicuna_mopsa_config + "/" + vicuna_config_file,
  "-clean-cur-only",
  "-hash-heap-address=true",
  "-disable-var-renaming-recursive-call",
  "-c-check-signed-arithmetic-overflow false",
  "-c-check-signed-implicit-cast-overflow false",
  "-c-check-unsigned-implicit-cast-overflow false",
  "-without-libc"
  # "-show-callstacks"
]
mopsa_verbose_flags = [
  "-no-warning", # suppresses stderr warning messages (e.g., unknown functions)
  "-no-detailed-errors-unimplemented" # suppresses detailed errors for functions where the test hit unimplemented functionality
]
mopsa_report_flags = [
  "-no-warning",
  "-no-detailed-errors-unimplemented",
  "-no-analysis-time", # suppresses the analysis time to obtain a stable report
  # "-no-detailed-errors", # suppresses the localized error descriptions
  # "-no-analysis-summary", # suppresses the analysis summary at the end
  # "-no-ffi-functions-report" # suppresses the report about FFI functions
]


mopsa_flags += [] if externals is None else ["-external-functions " + externals]
mopsa_flags += ["-runtimetest"] if check_externals else []

def mopsa_call(verbose_mopsa=False):
  call_args = []
  call_args += [mopsa_path]
  call_args += mopsa_flags
  call_args += mopsa_verbose_flags if verbose_mopsa else mopsa_report_flags
  call_args += mopsa_args
  call_args += ["-I " + i for i in includes]
  call_args += ["-ccopt " + i for i in clang_flags]
  call_args += c_files
  call_args += [ffi_implementation]

  return " ".join(call_args)



def remove_colors(text):
    ansi_escape = re.compile(r'\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])')
    return ansi_escape.sub('', text)

def output_result(out):
  out = remove_colors(out)
  if output_file is None:
    print(out)
  else:
    try:
      with open(output_file, 'w') as f:
        f.write(out)
    except PermissionError:
      # we don't have permission, so we produce no output
      return


## Inspection Hint
def inspect_analysis_hint_verbose():
  verbose_call = mopsa_call (verbose_mopsa=True)
  out= f"""\
## Inspect Analysis
You can manually re-run the analysis and inspect the output with the following command:
```
{verbose_call}
```

To increase the verbosity of the MOPSA output, consider doing any of the following:
- Remove the `-no-detailed-errors-unimplemented` flag to show the functions/callsites where the analysis fails
- Add the `-show-callstacks` to print all the checks that succeeded
- Remove the `-no-warning` flag, which suppresses warnings about unknown functions
"""
  return out

def inspect_analysis_hint():
  if args.verbose:
    return inspect_analysis_hint_verbose()
  else:
    return "For details, re-run the analysis with `-verbose` flag.\n"

## Summary
def print_analysis_summary(summary):
  out = \
f"""# Vicuna Analysis

## Summary

{summary}

{inspect_analysis_hint()}
"""

  output_result(out)
  return


def print_timeout(timeout):
  out = f"""\
# Vicuna Analysis

Sorry, the analysis could not be completed before a timeout of {timeout} seconds.

{inspect_analysis_hint()}
"""

  output_result(out)
  return


def print_ice(exit_code,output,error):
  out = f"""\
# Vicuna Analysis

Sorry, the C code could not be analyzed. (The analysis exited with exit code {exit_code}.)
This either means the C files could not be parsed successfully, or there was an internal error in the analysis that prevented it from completing.

{inspect_analysis_hint()}

"""
  log = f"""\
## Log
```
{output}
```
## Error
```
{error}
```
"""
  output_result(out + (log if args.verbose else ""))
  return


def print_empty():
    out = f"""\
# Vicuna Analysis

Nothing to do here. There were no external function declarations.

"""
    output_result(out)


# dryrun just echos the command we would have run
if args.dryrun:
    print(mopsa_call(verbose_mopsa=False))
    exit(0)

# if there's nothing to check, then we output this to the report and terminate.
if len(c_files) <= 0:
    if check_externals:
      with open(externals, 'r') as f:
          contents = f.read()

      regex = re.compile(r"\s*\(\(version\s+[^\)]+\)\s+\(extfuns\s+\(\)\)\)\s*")
      if not (re.match(regex, contents) is None):
        print_empty()
        exit(0)


# we are running in output mode
proc = subprocess.Popen(
  mopsa_call(verbose_mopsa=False),
  stdout=subprocess.PIPE,
  stderr=subprocess.PIPE,
  shell=True,
  universal_newlines=True,
  cwd=os.curdir,
  start_new_session=True
)
try:
  proc.wait(timeout=checker_timeout)
except subprocess.TimeoutExpired:
  os.killpg(os.getpgid(proc.pid), signal.SIGTERM)
  print_timeout(timeout=checker_timeout)
  exit(0)


# return code 0 for safe and 1 if we found a bug
if (proc.returncode == 0 or proc.returncode == 1):
  outp = "" if proc.stdout is None else proc.stdout.read()
  print_analysis_summary(summary=outp)
  exit(0)
else:
  outp = "" if proc.stdout is None else proc.stdout.read()
  err = "" if proc.stderr is None else proc.stderr.read()
  print_ice(exit_code=proc.returncode, output=outp, error=err)
  exit(0)
