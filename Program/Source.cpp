#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <queue>

using namespace std;

using Tio = int; // type of input and output elements
using Tn = unsigned char; // type of nodes
using Tl = unsigned short; // type of lengths
using Ti = int; // type of route indexes
using Th = int; // type of heap indexes

const Tl Tl_MAX = USHRT_MAX; // maximum value of lengths
const char Tn_SIZE = sizeof(Tn); // size of nodes
const char Ti_SIZE = sizeof(Ti); // size of route indexes
const char Tf_SIZE = Tn_SIZE + Ti_SIZE; // size of file elements

const Th HEAP_size = 240000000;
const Tn nodes = 87;

struct edge {
	Tn end;
	Tl weight;
};

ofstream o("output.txt");
vector<Tl> distance_from_startS(nodes + 1, Tl_MAX);

fstream r("routes.txt", ios::binary | ios::in | ios::out | ios::trunc);
Ti routes_count = 0;
void write(pair<Tn, Ti> route) { // last, index of previous
	r.seekp(routes_count * Tf_SIZE);
	r.write((char*)&route.first, Tn_SIZE);
	r.write((char*)&route.second, Ti_SIZE);
}
void read(pair<Tn, Ti> &result, Ti index) { // last, index of previous
	r.seekg(index * Tf_SIZE);
	r.read((char*)&result.first, Tn_SIZE);
	r.read((char*)&result.second, Ti_SIZE);
}
Tn read_node(Ti index) {
	Tn result;
	r.seekg(index * Tf_SIZE);
	r.read((char*)&result, Tn_SIZE);
	return result;
}

Th HEAP_count = 0;
pair<Tl, Ti> HEAP_routeS[HEAP_size + 1]; // length, index in "r"
void HEAP_push(Tl length, Tn last, Ti previous) {
	if (HEAP_count == HEAP_size/* || routes_count == routes_size*/) {
		cout << "Megtelt. " << HEAP_count << " " << routes_count << '\n';
		o << endl;
		exit(0);
	}
	length += distance_from_startS[last]; // prioritás legrövidebb körre vonatkozó alsó becslés alapján
	Th index = ++HEAP_count; // index a végén
	while (index != 1 && HEAP_routeS[index / 2].first > length) {
		HEAP_routeS[index] = HEAP_routeS[index / 2]; // fentebbi süllyesztése
		index /= 2;
	}
	HEAP_routeS[index] = { length, ++routes_count }; // új elem helyreillesztése
	write({ last, previous }); // új elem fájlba írása
}
void HEAP_pop(pair<Tl, Ti> &result) { // length, index in "r"
	Ti place = HEAP_routeS[1].second;
	result = { HEAP_routeS[1].first - distance_from_startS[read_node(place)], place }; // eredmény mentése a prioritási becslés kivonásával
	--HEAP_count;
	if (HEAP_count == 0) {
		return;
	}
	Th index = 1; // index az elején
	while (index * 2 <= HEAP_count) {
		bool shorter_child = index * 2 == HEAP_count ? 0 : HEAP_routeS[index * 2 + 1].first < HEAP_routeS[index * 2].first ? 1 : 0; // rövidebb gyerek
		if (HEAP_routeS[index * 2 + shorter_child].first >= HEAP_routeS[HEAP_count + 1].first) { // nem süllyeszhetõ
			break;
		}
		HEAP_routeS[index] = HEAP_routeS[index * 2 + shorter_child]; // rövidebb gyerek emelése
		index = index * 2 + shorter_child;
	}
	HEAP_routeS[index] = HEAP_routeS[HEAP_count + 1]; // utolsó elem helyreillesztése
}

Tio edges;
vector<vector<edge>> edgeS(nodes + 1);
Th HEAP_max = 0;

void generate_edges() {
	// --- Beolvasott éllistából szomszédsági mátrix --- //
	vector<vector<Tl>> edge_betweenS(nodes + 1, vector<Tl>(nodes + 1));
	for (Tio i = 0; i < edges; i++) {
		Tio s, e, w;
		cin >> s >> e >> w;
		edge_betweenS[(Tn)s][(Tn)e] = edge_betweenS[(Tn)e][(Tn)s] = (Tl)w;
	}
	// --- Zsákélek törlése --- //
	for (Tn i = 1; i <= nodes; i++) {
		for (Tn j = i + 1; j <= nodes; j++) {
			if (edge_betweenS[i][j] != 0) {
				// --- Kiválasztott él törlése --- //
				Tl length = edge_betweenS[i][j];
				edge_betweenS[i][j] = edge_betweenS[j][i] = 0;
				// --- Az 1-es csúcsot tartalmazó komponens szélességi bejárása --- //
				vector<bool> visitedS(nodes + 1);
				queue<Tn> currentS;
				currentS.push(1);
				visitedS[1] = true;
				while (!currentS.empty()) {
					for (Tn k = 1; k <= nodes; k++) {
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
				for (Tn k = 1; k <= nodes; k++) {
					if (!visitedS[k]) {
						for (Tn l = 1; l <= nodes; l++) {
							edge_betweenS[k][l] = edge_betweenS[l][k] = 0;
						}
					}
				}
			}
		}
	}
	// --- Szomszédsági lista felépítése a szomszédsági mátrix alapján --- //
	for (Tn i = 1; i <= nodes; i++) {
		for (Tn j = i; j <= nodes; j++) {
			if (edge_betweenS[i][j] != 0) {
				edgeS[i].push_back({ j, edge_betweenS[i][j] });
				edgeS[j].push_back({ i, edge_betweenS[i][j] });
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

void dijkstra(Tn n) {
	distance_from_startS[n] = 0;
	priority_queue<pair<Tl, Tn>, vector<pair<Tl, Tn>>, greater<pair<Tl, Tn>>> currentS;
	currentS.push({ 0, n });
	while (!currentS.empty()) {
		Tl length = currentS.top().first;
		Tn node = currentS.top().second;
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

void write_route(Ti index) {
	pair<Tn, Ti> route;
	read(route, index);
	if (index != 0) {
		write_route(route.second);
	}
	o << (Tio)route.first << " ";
}

void find_routes() {
	int answers = 0;
	write({ 1, 0 });
	pair<Tl, Ti> c;
	pair<Tn, Ti> route;
	HEAP_push(edgeS[1][0].weight, 2, 0);
	while (HEAP_count > 0) {
		// --- Útvonal kivétele a prioritási sorból --- //
		HEAP_pop(c);
		// --- Útvonal végének kiolvasása a fájlból --- //
		read(route, c.second);
		// --- Megtalált útvonal fájlba írása --- //
		if (route.first == 1) {
			HEAP_max = max(HEAP_max, HEAP_count);
			cout << ++answers << " " << c.first << " " << HEAP_count << " " << routes_count << '\n';
			o << c.first << '\n';
			write_route(c.second);
			o << '\n';
			continue;
		}
		// --- Kivett útvonal érintett éleinek mátrixba írása --- //
		bool visitedS[nodes + 1][nodes + 1] = {};
		Tn last = route.first;
		Ti previous = route.second;
		while (last != 1) {
			read(route, previous);
			visitedS[last][route.first] = visitedS[route.first][last] = true;
			last = route.first;
			previous = route.second;
		}
		// --- Kivett útvonal utolsó csúcsából továbbinduló utak prioritási sorba rakása --- //
		last = read_node(c.second);
		for (auto &e : edgeS[last]) {
			if (!visitedS[last][e.end]) {
				// --- 1-es csúcsba vezetõ szakasz prioritási sorba rakása --- //
				if (e.end == 1) {
					HEAP_push(c.first + e.weight, e.end, c.second);
					continue;
				}
				// --- Szélességi bejárás annak vizsgálatára, hogy a továbbinduló út körbeérhet-e --- //
				visitedS[last][e.end] = visitedS[e.end][last] = true;
				Tn bfs_visitedS[nodes + 1] = {};
				queue<Tn> from_firstS;
				from_firstS.push(1);
				bfs_visitedS[1] = 1;
				queue<Tn> from_lastS;
				from_lastS.push(e.end);
				bfs_visitedS[e.end] = 2;
				bool circle = false;
				while (!from_firstS.empty() && !from_lastS.empty()) {
					Tn bfs_c = from_firstS.front();
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
	r.close();
	o.close();
	cout << HEAP_max << '\n';
}