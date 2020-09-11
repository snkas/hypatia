
# Create the basic-sim module
cd simulator || exit 1

# Rebuild whichever build is configured right now
./waf -j4 || exit 1
