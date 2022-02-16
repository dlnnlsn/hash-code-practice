#include <algorithm>
#include <iostream>
#include <set>
#include <unordered_set>
#include <vector>

using namespace std;

vector<int> branch_and_bound(const vector<set<int>>& graph, vector<int>& included, int person, int best_so_far) {
	if (person == graph.size()) {
		if (included.size() > best_so_far) return vector<int>(included);
		return vector<int>();
	}
	unordered_set<int> conflicts;
	for (int client : included) {
		for (auto it = graph[client].lower_bound(person); it != graph[client].end(); ++it) conflicts.insert(*it);
	}
	const int left_bound = included.size() + graph.size() - person - 1 - conflicts.size() + conflicts.count(person);
	vector<int> left_solution;
	if (left_bound > best_so_far) {
		left_solution = branch_and_bound(graph, included, person + 1, best_so_far);
	}
	if (conflicts.count(person) != 0) {
		if (left_solution.size() > best_so_far) return left_solution;
		return vector<int>();
	}
	included.push_back(person);
	for (auto it = graph[person].upper_bound(person); it != graph[person].end(); ++it) conflicts.insert(*it);
	const int right_bound = included.size() + graph.size() - person - 1 - conflicts.size();
	const int best_including_left = max(best_so_far, (int)left_solution.size());
	vector<int> right_solution;
	if (right_bound > best_including_left) {
		right_solution = branch_and_bound(graph, included, person + 1, best_including_left);
	}	
	included.pop_back();
	if (right_solution.size() > left_solution.size()) {
		return right_solution.size() > best_so_far ? right_solution : vector<int>();
	}
	else {
		return left_solution.size() > best_so_far ? left_solution : vector<int>();
	}
}

int main() {

	int C; cin >> C;

	vector<set<string>> clientLikes;
	vector<set<string>> clientDislikes;
	clientLikes.reserve(C);
	clientDislikes.reserve(C);
	for (int i = 0; i < C; ++i) {
		clientLikes.push_back(set<string>());
		clientDislikes.push_back(set<string>());
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

	vector<set<int>> conflictGraph;
	conflictGraph.reserve(C);
	for (int i = 0; i < C; ++i) conflictGraph.push_back(set<int>());

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

	vector<int> included;
	vector<int> optimal = branch_and_bound(conflictGraph, included, 0, 0);
	std::cerr << "Branch and Bound: " << optimal.size() << endl;
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