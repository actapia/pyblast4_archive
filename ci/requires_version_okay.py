import argparse
import sys
try:
    import tomllib
except ImportError:
    import pip._vendor.tomli as tomllib
from pip._internal.utils.packaging import check_requires_python
from pathlib import Path

def handle_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("pyproject", type=Path)
    return parser.parse_args()

def main():
    args = handle_arguments()
    print("".join(map(str, sys.version_info[:2])))
    with open(args.pyproject, "rb") as pyproject:
        sys.exit(
            not check_requires_python(
                tomllib.load(pyproject)["project"]["requires-python"],
                sys.version_info[:3]
            )
        )

if __name__ == "__main__":
    main()
    
    
