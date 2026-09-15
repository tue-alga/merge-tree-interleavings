#include "interleaving.h"

Interleaving::Interleaving(const std::shared_ptr<MergeTree>& sourceTree,
                           const std::shared_ptr<MergeTree>& targetTree, double delta,
                           std::map<int, int> matching)
    : m_sourceTree(sourceTree), m_targetTree(targetTree), m_delta(delta), m_matching(matching) {}

double Interleaving::getDelta() const {
	return m_delta;
}

void Interleaving::setDelta(double delta) {
	m_delta = delta;
}

void Interleaving::print() const {
	for (auto [first, second] : m_matching) {
		std::cout << first << " -> " << second << std::endl;
	}
}
