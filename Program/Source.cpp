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
	int index = ++HEAP_count; // index a v�g�n
	int place = HEAP_routeS[index].second; // "routeS"-beli szabad hely ment�se
	while (index != 1 && HEAP_routeS[index / 2].first > r.length) {
		HEAP_routeS[index] = HEAP_routeS[index / 2]; // fentebbi s�llyeszt�se
		index /= 2;
	}
	HEAP_routeS[index] = { r.length, place }; // �j elem helyreilleszt�se
	routeS[place] = r.nodeS; // �j elem "routeS"-ba rak�sa
}
void HEAP_pop(route &result) {
	int place = HEAP_routeS[1].second; // "routeS"-beli hely
	result = { HEAP_routeS[1].first, routeS[place] }; // eredm�ny ment�se
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
	HEAP_routeS[HEAP_count + 1].second = place; // "routeS"-ban felszabadult hely kupacelemhez rendel�se
}

int edges;
vector<vector<edge>> edgeS(nodes + 1);

void generate_edges() {
	// --- Beolvasott �llist�b�l szomsz�ds�gi m�trix --- //
	vector<vector<int>> edge_betweenS(nodes + 1, vector<int>(nodes + 1));
	for (int i = 0; i < edges; i++) {
		int s, e, w;
		cin >> s >> e >> w;
		edge_betweenS[s][e] = edge_betweenS[e][s] = w;
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

void find_routes(ofstream &f) {
	int answers = 0;
	HEAP_load();
	route c = { edgeS[1][0].weight, { 2 } };
	HEAP_push(c);
	while (HEAP_count > 0) {
		// --- �tvonal kiv�tele a priorit�si sorb�l --- //
		HEAP_pop(c);
		// --- Megtal�lt �tvonal f�jlba �r�sa --- //
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
		// --- Kivett �tvonal �rintett �leinek m�trixba �r�sa --- //
		bool visitedS[nodes + 1][nodes + 1] = {};
		visitedS[1][2] = visitedS[2][1] = true;
		for (int i = 1; i < c.nodeS.size(); i++) {
			visitedS[c.nodeS[i]][c.nodeS[i - 1]] = visitedS[c.nodeS[i - 1]][c.nodeS[i]] = true;
		}
		// --- Kivett �tvonal utols� cs�cs�b�l tov�bbindul� utak priorit�si sorba rak�sa --- //
		unsigned char last = c.nodeS.back();
		for (auto &e : edgeS[last]) {
			if (!visitedS[last][e.end]) {
				// --- Sz�less�gi bej�r�s annak vizsg�lat�ra, hogy a tov�bbindul� �t k�rbe�rhet-e --- //
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
				// --- Ha az �t k�rbe�rhet, akkor priorit�si sorba rak�s --- //
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

	// --- Gr�f fel�p�t�se a beolvasott �llist�b�l --- //
	cin >> edges;
	if (edges > 255) {
		cout << "Sok.\n";
		return 0;
	}
	generate_edges();

	// --- �tvonalak visszaad�sa --- //
	ofstream f("output.txt");
	find_routes(f);
	f.close();
}