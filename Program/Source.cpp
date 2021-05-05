#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <queue>

using namespace std;

const int routes_size = 190000000;
const int HEAP_size = 50000000;
const int nodes = 87;

struct edge {
	unsigned char end;
	unsigned short weight;
};

ofstream o("output.txt");
int routes_count = 0;
pair<unsigned char, int> routeS[routes_size + 1]; // last, index of previous
vector<unsigned short> distance_from_startS(nodes + 1, USHRT_MAX);

int HEAP_count = 0;
pair<int, int> HEAP_routeS[HEAP_size + 1]; // length, index in "routeS"
void HEAP_push(unsigned short length, unsigned char last, int previous) {
	if (HEAP_count == HEAP_size || routes_count == routes_size) {
		cout << "Megtelt. " << HEAP_count << " " << routes_count << '\n';
		o << endl;
		exit(0);
	}
	length += distance_from_startS[last]; // prioritás legrövidebb körre vonatkozó alsó becslés alapján
	int index = ++HEAP_count; // index a végén
	while (index != 1 && HEAP_routeS[index / 2].first > length) {
		HEAP_routeS[index] = HEAP_routeS[index / 2]; // fentebbi süllyesztése
		index /= 2;
	}
	HEAP_routeS[index] = { length, ++routes_count }; // új elem helyreillesztése
	routeS[routes_count] = { last, previous }; // új elem "routeS"-ba rakása
}
void HEAP_pop(pair<unsigned short, int> &result) { // length, index in "routeS"
	int index = HEAP_routeS[1].second;
	result = { HEAP_routeS[1].first - distance_from_startS[routeS[index].first], index }; // eredmény mentése a prioritási becslés kivonásával
	--HEAP_count;
	if (HEAP_count == 0) {
		return;
	}
	index = 1; // index az elején
	while (index * 2 <= HEAP_count) {
		int shorter_child = index * 2 == HEAP_count ? 0 : HEAP_routeS[index * 2 + 1].first < HEAP_routeS[index * 2].first ? 1 : 0; // rövidebb gyerek
		if (HEAP_routeS[index * 2 + shorter_child].first >= HEAP_routeS[HEAP_count + 1].first) { // nem süllyeszhetõ
			break;
		}
		HEAP_routeS[index] = HEAP_routeS[index * 2 + shorter_child]; // rövidebb gyerek emelése
		index = index * 2 + shorter_child;
	}
	HEAP_routeS[index] = HEAP_routeS[HEAP_count + 1]; // utolsó elem helyreillesztése
}

int edges;
vector<vector<edge>> edgeS(nodes + 1);
int HEAP_max = 0;

void generate_edges() {
	// --- Beolvasott éllistából szomszédsági mátrix --- //
	vector<vector<unsigned short>> edge_betweenS(nodes + 1, vector<unsigned short>(nodes + 1));
	for (int i = 0; i < edges; i++) {
		int s, e, w;
		cin >> s >> e >> w;
		edge_betweenS[s][e] = edge_betweenS[e][s] = (unsigned short)w;
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

void dijkstra(int n) {
	distance_from_startS[n] = 0;
	priority_queue<pair<unsigned short, unsigned char>, vector<pair<unsigned short, unsigned char>>, greater<pair<unsigned short, unsigned char>>> currentS;
	currentS.push({ 0, n });
	while (!currentS.empty()) {
		unsigned short length = currentS.top().first;
		unsigned char node = currentS.top().second;
		currentS.pop();
		if (length != distance_from_startS[node]) { // már van rövidebb út a csúcsba
			continue;
		}
		for (auto e : edgeS[node]) {
			if (distance_from_startS[node] + e.weight < distance_from_startS[e.end]) {
				distance_from_startS[e.end] = distance_from_startS[node] + e.weight;
				currentS.push({ distance_from_startS[e.end], e.end });
			}
		}
	}
}

void write_route(int index) {
	if (index != 0) {
		write_route(routeS[index].second);
	}
	o << (int)routeS[index].first << " ";
}

void find_routes() {
	int answers = 0;
	routeS[0] = { 1, 0 };
	pair<unsigned short, int> c;
	HEAP_push(edgeS[1][0].weight, 2, 0);
	while (HEAP_count > 0) {
		// --- Útvonal kivétele a prioritási sorból --- //
		HEAP_pop(c);
		// --- Megtalált útvonal fájlba írása --- //
		if (routeS[c.second].first == 1) {
			HEAP_max = max(HEAP_max, HEAP_count);
			cout << ++answers << " " << c.first << " " << HEAP_count << " " << routes_count << '\n';
			o << c.first << '\n';
			write_route(c.second);
			o << '\n';
			continue;
		}
		// --- Kivett útvonal érintett éleinek mátrixba írása --- //
		bool visitedS[nodes + 1][nodes + 1] = {};
		unsigned char last = routeS[c.second].first;
		int previous = routeS[c.second].second;
		while (last != 1) {
			visitedS[last][routeS[previous].first] = visitedS[routeS[previous].first][last] = true;
			last = routeS[previous].first;
			previous = routeS[previous].second;
		}
		// --- Kivett útvonal utolsó csúcsából továbbinduló utak prioritási sorba rakása --- //
		last = routeS[c.second].first;
		for (auto &e : edgeS[last]) {
			if (!visitedS[last][e.end]) {
				// --- 1-es csúcsba vezetõ szakasz prioritási sorba rakása --- //
				if (e.end == 1) {
					HEAP_push(c.first + e.weight, e.end, c.second);
					continue;
				}
				// --- Szélességi bejárás annak vizsgálatára, hogy a továbbinduló út körbeérhet-e --- //
				visitedS[last][e.end] = visitedS[e.end][last] = true;
				unsigned char bfs_visitedS[nodes + 1] = {};
				queue<unsigned char> from_firstS;
				from_firstS.push(1);
				bfs_visitedS[1] = 1;
				queue<unsigned char> from_lastS;
				from_lastS.push(e.end);
				bfs_visitedS[e.end] = 2;
				bool circle = false;
				while (!from_firstS.empty() && !from_lastS.empty()) {
					unsigned char bfs_c = from_firstS.front();
					for (auto f : edgeS[bfs_c]) {
						if (bfs_visitedS[f.end] == 2) {
							circle = true;
							break;
						}
						if (!visitedS[bfs_c][f.end] && bfs_visitedS[f.end] == 0) {
							from_firstS.push(f.end);
							bfs_visitedS[f.end] = 1;
						}
					}
					if (circle) {
						break;
					}
					from_firstS.pop();
					bfs_c = from_lastS.front();
					for (auto f : edgeS[bfs_c]) {
						if (bfs_visitedS[f.end] == 1) {
							circle = true;
							break;
						}
						if (!visitedS[bfs_c][f.end] && bfs_visitedS[f.end] == 0) {
							from_lastS.push(f.end);
							bfs_visitedS[f.end] = 2;
						}
					}
					if (circle) {
						break;
					}
					from_lastS.pop();
				}
				// --- Ha az út körbeérhet, akkor prioritási sorba rakás --- //
				if (circle) {
					HEAP_push(c.first + e.weight, e.end, c.second);
				}
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
	dijkstra(1);
	find_routes();
	o.close();
	cout << HEAP_max << '\n';
}