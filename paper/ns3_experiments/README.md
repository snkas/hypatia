# ns-3 experiments

Given that you have generated the constellation dynamic state data over time in 
`satgenpy`, here you can run the ns-3 experiments used in the paper.

## A to B experiments

**Explanation**

There is communication for three directed pairs presented in the paper. 
These were chosen from among the 100 ground stations used, which were an 
input in `satgenpy`:

```
satgenpy/data/ground_stations_cities_sorted_by_estimated_2025_pop_top_100.basic.txt
```

The Kuiper-610 shell has 34 x 34 = 1156 satellites. Node identifiers start 
with the satellites, as such the 100 ground stations have node identifiers 
1156 (incl.) till 1256 (excl.). This similarly applies to Starlink (22 x 72 = 1584) 
and Telesat (27 x 13 = 351). 

The following ground stations are used:

```
City name          GID           Kuiper id        Starlink id         Telesat id
Rio de Janeiro     18     1156 + 18 = 1174   1584 + 18 = 1602    351 + 18 = 369
St. Petersburg     73     1156 + 73 = 1229   1584 + 73 = 1657    351 + 73 = 424
Manila             17     1156 + 17 = 1173   1584 + 17 = 1601    351 + 17 = 368
Dalian             85     1156 + 85 = 1241   1584 + 85 = 1669    351 + 85 = 436
Istanbul           14     1156 + 14 = 1170   1584 + 14 = 1598    351 + 14 = 365
Nairobi            96     1156 + 96 = 1252   1584 + 96 = 1680    351 + 96 = 447
Paris              24     1156 + 24 = 1180   1584 + 24 = 1608    351 + 24 = 375
Moscow             21     1156 + 21 = 1177   1584 + 21 = 1605    351 + 21 = 372
```

... of which the following directed pairs are run in Kuiper-610:

* Rio de Janeiro (1174) to St. Petersburg (1229)
* Manila (1173) to Dalian (1241)
* Istanbul (1170) to Nairobi (1252)
* Paris (1180) to Moscow (1177)

Additionally, for the Paris to Moscow case, we also have the ground station relay 
with Kuiper-610 experiment, which does not use the top 100 ground stations.
In here, Paris has GID 0 (= 1156 node id) and Moscow has GID 76 (= 1232).

**Commands**

Run and analyze these pairs by executing:

```
cd a_b || exit 1
python step_1_generate_runs.py || exit 1
python step_2_run.py || exit 1
python step_3_generate_plots.py || exit 1
```

## Traffic matrix

**Explanation**

In this experiment, it is investigated how well the TCP flow is able to make use 
of the available bandwidth, given that there is competing traffic present. One
specific pair is investigated, Rio de Janeiro (1174) to St. Petersburg (1229).
Beyond those pairs, a random reciprocal permutation pairing traffic matrix
is applied to serve as competing "background" traffic.  From the random 
permutation matrix we remove the pairs which had at any point has the 
same source or destination satellite as Rio de Janeiro or St. Petersburg 
to decrease the chance that the first or last hop is immediately the bottleneck.

**Commands**

```
cd traffic_matrix || exit 1
python step_1_generate_runs.py || exit 1
python step_2_run.py || exit 1
python step_3_generate_plots.py || exit 1
```

## Traffic matrix load (scalability)

**Explanation**

In these experiments the scalability of the simulator is investigated.
A traffic matrix, which is a random reciprocal permutation pairing,
is used to load the network. Entries in the traffic matrix correspond
to long-living TCP flows.

To test the scalability, the traffic matrix is run under a range of
link rates and experiment durations.

**Commands**

```
cd traffic_matrix_load || exit 1
python step_1_generate_runs.py || exit 1
python step_2_run.py || exit 1
python step_3_generate_plots.py || exit 1
```
