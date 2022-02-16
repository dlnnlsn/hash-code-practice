#include <iostream>
#include <unordered_set>
#include <vector>

using namespace std;

unordered_set<int> removeMostConflicting(vector<unordered_set<int> > graph) {
	unordered_set<int> satisfied;
	for (int i = 0; i < graph.size(); ++i) satisfied.insert(i);
	while (true) {
		int maxConflicts = 0;
		int mostConflictingPerson = -1;
		for (auto person : satisfied) {
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

unordered_set<int> addLeastConflicting(vector<unordered_set<int> > graph) {
	unordered_set<int> satisfied;
	unordered_set<int> potential;
	for (int i = 0; i < graph.size(); ++i) potential.insert(i);
	while (potential.size() > 0) {
		int leastConflicts = graph.size() + 1;
		int leastConflictingPerson = -1;
		for (auto person : potential) {
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

int main() {

	int C; cin >> C;

	vector<unordered_set<string>> clientLikes;
	vector<unordered_set<string>> clientDislikes;

	for (int client = 0; client < C; ++client) {
		int L; cin >> L;
		unordered_set<string> likes;
		for (int i = 0; i < L; ++i) {
			string ingredient; cin >> ingredient;
			likes.insert(ingredient);
		}
		clientLikes.push_back(likes);
		int D; cin >> D;
		unordered_set<string> dislikes;
		for (int i = 0; i < D; ++i) {
			string ingredient; cin >> ingredient;
			dislikes.insert(ingredient);
		}
		clientDislikes.push_back(dislikes);
	}

	vector<unordered_set<int>> conflictGraph;

	for (int i = 0; i < C; ++i) {
		unordered_set<int> neighbours;
		for (int j = 0; j < C; ++j) {
			for (auto ingredient : clientLikes[i]) {
				if (clientDislikes[j].count(ingredient) != 0) neighbours.insert(j);
			}
			for (auto ingredient : clientDislikes[i]) {
				if (clientLikes[j].count(ingredient) != 0) neighbours.insert(j);
			}
		}
		conflictGraph.push_back(neighbours);
	}

	unordered_set<int> mostConflictingHeuristic = removeMostConflicting(conflictGraph);
	cerr << "Most Conflicting Heuristic: " << mostConflictingHeuristic.size() << endl;
	unordered_set<string> ingredients;
	for (auto person : mostConflictingHeuristic) {
		for (auto ingredient : clientLikes[person]) {
			ingredients.insert(ingredient);
		}
	}
	cout << ingredients.size();
	for (auto ingredient : ingredients) {
		cout << " " << ingredient;
	}
	cout << endl;

	unordered_set<int> leastConflictingHeuristic = addLeastConflicting(conflictGraph);
	cerr << "Least Conflciting Heuristic: " << leastConflictingHeuristic.size() << endl;
	ingredients.clear();
	for (auto person : leastConflictingHeuristic) {
		for (auto ingredient : clientLikes[person]) {
			ingredients.insert(ingredient);
		}
	}
	cout << ingredients.size();
	for (auto ingredient : ingredients) {
		cout << " " << ingredient;
	}
	cout << endl;

	return 0;
}