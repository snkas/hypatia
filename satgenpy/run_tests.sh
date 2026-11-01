#!/usr/bin/env bash

# All tests
source ../.venv/bin/activate || exit 1
python3 -m unittest discover -v -s tests

# Command template for running an individual test:
# python -m unittest discover -v -s tests -p "[name of test file].py"
