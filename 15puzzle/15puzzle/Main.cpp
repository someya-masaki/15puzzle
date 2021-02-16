
# include <Siv3D.hpp> // OpenSiv3D v0.4.3

// シーンの名前
enum class State
{
	Title,
	Game
};

// シーン管理クラス
using App = SceneManager<State>;

// タイトルシーン
class Title :public App::Scene
{
private:
	Rect m_startButton = Rect(Arg::center = Scene::Center().moveBy(0, 100), 300, 60);
	Transition m_startTransition = Transition(0.4s, 0.2s);
public:
	Title(const InitData& init):IScene(init){}
	// 毎フレーム更新され、描画以外を処理
	void update() override
	{
		m_startTransition.update(m_startButton.mouseOver());
		// マウスがスタートボタンの上にいるときにマウスの形が手の平の形になる
		if (m_startButton.mouseOver())
		{
			Cursor::RequestStyle(CursorStyle::Hand);
		}
		// スタートボタンを押したらゲームに遷移する
		if (m_startButton.leftClicked())
		{
			changeScene(State::Game);
		}
	}
	// 毎フレーム更新され、描画関連を処理
	void draw() const override
	{
		FontAsset(U"Title")(U"15パズル").drawAt(m_startButton.center() - Vec2(0, 200), Palette::Aqua);
		FontAsset(U"Menu")(U"スタート").drawAt(m_startButton.center(), Palette::White);
	}
};

// ゲームシーン
class Game : public App::Scene
{
private:
	Texture image;
	Grid<int32> mutable panels = Grid<int32>(4, 4);
	Grid<int32> mutable fixPanels = Grid<int32>(4, 4);
	Point mutable endPoint = Point(3, 3);
	bool isGamePlay = true;
	static constexpr int32 CellSize = 128;
	Rect m_startButton = Rect(Arg::center = Scene::Center().moveBy(0, 100), 300, 60);
	Transition m_startTransition = Transition(0.4s, 0.2s);

	// スタートを押した時に画像パネルに移行
	void ChangePanel(Point p)const
	{
		int32 num = panels[p];
		panels[p] = 15;
		panels[endPoint] = num;
		endPoint = p;
	}

	// ランダムな操作でパズルをシャッフル
	void Shuffle()
	{
		for (auto i : step(1000))
		{
			Point pos = Point(0, 0);
			if (endPoint.x == 3)
				pos.x = RandomBool() ? -1 : 0;
			else if (endPoint.x == 0)
				pos.x = RandomBool() ? 1 : 0;
			else
				pos.x = RandomBool() ? -1 : RandomBool() ? 0 : 1;
			if (endPoint.y == 3)
				pos.y = RandomBool() ? -1 : 0;
			else if (endPoint.y == 0)
				pos.y = RandomBool() ? 1 : 0;
			else
				pos.y = RandomBool() ? -1 : RandomBool() ? 0 : 1;
			if (abs(pos.x) + abs(pos.y) > 1)
				continue;
			if (abs(pos.x) + abs(pos.y) > 1)
				continue;
			if (sqrt(pow(pos.x, 2) + pow(pos.y, 2)) > 1.0)
				continue;
			pos += endPoint;
			ChangePanel(pos);
		}
	}

	// パネルの描画
	void drawPanel() const
	{
		for(auto p : step(Size(4,4)))
		{
			auto panelNum = panels[p];
			if (isGamePlay && 15 == panelNum)
				continue;
			// 分けられたパネルを正方形に切り抜く
			image((panelNum % 4) * 128, (panelNum / 4) * 128, 128).draw(p * 128 + Point(44, 44));
		}
	}

	void drawCells() const
	{
		if (!isGamePlay)
			return;
		for (auto p : step(Size(4, 4)))
		{
			// 左下端は空ける
			if (15 == panels[p])
				continue;
			const Rect cell(p * CellSize + Point(44, 44), CellSize);
			// パネルにマウスカーソルが入った時の処理
			if (cell.mouseOver())
			{
				// マウスの表示が手になる
				Cursor::RequestStyle(CursorStyle::Hand);
				// パネルが薄くなる
				cell.stretched(-2).draw(ColorF(1.0, 0.6));
			}
			// 選択したパネルに左クリックを押した時の処理
			if (cell.leftClicked())
			{
				// 選択したパネルは空いているパネルに移動する
				if (sqrt(pow(p.x - endPoint.x, 2) + pow(p.y - endPoint.y, 2)) <= 1.0)
					ChangePanel(p);
			}
		}
	}

public:
	// ゲームシーンに入った時の処理
	Game(const InitData& init) : IScene(init)
	{
		isGamePlay = true;

		// Appから画像の素材を読み込む
		image = Texture(U"pancake.jpg");
		// ピースを15に分ける
		for (auto p : step(Size(4, 4)))
		{
			panels[p] = p.y * 4 + p.x;
			fixPanels[p] = p.y * 4 + p.x;
		}
		// パネルのシャッフルを読み込む
		Shuffle();
	}

	void update() override
	{
		isGamePlay = fixPanels != panels;
		if (!isGamePlay)
		{
			m_startTransition.update(m_startButton.mouseOver());
			// スタートボタンにマウスカーソルが入った時
			if (m_startButton.mouseOver())
				// マウスの表示が手になる
				Cursor::RequestStyle(CursorStyle::Hand);
			// スタートボタンにマウスが左クリックした時
			if (m_startButton.leftClicked())
				// ゲームシーンに移行
				changeScene(State::Game);
		}
	}

	// クリア時に表示
	void draw() const override
	{
		drawPanel();
		drawCells();
		if (!isGamePlay)
		{
			FontAsset(U"Title")(U"クリア!!!").drawAt(Scene::Center(), Palette::Lightskyblue);
			FontAsset(U"Menu")(U"リセット").drawAt(m_startButton.center(), Palette::Lightgreen);
		}
	}
};

void Main()
{
	// 使用するフォントアセットを登録
	FontAsset::Register(U"Title", 80, U"example/font/AnnyantRoman/AnnyantRoman.ttf");
	FontAsset::Register(U"Menu", 30, Typeface::Regular);

	// 背景色を設定
	Scene::SetBackground(Palette::Black);

	// 画面のサイズ
	Window::Resize(600, 600);

	// シーンと遷移時の色を設定
	App manager;
	manager
		.add<Title>(State::Title)
		.add<Game>(State::Game)
		.setFadeColor(ColorF(0));
	while (System::Update())
	{
		if (!manager.update())
		{
			break;
		}
	}
}

