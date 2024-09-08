//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2008-2023 Ryo Suzuki
//	Copyright (c) 2016-2023 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

//-----------------------------------------------
//	Author (OpenSiv3D 実装会)
//	- mak1a
//	- Luke
//	- sthairno
//-----------------------------------------------

# pragma once
# include <Siv3D.hpp>

/// @brief 従来のMultiplayer_Photonとの互換性を保つ場合には 1、新しいバージョンのみを使用する場合には 0
/// @remark Web版を使用する場合には必ず 0 に設定されます。
# define SIV3D_MULTIPLAYER_PHOTON_LEGACY 1

# if SIV3D_PLATFORM(WEB)
#	undef SIV3D_MULTIPLAYER_PHOTON_LEGACY
#	define SIV3D_MULTIPLAYER_PHOTON_LEGACY 0
# endif

# if SIV3D_PLATFORM(WINDOWS)
#	if SIV3D_BUILD(DEBUG)
#		pragma comment (lib, "Common-cpp/lib/Common-cpp_vc16_debug_windows_mt_x64")
#		pragma comment (lib, "Photon-cpp/lib/Photon-cpp_vc16_debug_windows_mt_x64")
#		pragma comment (lib, "LoadBalancing-cpp/lib/LoadBalancing-cpp_vc16_debug_windows_mt_x64")
#	else
#		pragma comment (lib, "Common-cpp/lib/Common-cpp_vc16_release_windows_mt_x64")
#		pragma comment (lib, "Photon-cpp/lib/Photon-cpp_vc16_release_windows_mt_x64")
#		pragma comment (lib, "LoadBalancing-cpp/lib/LoadBalancing-cpp_vc16_release_windows_mt_x64")
#	endif
# endif

# if not SIV3D_PLATFORM(WEB)
// Photon SDK クラスの前方宣言
namespace ExitGames::LoadBalancing
{
	class Listener;
	class Client;
	class RoomOptions;
}
# endif

namespace s3d
{
	/// @brief ルーム名
	using RoomName = String;

	/// @brief ルーム名の View
	using RoomNameView = StringView;

	/// @brief ルーム内でのローカル ID を表現する型
	using LocalPlayerID = int32;

	/// @brief ルーム内のローカルプレイヤーの情報
	struct LocalPlayer
	{
		/// @brief ルーム内でのローカル ID
		LocalPlayerID localID = 0;

		/// @brief ユーザ名
		String userName;

		/// @brief ユーザ ID
		String userID;

		/// @brief ルームのホストであるか
		bool isHost = false;

		/// @brief プレイヤーが現在ルームに接続しているか
		bool isActive = false;
	};

	/// @brief ロビーから参照可能なルームの情報
	struct RoomInfo
	{
		/// @brief ルーム名
		RoomName name;

		/// @brief ルームの現在の人数
		int32 playerCount = 0;

		/// @brief ルームの最大人数
		int32 maxPlayers = 0;

		/// @brief ルームに他のプレイヤーが参加できるか
		bool isOpen = false;

		// @brief ロビーから参照可能なルームプロパティ
		HashTable<uint8, String> properties;
	};

	/// @brief 通信時に用いるプロトコル
	/// @remark Web 版においてHTTPSを使用する場合は Wss を使用してください。
	enum class ConnectionProtocol : uint8
	{
		Default = 0,
	# if not SIV3D_PLATFORM(WEB)
		UDP = 0,
		TCP = 1,
	# else
		Ws = 0,
		Wss = 1,
	# endif
	};

	class RoomCreateOption
	{
	public:
		[[nodiscard]]
		constexpr RoomCreateOption() = default;

		/// @brief ルームがロビーから見えるかを設定します。
		/// @param isVisible ルームがロビーから見える場合 true, それ以外の場合は false
		/// @return 続けてメソッドを呼び出すための *this 参照
		RoomCreateOption& isVisible(bool isVisible);

		/// @brief 他のプレイヤーがルームに参加できるかを設定します。
		/// @param isOpen 他のプレイヤーがルームに参加できる場合 true, それ以外の場合は false
		/// @return 続けてメソッドを呼び出すための *this 参照
		RoomCreateOption& isOpen(bool isOpen);

		/// @brief ルーム内のプレイヤーのユーザ ID を公開するかを設定します。
		/// @param publishUserId ルーム内のプレイヤーのユーザ ID を公開する場合 true, それ以外の場合は false
		/// @return 続けてメソッドを呼び出すための *this 参照
		RoomCreateOption& publishUserId(bool publishUserId);

		/// @brief ルームの最大人数を設定します。
		/// @param maxPlayers ルームの最大人数（0の場合は上限なし）
		/// @return 続けてメソッドを呼び出すための *this 参照
		RoomCreateOption& maxPlayers(int32 maxPlayers);

		/// @brief ロビーから参照可能なルームに保存されるプロパティを設定します。
		/// @param properties 追加するプロパティの辞書
		/// @return 続けてメソッドを呼び出すための *this 参照
		RoomCreateOption& properties(const HashTable<uint8, String>& properties);

		/// @brief ルームから切断されたプレイヤーが再参加できる猶予時間を設定します。
		/// @param rejoinGracePeriod 再参加できる猶予時間（0msの場合は再参加不可能、noneで無限）
		/// @return 続けてメソッドを呼び出すための *this 参照
		RoomCreateOption& rejoinGracePeriod(const Optional<Milliseconds>& rejoinGracePeriod);

		/// @brief 誰もいないルームが破棄されるまでの猶予時間を設定します。
		/// @param roomDestroyGracePeriod 誰もいないルームが破棄されるまでの猶予時間（300000ms（5分）が最大値）
		/// @return 続けてメソッドを呼び出すための *this 参照
		RoomCreateOption& roomDestroyGracePeriod(Milliseconds roomDestroyGracePeriod);

		[[nodiscard]]
		bool isVisible() const noexcept;

		[[nodiscard]]
		bool isOpen() const noexcept;

		[[nodiscard]]
		bool publishUserId() const noexcept;

		[[nodiscard]]
		int32 maxPlayers() const noexcept;

		[[nodiscard]]
		const HashTable<uint8, String>& properties() const noexcept;

		[[nodiscard]]
		const Optional<Milliseconds>& rejoinGracePeriod() const noexcept;

		[[nodiscard]]
		Milliseconds roomDestroyGracePeriod() const noexcept;

	private:

		bool m_isVisible = true;

		bool m_isOpen = true;

		bool m_publishUserId = true;

		int32 m_maxPlayers = 0;

		HashTable<uint8, String> m_properties{};

		Optional<Milliseconds> m_rejoinGracePeriod = 0ms;

		Milliseconds m_roomDestroyGracePeriod = 0ms;
	};

	/// @brief ランダム入室時のマッチメイキングモード
	enum class MatchmakingMode : uint8
	{
		/// @brief 古いルームからうめていくように入室
		FillOldestRoom,

		/// @brief 順次均等に配分するように入室
		Serial,

		/// @brief ランダムに入室
		Random,
	};

	/// @brief ターゲット指定オプション。キャッシュを利用すると以降に入室するプレイヤーにも送信されます。
	enum class ReceiverOption : uint8
	{
		/// @brief 自分以外のプレイヤーに送信
		Others,

		/// @brief 自分以外のプレイヤーに送信。送信者が部屋を離れるまでキャッシュされ、後から入室したプレイヤーにも送信されます。
		Others_CacheUntilLeaveRoom,

		/// @brief 自分以外のプレイヤーに送信。永続的にキャッシュされ、後から入室したプレイヤーにも送信されます。
		Others_CacheForever,

		/// @brief 全員に送信
		All,

		/// @brief 全員に送信。送信者が部屋を離れるまでキャッシュされ、後から入室したプレイヤーにも送信されます。
		All_CacheUntilLeaveRoom,

		/// @brief 全員に送信。永続的にキャッシュされ、後から入室したプレイヤーにも送信されます。
		All_CacheForever,

		/// @brief ホストに送信
		Host,
	};

	/// @brief イベントターゲットグループを指定するためのクラス
	class TargetGroup
	{
	public:

		/// @param targetGroup 1以上255以下の整数（0の場合は全てのプレイヤー）
		[[nodiscard]]
		explicit TargetGroup(uint8 targetGroup) noexcept;

		[[nodiscard]]
		uint8 value() const noexcept;

	private:

		uint8 m_targetGroup = 0;
	};

	/// @brief 送信するイベントのオプション
	class MultiplayerEvent
	{
	public:
		SIV3D_NODISCARD_CXX20
		MultiplayerEvent() = default;

		/// @brief 送信するイベントのオプション
		/// @param eventCode イベントコード （1～199）
		/// @param receiverOption 送信先のターゲット指定オプション
		/// @param priorityIndex プライオリティインデックス　0に近いほど優先的に処理される
		/// @remark Web 版では priorityIndex は無視されます。
		template<class EventCode>
		SIV3D_NODISCARD_CXX20
		MultiplayerEvent(EventCode eventCode, ReceiverOption receiverOption = ReceiverOption::Others, uint8 priorityIndex = 0);

		/// @brief 送信するイベントのオプション
		/// @param eventCode イベントコード （1～199）
		/// @param targetList 送信先のプレイヤーのローカル ID のリスト
		/// @param priorityIndex プライオリティインデックス　0に近いほど優先的に処理される
		/// @remark Web 版では priorityIndex は無視されます。
		template<class EventCode>
		SIV3D_NODISCARD_CXX20
		MultiplayerEvent(EventCode eventCode, Array<LocalPlayerID> targetList, uint8 priorityIndex = 0);

		/// @brief 送信するイベントのオプション
		/// @param eventCode イベントコード （1～199）
		/// @param targetGroup 送信先のイベントターゲットグループ（1以上255以下の整数）
		/// @param priorityIndex プライオリティインデックス　0に近いほど優先的に処理される
		/// @remark Web 版では priorityIndex は無視されます。
		template<class EventCode>
		SIV3D_NODISCARD_CXX20
		MultiplayerEvent(EventCode eventCode, TargetGroup targetGroup, uint8 priorityIndex = 0);

		[[nodiscard]]
		uint8 eventCode() const noexcept;

		[[nodiscard]]
		uint8 priorityIndex() const noexcept;

		[[nodiscard]]
		uint8 targetGroup() const noexcept;

		[[nodiscard]]
		ReceiverOption receiverOption() const noexcept;

		[[nodiscard]]
		const Optional<Array<LocalPlayerID>>& targetList() const noexcept;

	private:

		uint8 m_eventCode = 0;

		uint8 m_priorityIndex = 0;

		uint8 m_targetGroup = 0;

		ReceiverOption m_receiverOption = ReceiverOption::Others;

		Optional<Array<LocalPlayerID>> m_targetList;
	};

	/// @brief Multiplayer_Photon クライアントの状態
	enum class ClientState : uint8 {
		Disconnected,
		ConnectingToLobby,
		InLobby,
		JoiningRoom,
		InRoom,
		LeavingRoom,
		Disconnecting,
	};

	class Multiplayer_Photon;

	namespace detail
	{
		using TypeErasedCallback = void(Multiplayer_Photon::*)();
		using CallbackWrapper = void(*)(Multiplayer_Photon&, TypeErasedCallback, LocalPlayerID, Deserializer<MemoryViewReader>&);

		using CustomEventReceiver = std::pair<TypeErasedCallback, CallbackWrapper>;
	}

	/// @brief マルチプレイヤー用クラス (Photon バックエンド)
	class Multiplayer_Photon
	{
	public:

		/// @brief デフォルトコンストラクタ。このコンストラクタを使用する場合は後で init を呼び出してください。
		SIV3D_NODISCARD_CXX20
		Multiplayer_Photon();

		/// @brief マルチプレイヤー用クラスを作成します。
		/// @param secretPhotonAppID Photon アプリケーション ID
		/// @param photonAppVersion アプリケーションのバージョン
		/// @param verbose デバッグ用の Print 出力をする場合 Verbose::Yes, それ以外の場合は Verbose::No
		/// @param protocol 通信に用いるプロトコル
		/// @remark アプリケーションバージョンが異なるプレイヤーとの通信はできません。
		SIV3D_NODISCARD_CXX20
		Multiplayer_Photon(std::string_view secretPhotonAppID, StringView photonAppVersion, Verbose verbose = Verbose::Yes, ConnectionProtocol protocol = ConnectionProtocol::Default);

		/// @brief マルチプレイヤー用クラスを作成します。
		/// @param secretPhotonAppID Photon アプリケーション ID
		/// @param photonAppVersion アプリケーションのバージョン
		/// @param verbose デバッグ用の Print 出力をする場合 Verbose::Yes, それ以外の場合は Verbose::No
		/// @param protocol 通信に用いるプロトコル
		/// @remark アプリケーションバージョンが異なるプレイヤーとの通信はできません。
		SIV3D_NODISCARD_CXX20
		Multiplayer_Photon(StringView secretPhotonAppID, StringView photonAppVersion, Verbose verbose = Verbose::Yes, ConnectionProtocol protocol = ConnectionProtocol::Default);

		/// @brief マルチプレイヤー用クラスを作成します。
		/// @param secretPhotonAppID Photon アプリケーション ID
		/// @param photonAppVersion アプリケーションのバージョン
		/// @param logger デバッグ用のログの出力先関数
		/// @param protocol 通信に用いるプロトコル
		/// @remark アプリケーションバージョンが異なるプレイヤーとの通信はできません。
		SIV3D_NODISCARD_CXX20
		Multiplayer_Photon(std::string_view secretPhotonAppID, StringView photonAppVersion, const std::function<void(StringView)>& logger, const Verbose verbose = Verbose::Yes, ConnectionProtocol protocol = ConnectionProtocol::Default);

		/// @brief マルチプレイヤー用クラスを作成します。
		/// @param secretPhotonAppID Photon アプリケーション ID
		/// @param photonAppVersion アプリケーションのバージョン
		/// @param logger デバッグ用のログの出力先関数
		/// @param protocol 通信に用いるプロトコル
		/// @remark アプリケーションバージョンが異なるプレイヤーとの通信はできません。
		SIV3D_NODISCARD_CXX20
		Multiplayer_Photon(StringView secretPhotonAppID, StringView photonAppVersion, const std::function<void(StringView)>& logger, const Verbose verbose = Verbose::Yes, ConnectionProtocol protocol = ConnectionProtocol::Default);

		/// @brief デストラクタ
		virtual ~Multiplayer_Photon();

		/// @brief マルチプレイヤー用クラスを初期化します。
		/// @param secretPhotonAppID Photon アプリケーション ID
		/// @param photonAppVersion アプリケーションのバージョン
		/// @param verbose デバッグ用の Print 出力をする場合 Verbose::Yes, それ以外の場合は Verbose::No
		/// @remark アプリケーションバージョンが異なるプレイヤーとの通信はできません。
		void init(std::string_view secretPhotonAppID, StringView photonAppVersion, Verbose verbose = Verbose::Yes, ConnectionProtocol protocol = ConnectionProtocol::Default);
		
		/// @brief マルチプレイヤー用クラスを初期化します。
		/// @param secretPhotonAppID Photon アプリケーション ID
		/// @param photonAppVersion アプリケーションのバージョン
		/// @param verbose デバッグ用の Print 出力をする場合 Verbose::Yes, それ以外の場合は Verbose::No
		/// @remark アプリケーションバージョンが異なるプレイヤーとの通信はできません。
		void init(StringView secretPhotonAppID, StringView photonAppVersion, Verbose verbose = Verbose::Yes, ConnectionProtocol protocol = ConnectionProtocol::Default);
		
		/// @brief マルチプレイヤー用クラスを初期化します。
		/// @param secretPhotonAppID Photon アプリケーション ID
		/// @param photonAppVersion アプリケーションのバージョン
		/// @param logger デバッグ用のログの出力先関数
		/// @param verbose デバッグ用の logger 出力をする場合 Verbose::Yes, それ以外の場合は Verbose::No
		/// @param protocol 通信に用いるプロトコル
		/// @remark アプリケーションバージョンが異なるプレイヤーとの通信はできません。
		void init(std::string_view secretPhotonAppID, StringView photonAppVersion, const std::function<void(StringView)>& logger = {}, const Verbose verbose = Verbose::Yes, ConnectionProtocol protocol = ConnectionProtocol::Default);

		/// @brief マルチプレイヤー用クラスを初期化します。
		/// @param secretPhotonAppID Photon アプリケーション ID
		/// @param photonAppVersion アプリケーションのバージョン
		/// @param logger デバッグ用のログの出力先関数
		/// @param verbose デバッグ用の logger 出力をする場合 Verbose::Yes, それ以外の場合は Verbose::No
		/// @param protocol 通信に用いるプロトコル
		/// @remark アプリケーションバージョンが異なるプレイヤーとの通信はできません。
		void init(StringView secretPhotonAppID, StringView photonAppVersion, const std::function<void(StringView)>& logger = {}, const Verbose verbose = Verbose::Yes, ConnectionProtocol protocol = ConnectionProtocol::Default);
		
		/// @brief Photon サーバへの接続を試みます。
		/// @param userName ユーザ名
		/// @param region 接続するサーバのリージョン。unspecified の場合は利用可能なサーバのうち最速のものが選択されます。
		/// @remark リージョンは https://doc.photonengine.com/en-us/pun/current/connection-and-authentication/regions を参照してください。
		/// @remark Web版では必ず region を設定する必要があります。
		bool connect(StringView userName, const Optional<String>& region = unspecified);

		/// @brief Photon サーバから切断を試みます。
		void disconnect();

		/// @brief サーバーと同期します。
		/// @remark 6 秒間以上この関数を呼ばないと自動的に切断されます。
		void update();

		/// @brief update() を呼ぶ必要がある状態であるかを返します。
		/// @return  update() を呼ぶ必要がある状態である場合 true, それ以外の場合は false
		[[nodiscard]]
		bool isActive() const noexcept;

		/// @brief ネットワークの状態を返します。
		/// @return 現在のネットワークの状態
		[[nodiscard]]
		ClientState getClientState() const;

		/// @brief 自分がロビーにいるかを返します。
		/// @return ロビーにいる場合 true, それ以外の場合は false
		[[nodiscard]]
		bool isInLobby() const;

		/// @brief 自分がロビーまたはルームにいるかを返します。
		/// @return ロビーまたはルームいる場合 true, それ以外の場合は false
		[[nodiscard]]
		bool isInLobbyOrInRoom() const;

		/// @brief 自分がルームに参加しているかを返します。
		/// @return ルームに参加している場合 true, それ以外の場合は false
		[[nodiscard]]
		bool isInRoom() const;

		/// @brief 存在するルームの一覧を返します。
		/// @return 存在するルームの一覧
		[[nodiscard]]
		Array<RoomInfo> getRoomList() const;

		/// @brief 存在するルームの名前の一覧を返します。
		/// @return 存在するルームの名前の一覧
		[[nodiscard]]
		Array<RoomName> getRoomNameList() const;
		
		/// @brief サーバのタイムスタンプ（ミリ秒）を返します。
		/// @return サーバのタイムスタンプ（ミリ秒）
		[[nodiscard]]
		int32 getServerTimeMillisec() const;

		/// @brief サーバのタイムスタンプとクライアントのシステムのタイムスタンプのオフセット（ミリ秒）を返します。
		/// @return サーバのタイムスタンプとクライアントのシステムのタイムスタンプのオフセット（ミリ秒）
		/// @remark Multiplayer_Photon::GetSystemTimeMillisec() の戻り値と足した値がサーバのタイムスタンプと一致します。
		[[nodiscard]]
		int32 getServerTimeOffsetMillisec() const;

		/// @brief サーバーとのラウンドトリップタイム（ping）を取得します。
		/// @return サーバーとのラウンドトリップタイム（ping）
		/// @remark Web 版ではロビー内でこの関数は利用できません。
		[[nodiscard]]
		int32 getPingMillisec() const;

		/// @brief getPingMillisec() で取得されるpingの更新頻度を取得します。
		/// @return pingの更新頻度（ミリ秒）
		[[nodiscard]]
		int32 getPingIntervalMillisec() const;

		/// @brief getPingMillisec() で取得されるpingの更新頻度を設定します。
		/// @param intervalMillisec pingの更新頻度（ミリ秒）
		void setPingIntervalMillisec(int32 intervalMillisec);

	# if not SIV3D_PLATFORM(WEB)
		/// @brief 受信したデータのサイズ（バイト）を返します。
		/// @return 受信したデータのサイズ（バイト）
		[[nodiscard]]
		int32 getBytesIn() const;

		/// @brief 送信したデータのサイズ（バイト）を返します。
		/// @return 送信したデータのサイズ（バイト）
		[[nodiscard]]
		int32 getBytesOut() const;
	# else
		/// @brief 受信したデータのサイズ（バイト）を返します。この関数は Web 版では利用できません。
		/// @return 受信したデータのサイズ（バイト）
		[[nodiscard]]
		int32 getBytesIn() const = delete;

		/// @brief 送信したデータのサイズ（バイト）を返します。この関数は Web 版では利用できません。
		/// @return 送信したデータのサイズ（バイト）
		[[nodiscard]]
		int32 getBytesOut() const = delete;
	# endif

		/// @brief ルームの数を返します。
		/// @return ルームの数
		[[nodiscard]]
		int32 getCountGamesRunning() const;

		/// @brief ゲームをプレイ中のプレイヤーの数を返します。
		/// @return ゲームをプレイ中のプレイヤーの数
		[[nodiscard]]
		int32 getCountPlayersIngame() const;

		/// @brief サーバに接続しているプレイヤーの数を返します。
		/// @return サーバに接続しているプレイヤーの数
		[[nodiscard]]
		int32 getCountPlayersOnline() const;

		/// @brief 既存のランダムなルームに参加を試みます。
		/// @param expectedMaxPlayers 最大人数が指定されたものと一致するルームにのみ参加を試みます。（0の場合は指定なし）
		/// @param matchmakingMode マッチメイキングモード
		/// @return リクエストに成功してコールバックが呼ばれる場合 true、それ以外の場合は false
		/// @remark maxPlayers は 最大 255, 無料の Photon アカウントの場合は 20
		bool joinRandomRoom(int32 expectedMaxPlayers = 0, MatchmakingMode matchmakingMode = MatchmakingMode::FillOldestRoom);

		/// @brief 既存のランダムなルームに参加を試みます。
		/// @param propertyFilter ルームプロパティのフィルタ
		/// @param expectedMaxPlayers 最大人数が指定されたものと一致するルームにのみ参加を試みます。（0の場合は指定なし）
		/// @param matchmakingMode マッチメイキングモード
		/// @return リクエストに成功してコールバックが呼ばれる場合 true、それ以外の場合は false
		/// @remark maxPlayers は 最大 255, 無料の Photon アカウントの場合は 20
		bool joinRandomRoom(const HashTable<uint8, String>& propertyFilter, int32 expectedMaxPlayers = 0, MatchmakingMode matchmakingMode = MatchmakingMode::FillOldestRoom);

		/// @brief 既存のランダムなルームに参加を試み、参加できるルームが無かった場合には新しいルームの作成を試みます。
		/// @param expectedMaxPlayers 最大人数が指定されたものと一致するルームにのみ参加を試みます。（0の場合は指定なし）
		/// @param roomName 新しいルーム名
		/// @return リクエストに成功してコールバックが呼ばれる場合 true、それ以外の場合は false
		[[deprecated("This overload has been deprecated.")]]
		bool joinRandomOrCreateRoom(int32 expectedMaxPlayers, RoomNameView roomName);

		/// @brief ランダムなルームに参加を試み、参加できるルームが無かった場合にルームの作成を試みます。
		/// @param roomName 新しいルーム名。空の場合はランダムな名前が割り当てられます。
		/// @param roomCreateOption ルーム作成オプション
		/// @param propertyFilter プロパティがその通り設定されたルームにのみ参加を試みます。 （空の場合は指定なし）
		/// @param expectedMaxPlayers 最大人数が指定されたものと一致するルームにのみ参加を試みます。（0の場合は指定なし）
		/// @param matchmakingMode マッチメイキングモード
		/// @return リクエストに成功してコールバックが呼ばれる場合 true、それ以外の場合は false
		bool joinRandomOrCreateRoom(RoomNameView roomName, const RoomCreateOption& roomCreateOption = {}, const HashTable<uint8, String>& propertyFilter = {}, int32 expectedMaxPlayers = 0, MatchmakingMode matchmakingMode = MatchmakingMode::FillOldestRoom);

		/// @brief 既存の指定した名前のルームに参加を試み、ルームがまだ作成されてなかった場合には、新しくルームの作成を試みます。
		/// @param roomName ルーム名。
		/// @param option ルーム作成オプション
		/// @return リクエストに成功してコールバックが呼ばれる場合 true、それ以外の場合は false
		bool joinOrCreateRoom(RoomNameView roomName, const RoomCreateOption& option = {});

		/// @brief 既存の指定した名前のルームに参加を試みます。
		/// @param roomName ルーム名
		/// @return リクエストに成功してコールバックが呼ばれる場合 true、それ以外の場合は false
		bool joinRoom(RoomNameView roomName);

		/// @brief 新しいルームの作成を試み、成功した場合にはそのルームに参加します。
		/// @param roomName ルーム名。空の場合はランダムな名前が割り当てられます。
		/// @param maxPlayers ルームの最大人数（0の場合は指定なし）
		/// @return リクエストに成功してコールバックが呼ばれる場合 true、それ以外の場合は false
		/// @remark maxPlayers は 最大 255, 無料の Photon アカウントの場合は 20
		bool createRoom(RoomNameView roomName, int32 maxPlayers = 0);

		/// @brief 新しいルームの作成を試み、成功した場合にはそのルームに参加します。
		/// @param roomName ルーム名。空の場合はランダムな名前が割り当てられます。
		/// @param option ルーム作成オプション
		/// @return リクエストに成功してコールバックが呼ばれる場合 true、それ以外の場合は false
		bool createRoom(RoomNameView roomName, const RoomCreateOption& option = {});

		/// @brief ルームからの退出を試みます。
		/// @param willComeBack 退出後に reconnectAndRejoin() で再参加する場合 true
		void leaveRoom(bool willComeBack = false);

		/// @brief 切断状態から、以前に参加していたルームに再参加を試みます。再参加可能な時間を過ぎている場合は失敗します。
		/// @return リクエストに成功してコールバックが呼ばれる場合 true、それ以外の場合は false
		bool reconnectAndRejoin();

		/// @brief 指定したイベントターゲットグループに参加します。
		/// @param targetGroup ターゲットグループ　(1以上255以下の整数)
		/// @return リクエストに成功してコールバックが呼ばれる場合 true、それ以外の場合は false
		void joinEventTargetGroup(const uint8 targetGroup);

		/// @brief 指定したイベントターゲットグループに参加します。
		/// @param targetGroups ターゲットグループの配列　(1以上255以下の整数)
		void joinEventTargetGroup(const Array<uint8>& targetGroups);

		/// @brief 全てのイベントターゲットグループに参加します。
		void joinAllEventTargetGroups();

		/// @brief 指定したイベントターゲットグループから退出します。
		/// @param targetGroup ターゲットグループ　(1以上255以下の整数)
		void leaveEventTargetGroup(const uint8 targetGroup);

		/// @brief 指定したイベントターゲットグループから退出します。
		/// @param targetGroups ターゲットグループの配列　(1以上255以下の整数)
		void leaveEventTargetGroup(const Array<uint8>& targetGroups);

		/// @brief 全てのイベントターゲットグループから退出します。
		void leaveAllEventTargetGroups();
		
		/// @brief ルームにイベントを送信します。
		/// @param event イベントの送信オプション
		/// @param args 送信するデータ
		/// @remark Argsにはシリアライズ可能かつデフォルト構築可能な型のみが指定できます。
		template<class... Args>
		void sendEvent(const MultiplayerEvent& event, Args... args);
		
		/// @brief ルームにイベントを送信します。
		/// @param event イベントの送信オプション
		/// @param writer 送信するデータを書き込んだシリアライザ
		void sendEvent(const MultiplayerEvent& event, const Serializer<MemoryWriter>& writer);

	# if SIV3D_MULTIPLAYER_PHOTON_LEGACY
		/// @brief ルームにイベントを送信します。
		/// @param eventCode イベントコード
		/// @param value 送信するデータ
		/// @param targets 送信先のプレイヤーのローカル ID, unspecified の場合は自分以外の全員
		void sendEvent(uint8 eventCode, bool value, const Optional<Array<LocalPlayerID>>& targets = unspecified);

		/// @brief ルームにイベントを送信します。
		/// @param eventCode イベントコード
		/// @param value 送信するデータ
		/// @param targets 送信先のプレイヤーのローカル ID, unspecified の場合は自分以外の全員
		void sendEvent(uint8 eventCode, uint8 value, const Optional<Array<LocalPlayerID>>& targets = unspecified);

		/// @brief ルームにイベントを送信します。
		/// @param eventCode イベントコード
		/// @param value 送信するデータ
		/// @param targets 送信先のプレイヤーのローカル ID, unspecified の場合は自分以外の全員
		void sendEvent(uint8 eventCode, int16 value, const Optional<Array<LocalPlayerID>>& targets = unspecified);

		/// @brief ルームにイベントを送信します。
		/// @param eventCode イベントコード
		/// @param value 送信するデータ
		/// @param targets 送信先のプレイヤーのローカル ID, unspecified の場合は自分以外の全員
		void sendEvent(uint8 eventCode, int32 value, const Optional<Array<LocalPlayerID>>& targets = unspecified);

		/// @brief ルームにイベントを送信します。
		/// @param eventCode イベントコード
		/// @param value 送信するデータ
		/// @param targets 送信先のプレイヤーのローカル ID, unspecified の場合は自分以外の全員
		void sendEvent(uint8 eventCode, int64 value, const Optional<Array<LocalPlayerID>>& targets = unspecified);

		/// @brief ルームにイベントを送信します。
		/// @param eventCode イベントコード
		/// @param value 送信するデータ
		/// @param targets 送信先のプレイヤーのローカル ID, unspecified の場合は自分以外の全員
		void sendEvent(uint8 eventCode, float value, const Optional<Array<LocalPlayerID>>& targets = unspecified);

		/// @brief ルームにイベントを送信します。
		/// @param eventCode イベントコード
		/// @param value 送信するデータ
		/// @param targets 送信先のプレイヤーのローカル ID, unspecified の場合は自分以外の全員
		void sendEvent(uint8 eventCode, double value, const Optional<Array<LocalPlayerID>>& targets = unspecified);

		/// @brief ルームにイベントを送信します。
		/// @param eventCode イベントコード
		/// @param value 送信するデータ
		/// @param targets 送信先のプレイヤーのローカル ID, unspecified の場合は自分以外の全員
		void sendEvent(uint8 eventCode, const char32* value, const Optional<Array<LocalPlayerID>>& targets = unspecified);

		/// @brief ルームにイベントを送信します。
		/// @param eventCode イベントコード
		/// @param value 送信するデータ
		/// @param targets 送信先のプレイヤーのローカル ID, unspecified の場合は自分以外の全員
		void sendEvent(uint8 eventCode, StringView value, const Optional<Array<LocalPlayerID>>& targets = unspecified);

		/// @brief ルームにイベントを送信します。
		/// @param eventCode イベントコード
		/// @param value 送信するデータ
		/// @param targets 送信先のプレイヤーのローカル ID, unspecified の場合は自分以外の全員
		void sendEvent(uint8 eventCode, const String& value, const Optional<Array<LocalPlayerID>>& targets = unspecified);

		/// @brief ルームにイベントを送信します。
		/// @param eventCode イベントコード
		/// @param value 送信するデータ
		/// @param targets 送信先のプレイヤーのローカル ID, unspecified の場合は自分以外の全員
		void sendEvent(uint8 eventCode, const Array<bool>& value, const Optional<Array<LocalPlayerID>>& targets = unspecified);

		/// @brief ルームにイベントを送信します。
		/// @param eventCode イベントコード
		/// @param value 送信するデータ
		/// @param targets 送信先のプレイヤーのローカル ID, unspecified の場合は自分以外の全員
		void sendEvent(uint8 eventCode, const Array<uint8>& value, const Optional<Array<LocalPlayerID>>& targets = unspecified);

		/// @brief ルームにイベントを送信します。
		/// @param eventCode イベントコード
		/// @param value 送信するデータ
		/// @param targets 送信先のプレイヤーのローカル ID, unspecified の場合は自分以外の全員
		void sendEvent(uint8 eventCode, const Array<int16>& value, const Optional<Array<LocalPlayerID>>& targets = unspecified);

		/// @brief ルームにイベントを送信します。
		/// @param eventCode イベントコード
		/// @param value 送信するデータ
		/// @param targets 送信先のプレイヤーのローカル ID, unspecified の場合は自分以外の全員
		void sendEvent(uint8 eventCode, const Array<int32>& value, const Optional<Array<LocalPlayerID>>& targets = unspecified);

		/// @brief ルームにイベントを送信します。
		/// @param eventCode イベントコード
		/// @param value 送信するデータ
		/// @param targets 送信先のプレイヤーのローカル ID, unspecified の場合は自分以外の全員
		void sendEvent(uint8 eventCode, const Array<int64>& value, const Optional<Array<LocalPlayerID>>& targets = unspecified);

		/// @brief ルームにイベントを送信します。
		/// @param eventCode イベントコード
		/// @param value 送信するデータ
		/// @param targets 送信先のプレイヤーのローカル ID, unspecified の場合は自分以外の全員
		void sendEvent(uint8 eventCode, const Array<float>& value, const Optional<Array<LocalPlayerID>>& targets = unspecified);

		/// @brief ルームにイベントを送信します。
		/// @param eventCode イベントコード
		/// @param value 送信するデータ
		/// @param targets 送信先のプレイヤーのローカル ID, unspecified の場合は自分以外の全員
		void sendEvent(uint8 eventCode, const Array<double>& value, const Optional<Array<LocalPlayerID>>& targets = unspecified);

		/// @brief ルームにイベントを送信します。
		/// @param eventCode イベントコード
		/// @param value 送信するデータ
		/// @param targets 送信先のプレイヤーのローカル ID, unspecified の場合は自分以外の全員
		void sendEvent(uint8 eventCode, const Array<String>& value, const Optional<Array<LocalPlayerID>>& targets = unspecified);

		/// @brief ルームにイベントを送信します。
		/// @param eventCode イベントコード
		/// @param value 送信するデータ
		/// @param targets 送信先のプレイヤーのローカル ID, unspecified の場合は自分以外の全員
		void sendEvent(uint8 eventCode, const Color& value, const Optional<Array<LocalPlayerID>>& targets = unspecified);

		/// @brief ルームにイベントを送信します。
		/// @param eventCode イベントコード
		/// @param value 送信するデータ
		/// @param targets 送信先のプレイヤーのローカル ID, unspecified の場合は自分以外の全員
		void sendEvent(uint8 eventCode, const ColorF& value, const Optional<Array<LocalPlayerID>>& targets = unspecified);

		/// @brief ルームにイベントを送信します。
		/// @param eventCode イベントコード
		/// @param value 送信するデータ
		/// @param targets 送信先のプレイヤーのローカル ID, unspecified の場合は自分以外の全員
		void sendEvent(uint8 eventCode, const HSV& value, const Optional<Array<LocalPlayerID>>& targets = unspecified);

		/// @brief ルームにイベントを送信します。
		/// @param eventCode イベントコード
		/// @param value 送信するデータ
		/// @param targets 送信先のプレイヤーのローカル ID, unspecified の場合は自分以外の全員
		void sendEvent(uint8 eventCode, const Point& value, const Optional<Array<LocalPlayerID>>& targets = unspecified);

		/// @brief ルームにイベントを送信します。
		/// @param eventCode イベントコード
		/// @param value 送信するデータ
		/// @param targets 送信先のプレイヤーのローカル ID, unspecified の場合は自分以外の全員
		void sendEvent(uint8 eventCode, const Vec2& value, const Optional<Array<LocalPlayerID>>& targets = unspecified);

		/// @brief ルームにイベントを送信します。
		/// @param eventCode イベントコード
		/// @param value 送信するデータ
		/// @param targets 送信先のプレイヤーのローカル ID, unspecified の場合は自分以外の全員
		void sendEvent(uint8 eventCode, const Vec3& value, const Optional<Array<LocalPlayerID>>& targets = unspecified);

		/// @brief ルームにイベントを送信します。
		/// @param eventCode イベントコード
		/// @param value 送信するデータ
		/// @param targets 送信先のプレイヤーのローカル ID, unspecified の場合は自分以外の全員
		void sendEvent(uint8 eventCode, const Vec4& value, const Optional<Array<LocalPlayerID>>& targets = unspecified);

		/// @brief ルームにイベントを送信します。
		/// @param eventCode イベントコード
		/// @param value 送信するデータ
		/// @param targets 送信先のプレイヤーのローカル ID, unspecified の場合は自分以外の全員, unspecified の場合は自分以外の全員
		void sendEvent(uint8 eventCode, const Float2& value, const Optional<Array<LocalPlayerID>>& targets = unspecified);

		/// @brief ルームにイベントを送信します。
		/// @param eventCode イベントコード
		/// @param value 送信するデータ
		/// @param targets 送信先のプレイヤーのローカル ID, unspecified の場合は自分以外の全員
		void sendEvent(uint8 eventCode, const Float3& value, const Optional<Array<LocalPlayerID>>& targets = unspecified);

		/// @brief ルームにイベントを送信します。
		/// @param eventCode イベントコード
		/// @param value 送信するデータ
		/// @param targets 送信先のプレイヤーのローカル ID, unspecified の場合は自分以外の全員
		void sendEvent(uint8 eventCode, const Float4& value, const Optional<Array<LocalPlayerID>>& targets = unspecified);

		/// @brief ルームにイベントを送信します。
		/// @param eventCode イベントコード
		/// @param value 送信するデータ
		/// @param targets 送信先のプレイヤーのローカル ID, unspecified の場合は自分以外の全員
		void sendEvent(uint8 eventCode, const Mat3x2& value, const Optional<Array<LocalPlayerID>>& targets = unspecified);

		/// @brief ルームにイベントを送信します。
		/// @param eventCode イベントコード
		/// @param value 送信するデータ
		/// @param targets 送信先のプレイヤーのローカル ID, unspecified の場合は自分以外の全員
		void sendEvent(uint8 eventCode, const Rect& value, const Optional<Array<LocalPlayerID>>& targets = unspecified);

		/// @brief ルームにイベントを送信します。
		/// @param eventCode イベントコード
		/// @param value 送信するデータ
		/// @param targets 送信先のプレイヤーのローカル ID, unspecified の場合は自分以外の全員
		void sendEvent(uint8 eventCode, const Circle& value, const Optional<Array<LocalPlayerID>>& targets = unspecified);

		/// @brief ルームにイベントを送信します。
		/// @param eventCode イベントコード
		/// @param value 送信するデータ
		/// @param targets 送信先のプレイヤーのローカル ID, unspecified の場合は自分以外の全員
		void sendEvent(uint8 eventCode, const Line& value, const Optional<Array<LocalPlayerID>>& targets = unspecified);

		/// @brief ルームにイベントを送信します。
		/// @param eventCode イベントコード
		/// @param value 送信するデータ
		/// @param targets 送信先のプレイヤーのローカル ID, unspecified の場合は自分以外の全員
		void sendEvent(uint8 eventCode, const Triangle& value, const Optional<Array<LocalPlayerID>>& targets = unspecified);

		/// @brief ルームにイベントを送信します。
		/// @param eventCode イベントコード
		/// @param value 送信するデータ
		/// @param targets 送信先のプレイヤーのローカル ID, unspecified の場合は自分以外の全員
		void sendEvent(uint8 eventCode, const RectF& value, const Optional<Array<LocalPlayerID>>& targets = unspecified);

		/// @brief ルームにイベントを送信します。
		/// @param eventCode イベントコード
		/// @param value 送信するデータ
		/// @param targets 送信先のプレイヤーのローカル ID, unspecified の場合は自分以外の全員
		void sendEvent(uint8 eventCode, const Quad& value, const Optional<Array<LocalPlayerID>>& targets = unspecified);

		/// @brief ルームにイベントを送信します。
		/// @param eventCode イベントコード
		/// @param value 送信するデータ
		/// @param targets 送信先のプレイヤーのローカル ID, unspecified の場合は自分以外の全員
		void sendEvent(uint8 eventCode, const Ellipse& value, const Optional<Array<LocalPlayerID>>& targets = unspecified);

		/// @brief ルームにイベントを送信します。
		/// @param eventCode イベントコード
		/// @param value 送信するデータ
		/// @param targets 送信先のプレイヤーのローカル ID, unspecified の場合は自分以外の全員
		void sendEvent(uint8 eventCode, const RoundRect& value, const Optional<Array<LocalPlayerID>>& targets = unspecified);

		/// @brief ルームにイベントを送信します。
		/// @param eventCode イベントコード
		/// @param value 送信するデータ
		/// @param targets 送信先のプレイヤーのローカル ID, unspecified の場合は自分以外の全員
		/// @remark ユーザ定義型を送信する際に利用します。
		void sendEvent(uint8 eventCode, const Serializer<MemoryWriter>& writer, const Optional<Array<LocalPlayerID>>& targets = unspecified);
	# endif

		/// @brief キャッシュされたイベントを削除します。
		/// @param eventCode 削除するイベントコード, 0 の場合は全てのイベントを削除
		template<class EventCode>
		void removeEventCache(EventCode eventCode = 0);

		/// @brief キャッシュされたイベントを削除します。targetsで指定したプレイヤーに紐づくイベントのみ削除します。
		/// @param eventCode 削除するイベントコード, 0 の場合は全てのイベントを削除
		/// @param targets 対象のプレイヤーのローカル ID
		/// @remark プレイヤーに紐づくイベントとは、ReceiverOption::○○○_CacheUntilLeaveRoomによってキャッシュされたイベントのことです。
		template<class EventCode>
		void removeEventCache(EventCode eventCode, const Array<LocalPlayerID>& targets);

		/// @brief 自身のプレイヤー情報を返します。
		LocalPlayer getLocalPlayer() const;

		/// @brief 指定したローカルプレイヤー ID のプレイヤー情報を返します。
		/// @param localPlayerID ローカルプレイヤー ID
		/// @return プレイヤー情報
		[[nodiscard]]
		LocalPlayer getLocalPlayer(LocalPlayerID localPlayerID) const;

		/// @brief 自身のユーザ名を返します。
		/// @return 自身のユーザ名
		[[nodiscard]]
		String getUserName() const;

		/// @brief 指定したローカルプレイヤー ID のユーザ名を返します。
		/// @param localPlayerID ローカルプレイヤー ID
		/// @return ユーザ名
		[[nodiscard]]
		String getUserName(LocalPlayerID localPlayerID) const;

		/// @brief 自身のユーザ ID を取得します。
		/// @return 自身のユーザ ID
		/// @remark ユーザ ID は connect を呼びだした後は変更することができません。
		/// @remark 現在は、ユーザー ID はユーザー名から自動的に生成されます。
		[[nodiscard]]
		String getUserID() const;

		/// @brief 指定したローカルプレイヤーのユーザ ID を取得します。
		/// @return ユーザ ID
		/// @remark ユーザ ID は connect を呼びだした後は変更することができません。
		/// @remark 現在は、ユーザー ID はユーザー名から自動的に生成されます。
		[[nodiscard]]
		String getUserID(LocalPlayerID localPlayerID) const;

		/// @brief 自分が現在のルームのホストであるかを返します。
		/// @return 自分が現在のルームのホストである場合 true, それ以外の場合は false
		[[nodiscard]]
		bool isHost() const;

		/// @brief ルーム内でのプレイヤー ID を返します。
		/// @return ルーム内でのプレイヤー ID, ルームに参加していない場合は -1
		[[nodiscard]]
		LocalPlayerID getLocalPlayerID() const;

		/// @brief ホストプレイヤーのローカルプレイヤー ID を返します。
		/// @return ホストプレイヤーのローカルプレイヤー ID, ルームに参加していない場合は -1
		[[nodiscard]]
		LocalPlayerID getHostLocalPlayerID() const;

		/// @brief 自身のユーザ名を設定します。
		/// @param name 自身のユーザ名
		void setUserName(StringView name);

		/// @brief 新たなルームのホストを設定します。
		/// @param playerID 新たなルームのホストのローカルプレイヤー ID
		void setHost(LocalPlayerID playerID);

		/// @brief 現在参加しているルームの情報を返します。
		/// @return 現在のルームの情報
		[[nodiscard]]
		RoomInfo getCurrentRoom() const;

		/// @brief 現在参加しているルーム名を返します。
		/// @return 現在のルーム名。ルームに参加していない場合は空の文字列
		[[nodiscard]]
		String getCurrentRoomName() const;

		/// @brief 現在のルームにいるローカルプレイヤーの情報一覧を返します。
		/// @return 現在のルームにいるローカルプレイヤーの情報一覧
		[[nodiscard]]
		Array<LocalPlayer> getLocalPlayers() const;

		/// @brief 現在のルームに存在するプレイヤーの人数を返します。
		/// @return プレイヤーの人数
		[[nodiscard]]
		int32 getPlayerCountInCurrentRoom() const;

		/// @brief 現在のルームの最大人数を返します。
		/// @return ルームの最大人数
		[[nodiscard]]
		int32 getMaxPlayersInCurrentRoom() const;

		/// @brief 現在のルームに他のプレイヤーが参加できるかを返します。
		/// @return 現在のルームに他のプレイヤーが参加できる場合 true, それ以外の場合は false
		[[nodiscard]]
		bool getIsOpenInCurrentRoom() const;

		/// @brief 現在のルームがロビーから見えるかを返します。
		/// @return ルームが見える場合 true, それ以外の場合は false
		[[nodiscard]]
		bool getIsVisibleInCurrentRoom() const;

		/// @brief 現在のルームに他のプレイヤーが参加できるかを設定します。
		/// @param isOpen 他のプレイヤーが参加できる場合 true, それ以外の場合は false
		void setIsOpenInCurrentRoom(bool isOpen);

		/// @brief 現在のルームがロビーから見えるかを設定します。
		/// @param isVisible ルームを見えるようにする場合 true, それ以外の場合は false
		void setIsVisibleInCurrentRoom(bool isVisible);

		/// @brief 現在のルームに紐づけられたロビーから参照可能なプロパティを取得します。
		/// @param key 0 以上 255 以下の整数
		/// @return key に対応する値。存在しない場合は空の文字列
		String getRoomProperty(uint8 key) const;

		/// @brief 現在のルームに紐づけられたロビーから参照可能なプロパティの一覧を取得します。
		HashTable<uint8, String> getRoomProperties() const;

		/// @brief 現在のルームに紐づけられたロビーから参照可能なプロパティを追加します。
		/// @param key 0 以上 255 以下の整数
		/// @param value key に対応させる値
		/// @remark 値にはなるべく短い文字列を用いることが推奨されます。
		void setRoomProperty(uint8 key, StringView value);

		/// @brief サーバーとの接続が切断されたときに呼ばれます。
		/// @param errorCode エラーコード
		virtual void connectionErrorReturn(int32 errorCode) {}

		/// @brief サーバに接続を試みた結果が通知されるときに呼ばれます。
		/// @param errorCode エラーコード。0 の場合には成功
		/// @param errorString エラー文字列
		/// @param region 接続した地域
		/// @param cluster クラスター
		/// @remark Web 版では region は常に connect 時に設定された文字列であり、cluster は常に空文字列です。
		virtual void connectReturn(int32 errorCode, const String& errorString, const String& region, const String& cluster) {}

		/// @brief サーバから切断したときに呼ばれます。
		virtual void disconnectReturn() {}

		/// @brief 自身がルームから退出したときに呼ばれます。
		/// @param errorCode エラーコード。0 の場合には成功
		/// @param errorString エラー文字列
		virtual void leaveRoomReturn(int32 errorCode, const String& errorString) {}

		/// @brief ランダムなルームへの参加を試みた結果が通知されるときに呼ばれます。
		/// @param playerID ルーム内のローカルプレイヤー ID
		/// @param errorCode エラーコード。0 の場合には成功
		/// @param errorString エラー文字列
		virtual void joinRandomRoomReturn(LocalPlayerID playerID, int32 errorCode, const String& errorString) {}

		/// @brief ルームへの参加を試みた結果が通知されるときに呼ばれます。
		/// @param playerID ルーム内のローカルプレイヤー ID
		/// @param errorCode エラーコード。0 の場合には成功
		/// @param errorString エラー文字列
		virtual void joinRoomReturn(LocalPlayerID playerID, int32 errorCode, const String& errorString) {}

		/// @brief 誰か（自分を含む）が現在のルームに参加したときに呼ばれます。
		/// @param newPlayer 参加者の情報
		/// @param playerIDs ルーム内のプレイヤー全員のローカルプレイヤー ID
		/// @param isSelf 参加したのが自分である場合 true, それ以外の場合は false
		virtual void joinRoomEventAction(const LocalPlayer& newPlayer, const Array<LocalPlayerID>& playerIDs, bool isSelf) {}

		/// @brief 現在参加しているルームから誰かが退出したときに呼ばれます。
		/// @param playerID 退出者のローカルプレイヤー ID
		/// @param isInactive 退出者が再参加できる場合 true, それ以外の場合は false
		virtual void leaveRoomEventAction(LocalPlayerID playerID, bool isInactive) {}

		/// @brief ルームの作成を試みた結果が通知されるときに呼ばれます。
		/// @param playerID 自身のローカルプレイヤー ID
		/// @param errorCode エラーコード。0 の場合には成功
		/// @param errorString エラー文字列
		virtual void createRoomReturn(LocalPlayerID playerID, int32 errorCode, const String& errorString) {}

		/// @brief ルームへの参加またはルームの作成を試みた結果が通知されるときに呼ばれます。
		/// @param playerID 自身のローカルプレイヤー ID
		/// @param errorCode エラーコード
		/// @param errorString エラー文字列
		virtual void joinOrCreateRoomReturn(LocalPlayerID playerID, int32 errorCode, const String& errorString) {}

		/// @brief ランダムなルームへの参加またはルームの作成を試みた結果が通知されるときに呼ばれます。
		/// @param playerID 自身のローカルプレイヤー ID
		/// @param errorCode エラーコード。0 の場合には成功
		/// @param errorString エラー文字列
		virtual void joinRandomOrCreateRoomReturn(LocalPlayerID playerID, int32 errorCode, const String& errorString) {}
		
		/// @brief ロビー内のルームが更新されたときに呼ばれます。
		virtual void onRoomListUpdate() {}

		/// @brief ルームのプロパティが変更されたときに呼ばれます。
		/// @param changes 変更されたプロパティのキーと値（Web 版ではこのパラメータは利用できません）
		/// @remark Web 版では、この関数はルームのプロパティが変更された時の他にも呼ばれることがあります。
		virtual void onRoomPropertiesChange(const HashTable<uint8, String>& changes) {}

		/// @brief ホストが変更されたときに呼ばれます。
		/// @param newHostPlayerID 新しいホストのローカルプレイヤー ID
		/// @param oldHostPlayerID 古いホストのローカルプレイヤー ID
		virtual void onHostChange(LocalPlayerID newHostPlayerID, LocalPlayerID oldHostPlayerID) {}

		/// @brief ルームのイベントを受信した際に呼ばれます。
		/// @param playerID 送信者のローカルプレイヤー ID
		/// @param eventCode イベントコード
		/// @param data 受信したデータ
		/// @remark ユーザ定義型を受信する際に利用します。
		virtual void customEventAction(LocalPlayerID playerID, uint8 eventCode, Deserializer<MemoryViewReader>& reader) {}

	# if SIV3D_MULTIPLAYER_PHOTON_LEGACY
		/// @brief ルームのイベントを受信した際に呼ばれます。
		/// @param playerID 送信者のローカルプレイヤー ID
		/// @param eventCode イベントコード
		/// @param data 受信したデータ
		virtual void customEventAction(LocalPlayerID playerID, uint8 eventCode, bool data);

		/// @brief ルームのイベントを受信した際に呼ばれます。
		/// @param playerID 送信者のローカルプレイヤー ID
		/// @param eventCode イベントコード
		/// @param data 受信したデータ
		virtual void customEventAction(LocalPlayerID playerID, uint8 eventCode, uint8 data);

		/// @brief ルームのイベントを受信した際に呼ばれます。
		/// @param playerID 送信者のローカルプレイヤー ID
		/// @param eventCode イベントコード
		/// @param data 受信したデータ
		virtual void customEventAction(LocalPlayerID playerID, uint8 eventCode, int16 data);

		/// @brief ルームのイベントを受信した際に呼ばれます。
		/// @param playerID 送信者のローカルプレイヤー ID
		/// @param eventCode イベントコード
		/// @param data 受信したデータ
		virtual void customEventAction(LocalPlayerID playerID, uint8 eventCode, int32 data);

		/// @brief ルームのイベントを受信した際に呼ばれます。
		/// @param playerID 送信者のローカルプレイヤー ID
		/// @param eventCode イベントコード
		/// @param data 受信したデータ
		virtual void customEventAction(LocalPlayerID playerID, uint8 eventCode, int64 data);

		/// @brief ルームのイベントを受信した際に呼ばれます。
		/// @param playerID 送信者のローカルプレイヤー ID
		/// @param eventCode イベントコード
		/// @param data 受信したデータ
		virtual void customEventAction(LocalPlayerID playerID, uint8 eventCode, float data);

		/// @brief ルームのイベントを受信した際に呼ばれます。
		/// @param playerID 送信者のローカルプレイヤー ID
		/// @param eventCode イベントコード
		/// @param data 受信したデータ
		virtual void customEventAction(LocalPlayerID playerID, uint8 eventCode, double data);

		/// @brief ルームのイベントを受信した際に呼ばれます。
		/// @param playerID 送信者のローカルプレイヤー ID
		/// @param eventCode イベントコード
		/// @param data 受信したデータ
		virtual void customEventAction(LocalPlayerID playerID, uint8 eventCode, const String& data);

		/// @brief ルームのイベントを受信した際に呼ばれます。
		/// @param playerID 送信者のローカルプレイヤー ID
		/// @param eventCode イベントコード
		/// @param data 受信したデータ
		virtual void customEventAction(LocalPlayerID playerID, uint8 eventCode, const Array<bool>& data);

		/// @brief ルームのイベントを受信した際に呼ばれます。
		/// @param playerID 送信者のローカルプレイヤー ID
		/// @param eventCode イベントコード
		/// @param data 受信したデータ
		virtual void customEventAction(LocalPlayerID playerID, uint8 eventCode, const Array<uint8>& data);

		/// @brief ルームのイベントを受信した際に呼ばれます。
		/// @param playerID 送信者のローカルプレイヤー ID
		/// @param eventCode イベントコード
		/// @param data 受信したデータ
		virtual void customEventAction(LocalPlayerID playerID, uint8 eventCode, const Array<int16>& data);

		/// @brief ルームのイベントを受信した際に呼ばれます。
		/// @param playerID 送信者のローカルプレイヤー ID
		/// @param eventCode イベントコード
		/// @param data 受信したデータ
		virtual void customEventAction(LocalPlayerID playerID, uint8 eventCode, const Array<int32>& data);

		/// @brief ルームのイベントを受信した際に呼ばれます。
		/// @param playerID 送信者のローカルプレイヤー ID
		/// @param eventCode イベントコード
		/// @param data 受信したデータ
		virtual void customEventAction(LocalPlayerID playerID, uint8 eventCode, const Array<int64>& data);

		/// @brief ルームのイベントを受信した際に呼ばれます。
		/// @param playerID 送信者のローカルプレイヤー ID
		/// @param eventCode イベントコード
		/// @param data 受信したデータ
		virtual void customEventAction(LocalPlayerID playerID, uint8 eventCode, const Array<float>& data);

		/// @brief ルームのイベントを受信した際に呼ばれます。
		/// @param playerID 送信者のローカルプレイヤー ID
		/// @param eventCode イベントコード
		/// @param data 受信したデータ
		virtual void customEventAction(LocalPlayerID playerID, uint8 eventCode, const Array<double>& data);

		/// @brief ルームのイベントを受信した際に呼ばれます。
		/// @param playerID 送信者のローカルプレイヤー ID
		/// @param eventCode イベントコード
		/// @param data 受信したデータ
		virtual void customEventAction(LocalPlayerID playerID, uint8 eventCode, const Array<String>& data);

		/// @brief ルームのイベントを受信した際に呼ばれます。
		/// @param playerID 送信者のローカルプレイヤー ID
		/// @param eventCode イベントコード
		/// @param data 受信したデータ
		virtual void customEventAction(LocalPlayerID playerID, uint8 eventCode, const Color& data);

		/// @brief ルームのイベントを受信した際に呼ばれます。
		/// @param playerID 送信者のローカルプレイヤー ID
		/// @param eventCode イベントコード
		/// @param data 受信したデータ
		virtual void customEventAction(LocalPlayerID playerID, uint8 eventCode, const ColorF& data);

		/// @brief ルームのイベントを受信した際に呼ばれます。
		/// @param playerID 送信者のローカルプレイヤー ID
		/// @param eventCode イベントコード
		/// @param data 受信したデータ
		virtual void customEventAction(LocalPlayerID playerID, uint8 eventCode, const HSV& data);

		/// @brief ルームのイベントを受信した際に呼ばれます。
		/// @param playerID 送信者のローカルプレイヤー ID
		/// @param eventCode イベントコード
		/// @param data 受信したデータ
		virtual void customEventAction(LocalPlayerID playerID, uint8 eventCode, const Point& data);

		/// @brief ルームのイベントを受信した際に呼ばれます。
		/// @param playerID 送信者のローカルプレイヤー ID
		/// @param eventCode イベントコード
		/// @param data 受信したデータ
		virtual void customEventAction(LocalPlayerID playerID, uint8 eventCode, const Vec2& data);

		/// @brief ルームのイベントを受信した際に呼ばれます。
		/// @param playerID 送信者のローカルプレイヤー ID
		/// @param eventCode イベントコード
		/// @param data 受信したデータ
		virtual void customEventAction(LocalPlayerID playerID, uint8 eventCode, const Vec3& data);

		/// @brief ルームのイベントを受信した際に呼ばれます。
		/// @param playerID 送信者のローカルプレイヤー ID
		/// @param eventCode イベントコード
		/// @param data 受信したデータ
		virtual void customEventAction(LocalPlayerID playerID, uint8 eventCode, const Vec4& data);

		/// @brief ルームのイベントを受信した際に呼ばれます。
		/// @param playerID 送信者のローカルプレイヤー ID
		/// @param eventCode イベントコード
		/// @param data 受信したデータ
		virtual void customEventAction(LocalPlayerID playerID, uint8 eventCode, const Float2& data);

		/// @brief ルームのイベントを受信した際に呼ばれます。
		/// @param playerID 送信者のローカルプレイヤー ID
		/// @param eventCode イベントコード
		/// @param data 受信したデータ
		virtual void customEventAction(LocalPlayerID playerID, uint8 eventCode, const Float3& data);

		/// @brief ルームのイベントを受信した際に呼ばれます。
		/// @param playerID 送信者のローカルプレイヤー ID
		/// @param eventCode イベントコード
		/// @param data 受信したデータ
		virtual void customEventAction(LocalPlayerID playerID, uint8 eventCode, const Float4& data);

		/// @brief ルームのイベントを受信した際に呼ばれます。
		/// @param playerID 送信者のローカルプレイヤー ID
		/// @param eventCode イベントコード
		/// @param data 受信したデータ
		virtual void customEventAction(LocalPlayerID playerID, uint8 eventCode, const Mat3x2& data);

		/// @brief ルームのイベントを受信した際に呼ばれます。
		/// @param playerID 送信者のローカルプレイヤー ID
		/// @param eventCode イベントコード
		/// @param data 受信したデータ
		virtual void customEventAction(LocalPlayerID playerID, uint8 eventCode, const Rect& data);

		/// @brief ルームのイベントを受信した際に呼ばれます。
		/// @param playerID 送信者のローカルプレイヤー ID
		/// @param eventCode イベントコード
		/// @param data 受信したデータ
		virtual void customEventAction(LocalPlayerID playerID, uint8 eventCode, const Circle& data);

		/// @brief ルームのイベントを受信した際に呼ばれます。
		/// @param playerID 送信者のローカルプレイヤー ID
		/// @param eventCode イベントコード
		/// @param data 受信したデータ
		virtual void customEventAction(LocalPlayerID playerID, uint8 eventCode, const Line& data);

		/// @brief ルームのイベントを受信した際に呼ばれます。
		/// @param playerID 送信者のローカルプレイヤー ID
		/// @param eventCode イベントコード
		/// @param data 受信したデータ
		virtual void customEventAction(LocalPlayerID playerID, uint8 eventCode, const Triangle& data);

		/// @brief ルームのイベントを受信した際に呼ばれます。
		/// @param playerID 送信者のローカルプレイヤー ID
		/// @param eventCode イベントコード
		/// @param data 受信したデータ
		virtual void customEventAction(LocalPlayerID playerID, uint8 eventCode, const RectF& data);

		/// @brief ルームのイベントを受信した際に呼ばれます。
		/// @param playerID 送信者のローカルプレイヤー ID
		/// @param eventCode イベントコード
		/// @param data 受信したデータ
		virtual void customEventAction(LocalPlayerID playerID, uint8 eventCode, const Quad& data);

		/// @brief ルームのイベントを受信した際に呼ばれます。
		/// @param playerID 送信者のローカルプレイヤー ID
		/// @param eventCode イベントコード
		/// @param data 受信したデータ
		virtual void customEventAction(LocalPlayerID playerID, uint8 eventCode, const Ellipse& data);

		/// @brief ルームのイベントを受信した際に呼ばれます。
		/// @param playerID 送信者のローカルプレイヤー ID
		/// @param eventCode イベントコード
		/// @param data 受信したデータ
		virtual void customEventAction(LocalPlayerID playerID, uint8 eventCode, const RoundRect& data);
	# endif

		/// @brief クライアントのシステムのタイムスタンプ（ミリ秒）を返します。
		/// @return クライアントのシステムのタイムスタンプ（ミリ秒）
		/// @remark この値に getServerTimeOffsetMillisec() の戻り値と足した値がサーバのタイムスタンプと一致します。
		[[nodiscard]]
		static int32 GetSystemTimeMillisec();

		class PhotonDetail;

		template<class T, class... Args>
		using EventCallbackType = void (T::*)(LocalPlayerID, Args...);

		template<class EventCode, class T, class... Args>
		void RegisterEventCallback(EventCode eventCode, EventCallbackType<T, Args...> callback);

		template<class... Args>
		void debugLog(Args&&... args) const
		{
			if (m_verbose and m_logger) {
				m_logger(Format(std::forward<Args>(args)...));
			}
		}

	protected:

		/// @brief 既存のランダムマッチが見つからなかった時のエラーコード
		/// @remark `joinRandomRoomReturn()` で使います。
		static constexpr int32 NoRandomMatchFound = (0x7FFF - 7);

		/// @brief Verbose モード (Print による詳細なデバッグ出力をする場合 true)
		bool m_verbose = true;

	private:

	# if not SIV3D_PLATFORM(WEB)
		std::unique_ptr<ExitGames::LoadBalancing::Listener> m_listener;

		std::unique_ptr<ExitGames::LoadBalancing::Client> m_client;

		ConnectionProtocol m_connectionProtocol = ConnectionProtocol::UDP;

		String m_lastJoinedRoomName;
	# else
		std::unique_ptr<PhotonDetail> m_detail;
	# endif

		String m_secretPhotonAppID;

		String m_photonAppVersion;

		Optional<String> m_requestedRegion;

		HashTable<uint8, detail::CustomEventReceiver> m_table;

		std::function<void(StringView)> m_logger;
	};

	namespace detail
	{
		template<class T, class... Args>
		struct EventWrapperImpl
		{
			static void wrapper(Multiplayer_Photon& client, TypeErasedCallback callback, LocalPlayerID player, Deserializer<MemoryViewReader>& reader)
			{
				std::tuple<std::remove_cvref_t<Args>...> args{};
				impl(static_cast<T&>(client), callback, player, reader, args, std::make_index_sequence<std::tuple_size_v<std::tuple<Args...>>>());
			}
	
			static void impl(T& client, TypeErasedCallback callback, LocalPlayerID player, Deserializer<MemoryViewReader>& reader, std::tuple<> args, std::integer_sequence<size_t>)
			{
				(client.*reinterpret_cast<Multiplayer_Photon::EventCallbackType<T, Args...>>(callback))(player);
			}
	
			template<std::size_t... I>
			static void impl(T& client, TypeErasedCallback callback, LocalPlayerID player, Deserializer<MemoryViewReader>& reader, std::tuple<std::remove_cvref_t<Args>...> args, std::integer_sequence<size_t, I...>)
			{
				reader(std::get<I>(args)...);
				(client.*reinterpret_cast<Multiplayer_Photon::EventCallbackType<T, Args...>>(callback))(player, static_cast<std::tuple_element_t<I, std::tuple<Args...>>>(std::get<I>(args))...);
			}
		};
	}

	template<class... Args>
	void Multiplayer_Photon::sendEvent(const MultiplayerEvent& event, Args... args)
	{
		sendEvent(event, Serializer<MemoryWriter> {}(args...));
	}

	template<>
	void Multiplayer_Photon::sendEvent<>(const MultiplayerEvent& event);

	template<class EventCode, class T, class ...Args>
	void Multiplayer_Photon::RegisterEventCallback(EventCode eventCode, Multiplayer_Photon::EventCallbackType<T, Args...> callback)
	{
		static_assert(std::is_integral_v<EventCode> or std::is_enum_v<EventCode>, "[Multiplayer_Photon] EventCode must be integral or enum");

		if constexpr (std::is_enum_v<EventCode>)
		{
			auto code = static_cast<std::underlying_type_t<EventCode>>(eventCode);
			if (code < 1 or 199 < code)
			{
				throw Error{ U"[Multiplayer_Photon] EventCode must be in a range of 1 to 199" };
			}
		}
		else
		{
			if (eventCode < 1 or 199 < eventCode)
			{
				throw Error{ U"[Multiplayer_Photon] EventCode must be in a range of 1 to 199" };
			}
		}

		m_table[static_cast<uint8>(eventCode)] = detail::CustomEventReceiver(reinterpret_cast<detail::TypeErasedCallback>(callback), &detail::EventWrapperImpl<T, Args...>::wrapper);
	}

	template<class EventCode>
	MultiplayerEvent::MultiplayerEvent(EventCode eventCode, ReceiverOption receiverOption, uint8 priorityIndex)
		: m_eventCode(static_cast<uint8>(eventCode))
		, m_receiverOption(receiverOption)
		, m_priorityIndex(priorityIndex)
	{
		static_assert(std::is_integral_v<EventCode> or std::is_enum_v<EventCode>, "[Multiplayer_Photon] EventCode must be integral or enum");

		if constexpr (std::is_enum_v<EventCode>)
		{
			auto code = static_cast<std::underlying_type_t<EventCode>>(eventCode);
			if (code < 1 or 199 < code)
			{
				throw Error{ U"[Multiplayer_Photon] EventCode must be in a range of 1 to 199" };
			}
		}
		else
		{
			if (eventCode < 1 or 199 < eventCode)
			{
				throw Error{ U"[Multiplayer_Photon] EventCode must be in a range of 1 to 199" };
			}
		}
	}

	template<class EventCode>
	MultiplayerEvent::MultiplayerEvent(EventCode eventCode, Array<LocalPlayerID> targetList, uint8 priorityIndex)
		: m_eventCode(static_cast<uint8>(eventCode))
		, m_targetList(targetList)
		, m_priorityIndex(priorityIndex)
	{
		static_assert(std::is_integral_v<EventCode> or std::is_enum_v<EventCode>, "[Multiplayer_Photon] EventCode must be integral or enum");

		if constexpr (std::is_enum_v<EventCode>)
		{
			auto code = static_cast<std::underlying_type_t<EventCode>>(eventCode);
			if (code < 1 or 199 < code)
			{
				throw Error{ U"[Multiplayer_Photon] EventCode must be in a range of 1 to 199" };
			}
		}
		else
		{
			if (eventCode < 1 or 199 < eventCode)
			{
				throw Error{ U"[Multiplayer_Photon] EventCode must be in a range of 1 to 199" };
			}
		}
	}

	template<class EventCode>
	MultiplayerEvent::MultiplayerEvent(EventCode eventCode, TargetGroup targetGroup, uint8 priorityIndex)
		: m_eventCode(static_cast<uint8>(eventCode))
		, m_targetGroup(targetGroup.value())
		, m_priorityIndex(priorityIndex)
	{
		static_assert(std::is_integral_v<EventCode> or std::is_enum_v<EventCode>, "[Multiplayer_Photon] EventCode must be integral or enum");
		
		if constexpr (std::is_enum_v<EventCode>)
		{
			auto code = static_cast<std::underlying_type_t<EventCode>>(eventCode);
			if (code < 1 or 199 < code)
			{
				throw Error{ U"[Multiplayer_Photon] EventCode must be in a range of 1 to 199" };
			}
		}
		else
		{
			if (eventCode < 1 or 199 < eventCode)
			{
				throw Error{ U"[Multiplayer_Photon] EventCode must be in a range of 1 to 199" };
			}
		}
	}

	template<>
	void Multiplayer_Photon::removeEventCache(uint8 eventCode);

	template<class EventCode>
	void Multiplayer_Photon::removeEventCache(EventCode eventCode)
	{
		static_assert(std::is_integral_v<EventCode> or std::is_enum_v<EventCode>, "[Multiplayer_Photon] EventCode must be integral or enum");
		
		if constexpr (std::is_enum_v<EventCode>)
		{
			auto code = static_cast<std::underlying_type_t<EventCode>>(eventCode);
			if (code < 1 or 199 < code)
			{
				throw Error{ U"[Multiplayer_Photon] EventCode must be in a range of 1 to 199" };
			}
		}
		else
		{
			if (eventCode < 1 or 199 < eventCode)
			{
				throw Error{ U"[Multiplayer_Photon] EventCode must be in a range of 1 to 199" };
			}
		}
		removeEventCache(static_cast<uint8>(eventCode));
	}

	template<>
	void Multiplayer_Photon::removeEventCache(uint8 eventCode, const Array<LocalPlayerID>& targets);

	template<class EventCode>
	void Multiplayer_Photon::removeEventCache(EventCode eventCode, const Array<LocalPlayerID>& targets)
	{
		static_assert(std::is_integral_v<EventCode> or std::is_enum_v<EventCode>, "[Multiplayer_Photon] EventCode must be integral or enum");
		
		if constexpr (std::is_enum_v<EventCode>)
		{
			auto code = static_cast<std::underlying_type_t<EventCode>>(eventCode);
			if (code < 1 or 199 < code)
			{
				throw Error{ U"[Multiplayer_Photon] EventCode must be in a range of 1 to 199" };
			}
		}
		else
		{
			if (eventCode < 1 or 199 < eventCode)
			{
				throw Error{ U"[Multiplayer_Photon] EventCode must be in a range of 1 to 199" };
			}
		}

		removeEventCache(static_cast<uint8>(eventCode), targets);
	}
}
