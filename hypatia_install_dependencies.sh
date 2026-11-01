# Main information
echo "Hypatia: installing dependencies"
echo ""
echo "It is highly recommend you use a recent Linux operating system (e.g., Ubuntu 20 or higher)."
echo "Python 3.8+, CMake 3.13+, and GCC 10.1+ (or Clang 11+) are required."
echo ""

# General
sudo apt-get update || exit 1

sudo apt-get install -y build-essential cmake git bzip2 screen python3 python3-venv || exit 1

python3 -m venv ./.venv || exit 1
source ./.venv/bin/activate || exit 1

# satgenpy
echo "Installing dependencies for satgenpy..."
pip install numpy astropy ephem networkx sgp4 geopy matplotlib statsmodels || exit 1
sudo apt-get install libproj-dev proj-data proj-bin libgeos-dev || exit 1
# Mac alternatives (to be able to pip install cartopy)
# brew install proj geos
# export CFLAGS=-stdlib=libc++
# MACOSX_DEPLOYMENT_TARGET=10.14
pip install git+https://github.com/snkas/exputilpy.git@v1.6 || exit 1
pip install cartopy || exit 1

# ns3-sat-sim
echo "Installing dependencies for ns3-sat-sim..."
sudo apt-get -y install openmpi-bin openmpi-common openmpi-doc libopenmpi-dev lcov gnuplot || exit 1
pip install numpy statsmodels || exit 1
pip install git+https://github.com/snkas/exputilpy.git@v1.6 || exit 1
git submodule update --init --recursive || exit 1

# satviz
echo "There are currently no dependencies for satviz."

# paper
echo "Installing dependencies for paper..."
pip install numpy || exit 1
pip install git+https://github.com/snkas/exputilpy.git@v1.6 || exit 1
pip install git+https://github.com/snkas/networkload.git@v1.3 || exit 1

# Confirmation dependencies are installed
echo ""
echo "Hypatia dependencies have been installed."
