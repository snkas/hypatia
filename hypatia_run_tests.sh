# Main information
echo "Hypatia: test"
echo ""
echo "It is highly recommend you use a recent Linux operating system (e.g., Ubuntu 20 or higher)."
echo "Python version 3.7+ is required."
echo ""

# satgenpy
echo "Running tests for satgenpy..."
cd satgenpy || exit 1
bash run_tests.sh || exit 1
cd .. || exit 1

# ns3-sat-sim
echo "Running tests for ns3-sat-sim..."
cd ns3-sat-sim || exit 1
bash build.sh --debug_all || exit 1
bash test.sh || exit 1
cd .. || exit 1

# satviz
echo "Nothing to test for satviz."

# Integration tests
echo "Running integration tests..."
cd integration_tests || exit 1
bash run_integration_tests.sh || exit 1
cd .. || exit 1

# paper
echo "Nothing to test for paper."

# Confirmation all tests were run
echo ""
echo "Hypatia tests were run and passed."
