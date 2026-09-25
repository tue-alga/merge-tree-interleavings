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

#pragma once

#include <bit>
#include <vector>

/**
 * A class for generating iterators of partitions of a given set
 */
template <typename T>
class Partition {
	public:
		explicit Partition(const std::vector<T>& data) : m_data(data) {};

		struct Iterator {
			public:
				Iterator() = default; // Necessary for C20 concept
				Iterator(const std::vector<T>& data, std::vector<unsigned long> state) : m_data(data), m_state(state) {
					build();
				}

				std::pair<std::vector<T>, std::vector<T>> operator*() const {
					return {a, b};
				};
				Iterator& operator++() {
					++m_state.front();
					int i = 0;
					while (m_state[i++] == 0) { // This is not pretty
						if (m_state.size() == i)
							m_state.push_back(0u);
						++m_state[i];
					}
					build();
					return *this;
				};
				friend bool operator==(const Iterator& lhs, const Iterator& rhs) {
					return lhs.m_state == rhs.m_state;
				}

			private:
				std::vector<unsigned long> m_state; // Dynamic bitset
				const std::vector<T>& m_data;
				std::vector<T> a, b;

				void build() {
					a.clear();
					b.clear();
					if (m_data.empty())
						return;
					int popcount = 0;
					for (const auto& mask : m_state) {
						popcount += std::popcount(mask);
					}
					a.reserve(popcount);
					b.reserve(m_data.size() - popcount);
					for (int i = 0; i < m_state.size() - 1; ++i) {
						for (int j = 0; j < BITLENGTH; ++j) {
							if ((m_state[i] >> j) & 1)
								a.emplace_back(m_data[i * BITLENGTH + j]);
							else
								b.emplace_back(m_data[i * BITLENGTH + j]);
						}
					}
					const int offset = (m_data.size() / BITLENGTH) *
					                   (m_state.size() - 1); // Number of processed bits
					for (int j = 0; j < m_data.size() - offset; ++j) {
						if ((m_state.back() >> j) & 1)
							a.emplace_back(m_data[offset + j]);
						else
							b.emplace_back(m_data[offset + j]);
					}
				}
		};

		Iterator begin() {
			return Iterator(m_data, {0u});
		}
		Iterator end() {
			int size = m_data.size();
			std::vector<unsigned long> state(size / BITLENGTH + 1, -1);
			state.back() = 1l << (size - size / BITLENGTH); // FIXME [ws] bug: doesn't set the rest of the state vector to zero
			return Iterator(m_data, state);
		}

	private:
		const std::vector<T>& m_data;
		static constexpr int BITLENGTH = sizeof(unsigned long) * 8;
};

/**
 * A class for generating iterators of subsequences of a given set
 */
template <typename T>
class Subsequence {
	public:
		explicit Subsequence(const std::vector<T>& data) : m_data(data) {};

		struct Iterator {
			public:
				Iterator() = default; // Necessary for C20 concept
				Iterator(const std::vector<T>& data, size_t state) : m_data(data), m_state(state) {
					build();
				}

				std::pair<std::vector<T>, std::vector<T>> operator*() const {
					return {a, b};
				};
				Iterator& operator++() {
					++m_state;
					build();
					return *this;
				};
				friend bool operator==(const Iterator& lhs, const Iterator& rhs) {
					return lhs.m_state == rhs.m_state; // && lhs.m_data == rhs.m_data;
				}

			private:
				size_t m_state = 0; // Dynamic bitset
				const std::vector<T>& m_data;
				std::vector<T> a, b;

				void build() {
					a.clear();
					b.clear();
					if (m_data.empty())
						return;
					a.assign(m_data.begin(), m_data.begin() + m_state);
					b.assign(m_data.begin() + m_state, m_data.end());
				}
		};

		Iterator begin() {
			return Iterator(m_data, 0u);
		}
		Iterator end() {
			return Iterator(m_data, m_data.size() / 2 + 1);
		}

	private:
		const std::vector<T>& m_data;
};
