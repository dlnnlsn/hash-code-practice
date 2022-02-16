#include <iostream>
#include <unordered_set>
#include <vector>

using namespace std;

vector<unordered_set<int>> copyGraph(vector<unordered_set<int> > graph) {
	vector<unordered_set<int> > copy;
	for (auto node : graph) {
		unordered_set<int> neighbours;
		for (auto it = node.begin(); it != node.end(); ++it) {
			neighbours.insert(*it);
		}
		copy.push_back(neighbours);
	}
	return copy;
}

unordered_set<int> removeMostConflicting(vector<unordered_set<int> > conflictGraph) {
	vector<unordered_set<int> > graph = copyGraph(conflictGraph);
	unordered_set<int> satisfied;
	for (int i = 0; i < graph.size(); ++i) satisfied.insert(i);
	while (true) {
		int maxConflicts = 0;
		int mostConflictingPerson = -1;
		for (auto it = satisfied.begin(); it != satisfied.end(); ++it) {
			int numConflicts = graph[*it].size();
			if (numConflicts > maxConflicts) {
				maxConflicts = numConflicts;
				mostConflictingPerson = *it;
			}
		}
		if (maxConflicts == 0) break;
		satisfied.erase(mostConflictingPerson);
		for (auto node : graph) node.erase(mostConflictingPerson);
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
			for (auto it = clientLikes[i].begin(); it != clientLikes[i].end(); ++it) {
				if (clientDislikes[j].count(*it) != 0) neighbours.insert(j);
			}
			for (auto it = clientDislikes[i].begin(); it != clientDislikes[i].end(); ++it) {
				if (clientLikes[j].count(*it) != 0) neighbours.insert(j);
			}
		}
		conflictGraph.push_back(neighbours);
	}

	unordered_set<int> mostConflictingHeuristic = removeMostConflicting(conflictGraph);
	cerr << "Satisfied people: " << mostConflictingHeuristic.size();
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

	return 0;
}