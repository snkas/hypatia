
# Create the basic-sim module
cd simulator || exit 1

# Rebuild whichever build is configured right now
./ns3 clean || exit 1
./ns3 build || exit 1
