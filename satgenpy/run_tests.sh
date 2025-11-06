#!/usr/bin/env bash

# All tests
source ../env/bin/activate
python3 -m unittest discover -v -s tests

# Command template for running an individual test:
# python -m unittest discover -v -s tests -p "[name of test file].py"
