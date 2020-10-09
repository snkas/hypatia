num_threads=20
for i in 2 5 8 11 14 1 4 7 10 13 0 3 6 9 12
do
  bash generate_for_paper.sh ${i} ${num_threads} || exit 1
done
