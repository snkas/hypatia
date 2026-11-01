# Usage help
if [ "$1" == "--help" ]; then
  echo "Usage: bash test.sh [--help] [--satnet, --coverage]*"
  exit 0
fi

# Test results output folder
mkdir -p test_results

# Rebuild to have the most up-to-date
bash rebuild.sh || exit 1

# Go into the ns-3 folder
cd simulator || exit 1

# Core tests
if [ "$1" == "" ] || [ "$1" == "--satnet" ] || [ "$2" == "--satnet" ] || [ "$3" == "--satnet" ] || [ "$4" == "--satnet" ]; then
  # Only reset counters when running tests, so --coverage can report a prior run.
  echo "Zeroing coverage counters"
  lcov --directory cmake-cache --zerocounters || exit 1
  echo "Performing satellite-network tests"
  cd test_data || exit 1
  bash extract_test_data.sh || exit 1
  cd .. || exit 1
  python3 test.py --no-build -v -s "satellite-network" -t ../test_results/test_results_satellite_network || exit 1
  cat ../test_results/test_results_satellite_network.txt
fi

# Back to main directory
cd .. || exit 1

# Coverage report
if [ "$1" == "" ] || [ "$1" == "--coverage" ] || [ "$2" == "--coverage" ] || [ "$3" == "--coverage" ] || [ "$4" == "--coverage" ]; then

  # Make coverage report
  rm -rf coverage_report
  mkdir -p coverage_report
  lcov --capture --directory simulator/cmake-cache/contrib/satellite-network --exclude "/usr/*" --output-file coverage_report/coverage.info || exit 1

  # Remove directories from coverage report which we don't want
  lcov -r coverage_report/coverage.info "*/test/*" --output-file coverage_report/coverage.info || exit 1

  # Generate html
  genhtml --output-directory coverage_report coverage_report/coverage.info || exit 1
  echo "Coverage report is located at: coverage_report/index.html"

  # Show results
  echo "Display test results"
  if [ "$1" == "" ] || [ "$1" == "--satnet" ] || [ "$2" == "--satnet" ] || [ "$3" == "--satnet" ] || [ "$4" == "--satnet" ]; then
    cat test_results/test_results_satellite_network.txt
  fi

fi
