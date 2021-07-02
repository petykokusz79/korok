#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <queue>
#include <string>

using namespace std;

using t_oI = int; // type of input and output elements
using t_nC = unsigned char; // type of nodes
using t_lS = unsigned short; // type of route lengths
using t_iI = int; // type of heap and route indexes
const t_lS t_l_MAX = USHRT_MAX; // maximum value of lengths
const char t_n_SIZE = sizeof(t_nC); // size of nodes
const char t_i_SIZE = sizeof(t_iI); // size of heap and route indexes

struct edge {
	t_nC end;
	t_lS weight;
};

t_nC nodes;
vector<vector<edge>> edgeS;
vector<t_lS> distance_from_startS;

const t_iI HEAP_size = 240000000, routes_in_one_file = 400000000, outputs_in_one_file = 1000000;
t_iI routes_count = 0, HEAP_count = 0, HEAP_max = 0;
pair<t_lS, t_iI> HEAP_routeS[HEAP_size + 1]; // length, index in "r"
vector<fstream> rS;
ofstream o;

void write(t_nC last, t_iI previous) { // last, index of previous
	if (routes_count % routes_in_one_file == 0) {
		rS.push_back(fstream("./files/r" + to_string(routes_count / routes_in_one_file) + ".txt", ios::binary | ios::in | ios::out | ios::trunc));
	}
	rS[routes_count / routes_in_one_file].seekp(routes_count % routes_in_one_file * (t_n_SIZE + t_i_SIZE));
	rS[routes_count / routes_in_one_file].write((char*)&last, t_n_SIZE);
	rS[routes_count / routes_in_one_file].write((char*)&previous, t_i_SIZE);
}
void read(pair<t_nC, t_iI> &result, t_iI index) { // last, index of previous
	rS[index / routes_in_one_file].seekg(index % routes_in_one_file * (t_n_SIZE + t_i_SIZE));
	rS[index / routes_in_one_file].read((char*)&result.first, t_n_SIZE);
	rS[index / routes_in_one_file].read((char*)&result.second, t_i_SIZE);
}
t_nC read_node(t_iI index) {
	t_nC result;
	rS[index / routes_in_one_file].seekg(index % routes_in_one_file * (t_n_SIZE + t_i_SIZE));
	rS[index / routes_in_one_file].read((char*)&result, t_n_SIZE);
	return result;
}

void HEAP_push(t_lS length, t_nC last, t_iI previous) {
	if (HEAP_count == HEAP_size) {
		cout << "Megtelt. " << HEAP_count << " " << routes_count << '\n';
		o << endl;
		exit(0);
	}
	length += distance_from_startS[last]; // prioritás legrövidebb körre vonatkozó alsó becslés alapján
	t_iI index = ++HEAP_count; // index a végén
	while (index != 1 && HEAP_routeS[index / 2].first > length) {
		HEAP_routeS[index] = HEAP_routeS[index / 2]; // fentebbi süllyesztése
		index /= 2;
	}
	HEAP_routeS[index] = { length, ++routes_count }; // új elem helyreillesztése
	write(last, previous); // új elem fájlba írása
}
void HEAP_pop(pair<t_lS, t_iI> &result) { // length, index in "r"
	t_iI place = HEAP_routeS[1].second;
	result = { HEAP_routeS[1].first - distance_from_startS[read_node(place)], place }; // eredmény mentése a prioritási becslés kivonásával
	--HEAP_count;
	if (HEAP_count == 0) {
		return;
	}
	t_iI index = 1; // index az elején
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

void generate_edges(t_oI edges) {
	// --- Beolvasott éllistából szomszédsági mátrix --- //
	vector<vector<t_lS>> edge_betweenS(nodes, vector<t_lS>(nodes));
	for (int i = 0; i < edges; i++) {
		t_oI s, e, w;
		cin >> s >> e >> w;
		--s;
		--e;
		edge_betweenS[s][e] = edge_betweenS[e][s] = w;
	}
	// --- Zsákélek törlése --- //
	for (t_nC i = 0; i < nodes; i++) {
		for (t_nC j = i + 1; j < nodes; j++) {
			if (edge_betweenS[i][j] != 0) {
				// --- Kiválasztott él törlése --- //
				t_lS length = edge_betweenS[i][j];
				edge_betweenS[i][j] = edge_betweenS[j][i] = 0;
				// --- Az 1-es csúcsot tartalmazó komponens szélességi bejárása --- //
				vector<bool> visitedS(nodes);
				queue<t_nC> currentS;
				currentS.push(0);
				visitedS[0] = true;
				while (!currentS.empty()) {
					for (t_nC k = 0; k < nodes; k++) {
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
				for (t_nC k = 0; k < nodes; k++) {
					if (!visitedS[k]) {
						for (t_nC l = 0; l < nodes; l++) {
							edge_betweenS[k][l] = edge_betweenS[l][k] = 0;
						}
					}
				}
			}
		}
	}
	// --- Szomszédsági lista felépítése a szomszédsági mátrix alapján --- //
	for (t_nC i = 0; i < nodes; i++) {
		for (t_nC j = i + 1; j < nodes; j++) {
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

void dijkstra(t_nC n) {
	distance_from_startS[n] = 0;
	priority_queue<pair<t_lS, t_nC>, vector<pair<t_lS, t_nC>>, greater<pair<t_lS, t_nC>>> currentS;
	currentS.push({ 0, n });
	while (!currentS.empty()) {
		t_lS length = currentS.top().first;
		t_nC node = currentS.top().second;
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

void write_route(t_iI index) {
	pair<t_nC, t_iI> route;
	read(route, index);
	if (index != 0) {
		write_route(route.second);
	}
	o << (t_oI)route.first + 1 << " ";
}

void find_routes() {
	int answers = 0;
	write(0, 0);
	pair<t_lS, t_iI> c;
	pair<t_nC, t_iI> route;
	HEAP_push(edgeS[0][0].weight, edgeS[0][0].end, 0);
	while (HEAP_count > 0) {
		// --- Útvonal kivétele a prioritási sorból --- //
		HEAP_pop(c);
		// --- Útvonal végének kiolvasása a fájlból --- //
		read(route, c.second);
		// --- Megtalált útvonal fájlba írása --- //
		if (route.first == 0) {
			if (answers % outputs_in_one_file == 0) {
				o.close();
				o.open("./files/o" + to_string(answers / outputs_in_one_file) + ".txt");
			}
			HEAP_max = max(HEAP_max, HEAP_count);
			cout << ++answers << " " << c.first << " " << HEAP_count << " " << routes_count << '\n';
			o << c.first << " ";
			write_route(c.second);
			o << '\n';
			continue;
		}
		// --- Kivett útvonal érintett éleinek mátrixba írása --- //
		bool *visitedS = new bool[nodes * nodes] {};
		t_nC last = route.first;
		t_iI previous = route.second;
		while (last != 0) {
			read(route, previous);
			visitedS[nodes * last + route.first] = visitedS[nodes * route.first + last] = true;
			last = route.first;
			previous = route.second;
		}
		// --- Kivett útvonal utolsó csúcsából továbbinduló utak prioritási sorba rakása --- //
		last = read_node(c.second);
		for (auto &e : edgeS[last]) {
			if (!visitedS[nodes * last + e.end]) {
				// --- 1-es csúcsba vezetõ szakasz prioritási sorba rakása --- //
				if (e.end == 0) {
					HEAP_push(c.first + e.weight, e.end, c.second);
					continue;
				}
				// --- Szélességi bejárás annak vizsgálatára, hogy a továbbinduló út körbeérhet-e --- //
				visitedS[nodes * last + e.end] = visitedS[nodes * e.end + last] = true;
				t_nC *bfs_visitedS = new t_nC[nodes] {};
				queue<t_nC> from_firstS;
				from_firstS.push(0);
				bfs_visitedS[0] = 1;
				queue<t_nC> from_lastS;
				from_lastS.push(e.end);
				bfs_visitedS[e.end] = 2;
				bool circle = false;
				while (!from_firstS.empty() && !from_lastS.empty()) {
					t_nC bfs_c = from_firstS.front();
					for (auto f : edgeS[bfs_c]) {
						if (bfs_visitedS[f.end] == 2) {
							circle = true;
							break;
						}
						if (!visitedS[nodes * bfs_c + f.end] && bfs_visitedS[f.end] == 0) {
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
						if (!visitedS[nodes * bfs_c + f.end] && bfs_visitedS[f.end] == 0) {
							from_lastS.push(f.end);
							bfs_visitedS[f.end] = 2;
						}
					}
					if (circle) {
						break;
					}
					from_lastS.pop();
				}
				delete[] bfs_visitedS;
				// --- Ha az út körbeérhet, akkor prioritási sorba rakás --- //
				if (circle) {
					HEAP_push(c.first + e.weight, e.end, c.second);
				}
			}
		}
		delete[] visitedS;
	}
}

int main() {
	cin.sync_with_stdio(false);
	cin.tie(nullptr);
	cout.sync_with_stdio(false);
	cout.tie(nullptr);

	// --- Gráf felépítése a beolvasott éllistából --- //
	t_oI N, edges;
	cin >> N >> edges;
	if (N > 255 || edges > 255) {
		cout << "Sok.\n";
		return 0;
	}
	nodes = (t_nC)N;
	edgeS.resize(nodes);
	generate_edges(edges);

	// --- Útvonalak visszaadása --- //
	distance_from_startS.resize(nodes, t_l_MAX);
	dijkstra(0);
	find_routes();
	for (auto &e : rS) {
		e.close();
	}
	o.close();
	cout << HEAP_max << '\n';
}