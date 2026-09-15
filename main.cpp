#include "interleavingdistance.h"

int main() {
    auto t1 = std::make_shared<MergeTree>();
	auto t2 = std::make_shared<MergeTree>();

    t1->addLeaf(2.0);
    t2->addLeaf(3.0);

	Interleaving forwardInterleaving = computeInterleavingDistance(t1, t2, SearchAlgorithm::DeltaLinearSearch, DeltaGoodMapAlgorithm::DP);
    std::cout << forwardInterleaving.getDelta();
    return 0;
}