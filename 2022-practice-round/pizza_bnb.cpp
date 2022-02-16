#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <vector>

using namespace std;

unordered_set<int> branch_and_bound(const vector<unordered_set<int>>& graph, unordered_set<int> included, unordered_set<int> conflicts, int person, int best_so_far) {
	if (person == graph.size()) return included.size() > best_so_far ? included : unordered_set<int>();
	conflicts.erase(person - 1);
	int leftBound = included.size() + graph.size() - person - 1 - conflicts.size() + conflicts.count(person);
	unordered_set<int> leftSolution;
	if (leftBound > best_so_far) leftSolution = branch_and_bound(graph, included, conflicts, person + 1, best_so_far);
	if (conflicts.count(person) != 0) return leftSolution.size() > best_so_far ? leftSolution : unordered_set<int>();
	included.insert(person);
	for (int neighbour : graph[person]) {
		if (neighbour > person) conflicts.insert(neighbour);
	} 
	int rightBound = included.size() + graph.size() - person - 1 - conflicts.size();
	unordered_set<int> rightSolution;
	if (rightBound > max(best_so_far, (int)leftSolution.size())) rightSolution = branch_and_bound(graph, included, conflicts, person + 1, max(best_so_far, (int)leftSolution.size()));
	if (rightSolution.size() > max(best_so_far, (int)leftSolution.size())) return rightSolution;
	if (leftSolution.size() > max(best_so_far, (int)rightSolution.size())) return leftSolution;
	return unordered_set<int>();
}

int main() {

	int C; cin >> C;

	vector<unordered_set<string>> clientLikes;
	vector<unordered_set<string>> clientDislikes;
	clientLikes.reserve(C);
	clientDislikes.reserve(C);
	for (int i = 0; i < C; ++i) {
		clientLikes.push_back(unordered_set<string>();
		clientDislikes[i] = unordered_set<string>();
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
	for (int i = 0; i < C; ++i) conflictGraph[i] = unordered_set<int>();

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
	
	unordered_set<int> optimal = branch_and_bound(conflictGraph, unordered_set<int>(), unordered_set<int>(), 0, 0);
	cerr << "Branch and Bound: " << optimal.size() << endl;
	unordered_set<string> ingredients;
	for (auto person : optimal) {
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