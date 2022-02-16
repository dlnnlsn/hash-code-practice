#include <iostream>
#include <unordered_map>
#include <vector>

using namespace std;

int main() {

	int C; cin >> C;
	unordered_map<string, int> ingredients;
	vector<string> ingredient_names;
	int numIngredients = 0;

	vector<vector<int>> clientLikes;
	vector<vector<int>> clientDislikes;

	for (int client = 0; client < C; ++client) {
		int L; cin >> L;
		vector<int> allLikes;
		for (int like = 0; like < L; ++like) {
			string ingredient; cin >> ingredient;
			if (ingredients.count(ingredient) == 0) {
				ingredients[ingredient] = numIngredients++;
				ingredient_names.push_back(ingredient);
			}
			allLikes.push_back(ingredients[ingredient]);
		}
		clientLikes.push_back(allLikes);
		int D; cin >> D;
		vector<int> allDislikes;
		for (int dislike = 0; dislike < D; ++dislike) {
			string ingredient; cin >> ingredient;
			if (ingredients.count(ingredient) == 0) {
				ingredients[ingredient] = numIngredients++;
				ingredient_names.push_back(ingredient);
			}
			allDislikes.push_back(ingredients[ingredient]);
		}
		clientDislikes.push_back(allDislikes);
	}

	return 0;
}