NS3_VERSION="ns-3.41"

# Usage help
if [ "$1" == "--help" ]; then
  echo "Usage: bash build.sh [--help, --debug_all, --debug_minimal, --optimized, --optimized_with_tests]"
  exit 0
fi

# Extract copy of ns-3
echo "Unzipping clean ns-3 (no overwrites)"
unzip ${NS3_VERSION}.zip || exit 1
cp -r -n ${NS3_VERSION}/* simulator/ || exit 1
rm -r ${NS3_VERSION} || exit 1
cd simulator || exit 1

# # Configure the build
if [ "$1" == "--debug_all" ]; then
  ./ns3 configure --build-profile=debug --disable-werror --enable-mpi --enable-examples --enable-tests --enable-gcov --out=build/debug_all || exit 1

elif [ "$1" == "--debug_minimal" ]; then
  ./ns3 configure --build-profile=debug --disable-werror --enable-mpi --out=build/debug_minimal || exit 1

elif [ "$1" == "--optimized" ]; then
  ./ns3 configure --build-profile=optimized --enable-mpi --out=build/optimized || exit 1

elif [ "$1" == "--optimized_with_tests" ]; then
  ./ns3 configure --build-profile=optimized --enable-mpi --enable-tests --out=build/optimized_with_tests || exit 1

elif [ "$1" == "" ]; then
  # Default is debug_all
  ./ns3 configure --build-profile=debug --disable-werror --enable-mpi --enable-examples --enable-tests --enable-gcov --out=build/debug_all || exit 1

else
  echo "Invalid build option: $1"
  echo "Usage: bash build.sh [--debug_all, --debug_minimal, --optimized, --optimized_with_tests]"
  exit 1
fi

# Perform the build
./ns3 build || exit 1
