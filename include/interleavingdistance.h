#pragma once

#include <functional>
#include <iostream>
#include <optional>

#include "interleaving.h"
#include "mergetree.h"

using RestrictionMatrix = std::vector<std::vector<double>>;

enum class SearchAlgorithm {
	DeltaLinearSearch,
	DeltaExponentialSearch
};

enum class DeltaGoodMapAlgorithm {
	DP,
	Sweepline
};

/// Computes an optimal interleaving between the two given trees.
/// The distance is computed using the specified search and decision procedures.
/// Optionally, one can use a restriction matrix to compute a restricted interleaving.
Interleaving computeInterleavingDistance(const std::shared_ptr<MergeTree>& sourceTree,
                                         const std::shared_ptr<MergeTree>& targetTree,
                                         SearchAlgorithm searchAlgorithm,
                                         DeltaGoodMapAlgorithm deltaGoodMapAlgorithm,
                                         RestrictionMatrix restrictions = {},
                                         std::function<void(std::pair<double, double>)> onMovedSweepline = nullptr,
                                         std::function<void(double)> onStartedDelta = nullptr,
                                         std::function<void(double)> onEndedDelta = nullptr);
