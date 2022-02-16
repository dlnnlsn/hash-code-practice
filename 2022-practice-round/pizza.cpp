#include <iostream>
#include <random>
#include <unordered_set>
#include <vector>

using namespace std;

unordered_set<int> randomResolution(const vector<unordered_set<int>>& graph) {
	random_device dev;
	mt19937 gen(dev());
	uniform_int_distribution<int> random_bool(0, 1);

	unordered_set<int> satisfied;
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

unordered_set<int> addLeastConflicting(const vector<unordered_set<int>>& graph) {
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

void printIngredients(string label, const unordered_set<int>& clients, const vector<unordered_set<string>>& clientLikes) {
	cerr << label << ": " << clients.size() << endl;
	unordered_set<string> ingredients;
	for (auto person : clients) {
		for (auto ingredient : clientLikes[person]) {
			ingredients.insert(ingredient);
		}
	}
	cout << ingredients.size();
	for (auto ingredient : ingredients) {
		cout << " " << ingredient;
	}
	cout << endl;
}

int main() {

	int C; cin >> C;

	vector<unordered_set<string>> clientLikes;
	vector<unordered_set<string>> clientDislikes;
	clientLikes.reserve(C);
	clientDislikes.reserve(C);
	for (int i = 0; i < C; ++i) {
		clientLikes.push_back(unordered_set<string>());
		clientDislikes.push_back(unordered_set<string>());
	}

	for (int client = 0; client < C; ++client) {
		int L; cin >> L;
		for (int i = 0; i < L; ++i) {
			string ingredient; cin >> ingredient;
			clientLikes[client].insert(ingredient);
		}
		int D; cin >> D;
		for (int i = 0; i < D; ++i) {
			string ingredient; cin >> ingredient;
			clientDislikes[client].insert(ingredient);
		}
	}

	vector<unordered_set<int>> conflictGraph;
	conflictGraph.reserve(C);
	for (int i = 0; i < C; ++i) conflictGraph.push_back(unordered_set<int>());

	for (int i = 0; i < C; ++i) {
		for (int j = 0; j < C; ++j) {
			if (i == j) continue;
			for (auto ingredient : clientLikes[i]) {
				if (clientDislikes[j].count(ingredient) != 0) {
					conflictGraph[i].insert(j);
					conflictGraph[j].insert(i);
					break;
				}
			}
		}
	}

	unordered_set<int> mostConflictingHeuristic = removeMostConflicting(conflictGraph);
	printIngredients("Most Conflicting Heuristic", mostConflictingHeuristic, clientLikes);

	unordered_set<int> leastConflictingHeuristic = addLeastConflicting(conflictGraph);
	printIngredients("Least Conflicting Heuristic", leastConflictingHeuristic, clientLikes);

	return 0;
}