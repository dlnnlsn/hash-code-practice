#include <algorithm>
#include <iostream>
#include <set>
#include <signal.h>
#include <stack>
#include <unordered_set>
#include <vector>

using namespace std;

bool running = true;

void sigint_handler(int sig) {
	cerr << "Writing best solution found so far..." << endl;
	running = false;
}

typedef struct StackFrame {
	int person;
	vector<int> included;
	unordered_set<int> conflicts;
	StackFrame(int _person, vector<int> _included, unordered_set<int> _conflicts): person(_person), included(_included), conflicts(_conflicts) {}
} StackFrame;

vector<int> branch_and_bound(const vector<set<int>>& graph) {
	vector<int> best_so_far;

	stack<StackFrame> call_stack;
	call_stack.push(StackFrame(0, vector<int>(), unordered_set<int>()));

	while (running && call_stack.size() > 0) {
		StackFrame frame = call_stack.top();
		call_stack.pop();

		const int person = frame.person;
		const int bound = frame.included.size() + graph.size() - person - frame.conflicts.size() + frame.conflicts.count(person);
		if (bound <= best_so_far.size()) continue;

		if (person == graph.size()) {
			if (frame.included.size() > best_so_far.size()) {
				best_so_far = frame.included;
				cerr << "Best so far: " << best_so_far.size() << endl;
			}
			continue;
		}

		const bool has_current_person = frame.conflicts.count(person) != 0;
		frame.conflicts.erase(person);

		const int left_bound = frame.included.size() + graph.size() - person - 1 - frame.conflicts.size();
		if (left_bound > best_so_far.size()) {
			call_stack.push(StackFrame(person + 1, frame.included, frame.conflicts));
		}

		if (has_current_person) continue;
		frame.included.push_back(person);
		for (auto it = graph[person].upper_bound(person); it != graph[person].end(); ++it) {
			frame.conflicts.insert(*it);
		}

		const int right_bound = frame.included.size() + graph.size() - person - 1 - frame.conflicts.size();
		if (right_bound > best_so_far.size()) {
			call_stack.push(StackFrame(person + 1, frame.included, frame.conflicts));
		}
	}

	return best_so_far;
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

	signal(SIGINT, sigint_handler);

	vector<int> optimal = branch_and_bound(conflictGraph);
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