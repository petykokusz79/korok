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

int HEAP_count = 0;
pair<int, int> HEAP_routeS[HEAP_size + 1]; // length, index in "routeS"
void HEAP_push(unsigned short length, unsigned char last, int previous) {
	if (HEAP_count == HEAP_size || routes_count == routes_size) {
		cout << "Megtelt. " << HEAP_count << " " << routes_count << '\n';
		o << endl;
		exit(0);
	}
	int index = ++HEAP_count; // index a v�g�n
	while (index != 1 && HEAP_routeS[index / 2].first > length) {
		HEAP_routeS[index] = HEAP_routeS[index / 2]; // fentebbi s�llyeszt�se
		index /= 2;
	}
	HEAP_routeS[index] = { length, ++routes_count }; // �j elem helyreilleszt�se
	routeS[routes_count] = { last, previous }; // �j elem "routeS"-ba rak�sa
}
void HEAP_pop(pair<unsigned short, int> &result) { // length, index in "routeS"
	result = HEAP_routeS[1]; // eredm�ny ment�se
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
		index = index * 2 + shorter_child;
	}
	HEAP_routeS[index] = HEAP_routeS[HEAP_count + 1]; // utols� elem helyreilleszt�se
}

int edges;
vector<vector<edge>> edgeS(nodes + 1);
int HEAP_max = 0;

void generate_edges() {
	// --- Beolvasott �llist�b�l szomsz�ds�gi m�trix --- //
	vector<vector<unsigned short>> edge_betweenS(nodes + 1, vector<unsigned short>(nodes + 1));
	for (int i = 0; i < edges; i++) {
		int s, e, w;
		cin >> s >> e >> w;
		edge_betweenS[s][e] = edge_betweenS[e][s] = (unsigned short)w;
	}
	// --- Zs�k�lek t�rl�se --- //
	for (int i = 1; i <= nodes; i++) {
		for (int j = i + 1; j <= nodes; j++) {
			if (edge_betweenS[i][j] != 0) {
				// --- Kiv�lasztott �l t�rl�se --- //
				int length = edge_betweenS[i][j];
				edge_betweenS[i][j] = edge_betweenS[j][i] = 0;
				// --- Az 1-es cs�csot tartalmaz� komponens sz�less�gi bej�r�sa --- //
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
				// --- Kiv�lasztott �l vissza�r�sa --- //
				edge_betweenS[i][j] = edge_betweenS[j][i] = length;
				// --- Az el�z�leg nem bej�rt cs�csokat �rint� �lek t�rl�se --- //
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
	// --- Szomsz�ds�gi lista fel�p�t�se a szomsz�ds�gi m�trix alapj�n --- //
	for (int i = 1; i <= nodes; i++) {
		for (int j = i; j <= nodes; j++) {
			if (edge_betweenS[i][j] != 0) {
				edgeS[i].push_back({ (unsigned char)j, edge_betweenS[i][j] });
				edgeS[j].push_back({ (unsigned char)i, edge_betweenS[i][j] });
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
		// --- �tvonal kiv�tele a priorit�si sorb�l --- //
		HEAP_pop(c);
		// --- Megtal�lt �tvonal f�jlba �r�sa --- //
		if (routeS[c.second].first == 1) {
			HEAP_max = max(HEAP_max, HEAP_count);
			cout << ++answers << " " << c.first << " " << HEAP_count << " " << routes_count << '\n';
			o << c.first << '\n';
			write_route(c.second);
			o << '\n';
			continue;
		}
		// --- Kivett �tvonal �rintett �leinek m�trixba �r�sa --- //
		bool visitedS[nodes + 1][nodes + 1] = {};
		unsigned char last = routeS[c.second].first;
		int previous = routeS[c.second].second;
		while (last != 1) {
			visitedS[last][routeS[previous].first] = visitedS[routeS[previous].first][last] = true;
			last = routeS[previous].first;
			previous = routeS[previous].second;
		}
		// --- Kivett �tvonal utols� cs�cs�b�l tov�bbindul� utak priorit�si sorba rak�sa --- //
		last = routeS[c.second].first;
		for (auto &e : edgeS[last]) {
			if (!visitedS[last][e.end]) {
				// --- 1-es cs�csba vezet� szakasz priorit�si sorba rak�sa --- //
				if (e.end == 1) {
					HEAP_push(c.first + e.weight, e.end, c.second);
					continue;
				}
				// --- Sz�less�gi bej�r�s annak vizsg�lat�ra, hogy a tov�bbindul� �t k�rbe�rhet-e --- //
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
				// --- Ha az �t k�rbe�rhet, akkor priorit�si sorba rak�s --- //
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

	// --- Gr�f fel�p�t�se a beolvasott �llist�b�l --- //
	cin >> edges;
	if (edges > 255) {
		cout << "Sok.\n";
		return 0;
	}
	generate_edges();

	// --- �tvonalak visszaad�sa --- //
	find_routes();
	o.close();
	cout << HEAP_max << '\n';
}