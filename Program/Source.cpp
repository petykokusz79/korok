#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <queue>

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

vector<unsigned char> routeS[array_size];
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
	int index = ++HEAP_count; // index a végén
	int place = HEAP_routeS[index].second; // "routeS"-beli szabad hely mentése
	while (index != 1 && HEAP_routeS[index / 2].first > r.length) {
		HEAP_routeS[index] = HEAP_routeS[index / 2]; // fentebbi süllyesztése
		index /= 2;
	}
	HEAP_routeS[index] = { r.length, place }; // új elem helyreillesztése
	routeS[place] = r.nodeS; // új elem "routeS"-ba rakása
}
void HEAP_pop(route &result) {
	int place = HEAP_routeS[1].second; // "routeS"-beli hely
	result = { HEAP_routeS[1].first, routeS[place] }; // eredmény mentése
	--HEAP_count;
	if (HEAP_count == 0) {
		return;
	}
	int index = 1; // index az elején
	while (index * 2 <= HEAP_count) {
		int shorter_child = index * 2 == HEAP_count ? 0 : HEAP_routeS[index * 2 + 1].first < HEAP_routeS[index * 2].first ? 1 : 0; // rövidebb gyerek
		if (HEAP_routeS[index * 2 + shorter_child].first >= HEAP_routeS[HEAP_count + 1].first) { // nem süllyeszhetõ
			break;
		}
		HEAP_routeS[index] = HEAP_routeS[index * 2 + shorter_child]; // rövidebb gyerek emelése
		index = index * 2 + shorter_child;
	}
	HEAP_routeS[index] = HEAP_routeS[HEAP_count + 1]; // utolsó elem helyreillesztése
	HEAP_routeS[HEAP_count + 1].second = place; // "routeS"-ban felszabadult hely kupacelemhez rendelése
}

int edges;
vector<vector<edge>> edgeS(nodes + 1);

void generate_edges() {
	// --- Beolvasott éllistából szomszédsági mátrix --- //
	vector<vector<int>> edge_betweenS(nodes + 1, vector<int>(nodes + 1));
	for (int i = 0; i < edges; i++) {
		int s, e, w;
		cin >> s >> e >> w;
		edge_betweenS[s][e] = edge_betweenS[e][s] = w;
	}
	// --- Zsákélek törlése --- //
	for (int i = 1; i <= nodes; i++) {
		for (int j = i + 1; j <= nodes; j++) {
			if (edge_betweenS[i][j] != 0) {
				// --- Kiválasztott él törlése --- //
				int length = edge_betweenS[i][j];
				edge_betweenS[i][j] = edge_betweenS[j][i] = 0;
				// --- Az 1-es csúcsot tartalmazó komponens szélességi bejárása --- //
				vector<bool> visitedS(nodes + 1);
				queue<unsigned char> currentS;
				currentS.push(1);
				visitedS[1] = true;
				while (!currentS.empty()) {
					for (int k = 1; k <= nodes; k++) {
						if (edge_betweenS[currentS.front()][k] != 0 && !visitedS[k]) {
							currentS.push(k);
							visitedS[k] = true;
						}
					}
					currentS.pop();
				}
				// --- Kiválasztott él visszaírása --- //
				edge_betweenS[i][j] = edge_betweenS[j][i] = length;
				// --- Az elõzõleg nem bejárt csúcsokat érintõ élek törlése --- //
				for (int k = 1; k <= nodes; k++) {
					if (!visitedS[k]) {
						for (int l = 1; l <= nodes; l++) {
							edge_betweenS[k][l] = edge_betweenS[l][k] = 0;
						}
					}
				}
			}
		}
	}
	// --- Szomszédsági lista felépítése a szomszédsági mátrix alapján --- //
	for (int i = 1; i <= nodes; i++) {
		for (int j = i; j <= nodes; j++) {
			if (edge_betweenS[i][j] != 0) {
				edgeS[i].push_back({ (unsigned char)j, edge_betweenS[i][j] });
				edgeS[j].push_back({ (unsigned char)i, edge_betweenS[i][j] });
			}
		}
	}
	// --- Szomszédsági lista adott csúcsból kiinduló éleinek rendezése növekvõ hossz szerint --- //
	for (auto &e : edgeS) {
		sort(e.begin(), e.end(), [](edge a, edge b) {
			return a.weight < b.weight;
		});
	}
}

void find_routes(ofstream &f) {
	int answers = 0;
	HEAP_load();
	route c = { edgeS[1][0].weight, { 2 } };
	HEAP_push(c);
	while (HEAP_count > 0) {
		// --- Útvonal kivétele a prioritási sorból --- //
		HEAP_pop(c);
		// --- Megtalált útvonal fájlba írása --- //
		if (c.nodeS.back() == 3) {
			c.length += edgeS[1][1].weight;
			cout << ++answers << " " << c.length << " " << HEAP_count << '\n';
			f << c.length << '\n' << 1 << " ";
			for (auto e : c.nodeS) {
				f << (int)e << " ";
			}
			f << 1 << '\n';
			continue;
		}
		// --- Kivett útvonal érintett éleinek mátrixba írása --- //
		bool visitedS[nodes + 1][nodes + 1] = {};
		visitedS[1][2] = visitedS[2][1] = true;
		for (int i = 1; i < c.nodeS.size(); i++) {
			visitedS[c.nodeS[i]][c.nodeS[i - 1]] = visitedS[c.nodeS[i - 1]][c.nodeS[i]] = true;
		}
		// --- Kivett útvonal utolsó csúcsából továbbinduló utak prioritási sorba rakása --- //
		unsigned char last = c.nodeS.back();
		for (auto &e : edgeS[last]) {
			if (!visitedS[last][e.end]) {
				// --- Szélességi bejárás annak vizsgálatára, hogy a továbbinduló út körbeérhet-e --- //
				visitedS[last][e.end] = visitedS[e.end][last] = true;
				unsigned char bfs_visitedS[nodes + 1] = {};
				queue<unsigned char> from_firstS;
				from_firstS.push(1);
				queue<unsigned char> from_lastS;
				from_lastS.push(e.end);
				bool circle = false;
				while (!from_firstS.empty() && !from_lastS.empty()) {
					unsigned char c = from_firstS.front();
					for (auto e : edgeS[c]) {
						if (bfs_visitedS[e.end] == 2) {
							circle = true;
							break;
						}
						if (!visitedS[c][e.end] && bfs_visitedS[e.end] == 0) {
							from_firstS.push(e.end);
							bfs_visitedS[e.end] = 1;
						}
					}
					from_firstS.pop();
					if (circle) {
						break;
					}
					c = from_lastS.front();
					for (auto e : edgeS[c]) {
						if (bfs_visitedS[e.end] == 1) {
							circle = true;
							break;
						}
						if (!visitedS[c][e.end] && bfs_visitedS[e.end] == 0) {
							from_lastS.push(e.end);
							bfs_visitedS[e.end] = 2;
						}
					}
					from_lastS.pop();
					if (circle) {
						break;
					}
				}
				visitedS[last][e.end] = visitedS[e.end][last] = false;
				// --- Ha az út körbeérhet, akkor prioritási sorba rakás --- //
				if (!circle) {
					continue;
				}
				c.nodeS.push_back(e.end);
				c.length += e.weight;
				HEAP_push(c);
				c.length -= e.weight;
				c.nodeS.pop_back();
			}
		}
	}
}

int main() {
	cin.sync_with_stdio(false);
	cin.tie(nullptr);

	// --- Gráf felépítése a beolvasott éllistából --- //
	cin >> edges;
	if (edges > 255) {
		cout << "Sok.\n";
		return 0;
	}
	generate_edges();

	// --- Útvonalak visszaadása --- //
	ofstream f("output.txt");
	find_routes(f);
	f.close();
}