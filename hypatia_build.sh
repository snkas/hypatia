# Main information
echo "Hypatia: build"
echo ""
echo "It is highly recommend you use a recent Linux operating system (e.g., Ubuntu 18 or higher)."
echo "Python version 3.7+ is required."
echo ""

# ns3-sat-sim
echo "Building ns3-sat-sim..."
cd ns3-sat-sim || exit 1
bash build.sh --debug_all || exit 1
# For optimized: bash build.sh --optimized || exit 1
cd .. || exit 1

# satgenpy
echo "Nothing to build for satgenpy."

# satviz
echo "Nothing to build for satviz."

# paper
echo "Nothing to build for paper."
