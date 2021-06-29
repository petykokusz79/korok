#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <queue>

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

const t_iI HEAP_size = 240000000;
t_iI routes_count = 0, HEAP_count = 0, HEAP_max = 0;
pair<t_lS, t_iI> HEAP_routeS[HEAP_size + 1]; // length, index in "r"
fstream r("routes.txt", ios::binary | ios::in | ios::out | ios::trunc);
ofstream o("output.txt");

void write(t_nC last, t_iI previous) { // last, index of previous
	r.seekp(routes_count * (t_n_SIZE + t_i_SIZE));
	r.write((char*)&last, t_n_SIZE);
	r.write((char*)&previous, t_i_SIZE);
}
void read(pair<t_nC, t_iI> &result, t_iI index) { // last, index of previous
	r.seekg(index * (t_n_SIZE + t_i_SIZE));
	r.read((char*)&result.first, t_n_SIZE);
	r.read((char*)&result.second, t_i_SIZE);
}
t_nC read_node(t_iI index) {
	t_nC result;
	r.seekg(index * (t_n_SIZE + t_i_SIZE));
	r.read((char*)&result, t_n_SIZE);
	return result;
}

void HEAP_push(t_lS length, t_nC last, t_iI previous) {
	if (HEAP_count == HEAP_size) {
		cout << "Megtelt. " << HEAP_count << " " << routes_count << '\n';
		o << endl;
		exit(0);
	}
	length += distance_from_startS[last]; // priorit�s legr�videbb k�rre vonatkoz� als� becsl�s alapj�n
	t_iI index = ++HEAP_count; // index a v�g�n
	while (index != 1 && HEAP_routeS[index / 2].first > length) {
		HEAP_routeS[index] = HEAP_routeS[index / 2]; // fentebbi s�llyeszt�se
		index /= 2;
	}
	HEAP_routeS[index] = { length, ++routes_count }; // �j elem helyreilleszt�se
	write(last, previous); // �j elem f�jlba �r�sa
}
void HEAP_pop(pair<t_lS, t_iI> &result) { // length, index in "r"
	t_iI place = HEAP_routeS[1].second;
	result = { HEAP_routeS[1].first - distance_from_startS[read_node(place)], place }; // eredm�ny ment�se a priorit�si becsl�s kivon�s�val
	--HEAP_count;
	if (HEAP_count == 0) {
		return;
	}
	t_iI index = 1; // index az elej�n
	while (index * 2 <= HEAP_count) {
		bool shorter_child = index * 2 == HEAP_count ? 0 : HEAP_routeS[index * 2 + 1].first < HEAP_routeS[index * 2].first ? 1 : 0; // r�videbb gyerek
		if (HEAP_routeS[index * 2 + shorter_child].first >= HEAP_routeS[HEAP_count + 1].first) { // nem s�llyeszhet�
			break;
		}
		HEAP_routeS[index] = HEAP_routeS[index * 2 + shorter_child]; // r�videbb gyerek emel�se
		index = index * 2 + shorter_child;
	}
	HEAP_routeS[index] = HEAP_routeS[HEAP_count + 1]; // utols� elem helyreilleszt�se
}

void generate_edges(t_oI edges) {
	// --- Beolvasott �llist�b�l szomsz�ds�gi m�trix --- //
	vector<vector<t_lS>> edge_betweenS(nodes + 1, vector<t_lS>(nodes + 1));
	for (int i = 0; i < edges; i++) {
		t_oI s, e, w;
		cin >> s >> e >> w;
		edge_betweenS[s][e] = edge_betweenS[e][s] = w;
	}
	// --- Zs�k�lek t�rl�se --- //
	for (t_nC i = 1; i <= nodes; i++) {
		for (t_nC j = i + 1; j <= nodes; j++) {
			if (edge_betweenS[i][j] != 0) {
				// --- Kiv�lasztott �l t�rl�se --- //
				t_lS length = edge_betweenS[i][j];
				edge_betweenS[i][j] = edge_betweenS[j][i] = 0;
				// --- Az 1-es cs�csot tartalmaz� komponens sz�less�gi bej�r�sa --- //
				vector<bool> visitedS(nodes + 1);
				queue<t_nC> currentS;
				currentS.push(1);
				visitedS[1] = true;
				while (!currentS.empty()) {
					for (t_nC k = 1; k <= nodes; k++) {
						if (edge_betweenS[currentS.front()][k] != 0 && !visitedS[k]) {
							currentS.push(k);
							visitedS[k] = true;
						}
					}
					currentS.pop();
				}
				// --- Kiv�lasztott �l vissza�r�sa --- //
				edge_betweenS[i][j] = edge_betweenS[j][i] = length;
				// --- Az el�z�leg nem bej�rt cs�csokat �rint� �lek t�rl�se --- //
				for (t_nC k = 1; k <= nodes; k++) {
					if (!visitedS[k]) {
						for (t_nC l = 1; l <= nodes; l++) {
							edge_betweenS[k][l] = edge_betweenS[l][k] = 0;
						}
					}
				}
			}
		}
	}
	// --- Szomsz�ds�gi lista fel�p�t�se a szomsz�ds�gi m�trix alapj�n --- //
	for (t_nC i = 1; i <= nodes; i++) {
		for (t_nC j = i + 1; j <= nodes; j++) {
			if (edge_betweenS[i][j] != 0) {
				edgeS[i].push_back({ j, edge_betweenS[i][j] });
				edgeS[j].push_back({ i, edge_betweenS[i][j] });
			}
		}
	}
	// --- Szomsz�ds�gi lista adott cs�csb�l kiindul� �leinek rendez�se n�vekv� hossz szerint --- //
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
		if (length != distance_from_startS[node]) { // m�r van r�videbb �t a cs�csba
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
	o << (t_oI)route.first << " ";
}

void find_routes() {
	int answers = 0;
	write(1, 0);
	pair<t_lS, t_iI> c;
	pair<t_nC, t_iI> route;
	HEAP_push(edgeS[1][0].weight, 2, 0);
	while (HEAP_count > 0) {
		// --- �tvonal kiv�tele a priorit�si sorb�l --- //
		HEAP_pop(c);
		// --- �tvonal v�g�nek kiolvas�sa a f�jlb�l --- //
		read(route, c.second);
		// --- Megtal�lt �tvonal f�jlba �r�sa --- //
		if (route.first == 1) {
			HEAP_max = max(HEAP_max, HEAP_count);
			cout << ++answers << " " << c.first << " " << HEAP_count << " " << routes_count << '\n';
			o << c.first << '\n';
			write_route(c.second);
			o << '\n';
			continue;
		}
		// --- Kivett �tvonal �rintett �leinek m�trixba �r�sa --- //
		bool *visitedS = new bool[(nodes + 1) * (nodes + 1)] {};
		t_nC last = route.first;
		t_iI previous = route.second;
		while (last != 1) {
			read(route, previous);
			visitedS[(nodes + 1) * last + route.first] = visitedS[(nodes + 1) * route.first + last] = true;
			last = route.first;
			previous = route.second;
		}
		// --- Kivett �tvonal utols� cs�cs�b�l tov�bbindul� utak priorit�si sorba rak�sa --- //
		last = read_node(c.second);
		for (auto &e : edgeS[last]) {
			if (!visitedS[(nodes + 1) * last + e.end]) {
				// --- 1-es cs�csba vezet� szakasz priorit�si sorba rak�sa --- //
				if (e.end == 1) {
					HEAP_push(c.first + e.weight, e.end, c.second);
					continue;
				}
				// --- Sz�less�gi bej�r�s annak vizsg�lat�ra, hogy a tov�bbindul� �t k�rbe�rhet-e --- //
				visitedS[(nodes + 1) * last + e.end] = visitedS[(nodes + 1) * e.end + last] = true;
				t_nC *bfs_visitedS = new t_nC[nodes + 1] {};
				queue<t_nC> from_firstS;
				from_firstS.push(1);
				bfs_visitedS[1] = 1;
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
						if (!visitedS[(nodes + 1) * bfs_c + f.end] && bfs_visitedS[f.end] == 0) {
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
						if (!visitedS[(nodes + 1) * bfs_c + f.end] && bfs_visitedS[f.end] == 0) {
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
				// --- Ha az �t k�rbe�rhet, akkor priorit�si sorba rak�s --- //
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

	// --- Gr�f fel�p�t�se a beolvasott �llist�b�l --- //
	t_oI N, edges;
	cin >> N >> edges;
	if (N > 255 || edges > 255) {
		cout << "Sok.\n";
		return 0;
	}
	nodes = (t_nC)N;
	edgeS.resize(nodes + 1);
	generate_edges(edges);

	// --- �tvonalak visszaad�sa --- //
	distance_from_startS.resize(nodes + 1, t_l_MAX);
	dijkstra(1);
	find_routes();
	r.close();
	o.close();
	cout << HEAP_max << '\n';
}