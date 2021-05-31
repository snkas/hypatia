# Paper reproduction

This is the code repository introduced and used in "Exploring the “Internet from space” with Hypatia" 
by Simon Kassing*, Debopam Bhattacherjee*, André Baptista Águas, Jens Eirik Saethre and Ankit Singla
(*equal contribution), which is published in the Internet Measurement Conference (IMC) 2020.

It is highly recommend you use a Linux operating system (e.g., Ubuntu 18 or higher).

## Getting the data without running anything

Some parts of Hypatia take significant time to run. As such, if you want to get started quickly,
you can download and extract all (temporary) data which Hypatia generates for the paper.

1. Download `hypatia_paper_temp_data.tar.gz` and put it into `<hypatia>/paper/`.

   Download link (password (if asked): "hypatia_paper"): 
   * (v1: preliminary) https://polybox.ethz.ch/index.php/s/Y35PwifNOKkcnvR
   
     SHA-256 checksum:
     18d761a28706723b57772e0636fbc40b7d57161f4c54069eede0c8ae740cbe2d
     
   * (Previous versions: v0)
   
2. Double-check: the archive `<hypatia>/paper/hypatia_paper_temp_data.tar.gz` now exists.

3. Make sure you have the `numpy`, `exputil` and `networkload` Python packages installed:
   ```
   pip install numpy
   pip install git+https://github.com/snkas/exputilpy.git@v1.6
   pip install git+https://github.com/snkas/networkload.git@v1.3
   ```
   
4. Make sure gnuplot is installed:
   ```
   sudo apt-get install gnuplot
   ```

5. Extract the temporary data:
   ```
   cd paper
   python extract_temp_data.py
   ```

## Steps to run

**Step 1: generating LEO satellite network dynamic state over time**

Instructions can be found in `<hypatia>/paper/satellite_networks_state/README.md`

**Step 2: build ns-3 simulator**

Instructions can be found in `<hypatia>/ns3-sat-sim/README.md`

**Step 3: performing analysis using satgenpy**

Instructions can be found in `<hypatia>/paper/satgenpy_analysis/README.md`

**Step 4: running ns-3 experiments**

Instructions can be found in `<hypatia>/paper/ns3_experiments/README.md`

**Step 5: generating satviz figures**

Instructions can be found in `<hypatia>/satviz/README.md` under `Visualizations in the paper`.

**Step 6: plotting figures of the paper**

Instructions can be found in `<hypatia>/paper/figures/README.md`
