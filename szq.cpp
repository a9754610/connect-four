#include <iostream>
#include <unordered_map>
#include <random>

#pragma GCC optimize ("O3")

const int N = 16;
const int H = 10;
const int P = 4;
const int INF = 2147483647;
bool chessboard[N][H];
int top[N];
bool winner;
int last_place;
bool suggest[N];

const int STEPS = 7;
bool mode;
int step;

unsigned long long hashconst[2][N][H];
unsigned long long current_hash;
std::unordered_map<unsigned long long, int> value_table;

char chessboard_str[41];

char format(int i, bool cap){
	if(i < 10) return '0'+i;
	else       return (cap?'A':'a')+i-10;
}

int deformat(char i){
	     if('0' <= i and i <= '9') return i-'0';
	else if('A' <= i and i <= 'Z') return i-'A'+10;
	else if('a' <= i and i <= 'z') return i-'a'+10;
	else return i;
}

void getpack(){
	chessboard_str[0] = format(last_place, true);
	int now;
	for(int i = 0; i < N; i++){
		for(int j = 0; j < 2; j++){
			now = (chessboard[i][j*5]<<4 ^ chessboard[i][j*5+1]<<3 ^ chessboard[i][j*5+2]<<2 ^ chessboard[i][j*5+3]<<1 ^ chessboard[i][j*5+4])^i;
			chessboard_str[i*2+j+1] = format(now, false);
		}
	}
}

void unpack(){
	int now;
	for(int i = 0; i < N; i++){
		for(int j = 0; j < 2; j++){
			now = deformat(chessboard_str[i*2+j+1])^i;
			for(int k = 0; k < 5; k++){
				chessboard[i][j*5+k] = (now>>(4-k))&1;
				if(chessboard[i][j*5+k]) top[i] = j*5+k;
			}
		}
	}
	step = 0;
	for(int i = 0; i < N; i++){
		step += top[i];
	}
	last_place = deformat(chessboard_str[0]);
}

void init_hash(){
	std::mt19937_64 rng(std::random_device{}());
	for(int i = 0; i < N; i++){
		for(int j = 0; j < H; j++){
			hashconst[0][i][j] = rng();
			hashconst[1][i][j] = rng();
		}
	}
}

int randint(int n){
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dist(0, n-1);
	return dist(gen);
}

void init(){
	init_hash();
	last_place = N;
	current_hash = 0;
	for(int i = 0; i < N; i++){
		top[i] = 0;
		suggest[i] = 0;
	}
}

void place(int i, bool player){
	chessboard[i][top[i]] = player;
	current_hash ^= hashconst[player][i][top[i]];
	top[i]++;
	chessboard[i][top[i]] = 1;
}
void unplace(int i){
	chessboard[i][top[i]] = 0;
	top[i]--;
	current_hash ^= hashconst[chessboard[i][top[i]]][i][top[i]];
	chessboard[i][top[i]] = 1;
}

bool checks[7];

bool very_efficient_check_four(){
	if(checks[1] && checks[2] && (checks[0] || checks[4])) return true;
	if(checks[5] && checks[4] && (checks[2] || checks[6])) return true;
	return false;
}
bool check_win_at(bool player, int where){
	if(top[where]-4>=0 && chessboard[where][top[where]-1]==player && chessboard[where][top[where]-2]==player
	                   && chessboard[where][top[where]-3]==player && chessboard[where][top[where]-4]==player) return true;
	for(int i = 0; i < 3; i++) checks[i] = where+i-3>=0 && where+i-3<N && top[where+i-3]>=top[where] && chessboard[where+i-3][top[where]-1]==player;
	for(int i = 4; i < 7; i++) checks[i] = where+i-3>=0 && where+i-3<N && top[where+i-3]>=top[where] && chessboard[where+i-3][top[where]-1]==player;
	if(very_efficient_check_four()) return true;
	for(int i = 0; i < 3; i++) checks[i] = where+i-3>=0 && where+i-3<N && top[where+i-3]>=top[where]+i-3 && top[where]+i-3-1>=0 && chessboard[where+i-3][top[where]+i-3-1]==player;
	for(int i = 4; i < 7; i++) checks[i] = where+i-3>=0 && where+i-3<N && top[where+i-3]>=top[where]+i-3 && top[where]+i-3-1>=0 && chessboard[where+i-3][top[where]+i-3-1]==player;
	if(very_efficient_check_four()) return true;
	for(int i = 0; i < 3; i++) checks[i] = where+i-3>=0 && where+i-3<N && top[where+i-3]>=top[where]-i+3 && top[where]-i+3-1>=0 && chessboard[where+i-3][top[where]-i+3-1]==player;
	for(int i = 4; i < 7; i++) checks[i] = where+i-3>=0 && where+i-3<N && top[where+i-3]>=top[where]-i+3 && top[where]-i+3-1>=0 && chessboard[where+i-3][top[where]-i+3-1]==player;
	if(very_efficient_check_four()) return true;
	return false;
}

int val[N], max_val, avg_val, min_val;

int last_th, last_th_;
void think(int th){
	     if(th-last_th <= 1 && last_th - last_th_ >= 3) th++;
	else if(th-last_th >= 3 && last_th - last_th_ <= 1) th--;
	if(th < last_th) th = last_th;
	if(th > 2*N-1) th = 2*N-1;
	last_th_ = last_th;
	last_th = th;
	std::cout << "\r\033[K    [";
	for(int i = 0; i < 2*N-1; i++){
		std::cout << ((i < th)?"&":"-");
	}
	std::cout << "]";
	std::printf("%3d%%", (int)(100*(float)th/(2*N-1)));
	std::cout << std::flush;
}


void show_chessboard(int step, bool show_suggest){
	std::printf("\033c");
	std::printf("\n Step %d; Value of bot: %d / %d / %d\n\n    ", step, max_val, avg_val, min_val);
	for(int i = 0; i < N; i++){
		std::printf(" %c", format(i, true));
	}
	std::printf("\n    |");
	for(int i = 0; i < 2*N-1; i++){
		std::printf("-");
	}
	std::printf("|\n");
	for(int j = H-2; j >= 0; j--){
		std::printf(" %3d|", j);
		for(int i = 0; i < N; i++){
			if(i != 0)                       std::printf(" ");
			if(i==last_place && top[i]+1==j) std::printf("!");
			else if(show_suggest && suggest[i] && top[i]==j) std::printf(".");
			else if(j >= top[i])             std::printf(" ");
			else if(chessboard[i][j])        std::printf("#");
			else                             std::printf("O");
		}
		std::printf("|%d   ", j);
		     if(j == 5) std::cout << " Legend:";
		else if(j == 4) std::cout << "# = black";
		else if(j == 3) std::cout << "O = white";
		else if(j == 2) std::cout << "! = last";
		else if(j == 1) std::cout << ". = suggest";
		else if(j == 8) std::cout << " Chessboard size:";
		else if(j == 7) std::printf("width %d * height %d", N, (H-1));
		std::cout << '\n';
	}
	std::printf("    |");
	for(int i = 0; i < 2*N-1; i++){
		std::printf("-");
	}
	std::printf("|\n    ");
	for(int i = 0; i < N; i++){
		std::printf(" %c", format(i, true));
	}
	std::printf("\n\n    Chessboard code:\n    ");
	getpack();
	std::printf(chessboard_str);
	std::printf("\n\n");
	think((step>1)?(2*N):0);
}


int search(bool bot_is, bool turn, int depth, int where, int alpha, int beta){
	if(depth==STEPS-1){
		if(where==N/3||where==N*2/3) think(last_th+1);
		else if(!randint(1<<(9-STEPS))) think(last_th);
	}
	auto it = value_table.find(current_hash);
	if(it != value_table.end()) return value_table[current_hash];
	else{
		if(check_win_at(!turn, where)){
			int deepval = ((turn != bot_is)?1:(-1))*(2<<depth);
			value_table[current_hash] = deepval;
			return deepval;
		}
		if(depth == 0) return 0;
		if(turn != bot_is){
			int min_val = INF, temp_val;
			for(int i = where+P; i >= where-P; i--){
				if(i < 0 || i >= N) continue;
				if(top[i] >= H-1) continue;
				place(i, turn);
				temp_val = search(bot_is, !turn, depth-1, i, alpha, beta);
				unplace(i);
				min_val = std::min(min_val, temp_val);
				beta = std::min(beta, temp_val);
				if(beta <= alpha || min_val == -(1<<depth)){
					value_table[current_hash] = min_val;
					return min_val;
				}
			}
			for(int i = 0; i < N; i++){
				if(where-P <= i and i <= where+P) continue;
				if(top[i] >= H-1) continue;
				place(i, turn);
				temp_val = search(bot_is, !turn, depth-1, i, alpha, beta);
				unplace(i);
				min_val = std::min(min_val, temp_val);
				beta = std::min(beta, temp_val);
				if(beta <= alpha || min_val == -(1<<depth)){
					value_table[current_hash] = min_val;
					return min_val;
				}
			}
			value_table[current_hash] = min_val;
			return min_val;
		}
		else{
			int max_val = -INF, temp_val;
			for(int i = where-P; i <= where+P; i++){
				if(i < 0 || i >= N) continue;
				if(top[i] >= H-1) continue;
				place(i, turn);
				temp_val = search(bot_is, !turn, depth-1, i, alpha, beta);
				unplace(i);
				max_val = std::max(max_val, temp_val);
				alpha = std::max(alpha, temp_val);
				if(beta <= alpha || max_val == (1<<depth)){
					value_table[current_hash] = max_val;
					return max_val;
				}
			}
			for(int i = 0; i < N; i++){
				if(where-P <= i and i <= where+P) continue;
				if(top[i] >= H-1) continue;
				place(i, turn);
				temp_val = search(bot_is, !turn, depth-1, i, alpha, beta);
				unplace(i);
				max_val = std::max(max_val, temp_val);
				alpha = std::max(alpha, temp_val);
				if(beta <= alpha || max_val == (1<<depth)){
					value_table[current_hash] = max_val;
					return max_val;
				}
			}
			value_table[current_hash] = max_val;
			return max_val;
		}
	}
}

int user_move(){
	std::printf("\n\n  #--> ");
	char move;
	std::cin >> move;
	move = deformat(move);
	if(move >= N) move = user_move();
	if(top[move] >= H-1) move = user_move();
	return (int)move;
}

int bot_move(bool bot_is, int step){
	last_th = 0;
	if(step == 0) return (N-1)/2;
	value_table.clear();
	max_val = -INF, avg_val = 0, min_val = INF;
	int i0 = 0;
	for(int i = last_place-P; i <= last_place+P; i++){
		if(i < 0 || i >= N) continue;
		if(top[i] >= H-1) continue;
		place(i, bot_is);
		val[i] = search(bot_is, !bot_is, STEPS, i, -INF, INF);
		avg_val += val[i];
		max_val = std::max(max_val, val[i]);
		think((int)((float)(i0)/(N)*(2*N-1)));
		unplace(i);
		i0++;
	}
	for(int i = 0; i < N; i++){
		if(last_place-P <= i and i <= last_place+P) continue;
		if(top[i] >= H-1) continue;
		place(i, bot_is);
		val[i] = search(bot_is, !bot_is, STEPS, i, -INF, INF);
		avg_val += val[i];
		max_val = std::max(max_val, val[i]);
		min_val = std::min(min_val, val[i]);
		//think((int)((double)((i+1)*N-i*(i+1)/2)/(N*(N+1)/2)*(2*N-1)));
		think((int)((float)(i)/(N)*(2*N-1)));
		unplace(i);
		i0++;
	}
	avg_val = avg_val/N;
	int res;
	for(int i = 0; i < N; i++){
		if(val[i] == max_val && top[i] < H-1) suggest[i] = true;
		else suggest[i] = false;
	}
	while(true){
		res = randint(N);
		if(suggest[res]) break;
	}
	if(max_val == min_val){
		for(int i = 0; i < N; i++){
			suggest[i] = false;
		}
	}
	//std::this_thread::sleep_for(std::chrono::milliseconds(80));
	return res;
	//return 2;
}

void end_game(int step){
	show_chessboard(step, false);
	std::cout << std::flush;
	std::printf("\n\n Player ");
	std::printf(winner?"#":"O");
	std::printf(" won;\n");
}

void game(){
	step = 0;
	int move_0, move_1;
	init();
	bool new_chessboard;
	std::printf("\n Input 1 for a new chessboard, 0 for an existing chessboard: ");
	std::cin >> new_chessboard;
	if(!new_chessboard){
		std::printf(" Chessboard code: ");
		std::cin >> chessboard_str;
		unpack();
	}
	std::printf(" Input 1 for user-bot game, 0 for bot-bot game: ");
	std::cin >> mode;
	while(true){
		if(step%2 == 0){
			if(mode) bot_move(0, step);
			show_chessboard(step, mode);
			move_1 = mode?(user_move()):(bot_move(1, step));
			last_place = move_1;
			step++;
			place(move_1, 1);
			if(check_win_at(1, move_1)) {winner = 1; break;}
		}
		show_chessboard(step, false);
		move_0 = bot_move(0, step);
		last_place = move_0;
		step++;
		place(move_0, 0);
		if(check_win_at(0, move_0)) {winner = 0; break;}
	}
	end_game(step);
}

int main(){
	init();
	show_chessboard(0, false);
	std::cout << '\n';
	while(true) game();
	return 0;
}