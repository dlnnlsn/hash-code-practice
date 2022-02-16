#include <algorithm>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <set>
#include <signal.h>
#include <stack>
#include <thread>
#include <unordered_set>
#include <vector>

using namespace std;

void spawn_thread(int, vector<int>, unordered_set<int>);

bool running = true;

void sigint_handler(int sig) {
	cerr << "Writing best solution found so far..." << endl;
	running = false;
}

int running_threads = 0;
constexpr int max_threads = 128;

vector<int> best_so_far;
mutex best_lock;

mutex thread_lock;
condition_variable thread_check;

mutex thread_count_lock;

vector<set<int>> graph;

bool can_spwan_thread() {
	thread_count_lock.lock();
	bool can_spawn = running_threads < max_threads;
	thread_count_lock.unlock();
	return can_spawn;
}

int best_size() {
	best_lock.lock();
	int value = best_so_far.size();
	best_lock.unlock();
	return value;
}

typedef struct StackFrame {
	int person;
	vector<int> included;
	unordered_set<int> conflicts;
	StackFrame(int _person, vector<int> _included, unordered_set<int> _conflicts): person(_person), included(_included), conflicts(_conflicts) {}
} StackFrame;

void branch_and_bound(stack<StackFrame> call_stack) {
	while (running && call_stack.size() > 0) {
		StackFrame frame = call_stack.top();
		call_stack.pop();

		const int person = frame.person;
		const int bound = frame.included.size() + graph.size() - person - frame.conflicts.size() + frame.conflicts.count(person);
		if (bound <= best_size()) continue;

		best_lock.lock();
		if (frame.included.size() > best_so_far.size()) {
			best_so_far = frame.included;
			cerr << "Best so far: " << best_so_far.size() << endl;
		}
		best_lock.unlock();

		if (person == graph.size()) {
			continue;
		}

		const bool has_current_person = frame.conflicts.count(person) != 0;
		frame.conflicts.erase(person);

		const int left_bound = frame.included.size() + graph.size() - person - 1 - frame.conflicts.size();
		if (left_bound > best_size()) {
			if (can_spwan_thread()) {
				spawn_thread(person + 1, frame.included, frame.conflicts);
			}
			else {
				call_stack.push(StackFrame(person + 1, frame.included, frame.conflicts));
			}
		}

		if (has_current_person) continue;
		frame.included.push_back(person);
		for (auto it = graph[person].upper_bound(person); it != graph[person].end(); ++it) {
			frame.conflicts.insert(*it);
		}

		const int right_bound = frame.included.size() + graph.size() - person - 1 - frame.conflicts.size();
		if (right_bound > best_size()) {
			if (can_spwan_thread()) {
				spawn_thread(person + 1, frame.included, frame.conflicts);
			}
			else {
				call_stack.push(StackFrame(person + 1, frame.included, frame.conflicts));
			}
		}
	}
	
	thread_count_lock.lock();
	running_threads--;
	if (running_threads == 0) {
		thread_check.notify_all();
	}
	thread_count_lock.unlock();
}

void spawn_thread(int person, vector<int> included, unordered_set<int> conflicts) {
	thread_count_lock.lock();
	stack<StackFrame> thread_stack;
	thread_stack.push(StackFrame(person, included, conflicts));
	thread new_thread(branch_and_bound, thread_stack);
	running_threads++;
	new_thread.detach();
	thread_count_lock.unlock();
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

	graph.reserve(C);
	for (int i = 0; i < C; ++i) graph.push_back(set<int>());

	for (int i = 0; i < C; ++i) {
		for (int j = 0; j < C; ++j) {
			if (i == j) continue;
			for (auto ingredient : clientLikes[i]) {
				if (clientDislikes[j].count(ingredient) != 0) {
					graph[i].insert(j);
					graph[j].insert(i);
					break;
				}
			}
		}
	}

	signal(SIGINT, sigint_handler);

	spawn_thread(0, vector<int>(), unordered_set<int>());
	unique_lock<mutex> locker(thread_lock);
	while (running) {
		thread_count_lock.lock();
		if (running_threads == 0) {
			thread_count_lock.unlock();
			break;
		}
		thread_count_lock.unlock();
		thread_check.wait(locker);
	}

	std::cerr << "Branch and Bound: " << best_so_far.size() << endl;
	unordered_set<string> ingredients;
	for (auto person : best_so_far) {
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