# include <Siv3D.hpp> // OpenSiv3D v0.6.4
# include "Multiplayer_Photon.hpp"
# include "PHOTON_APP_ID.SECRET"


struct ScrollBar
{
	RectF rect{};
	Optional<double> dragOffset;

	double viewHeight = 600;
	double pageHeight = 1000;
	double viewTop = 0;
	double viewVelocity = 0;

	double accumulateTime = 0;
	static constexpr double stepTime = 1.0 / 200;
	static constexpr double resistance = 10;

	Transition sliderWidthTransition = Transition(0.1s, 0.1s);

	ScrollBar() = default;

	ScrollBar(const RectF& rect, double viewHeight, double pageHeight)
		: rect(rect)
		, viewHeight(viewHeight)
		, pageHeight(pageHeight)
	{

	}

	double sliderHeight() const
	{
		return Max(rect.h * viewHeight / pageHeight, 20.0);
	}

	double sliderYPerViewY() const
	{
		return (rect.h - sliderHeight()) / (pageHeight - viewHeight);
	}

	double sliderY() const
	{
		return viewTop * sliderYPerViewY();
	}

	RectF sliderRect() const
	{
		return RectF(rect.x, rect.y + sliderY(), rect.w, sliderHeight());
	}

	bool existSlider() const
	{
		return viewHeight < pageHeight;
	}

	bool isSliderMouseOver() const
	{
		return sliderRect().stretched(5).mouseOver();
	}

	bool isSliderThick() const
	{
		return isSliderMouseOver() || dragOffset;
	}

	Transformer2D createTransformer() const
	{
		return Transformer2D(Mat3x2::Translate(0, -viewTop), TransformCursor::Yes);
	}

	void scrollBy(double h) {
		viewVelocity += resistance * h;
	}

	void scrollTopTo(double y) {
		scrollBy(y - viewTop);
	}

	void scrollBottomTo(double y) {
		scrollBy(y - viewTop - viewHeight);
	}

	void scrollCenterTo(double y) {
		scrollBy(y - viewTop - viewHeight / 2);
	}

	void update(double wheel = Mouse::Wheel(), double delta = Scene::DeltaTime())
	{
		if (not existSlider()) {
			viewTop = 0;
			viewVelocity = 0;
			dragOffset.reset();
			sliderWidthTransition.reset();
			return;
		}

		for (accumulateTime += delta; accumulateTime >= stepTime; accumulateTime -= stepTime)
		{
			if (not dragOffset) {
				viewTop += viewVelocity * stepTime;
			}

			if (viewVelocity != 0)
			{
				viewVelocity += -viewVelocity * stepTime * resistance;
			}
		}

		if (dragOffset)
		{
			const double prevTop = viewTop;
			viewTop = (Cursor::PosF().y - *dragOffset) / sliderYPerViewY();
			viewVelocity = (viewTop - prevTop) / delta;
		}


		if (isSliderMouseOver() and MouseL.down())
		{
			dragOffset = Cursor::PosF().y - sliderY();
		}
		else if (dragOffset && MouseL.up())
		{
			dragOffset.reset();
		}

		if (wheel) {
			viewVelocity = wheel * 2000;
		}

		if (viewTop < 0)
		{
			viewTop = 0;
			viewVelocity = 0;
		}
		else if (viewTop + viewHeight > pageHeight)
		{
			viewTop = pageHeight - viewHeight;
			viewVelocity = 0;
		}

		sliderWidthTransition.update(isSliderThick());

	}

	void draw(const ColorF& color = Palette::Dimgray) const
	{
		if (not existSlider()) return;

		double w = rect.w * (sliderWidthTransition.value() * 0.5 + 0.5);

		RectF(rect.x - w + rect.w, rect.y + sliderY(), w, sliderHeight()).rounded(rect.w / 2).draw(color);
	}

	double progress0_1() {
		return viewTop / (pageHeight - viewHeight);
	}
};


// ユーザ定義型
struct MyData
{
	String word;

	Point pos;

	// シリアライズに対応させるためのメンバ関数を定義する
	template <class Archive>
	void SIV3D_SERIALIZE(Archive& archive)
	{
		archive(word, pos);
	}
};

class MyNetwork : public Multiplayer_Photon
{
public:

	//static constexpr int32 MaxPlayers = 3;

	using Multiplayer_Photon::Multiplayer_Photon;

	void initResister() {
		// シリアライズデータを受信したときに呼ばれる関数を登録する
		RegisterEventCallback(111, &MyNetwork::customDataReceive111);
	}

private:

	Array<LocalPlayer> m_localPlayers;

	

	void customDataReceive111(LocalPlayerID sender, const int32& i, const double& d, const Vec2& v) {
		Print << U"<<< 111を受信:{},{},{}"_fmt(i, d, v);
	}

	void connectReturn([[maybe_unused]] const int32 errorCode, const String& errorString, const String& region, [[maybe_unused]] const String& cluster) override
	{
		if (m_verbose)
		{
			Print << U"MyNetwork::connectReturn() [サーバへの接続を試みた結果を処理する]";
		}

		if (errorCode)
		{
			if (m_verbose)
			{
				Print << U"[サーバへの接続に失敗] " << errorString;
			}

			return;
		}

		if (m_verbose)
		{
			Print << U"[サーバへの接続に成功]";
			Print << U"[region: {}]"_fmt(region);
			Print << U"[ユーザ名: {}]"_fmt(getUserName());
			Print << U"[ユーザ ID: {}]"_fmt(getUserID());
		}

		Scene::SetBackground(ColorF{ 0.4, 0.5, 0.6 });

		Print << U"isInLobby:" << isInLobby();

		Print << getRoomNameList();

		for(const auto& room : getRoomInfoList())
		{
			Print << U"RoomName:" << room.name;
			for(const auto& prop : room.properties)
			{
				Print << U"RoomProp:" << prop.first << U":" << prop.second;
			}
		}
	}

	void disconnectReturn() override
	{
		if (m_verbose)
		{
			Print << U"MyNetwork::disconnectReturn() [サーバから切断したときに呼ばれる]";
		}

		m_localPlayers.clear();

		Scene::SetBackground(Palette::DefaultBackground);
	}

	void joinRandomRoomReturn([[maybe_unused]] const LocalPlayerID playerID, const int32 errorCode, const String& errorString) override
	{
		if (m_verbose)
		{
			Print << U"MyNetwork::joinRandomRoomReturn() [既存のランダムなルームに参加を試みた結果を処理する]";
		}

		if (errorCode == NoRandomMatchFound)
		{
			const RoomName roomName = (getUserName() + U"'s room-" + ToHex(RandomUint32()));

			if (m_verbose)
			{
				Print << U"[参加可能なランダムなルームが見つからなかった]";
				Print << U"[自分でルーム " << roomName << U" を新規作成する]";
			}
			HashTable<String, String> prop{
				{U"a", U"b"},
				{U"a2", U"b2"}
			};
			createRoom(roomName,RoomCreateOption().properties(prop));

			return;
		}
		else if (errorCode)
		{
			if (m_verbose)
			{
				Print << U"[既存のランダムなルームへの参加でエラーが発生] " << errorString;
			}

			return;
		}

		if (m_verbose)
		{
			Print << U"[既存のランダムなルームに参加できた]";
		}
	}

	void createRoomReturn([[maybe_unused]] const LocalPlayerID playerID, const int32 errorCode, const String& errorString) override
	{
		if (m_verbose)
		{
			Print << U"MyNetwork::createRoomReturn() [ルームを新規作成した結果を処理する]";
		}

		if (errorCode)
		{
			if (m_verbose)
			{
				Print << U"[ルームの新規作成でエラーが発生] " << errorString;
			}

			return;
		}

		if (m_verbose)
		{
			Print << U"[ルーム " << getCurrentRoomName() << U" の作成に成功]";
		}
	}

	void joinRoomEventAction(const LocalPlayer& newPlayer, [[maybe_unused]] const Array<LocalPlayerID>& playerIDs, const bool isSelf) override
	{
		if (m_verbose)
		{
			Print << U"MyNetwork::joinRoomEventAction() [誰か（自分を含む）が現在のルームに参加したときに呼ばれる]";
		}

		m_localPlayers = getLocalPlayers();

		if (m_verbose)
		{
			Print << U"[{} (ID: {}) がルームに参加した。ローカル ID: {}] {}"_fmt(newPlayer.userName, newPlayer.userID, newPlayer.localID, (isSelf ? U"(自分自身)" : U""));

			Print << U"現在の " << getCurrentRoomName() << U" のルームメンバー";

			for (const auto& player : m_localPlayers)
			{
				Print << U"- [{}] {} (id: {}) {}"_fmt(player.localID, player.userName, player.userID, player.isHost ? U"(host)" : U"");
			}
		}

		if (isSelf)
		{
			// 自分がルームに参加したときの処理
			Print << U"visibleProp:" << getVisibleRoomPropertyKeys();
			setRoomProperty(U"key", U"value");
			setRoomProperty(U"key2", U"value2");
			setVisibleRoomPropertyKeys({ U"key2" });
			Print << U"visibleProp:" << getVisibleRoomPropertyKeys();
		}
		else
		{
			// 他のプレイヤーがルームに参加したときの処理
		}
	}

	void leaveRoomEventAction(const LocalPlayerID playerID, [[maybe_unused]] const bool isInactive) override
	{
		if (m_verbose)
		{
			Print << U"MyNetwork::leaveRoomEventAction() [誰かがルームから退出したら呼ばれる]";
		}

		m_localPlayers = getLocalPlayers();

		if (m_verbose)
		{
			for (const auto& player : m_localPlayers)
			{
				if (player.localID == playerID)
				{
					Print << U"[{} (ID: {}, ローカル ID: {}) がルームから退出した]"_fmt(player.userName, player.userID, player.localID);
				}
			}

			Print << U"現在の " << getCurrentRoomName() << U" のルームメンバー";

			for (const auto& player : m_localPlayers)
			{
				Print << U"- [{}] {} (ID: {}) {}"_fmt(player.localID, player.userName, player.userID, player.isHost ? U"(host)" : U"");
			}
		}
	}

	void leaveRoomReturn(int32 errorCode, const String& errorString) override
	{
		if (m_verbose)
		{
			Print << U"MyNetwork::leaveRoomReturn() [ルームから退出したときに呼ばれる]";
		}

		m_localPlayers.clear();

		if (errorCode)
		{
			if (m_verbose)
			{
				Print << U"[ルームからの退出でエラーが発生] " << errorString;
			}

			return;
		}
	}

	void customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const int32 data) override
	{
		Print << U"<<< [" << playerID << U"] からの eventCode: " << eventCode << U", data: int32(" << data << U") を受信";
	}

	void customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const String& data) override
	{
		Print << U"<<< [" << playerID << U"] からの eventCode: " << eventCode << U", data: String(" << data << U") を受信";
	}

	void customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const Point& data) override
	{
		Print << U"<<< [" << playerID << U"] からの eventCode: " << eventCode << U", data: Point" << data << U" を受信";
	}

	void customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const Array<int32>& data) override
	{
		Print << U"<<< [" << playerID << U"] からの eventCode: " << eventCode << U", data: Array<int32>" << data << U" を受信";
	}

	void customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const Array<String>& data) override
	{
		Print << U"<<< [" << playerID << U"] からの eventCode: " << eventCode << U", data: Array<String>" << data << U" を受信";
	}

	// シリアライズデータを受信したときに呼ばれる関数をオーバーライドしてカスタマイズする
	void customEventAction(const LocalPlayerID playerID, const uint8 eventCode, Deserializer<MemoryViewReader>& reader) override
	{
		if (eventCode == 123)
		{
			MyData mydata;
			reader(mydata);
			Print << U"<<< [" << playerID << U"] からの MyData(" << mydata.word << U", " << mydata.pos << U") を受信";
		}

		if (eventCode == 111) {
			int32 i;
			double d;
			Vec2 v;
			reader(i, d, v);
			Print << U"<<< [" << playerID << U"] からの{},{},{}"_fmt(i, d, v) << U") を受信";
		}

		if (eventCode == 112) {
			String message;
			reader(message);
			Print << U"<<< [" << playerID << U"] からの message:" << message << U" を受信";
		}

		if (eventCode == 113) {
			String message;
			reader(message);
			Print << U"<<< [" << playerID << U"] からの message:" << message << U" を受信";
		}
	}
};

void Main()
{
	Window::Resize(1280, 720);
	const std::string secretAppID{ SIV3D_OBFUSCATE(PHOTON_APP_ID) };
	MyNetwork network{ secretAppID, U"1.0", Verbose::Yes };
	network.initResister();
	int32 state = -2;

	ScrollBar scrollBar{ RectF{ 1268, 0, 10, 720 }, 720, 2000 };

	while (System::Update())
	{
		scrollBar.update();
		{
			auto t = scrollBar.createTransformer();


			network.update();

			int32 prev_state = state;
			state = network.getState();
			if (state != prev_state) {
				//Console << U"state:{}"_fmt(state);
				//Console <<U"LoomNameList:" << network.getRoomNameList();
				//Console << U"RoomName:" << network.getCurrentRoomName();
			}

			if (KeySpace.down()) {
				//Console << U"state:{}"_fmt(state);
				//Console << U"LoomNameList:" << network.getRoomNameList();
				//Console << U"RoomName:" << network.getCurrentRoomName();
			}
			PutText(U"state:{}"_fmt(state), Scene::Center());

			double y = -20;
			if (SimpleGUI::Button(U"Connect", Vec2{ 1000, (y+=40) }, 160, (not network.isActive())))
			{
				const String userName = U"Siv";
				network.connect(userName, U"jp");
			}

			if (SimpleGUI::Button(U"Disconnect", Vec2{ 1000, (y += 40) }, 160, network.isActive()))
			{
				network.disconnect();
			}

			if (SimpleGUI::Button(U"Join Room", Vec2{ 1000, (y += 40) }, 160, network.isInLobby()))
			{
				network.joinRandomRoom();
			}

			if (SimpleGUI::Button(U"Leave Room", Vec2{ 1000, (y += 40) }, 160, network.isInRoom()))
			{
				network.leaveRoom();
			}

			if (SimpleGUI::Button(U"Send int32", Vec2{ 1000, (y += 40) }, 200, network.isInRoom()))
			{
				const int32 n = Random(0, 10000);
				Print << U"eventCode: 0, int32(" << n << U") を送信 >>>";
				network.sendEvent(0, n);
			}

			if (SimpleGUI::Button(U"Send String", Vec2{ 1000, (y += 40) }, 200, network.isInRoom()))
			{
				const String s = Sample({ U"Hello!", U"Thank you!", U"Nice!" });
				Print << U"eventCode: 0, String(" << s << U") を送信 >>>";
				network.sendEvent(0, s);
			}

			if (SimpleGUI::Button(U"Send Point", Vec2{ 1000, (y += 40) }, 200, network.isInRoom()))
			{
				const Point pos = RandomPoint(Scene::Rect());
				Print << U"eventCode: 0, Point" << pos << U" を送信 >>>";
				network.sendEvent(0, pos);
			}

			if (SimpleGUI::Button(U"Send Array<int32>", Vec2{ 1000, (y += 40) }, 200, network.isInRoom()))
			{
				Array<int32> v(3);
				for (auto& n : v)
				{
					n = Random(0, 1000);
				}
				Print << U"eventCode: 0, Array<int32>" << v << U" を送信 >>>";
				network.sendEvent(0, v);
			}

			if (SimpleGUI::Button(U"Send Array<String>", Vec2{ 1000, (y += 40) }, 200, network.isInRoom()))
			{
				Array<String> words(3);
				for (auto& word : words)
				{
					word = Sample({ U"apple", U"bird", U"cat", U"dog" });
				}
				Print << U"eventCode: 0, Array<String>" << words << U" を送信 >>>";
				network.sendEvent(0, words);
			}

			// ランダムな MyData を送るボタン
			if (SimpleGUI::Button(U"Send MyData", Vec2{ 1000, (y += 40) }, 200, network.isInRoom()))
			{
				MyData myData;
				myData.word = Sample({ U"apple", U"bird", U"cat", U"dog" });
				myData.pos = RandomPoint(Scene::Rect());

				Print << U"eventCode: 123, MyData(" << myData.word << U", " << myData.pos << U") を送信 >>>";
				network.sendEvent(123, Serializer<MemoryWriter>{}(myData));
			}

			// ランダムな MyData を送るボタン
			if (SimpleGUI::Button(U"ResisterTest", Vec2{ 1000, (y += 40) }, 200, network.isInRoom()))
			{


				Print << U"eventCode: 111 を送信 >>>";
				network.sendEvent(MultiplayerEvent(111), int32(1), 2.2, Vec2(3, 3));
			}

			if (SimpleGUI::Button(U"setPropaty", Vec2{ 1000, (y += 40) }, 200, network.isInRoom()))
			{
				Print << U"setPropaty";
				//network.setPlayerProperty(U"key", U"value");
				network.setRoomProperty(U"key", U"value");
			}

			if (SimpleGUI::Button(U"getPropaty", Vec2{ 1000, (y += 40) }, 200, network.isInRoom()))
			{
				Print << U"getPropaty";
				//Print << network.getPlayerProperty(network.getLocalPlayerID(), U"key");
				Print << network.getRoomProperty(U"key");
				Print << network.getRoomProperty(U"a");
				Print << network.getRoomProperty(U"a2");
			}

			if (SimpleGUI::Button(U"removePropaty", Vec2{ 1000, (y += 40) }, 200, network.isInRoom()))
			{
				Print << U"removePropaty";
				//network.removePlayerProperty(U"key");
				network.removeRoomProperty(U"key");
			}

			if (SimpleGUI::Button(U"getRoomList", Vec2{ 1000, (y += 40) }, 200, network.isInLobby()))
			{
				Print << U"getRoomList";
				Print << network.getRoomNameList();
				for (const auto& room : network.getRoomInfoList())
				{
					Print << U"RoomName:" << room.name;
					for (const auto& prop : room.properties)
					{
						Print << U"RoomProp:" << prop.first << U":" << prop.second;
					}
				}
			}

			if (SimpleGUI::Button(U"cache", Vec2{ 1000, (y += 40) }, 200, network.isInRoom()))
			{
				Print << U"cache >>>";
				network.sendEvent(MultiplayerEvent(112, EventReceiverOption::Others_CacheUntilLeaveRoom), String(U"こんにちは。u"));
				network.sendEvent(MultiplayerEvent(113, EventReceiverOption::Others_CacheUntilLeaveRoom), String(U"こんにちは。u2"));
				network.sendEvent(MultiplayerEvent(112, EventReceiverOption::Others_CacheForever), String(U"こんにちは。f"));
				network.sendEvent(MultiplayerEvent(113, EventReceiverOption::Others_CacheForever), String(U"こんにちは。f2"));

			}

			if (SimpleGUI::Button(U"removeCache", Vec2{ 1000, (y += 40) }, 200, network.isInRoom()))
			{
				Print << U"removeCache >>>";
				network.removeEventCache(112);
			}

			if (SimpleGUI::Button(U"removeCache_1", Vec2{ 1000, (y += 40) }, 200, network.isInRoom()))
			{
				Print << U"removeCache >>>";
				network.removeEventCache(112, { 1 });
			}

		}

		scrollBar.draw();
	}
}
