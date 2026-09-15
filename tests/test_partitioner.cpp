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
