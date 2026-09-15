#pragma once

#include <map>
#include <memory>

#include "mergetree.h"

/// An interleaving between two merge trees.
class Interleaving {

	public:
		/// Creates an interleaving for the given trees, with the given delta
		/// value and matching.
		Interleaving(const std::shared_ptr<MergeTree>& sourceTree,
		             const std::shared_ptr<MergeTree>& targetTree, double delta,
		             std::map<int, int> matching);

		/// Returns the shift of this interleaving.
		double getDelta() const;
		/// Sets the shift of this interleaving.
		void setDelta(double delta);

		void print() const;

	private:
		/// The source tree.
		std::shared_ptr<MergeTree> m_sourceTree;
		/// The target tree.
		std::shared_ptr<MergeTree> m_targetTree;
		/// The shift of this interleaving.
		double m_delta;
		/// The matching from source tree nodes to target tree nodes.
		std::map<int, int> m_matching;
};
