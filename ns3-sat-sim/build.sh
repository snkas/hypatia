NS3_VERSION="ns-3.45"

# Usage help
if [ "$1" == "--help" ]; then
  echo "Usage: bash build.sh [--help, --debug_all, --debug_minimal, --optimized, --optimized_with_tests]"
  exit 0
fi

# Extract copy of ns-3
echo "Unzipping clean ns-3 (no overwrites)"
tar jxvf ${NS3_VERSION}.tar.bz2 || exit 1
# Keep Hypatia's ignore rules instead of replacing them with ns-3's versions.
rm -f "${NS3_VERSION}/.gitignore" \
      "${NS3_VERSION}/src/.gitignore" \
      "${NS3_VERSION}/contrib/.gitignore" \
      "${NS3_VERSION}/scratch/.gitignore" || exit 1
# Preserve timestamps so unchanged ns-3 sources do not trigger a full rebuild.
cp -a "${NS3_VERSION}/." simulator/ || exit 1
rm -r ${NS3_VERSION} || exit 1
cd simulator || exit 1

# Update the basic-sim module
echo "Updating git submodules"
git submodule update || exit 1

if [ -f ../../.venv/bin/activate ]; then
  source ../../.venv/bin/activate || exit 1
fi

# Configure the build
if [ "$1" == "--debug_all" ]; then
  ./ns3 configure --build-profile=debug --enable-mpi --enable-examples --enable-tests --enable-gcov --out=build/debug_all || exit 1

elif [ "$1" == "--debug_minimal" ]; then
  ./ns3 configure --build-profile=debug --enable-mpi --out=build/debug_minimal || exit 1

elif [ "$1" == "--optimized" ]; then
  ./ns3 configure --build-profile=optimized --enable-mpi --out=build/optimized || exit 1

elif [ "$1" == "--optimized_with_tests" ]; then
  ./ns3 configure --build-profile=optimized --enable-mpi --enable-tests --out=build/optimized_with_tests || exit 1

elif [ "$1" == "" ]; then
  # Default is debug_all
  ./ns3 configure --build-profile=debug --enable-mpi --enable-examples --enable-tests --enable-gcov --out=build/debug_all || exit 1

else
  echo "Invalid build option: $1"
  echo "Usage: bash build.sh [--debug_all, --debug_minimal, --optimized, --optimized_with_tests]"
  exit 1
fi

# Perform the build
./ns3 build -j4 || exit 1
