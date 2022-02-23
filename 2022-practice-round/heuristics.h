#pragma once

#include <random>
#include <unordered_set>
#include <vector>

std::unordered_set<int> randomResolution(const std::vector<std::unordered_set<int>>& graph) {
	std::random_device dev;
	std::mt19937 gen(dev());
	std::uniform_int_distribution<int> random_bool(0, 1);

	std::unordered_set<int> satisfied;
	for (int i = 0; i < graph.size(); ++i) satisfied.insert(i);
	bool hasConflict = true;
	while (hasConflict) {
		hasConflict = false;
		for (auto a : satisfied) {
			for (auto b : satisfied) {
				if (graph[a].count(b) != 0) {
					bool removeA = random_bool(gen);
					satisfied.erase(removeA ? a : b);
					hasConflict = true;
					break;
				}
			}
			if (hasConflict) break;
		}
	}
	return satisfied;
}

std::unordered_set<int> uniformRandomResolution(const std::vector<std::unordered_set<int>>& graph) {
	std::random_device dev;
	std::mt19937 gen(dev());

	std::unordered_set<int> satisfied;
	for (int i = 0; i < graph.size(); ++i) satisfied.insert(i);
	while (true) {
		std::vector<int> conflicts;
		for (auto a : satisfied) {
			for (auto b : satisfied) {
				if (graph[a].count(b) != 0) {
					conflicts.push_back(a);
					break;
				}
			}
		}
		if (conflicts.size() == 0) break;
		std::uniform_int_distribution<int> dist(0, conflicts.size() - 1);
		satisfied.erase(conflicts[dist(gen)]);
	}
	return satisfied;
}

template <typename T>
std::unordered_set<T> removeMostConflicting(std::vector<std::unordered_set<T> > graph) {
	std::unordered_set<T> satisfied;
	for (T i = 0; i < graph.size(); ++i) satisfied.insert(i);
	while (true) {
		int maxConflicts = 0;
		int mostConflictingPerson = -1;
		for (T person : satisfied) {
			int numConflicts = graph[person].size();
			if (numConflicts > maxConflicts) {
				maxConflicts = numConflicts;
				mostConflictingPerson = person;
			}
		}
		if (maxConflicts == 0) break;
		satisfied.erase(mostConflictingPerson);
		for (auto& node : graph) node.erase(mostConflictingPerson);
	}
	return satisfied;
}

template <typename T>
std::unordered_set<T> addLeastConflicting(const std::vector<std::unordered_set<T>>& graph) {
	std::unordered_set<T> satisfied;
	std::unordered_set<T> potential;
	for (T i = 0; i < graph.size(); ++i) potential.insert(i);
	while (potential.size() > 0) {
		int leastConflicts = graph.size() + 1;
		int leastConflictingPerson = -1;
		for (T person : potential) {
			int numConflicts = graph[person].size();
			if (numConflicts < leastConflicts) {
				leastConflicts = numConflicts;
				leastConflictingPerson = person;
			}
		}
		satisfied.insert(leastConflictingPerson);
		for (auto person : graph[leastConflictingPerson]) potential.erase(person);
		potential.erase(leastConflictingPerson);
	}
	return satisfied;
}

template <class DislikesType>
std::unordered_set<int> leastDislikes(const std::vector<std::unordered_set<int>>& graph, const std::vector<DislikesType>& clientDislikes) {
	std::unordered_set<int> satisfied;
	std::unordered_set<int> potential;
	for (int i = 0; i < clientDislikes.size(); ++i) potential.insert(i);
	while (potential.size() > 0) {
		int leastDislikes = -1;
		int leastFussyPerson = -1;
		for (int person : potential) {
			if (leastDislikes == -1 || clientDislikes[person].size() < leastDislikes) {
				leastDislikes = clientDislikes[person].size();
				leastFussyPerson = person;
			}
		}
		satisfied.insert(leastFussyPerson);
		potential.erase(leastFussyPerson);
		for (int person : graph[leastFussyPerson]) potential.erase(person);
	}
	return satisfied;
}

template <class LikesType, class DislikesType>
std::unordered_set<int> fewestPreferences(
	const std::vector<std::unordered_set<int>>& graph, 
	const std::vector<LikesType>& clientLikes,
	const std::vector<DislikesType>& clientDislikes
) {
	std::unordered_set<int> satisfied;
	std::unordered_set<int> potential;
	for (int i = 0; i < clientDislikes.size(); ++i) potential.insert(i);
	while (potential.size() > 0) {
		int fewestPreferences = -1;
		int leastFussyPerson = -1;
		for (int person : potential) {
			if (fewestPreferences== -1 || clientDislikes[person].size() + clientLikes[person].size() < fewestPreferences) {
				fewestPreferences = clientDislikes[person].size() + clientLikes[person].size();
				leastFussyPerson = person;
			}
		}
		satisfied.insert(leastFussyPerson);
		potential.erase(leastFussyPerson);
		for (int person : graph[leastFussyPerson]) potential.erase(person);
	}
	return satisfied;
}