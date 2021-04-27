#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>

using namespace std;

struct edge {
	unsigned char end;
	int weight;
};

struct route {
	int length;
	vector<unsigned char> nodeS;
};

pair<vector<unsigned char>, int> routeS[20000000]; // route, index in "HEAP_routeS"
int HEAP_count = 0;
vector<pair<int, int>> HEAP_routeS(1); // length, index in "routeS"
void HEAP_push(int length) { // length, route
	HEAP_routeS.push_back({ 0, 0 }); // végére új elem
	int index = ++HEAP_count; // index a végén
	while (index != 1 && HEAP_routeS[index / 2].first > length) {
		HEAP_routeS[index] = HEAP_routeS[index / 2]; // fentebbi süllyesztése
		routeS[HEAP_routeS[index].second].second = index; // fentebbi "HEAP_routeS"-beli indexének átírása "routeS"-ban
		index /= 2;
	}
	HEAP_routeS[index] = { length, HEAP_count - 1 }; // új elem helyreillesztése
	routeS[HEAP_count - 1].second = index; // új elem "HEAP_routeS"-beli indexének átírása "routeS"-ban
}
void HEAP_pop(route &result) { // length, route
	int index = HEAP_routeS[1].second; // "routeS"-beli index
	result = { HEAP_routeS[1].first, routeS[index].first }; // eredmény mentése
	routeS[index] = routeS[--HEAP_count]; // "routeS" végérõl áthelyezés
	HEAP_routeS[routeS[index].second].second = index; // áthelyezett "routeS"-beli indexének átírása "HEAP_routeS"-ban
	index = 1; // index az elején
	while (index * 2 <= HEAP_count) {
		int shorter_child = index * 2 == HEAP_count ? 0 : HEAP_routeS[index * 2 + 1].first < HEAP_routeS[index * 2].first ? 1 : 0; // rövidebb gyerek
		if (HEAP_routeS[index * 2 + shorter_child].first >= HEAP_routeS[HEAP_count + 1].first) { // nem süllyeszhetõ
			break;
		}
		HEAP_routeS[index] = HEAP_routeS[index * 2 + shorter_child]; // rövidebb gyerek emelése
		routeS[HEAP_routeS[index].second].second = index; // rövidebb gyerek "HEAP_routeS"-beli indexének átírása "routeS"-ban
		index = index * 2 + shorter_child;
	}
	HEAP_routeS[index] = HEAP_routeS[HEAP_count + 1]; // utolsó elem helyreillesztése
	HEAP_routeS.pop_back();
	if (HEAP_count > 0) {
		routeS[HEAP_routeS[index].second].second = index; // utolsó elem "HEAP_routeS"-beli indexének átírása "routeS"-ben
	}
}

int main() {
	cin.sync_with_stdio(false);
	cin.tie(nullptr);

	int nodes, edges;
	cin >> nodes >> edges;
	if (nodes > 255 || edges > 255) {
		cout << "Sok.\n";
		return 0;
	}
	vector<vector<edge>> edgeS(nodes + 1);
	for (int i = 0; i < edges; i++) {
		int s, e, w;
		cin >> s >> e >> w;
		edgeS[s].push_back({ (unsigned char)e, w });
		edgeS[e].push_back({ (unsigned char)s, w });
	}
	for (auto &e : edgeS) {
		sort(e.begin(), e.end(), [](edge a, edge b) {
			return a.weight < b.weight;
		});
	}

	ofstream f("output.txt");
	int answers = 0;
	route c = { edgeS[1][0].weight, vector<unsigned char>() };
	c.nodeS.push_back(1);
	c.nodeS.push_back(2);
	routeS[HEAP_count] = { c.nodeS, 0 };
	HEAP_push(c.length);
	while (HEAP_count > 0) {
		HEAP_pop(c);
		if (c.nodeS[c.nodeS.size() - 1] == 1) {
			cout << ++answers << " " << c.length << " " << HEAP_count << '\n';
			f << c.length << '\n';
			for (auto e : c.nodeS) {
				f << (int)e << " ";
			}
			f << '\n';
			continue;
		}
		unsigned char last = c.nodeS[c.nodeS.size() - 1];
		vector<vector<bool>> visitedS(nodes + 1, vector<bool>(nodes + 1));
		for (int i = 1; i < c.nodeS.size(); i++) {
			visitedS[c.nodeS[i]][c.nodeS[i - 1]] = true;
			visitedS[c.nodeS[i - 1]][c.nodeS[i]] = true;
		}
		for (auto &e : edgeS[last]) {
			if (!visitedS[last][e.end]) {
				c.nodeS.push_back(e.end);
				routeS[HEAP_count] = { c.nodeS, 0 };
				HEAP_push(c.length + e.weight);
				c.nodeS.pop_back();
			}
		}
	}
	f.close();
}