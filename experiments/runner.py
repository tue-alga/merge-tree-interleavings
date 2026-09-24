#!/usr/bin/python3

from pathlib import Path
import subprocess
import sys
import os
from time import sleep

print('filename\talgorithm\tdelta\ttime_seconds\tattempt_count\tattempt_delta\tattempt_time_seconds\t...', flush=True)

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
	# print('filename\talgorithm\tdelta\ttime_seconds\tattempt_count\tattempt_delta\tattempt_time_seconds\t...', flush=True)
	datasets = sys.argv[1]
	runTrial(datasets, 'sweepline/exponential')
	runTrial(datasets, 'DP/exponential')
