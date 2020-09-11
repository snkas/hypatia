#!/usr/bin/env bash

pip install coverage
coverage run --omit="tests/*" -m unittest discover -v -s tests
coverage html
rm .coverage
