#include <algorithm>
#include <bitset>
#include "heuristics.h"
#include <iostream>
#include <random>
#include <signal.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;

constexpr int pool_size = 1000;
constexpr double bit_flip_probability = 0.5;
constexpr double client_satisfaction_probability = 0.5;

bool running = true;
bool evolution_started = false;

void sigint_handler(int sig) {
	running = false;
	if (!evolution_started) exit(0);
}

struct seed {
	typedef unsigned int result_type;
	random_device dev;
	template <class RandomAccessIterator>
	void generate(RandomAccessIterator begin, RandomAccessIterator end) {
		for (RandomAccessIterator item = begin; item != end; ++item) {
			*item = dev();
		}
	}
	seed() : dev{} {}
};

typedef bitset<10000> bits;

typedef struct Gene {
	bits ingredients;
	size_t fitness;
	Gene(bits ingredients, size_t fitness) : ingredients(ingredients), fitness(fitness) {}
	friend constexpr bool operator<(const Gene& a, const Gene& b) {
		if (a.fitness != b.fitness) return a.fitness < b.fitness;
		for (size_t i = 0; i < 10000; ++i) {
			if (a.ingredients[i] != b.ingredients[i]) return a.ingredients[i] < b.ingredients[i];
		}
		return false;
	}
} Gene;

const size_t evaluate_fitness(const bits& ingredients, const vector<bits>& client_likes, const vector<bits>& client_dislikes) {
	size_t satisfied = 0;
	for (size_t client = 0; client < client_likes.size(); ++client) {
		if ((ingredients & client_likes[client]) == client_likes[client]) {
			if ((ingredients & client_dislikes[client]).none()) {
				++satisfied;
			}
		}
	}
	return satisfied;
}

template <class Generator>
const bits random_bitset(Generator& generator, size_t num_ingredients) {
	bits result;
	bernoulli_distribution dist(0.5);

	for (size_t i = 0; i < num_ingredients; ++i) {
		result[i] = dist(generator);
	}

	return result;
}

const bits ingredients_from_client_set(const unordered_set<size_t>& clients, const vector<bits>& client_likes) {
	bits ingredients;
	for (size_t client: clients) {
		ingredients |= client_likes[client];
	}
	return ingredients;
}

template <class Generator>
const bits flip_random_bits(Generator& generator, const bits current_bits, const size_t num_ingredients) {
	bits new_bits(current_bits);
	uniform_int_distribution<size_t> dist(0, num_ingredients - 1);
	uniform_real_distribution<double> real_dist(0, 1);
	do {
		new_bits.flip(dist(generator));
	} while (real_dist(generator) < bit_flip_probability);
	return new_bits;
}

template <class Generator>
const bits satisfy_random_clients(Generator& generator, const bits current_bits, const vector<bits>& client_likes, const vector<bits>& client_dislikes) {
	uniform_int_distribution<size_t> dist(0, client_likes.size() - 1);
	uniform_real_distribution<double> real_dist(0, 1);
	bits result = bits(current_bits);
	do {
		const size_t client_index = dist(generator);
		result |= client_likes[client_index];
		result &= (~client_dislikes[client_index]);
	} while (real_dist(generator) < client_satisfaction_probability);
	return result;
}

int main() {

	struct seed seeder;
	mt19937_64 generator(seeder);

	signal(SIGINT, sigint_handler);

	size_t num_clients; cin >> num_clients;

	unordered_map<string, size_t> ingredient_ids;
	vector<string> ingredient_names;
	size_t num_ingredients = 0;

	vector<bits> client_likes;
	client_likes.reserve(num_clients);
	vector<bits> client_dislikes;
	client_dislikes.reserve(num_clients);

	for (int client = 0; client < num_clients; ++client) {
		int num_likes; cin >> num_likes;
		bits current_likes;
		for (int i = 0; i < num_likes; ++i) {
			string name; cin >> name;
			if (ingredient_ids.count(name) == 0) {
				ingredient_ids[name] = num_ingredients++;
				ingredient_names.push_back(name);
			}
			current_likes[ingredient_ids[name]] = true;
		}
		client_likes.push_back(current_likes);

		int num_dislikes; cin >> num_dislikes;
		bits current_dislikes;
		for (int i = 0; i < num_dislikes; ++i) {
			string name; cin >> name;
			if (ingredient_ids.count(name) == 0) {
				ingredient_ids[name] = num_ingredients++;
				ingredient_names.push_back(name);
			}
			current_dislikes[ingredient_ids[name]] = true;
		}
		client_dislikes.push_back(current_dislikes);
	}

	vector<unordered_set<size_t>> conflict_graph;
	conflict_graph.reserve(num_clients);
	for (size_t i = 0; i < num_clients; ++i) conflict_graph.push_back(unordered_set<size_t>());

	for (size_t i = 0; i < num_clients; ++i) {
		for (size_t j = 0; j < num_clients; ++j) {
			if (i == j) continue;
			if (!(client_likes[i] & client_dislikes[j]).none()) {
				conflict_graph[i].insert(j);
				conflict_graph[j].insert(i);
			}
		}
	}

	cerr << "Creating initial gene pool..." << endl;

	vector<Gene> pool; pool.reserve(pool_size);

	unordered_set<size_t> most_conflicting = removeMostConflicting(conflict_graph);
	bits most_conflicting_ingredients = ingredients_from_client_set(most_conflicting, client_likes);
	cerr << "Most conflicting heuristic: ";
	cerr << evaluate_fitness(most_conflicting_ingredients, client_likes, client_dislikes) << endl;

	unordered_set<size_t> least_conflicting = addLeastConflicting(conflict_graph);
	bits least_conflicting_ingredients = ingredients_from_client_set(least_conflicting, client_likes);
	cerr << "Least conflicting heuristic: ";
	cerr << evaluate_fitness(least_conflicting_ingredients, client_likes, client_dislikes) << endl;

	for (size_t i = 2; i < pool_size; ++i) {
		const bits ingredients = random_bitset(generator, num_ingredients);
		const size_t fitness = evaluate_fitness(ingredients, client_likes, client_dislikes);
		pool.push_back(Gene(ingredients, fitness));
	}

	sort(pool.begin(), pool.end());

	evolution_started = true;
	size_t generation = 0;

	while (running) {
		++generation;
		cerr << "Generation: " << generation << ". Best fitness: " << pool.back().fitness << endl;
		vector<Gene> new_pool; new_pool.reserve(3 * pool_size);
		for (Gene gene : pool) {
			new_pool.push_back(gene);
			const bits random_ingredients_flipped = flip_random_bits(generator, gene.ingredients, num_ingredients);
			new_pool.push_back(Gene(random_ingredients_flipped, evaluate_fitness(random_ingredients_flipped, client_likes, client_dislikes)));
			const bits random_clients_satisfied = satisfy_random_clients(generator, gene.ingredients, client_likes, client_dislikes);
			new_pool.push_back(Gene(random_clients_satisfied, evaluate_fitness(random_clients_satisfied, client_likes, client_dislikes)));
		}
		sort(new_pool.begin(), new_pool.end());
		vector<Gene> filtered_pool; filtered_pool.reserve(3 * pool_size);
		filtered_pool.push_back(new_pool.front());
		for (Gene gene : new_pool) {
			if (gene.ingredients != filtered_pool.back().ingredients) {
				filtered_pool.push_back(gene);
			}
		}
		while (filtered_pool.size() < pool_size) {
			const bits ingredients = random_bitset(generator, num_ingredients);
			filtered_pool.push_back(Gene(ingredients, evaluate_fitness(ingredients, client_likes, client_dislikes)));
		}
		sort(filtered_pool.begin(), filtered_pool.end());
		pool = vector<Gene>(filtered_pool.end() - pool_size, filtered_pool.end());
	}

	cerr << "Writing best solution found..." << endl;
	bits ingredients = pool.back().ingredients;
	cout << ingredients.count();
	for (size_t i = 0; i < num_ingredients; ++i) {
		if (ingredients[i]) cout << " " << ingredient_names[i];
	}
	cout << endl;

	return 0;
}