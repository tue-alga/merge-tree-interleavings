# Copyright (C) 2025-2026 TU Eindhoven
#
# This file is part of merge-tree-interleavings.
# 
# merge-tree-interleavings is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published by
# the the Free Software Foundation, either version 3 of the License, or (at
# your option) any later version.
# 
# merge-tree-interleavings is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
# Public License for more details.
# 
# You should have received a copy of the GNU General Public License along with
# merge-tree-interleavings. If not, see <https://www.gnu.org/licenses/>.

#!/usr/bin/python3

from pathlib import Path
import subprocess
import sys
import os
from time import sleep

def runTrial(dataset, algorithm):

	merge_tree_files = sorted(f for f in os.listdir(dataset) if f.startswith("merge-tree-") and f.endswith(".txt"))
	ids = [name[len("merge-tree-"):-len(".txt")] for name in merge_tree_files]

	radius_directories = sorted(d for d in os.listdir(dataset) if d.startswith("radius-"))
	for r in radius_directories:
		for i in range(len(merge_tree_files)):
			for j in range(i, len(merge_tree_files)):
				source_file = f'{dataset}merge-tree-{ids[i]}.txt'
				target_file = f'{dataset}merge-tree-{ids[j]}.txt'
				restriction_file = f'{dataset}{r}/{ids[i]}_{ids[j]}.txt'
				try:
					subprocess.run(['build/interleavingcli', source_file, target_file, restriction_file, algorithm], timeout=310)  # timeout of 5 minutes + 10 seconds grace period
				except subprocess.TimeoutExpired:
					sleep(1)
					print("\nExperiment killed by timeout", file=sys.stderr)
					print(dataset + '\t' + algorithm + '\t?\ttimeout', flush=True)
					sleep(1)

if __name__ == "__main__":
	print('filename\talgorithm\tdelta\ttime_seconds\tattempt_count\tattempt_delta\tattempt_time_seconds\t...', flush=True)
	datasets = sys.argv[1]
	runTrial(datasets, 'sweepline/exponential')
	runTrial(datasets, 'DP/exponential')
