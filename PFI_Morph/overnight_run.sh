#!/bin/bash
set -e

# Uncomment one of these
# 1A. Run all NxN morphs and save reports. The morph animations are not saved
time ./morph_tests.py -i 500 -p all

# 1B. Same as the previous command, but save vdb files for the animation
# CAREFUL: This generates a TON of data. Use only if you have the disk space
# and the time to wait.
#time ./morph_tests.py -i 500 -ps all

# 2. Post-process the JSON files into a big CSV table + HTML Table
./post_process.py

# 3. Refresh the ground truth data. This doesn't need to be done every time,
# but it doesn't take very long to compute compared to the other steps
./ground_truth.py

# 4. Apply machine learning to the results
./analyze_morphs.py
