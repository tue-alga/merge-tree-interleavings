// Copyright (C) 2025-2026 TU Eindhoven
//
// This file is part of merge-tree-interleavings.
// 
// merge-tree-interleavings is free software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published by
// the the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
// 
// merge-tree-interleavings is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
// Public License for more details.
// 
// You should have received a copy of the GNU General Public License along with
// merge-tree-interleavings. If not, see <https://www.gnu.org/licenses/>.

#include "catch.hpp"

#include "partitioner.h"

TEST_CASE("make a partition of three elements") {
	std::vector<int> elements = {1, 2, 3};
	std::vector<std::pair<std::vector<int>, std::vector<int>>> partitions;
	for (const auto& partition : Partition<int>(elements)) {
		partitions.push_back(partition);
	}
	REQUIRE(partitions.size() == 8);
}

TEST_CASE("make a partition of two elements") {
	std::vector<int> elements = {1, 2};
	std::vector<std::pair<std::vector<int>, std::vector<int>>> partitions;
	for (const auto& partition : Partition<int>(elements)) {
		partitions.push_back(partition);
	}
	REQUIRE(partitions.size() == 4);
}

TEST_CASE("make a partition of one element") {
	std::vector<int> elements = {1};
	std::vector<std::pair<std::vector<int>, std::vector<int>>> partitions;
	for (const auto& partition : Partition<int>(elements)) {
		partitions.push_back(partition);
	}
	REQUIRE(partitions.size() == 2);
}

TEST_CASE("make a partition of zero elements") {
	std::vector<int> elements = {};
	std::vector<std::pair<std::vector<int>, std::vector<int>>> partitions;
	for (const auto& partition : Partition<int>(elements)) {
		partitions.push_back(partition);
	}
	REQUIRE(partitions.size() == 1);
}
