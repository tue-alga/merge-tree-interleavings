
#include <chrono>
#include <filesystem>
#include <iostream>

#include "mergetreereader.h"
#include "interleaving.h"
#include "interleavingdistance.h"

int NUM_TRIALS = 1;

void runTrial(std::string path, const std::shared_ptr<MergeTree>& sourceTree, const std::shared_ptr<MergeTree>& targetTree,
		const RestrictionMatrix& restrictions, SearchAlgorithm searchAlgorithm, DeltaGoodMapAlgorithm deltaGoodMapAlgorithm, std::string algorithmName) {
	
	struct DeltaResult {
		double delta;
		long nsecs;
	};

	using Clock = std::chrono::steady_clock;
	auto timerStart = Clock::now();
	auto deltaStart = Clock::time_point{};

	std::vector<DeltaResult> results;
	Interleaving interleaving = computeInterleavingDistance(sourceTree, targetTree, searchAlgorithm, deltaGoodMapAlgorithm, restrictions, nullptr, [&](double delta) {
		deltaStart = Clock::now();
	}, [&](double delta) {
		auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - deltaStart).count();
		results.emplace_back(delta, elapsed);
	});

	std::cout << path << "\t" << algorithmName << "\t" << interleaving.getDelta() << "\t" << std::chrono::duration<double>(Clock::now() - timerStart).count() << "\t" << results.size();
	for (DeltaResult& result : results) {
		double seconds = std::chrono::duration<double>(std::chrono::nanoseconds(result.nsecs)).count();
		std::cout << "\t" << result.delta << "\t" << seconds;
	}
	std::cout << std::endl;
}

int main(int argc, char* argv[]) {
	if (argc != 5) {
		std::cerr << "Usage: interleavingcli <source tree file> <target tree file> <restriction matrix file> <algorithm>";
		return 1;
	}

	std::string sourceTreeFile = argv[1];
	std::string targetTreeFile = argv[2];
	std::string restrictionMatrixFile = argv[3];
	std::string algorithm = argv[4];

	MergeTreeReader::MergeTrees trees;
	try {
		trees = MergeTreeReader::readMergeTrees(sourceTreeFile, targetTreeFile, restrictionMatrixFile);
	} catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}

	auto sourceTree = std::make_shared<MergeTree>(trees.m_sourceTree);
	auto targetTree = std::make_shared<MergeTree>(trees.m_targetTree);

	std::filesystem::path source(sourceTreeFile);
	std::filesystem::path target(targetTreeFile);
	std::filesystem::path restrictionMatrix(restrictionMatrixFile);
	std::string identifier = source.filename().string() + " | " + target.filename().string() + " | " + restrictionMatrix.filename().string(); 


	for (int i = 0; i < NUM_TRIALS; i++) {
		if (algorithm == "DP/linear") {
			runTrial(identifier, sourceTree, targetTree, trees.m_restrictions, SearchAlgorithm::DeltaLinearSearch, DeltaGoodMapAlgorithm::DP, algorithm);
		} else if (algorithm == "DP/exponential") {
			runTrial(identifier, sourceTree, targetTree, trees.m_restrictions, SearchAlgorithm::DeltaExponentialSearch, DeltaGoodMapAlgorithm::DP, algorithm);
		} else if (algorithm == "sweepline/linear") {
			runTrial(identifier, sourceTree, targetTree, trees.m_restrictions, SearchAlgorithm::DeltaLinearSearch, DeltaGoodMapAlgorithm::Sweepline, algorithm);
		} else if (algorithm == "sweepline/exponential") {
			runTrial(identifier, sourceTree, targetTree, trees.m_restrictions, SearchAlgorithm::DeltaExponentialSearch, DeltaGoodMapAlgorithm::Sweepline, algorithm);
		}
	}
}
