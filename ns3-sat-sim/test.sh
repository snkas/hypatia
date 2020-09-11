mkdir -p test-results
cd simulator || exit 1
python test.py -v -s "satellite-network"
