#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>

using namespace std;

const int array_size = 10000000;
const int nodes = 87;

struct edge {
	unsigned char end;
	int weight;
};

struct route {
	int length;
	vector<unsigned char> nodeS;
};

pair<vector<unsigned char>, int> routeS[array_size]; // route, index in "HEAP_routeS"
int HEAP_count = 0;
pair<int, int> HEAP_routeS[array_size + 1]; // length, index in "routeS"
void HEAP_load() {
	for (int i = 0; i < array_size; i++) {
		HEAP_routeS[i + 1].second = i;
	}
}
void HEAP_push(route &r) {
	if (HEAP_count == array_size) {
		cout << "Megtelt.\n";
		exit(0);
	}
	int index = ++HEAP_count; // index a v�g�n
	int place = HEAP_routeS[index].second; // "routeS"-beli szabad hely ment�se
	while (index != 1 && HEAP_routeS[index / 2].first > r.length) {
		HEAP_routeS[index] = HEAP_routeS[index / 2]; // fentebbi s�llyeszt�se
		routeS[HEAP_routeS[index].second].second = index; // fentebbi "HEAP_routeS"-beli index�nek �t�r�sa "routeS"-ban
		index /= 2;
	}
	HEAP_routeS[index] = { r.length, place }; // �j elem helyreilleszt�se
	routeS[place] = { r.nodeS, index }; // �j elem "routeS"-ba rak�sa
}
void HEAP_pop(route &result) {
	int place = HEAP_routeS[1].second; // "routeS"-beli hely
	result = { HEAP_routeS[1].first, routeS[place].first }; // eredm�ny ment�se
	--HEAP_count;
	if (HEAP_count == 0) {
		return;
	}
	int index = 1; // index az elej�n
	while (index * 2 <= HEAP_count) {
		int shorter_child = index * 2 == HEAP_count ? 0 : HEAP_routeS[index * 2 + 1].first < HEAP_routeS[index * 2].first ? 1 : 0; // r�videbb gyerek
		if (HEAP_routeS[index * 2 + shorter_child].first >= HEAP_routeS[HEAP_count + 1].first) { // nem s�llyeszhet�
			break;
		}
		HEAP_routeS[index] = HEAP_routeS[index * 2 + shorter_child]; // r�videbb gyerek emel�se
		routeS[HEAP_routeS[index].second].second = index; // r�videbb gyerek "HEAP_routeS"-beli index�nek �t�r�sa "routeS"-ban
		index = index * 2 + shorter_child;
	}
	HEAP_routeS[index] = HEAP_routeS[HEAP_count + 1]; // utols� elem helyreilleszt�se
	routeS[HEAP_routeS[index].second].second = index; // utols� elem "HEAP_routeS"-beli index�nek �t�r�sa "routeS"-ban
	HEAP_routeS[HEAP_count + 1].second = place; // "routeS"-ban felszabadult hely kupacelemhez rendel�se
}

int main() {
	cin.sync_with_stdio(false);
	cin.tie(nullptr);

	int edges;
	cin >> edges;
	if (edges > 255) {
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
	HEAP_load();
	route c = { edgeS[1][0].weight, { 1, 2 } };
	HEAP_push(c);
	while (HEAP_count > 0) {
		HEAP_pop(c);
		if (c.nodeS.back() == 1) {
			cout << ++answers << " " << c.length << " " << HEAP_count << '\n';
			f << c.length << '\n';
			for (auto e : c.nodeS) {
				f << (int)e << " ";
			}
			f << '\n';
			continue;
		}
		unsigned char last = c.nodeS.back();
		bool visitedS[nodes + 1][nodes + 1] = {};
		for (int i = 1; i < c.nodeS.size(); i++) {
			visitedS[c.nodeS[i]][c.nodeS[i - 1]] = true;
			visitedS[c.nodeS[i - 1]][c.nodeS[i]] = true;
		}
		for (auto &e : edgeS[last]) {
			if (!visitedS[last][e.end]) {
				c.nodeS.push_back(e.end);
				c.length += e.weight;
				HEAP_push(c);
				c.length -= e.weight;
				c.nodeS.pop_back();
			}
		}
	}
	f.close();
}