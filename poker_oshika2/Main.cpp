///ポーカーの手役がかぶったときの勝敗 https://www.tsuchimi-blog.com/game-pokerchase-5/#toc10
///ポーカーのルール https://www.youtube.com/watch?v=tGoA4OWzzAk
///参考サイト https://daeudaeu.com/c-poker/#i-2
///playingCard.hppのコード https://github.com/Siv3D/OpenSiv3D/blob/main/Siv3D/include/Siv3D/PlayingCard.hpp

# include <Siv3D.hpp>
# include <vector>

/// @brief スクリーンの設定をするための構造体
typedef struct Screen {
	double width;
	double height;
	Vec2 center;
	ColorF color;
}SCREEN;

/// @brief ポーカーのテーブルの設定をするための構造体
typedef struct Table {
	double width;
	double height;
	double corner;
	ColorF color;
}TABLE;

/// @brief ポーカーの手役の構造体
typedef struct Hand {
	/// @param status ポーカーの手役の種類(0:ハイカード, 1:ワンペア, 2:ツーペア, 3:スリーカード, 4:ストレート, 5:フラッシュ, 6:フルハウス, 7:フォーカード, 8:ストレートフラッシュ, 9:ロイヤルストレートフラッシュ)
	int status;

	/// @param suited_suit フラッシュのときそろっているマーク
	int suited_suit;

	/// @param best_combi_index 
/*
  最も強い組み合わせのインデックス
  ※ポケット→0,1, テーブル→2,3,4,5,6
  例: ポケット(6♦, 8♥)　テーブル(7♥, 4♠, J♥, 6♠, 5♣)
	  最も強い組み合わせは(4♠,5♣, 6♠,7♥,8♥)よりbest_combi_index={3,6,5,2,1};
*/
	std::set<int> best_combi_index;

	/// @param max_rank ハンドの一番大きな数字(手役がかぶったときにつかう)
	std::vector<int> max_rank;
}HAND;

/// @brief プレイヤーの構造体
typedef struct Player {

	/// @param cards プレイヤーのカード(ポケット)
	PlayingCard::Card cards[2];

	/// @param hand プレイヤーの完成している手役
	HAND hand;

}PLAYER;

/// @brief カードの描画設定に関する構造体
typedef struct Card_Draw {
	double width;
	ColorF color;
	double player_space_width;
	double player_space_height;
	double table_card_space_width;
}CARD_DRAW;



/// @param phase ターン数(プリフロップ 0, フロップ 1, ターン 2, リバー 3)
int phase = 0;

/// @param num_table_card phaseとテーブルにあるカードの枚数の対応関係を示した配列
int num_table_card[] = { 0,3,4,5 };

/// @param order ポーカーはAが一番つよいのでその序列を示した配列
int order[] = { 2,3,4,5,6,7,8,9,10,11,12,13,1 };

const Array<String> teyaku =
{
	U"ハイカード",
	U"ワンペア",
	U"ツーペア",
	U"スリーカード",
	U"ストレート",
	U"フラッシュ",
	U"フルハウス",
	U"フォーカード",
	U"ストレートフラッシュ",
	U"ロイヤルストレートフラッシュ",
};


/// @brief 手役を判定するときの関数
void hand_judge(PLAYER& player, PlayingCard::Card table_cards[5]);

/// @brief 勝敗判定関数
/// @return 勝ったプレイヤーの番号
int win_judge(PLAYER player[2]);

void card_init(std::span<PLAYER> player, std::span<PlayingCard::Card> table_cards);

/// @brief player.hand.best_combi_indexにrank_countを代入するときに使う関数
void best_combi_index_assign(PLAYER& player, std::vector<int> rank_count);

/// @brief 手役がストレートかどうかを判定するときの関数
/// @return 与えられたカードの中で一番数字が大きい値を返す. 
int straight(PLAYER& player, std::vector<std::vector<int>> count_rank, int hand_status);

/// @return 与えられたインデックスのカードの数字を降順で返す.
/// @param index 並び替えたいかたまりのインデックス
std::vector<int> rank_sort(PLAYER& player, PlayingCard::Card table_cards[5], std::set<int>index);

void player_card_draw_red_border(double x, double y, double cardWidth, double cardHeight, int player_num);
void table_card_draw_red_border(double centerX, double centerY, double cardWidth, double cardHeight, int player_num);

/// @return ポーカー的にどっちが大きいか返す (a=1,b=6)だったらreturn 1;
int poker_max(int a, int b);

int poker_max(std::vector<int> a);

void Main()
{
	SCREEN screen;
	screen.width = 1280;
	screen.height = 720;
	screen.center.x = screen.width / 2;
	screen.center.y = screen.height / 2;
	screen.color = ColorF{ 0.6, 0.8, 0.7 };

	TABLE table;
	table.width = 700;
	table.height = 200;
	table.corner = 200;
	table.color = Palette::Darkgreen;

	CARD_DRAW card_draw;
	card_draw.width = 75;
	card_draw.color = Palette::Red;
	card_draw.player_space_width = 20;
	card_draw.player_space_height = 20;
	card_draw.table_card_space_width = 100;

	const PlayingCard::Pack pack{ card_draw.width, card_draw.color };
	Array<PlayingCard::Card> cards = PlayingCard::CreateDeck();
	PLAYER player[2];
	/// @param table_card テーブルにあるカード
	PlayingCard::Card table_card[5];
	
	int cards_top = 0;
	// 太文字のフォントを作成する | Create a bold font with MSDF method
	const Font font{ FontMethod::MSDF, 48, Typeface::Bold };

	card_init(player, table_card);

	Window::Resize(screen.width, screen.height);

	Scene::SetBackground(screen.color);


	while (System::Update())
	{

		/// スクリーン(center.x, center.y)を中心にテーブルを描画

		RoundRect{ Arg::center(screen.center.x, screen.center.y), table.width, table.height,table.corner }.draw(table.color);


		/*プレイヤーのカードの描画*/
		
		double width_space = card_draw.player_space_width;
		double height_space = card_draw.player_space_height;
		std::vector<std::vector<Vec2>> playercard_pos(2, std::vector<Vec2>(2));

		playercard_pos[0][0] = Vec2(screen.center.x - pack.width() - width_space / 2, screen.center.y - table.height / 2 - pack.height() - height_space);
		playercard_pos[0][1] = Vec2(screen.center.x + width_space / 2, screen.center.y - table.height / 2 - pack.height() - height_space);
		playercard_pos[1][0] = Vec2(screen.center.x - pack.width() - width_space / 2, screen.center.y + table.height / 2 + height_space);
		playercard_pos[1][1] = Vec2(screen.center.x + width_space / 2, screen.center.y + table.height / 2 + height_space);
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 2; j++) 
				pack(player[i].cards[j]).draw(playercard_pos[i][j].x, playercard_pos[i][j].y);
		}

				
		/*テーブル上のカードの描画*/ 

		double table_width_space = card_draw.table_card_space_width;
		double card_width_space = (table.width- table_width_space*2-pack.width()*5)/4;
		double left_edge = screen.center.x - (card_width_space + pack.width()) * 2;
		
		std::vector<Vec2> table_card_pos(5);
		for (int i = 0; i < num_table_card[phase]; i++) 
			table_card_pos[i] = Vec2(left_edge + (card_width_space + pack.width()) * i, screen.center.y);

		for (int i = 0; i < num_table_card[phase]; i++) 
			pack(table_card[i]).drawAt(table_card_pos[i].x, table_card_pos[i].y);
		
		if (SimpleGUI::Button(U"ターン数: {}"_fmt(phase), Vec2{ 1000, 600 }, 200))
		{
			// カウントを増やす | Increase the count
			++phase;
			phase %= 4;
			if (phase == 0)
				card_init(player, table_card);

		}
				
		for (int i = 0; i < 2; i++)
			hand_judge(player[i], table_card);
		for (int i = 0; i < 2; i++) {
			for (int j : player[i].hand.best_combi_index) {
				if(j<2)
				player_card_draw_red_border(playercard_pos[i][j].x, playercard_pos[i][j].y, pack.width(), pack.height(), i);
				else
				table_card_draw_red_border(table_card_pos[j-2].x, table_card_pos[j-2].y, pack.width(), pack.height(), i);
			}
		}
		
		font(teyaku[player[0].hand.status])
				.drawAt(Vec2(screen.center.x,playercard_pos[0][0].y-100), Palette::Black);
		font(teyaku[player[1].hand.status])
			.drawAt(Vec2(screen.center.x, playercard_pos[1][0].y + 200), Palette::Black);
		if (phase == 3) {
			if (win_judge(player)==1) {
				font(U"負け")
					.drawAt(Vec2(screen.center.x, playercard_pos[0][0].y - 50), Palette::Black);
				font(U"勝ち")
					.drawAt(Vec2(screen.center.x, playercard_pos[1][0].y + 150), Palette::Black);
			}
			else if (win_judge(player) == 0) {
				font(U"勝ち")
					.drawAt(Vec2(screen.center.x, playercard_pos[0][0].y - 50), Palette::Black);
				font(U"負け")
					.drawAt(Vec2(screen.center.x, playercard_pos[1][0].y + 150), Palette::Black);
			}
			else if (win_judge(player) == -1) {
				font(U"引き分け")
					.drawAt(Vec2(screen.center.x, playercard_pos[0][0].y - 50), Palette::Black);
				font(U"引き分け")
					.drawAt(Vec2(screen.center.x, playercard_pos[1][0].y + 150), Palette::Black);
			}
		}

		
		

	}
}

void player_card_draw_red_border(double x,double y, double cardWidth, double cardHeight,int player_num) {
	ColorF color;
	if (player_num == 0)
		color = Palette::Yellow;
	else if(player_num == 1)
		color = Palette::Blue;
	RectF( x- 2 - 4 * player_num, y - 2 - 4* player_num, cardWidth + 4+8*player_num, cardHeight + 4 + 8 * player_num).drawFrame(2.0, 0.0, color);
}

void table_card_draw_red_border(double centerX, double centerY, double cardWidth, double cardHeight,int player_num) {
	double x = centerX - cardWidth / 2;
	double y = centerY - cardHeight / 2;
	ColorF color;
	if (player_num == 0)
		color = Palette::Yellow;
	else if (player_num == 1)
		color = Palette::Blue;
	RectF(x - 2 - 4 * player_num, y - 2 - 4 * player_num, cardWidth + 4+ 8 * player_num, cardHeight + 4+ 8 * player_num).drawFrame(2.0, 0.0, color);
}

void card_init(std::span<PLAYER> player, std::span<PlayingCard::Card> table_cards)
{
	Array<PlayingCard::Card> random_deck = PlayingCard::CreateDeck().shuffle();
	for (int i = 0; i < 2; i++) {
		player[i].hand.best_combi_index.clear();
		player[i].hand.max_rank.clear();
	}
	
	int cards_top = 0;
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			player[i].cards[j] = random_deck[2 * i + j];
			cards_top++;
		}
	}

	for (int i = 0; i < 5; i++)
		table_cards[i] = random_deck[i + cards_top];

}

int win_judge(PLAYER player[]) {
	if (player[0].hand.status > player[1].hand.status)
		return 0;
	else if (player[0].hand.status < player[1].hand.status)
		return 1;
	else
	{
		for (int i = 0; i < player[0].hand.max_rank.size(); i++) {
			if (player[0].hand.max_rank[i] == player[1].hand.max_rank[i])
				continue;
			else if (poker_max(player[0].hand.max_rank[i],player[1].hand.max_rank[i])== player[0].hand.max_rank[i])
				return 0;
			else
				return 1;
		}
		return -1;
	}
}
void hand_judge(PLAYER &player, PlayingCard::Card table_cards[5]) {
	player.hand.status = 0;
	player.hand.max_rank.clear();
	/// @param count_suit マークのインデックス
	std::vector<std::vector<int>> count_suit(4);

	/// @param count_rank カードの数字のインデックス
	std::vector<std::vector<int>> count_rank(14);

	/// @param max_num 手役が完成しているとき一番大きい数字
	std::vector<std::vector<int>> max_num(10);

	/* 各々のマークと数字の枚数をカウント */
	for (int i = 0; i < 2; i++) {
		count_suit[player.cards[i].suit].push_back(i);
		count_rank[player.cards[i].rank].push_back(i);
	}
	for (int i = 0; i < num_table_card[phase]; i++) {
		count_suit[table_cards[i].suit].push_back(2 + i);
		count_rank[table_cards[i].rank].push_back(2 + i);
	}
	/* ワンペア*/
	for (int i = 12; i >= 0; i--) {
		if (count_rank[order[i]].size() == 2) {
			player.hand.status = 1;
			player.hand.best_combi_index.clear();
			max_num[player.hand.status].push_back(order[i]);
			best_combi_index_assign(player, count_rank[order[i]]);
			break;
		}
	}
	/*バグ防止*/
	if (phase == 0)
		return;

	/*ツーペア*/

	bool two_pair_flag = 0;
	for (int a = 12; a >= 0; a--) {
		if (two_pair_flag)
			break;
		for (int b = 12; b >= 0; b--) {
			if (a == b)
				continue;
			if (count_rank[order[a]].size() == 2 && count_rank[order[b]].size() == 2)
			{
				player.hand.status = 2;
				two_pair_flag = 1;
				player.hand.best_combi_index.clear();
				max_num[player.hand.status].push_back(order[a]);
				max_num[player.hand.status].push_back(order[b]);
				best_combi_index_assign(player, count_rank[order[a]]);
				best_combi_index_assign(player, count_rank[order[b]]);
				break;
			}
		}		
	}
		
	/*スリーカード*/
	for (int i = 12; i >= 0; i--) {
		if (count_rank[order[i]].size() == 3) {
			player.hand.status = 3;
			player.hand.best_combi_index.clear();
			max_num[player.hand.status].push_back(order[i]);
			best_combi_index_assign(player, count_rank[order[i]]);
			break;
		}
	}
	/*ストレート*/
	max_num[4].push_back(straight(player, count_rank, 4));

	/* フラッシュ*/
	for (int i = 0; i < 4; i++) {
		if (count_suit[i].size() >= 5) {
			player.hand.suited_suit = i;
			player.hand.status = 5;
			player.hand.best_combi_index.clear();
			best_combi_index_assign(player, count_suit[i]);
			max_num[5].push_back(rank_sort(player, table_cards, player.hand.best_combi_index)[0]);
			break;
		}
	}

	/*フルハウス*/
	bool full_house_flag = 0;
	for (int a = 12; a >= 0; a--) {
		if (full_house_flag)
			break;
		for (int b = 12; b >= 0; b--) {
			if (a == b)continue;
			if (count_rank[order[a]].size() == 3 && count_rank[order[b]].size() >= 2) {
				player.hand.status = 6;
				player.hand.best_combi_index.clear();
				best_combi_index_assign(player, count_rank[order[a]]);
				player.hand.best_combi_index.insert(count_rank[order[b]][0]);
				player.hand.best_combi_index.insert(count_rank[order[b]][1]);
				max_num[player.hand.status].push_back(order[a]);
				max_num[player.hand.status].push_back(order[b]);
				full_house_flag = 1;
				break;
			}
		}
	}
	/*フォーカード*/
	for (int i = 12; i >= 0; i--) {
		if (count_rank[order[i]].size() == 4) {
			player.hand.status = 7;
			player.hand.best_combi_index.clear();
			max_num[player.hand.status].push_back(order[i]);
			best_combi_index_assign(player, count_rank[order[i]]);
			break;
		}
	}

	/*ストレートフラッシュ(ロイヤルストレートフラッシュ)*/
	for (int a = 0; a < 4; a++) {
		if (count_suit[a].size() >= 5) {
			std::vector<std::vector<int>> tmp_count_rank(14);
			player.hand.suited_suit = a;
			for (int b : count_suit[a]) {
				if (b < 2)
					tmp_count_rank[player.cards[b].rank].push_back(b);
				else
					tmp_count_rank[table_cards[b - 2].rank].push_back(b);
			}
			int tmp_max=straight(player, tmp_count_rank, 8);
			max_num[8].push_back(tmp_max);
			if (tmp_max == 1) {
				player.hand.status = 9;
				max_num[9].push_back(tmp_max);
			}
		}
	}

	if (player.hand.status == 0)
		max_num[0].push_back(poker_max(player.cards[0].rank, player.cards[1].rank));
	
	/*キッカー*/
	std::vector<int> kiker_needs= {0,1,2,3,7};
	for (int i = 0; i < kiker_needs.size(); i++) {
		if (player.hand.status == kiker_needs[i]) {

			//手役に絡まないカードのインデックスをまとめる
			std::set<int> tmp1;
			for (int a = 0; a < 7; a++)
				tmp1.insert(a);
			for (int a : player.hand.best_combi_index)
				tmp1.erase(a);

			///手役に絡まないカードの大きな値をmax_numに代入していく
			std::vector<int> tmp2=rank_sort(player, table_cards, tmp1);
			for(int a=0;a< 5-player.hand.best_combi_index.size();a++)
				max_num[player.hand.status].push_back(tmp2[a]);
		}
	}

	for (int i : max_num[player.hand.status])
		player.hand.max_rank.push_back(i);
}

void best_combi_index_assign(PLAYER& player, std::vector<int> rank_count) {
	for (int x : rank_count) 
		player.hand.best_combi_index.insert(x);
}

//最大値を見つけるときにもっと効率のいい方法はあるか?
std::vector<int> rank_sort(PLAYER& player, PlayingCard::Card table_cards[5],std::set<int>index) {
	std::vector<int> modoriti;
	std::vector<int> tmp_index_rank(14, 0);
	for (int i : index) {
		if (i < 2)
			tmp_index_rank[player.cards[i].rank] = 1;
		else
			tmp_index_rank[table_cards[i - 2].rank] = 1;
	}

	for (int i = 12; i >= 0; i--) {
		if (tmp_index_rank[order[i]] ==1) {
			modoriti.push_back(order[i]);
		}
	}
	return modoriti;
}
int poker_max(int a, int b) {
	for (int i = 12; i >=0; i--) {
		if (order[i] == a)
			return a;
		if (order[i] == b)
			return b;
	}
}
int poker_max(std::vector<int> a) {
	for (int i = 12; i >= 0; i--) {
		for (int x : a) {
			if (order[i] == x)
				return x;
		}
	}
}
int straight(PLAYER& player, std::vector<std::vector<int>> count_rank,int hand_status) {
	for (int a = 12; a >= 4; a--) {
		int b;
		for (b = a; b >= a - 4; b--) {
			if (count_rank[order[b]].size() == 0)
				break;
		}
		if (a == b + 5) {
			player.hand.status = hand_status;
			player.hand.best_combi_index.clear();
			for (int c = b+1; c <= a; c++)
				player.hand.best_combi_index.insert( count_rank[order[c]][0]);
			return order[a];
			break;
		}
	}
	///例外ストレート
	int i;
	for (i = 1; i <= 5; i++) {
		if (count_rank[i].size() == 0)
			break;
	}
	if (i == 6) {
		player.hand.status = hand_status;
		player.hand.best_combi_index.clear();
		for (int j = 1; j <= 5; j++)
			player.hand.best_combi_index.insert(count_rank[j][0]);
		return 5;
	}
	return 0;
}
