#include "interleavingdistance.h"
#include "mergetreereader.h"

int main() {
    auto t1 = std::make_shared<MergeTree>();
	auto t2 = std::make_shared<MergeTree>();

    t1->addLeaf(2.0);
    t2->addLeaf(3.0);

	Interleaving interleaving = computeInterleavingDistance(t1, t2, SearchAlgorithm::DeltaLinearSearch, DeltaGoodMapAlgorithm::DP);

    auto trees = MergeTreeReader::readMergeTrees("example/source-tree.txt", "example/target-tree.txt", "example/restriction-matrix.txt");
    interleaving = computeInterleavingDistance(std::make_shared<MergeTree>(trees.m_sourceTree), std::make_shared<MergeTree>(trees.m_targetTree), SearchAlgorithm::DeltaExponentialSearch, DeltaGoodMapAlgorithm::DP, trees.m_restrictions);
    std::cout << interleaving.getDelta();

    return 0;
}