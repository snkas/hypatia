mkdir -p test-results
cd simulator || exit 1
cd test_data || exit 1
bash extract_test_data.sh || exit 1
cd .. || exit 1
python test.py -v -s "satellite-network"
