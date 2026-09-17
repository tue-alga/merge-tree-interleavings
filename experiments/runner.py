#!/usr/bin/python3

import glob
import subprocess
import sys
from time import sleep

print('filename\talgorithm\tdelta\ttime_seconds\tattempt_count\tattempt_delta\tattempt_time_seconds\t...', flush=True)

datasets = glob.glob(sys.argv[1] + '/**/*', recursive=True)

def runTrial(dataset, algorithm):
	try:
		subprocess.run(['build/interleavingcli', dataset, algorithm], timeout=310)  # timeout of 5 minutes + 10 seconds grace period
	except subprocess.TimeoutExpired:
		sleep(1)
		print("\nExperiment killed by timeout", file=sys.stderr)
		print(dataset + '\t' + algorithm + '\t?\ttimeout', flush=True)
		sleep(1)

for dataset in datasets:
	runTrial(dataset, 'sweepline/exponential')
	runTrial(dataset, 'DP/exponential')
