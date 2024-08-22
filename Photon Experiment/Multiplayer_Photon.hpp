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
# define SIV3D_MULTIPLAYER_PHOTON_LAGACY 1

# if SIV3D_PLATFORM(WEB)
#	undef SIV3D_MULTIPLAYER_PHOTON_LAGACY
#	define SIV3D_MULTIPLAYER_PHOTON_LAGACY 0
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

		/// @brief アクティブであるか
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
		HashTable<String, String> properties;
	};

	enum class ConnectionProtocol : uint8
	{
		Default = 0,
		Ws = 0,
		Wss = 1,
		UDP = 0,
		TCP = 1,
	};

	class Multiplayer_Photon;

	namespace detail
	{
		class RoomCreateOptionDetail;

		using TypeErasedCallback = void(Multiplayer_Photon::*)();
		using CallbackWrapper = void(*)(Multiplayer_Photon&, TypeErasedCallback, LocalPlayerID, Deserializer<MemoryViewReader>&);

		using CustomEventReceiver = std::pair<TypeErasedCallback, CallbackWrapper>;
	}

	/// @brief ルーム作成オプション
	class RoomCreateOption
	{
	public:
		[[nodiscard]]
		constexpr RoomCreateOption() = default;

		/// @brief ルームがロビーから見えるかを設定します。
		/// @param isVisible ルームがロビーから見える場合 true, それ以外の場合は false
		/// @return *this
		RoomCreateOption& isVisible(bool isVisible);

		/// @brief 他のプレイヤーがルームに参加できるかを設定します。
		/// @param isOpen 他のプレイヤーがルームに参加できる場合 true, それ以外の場合は false
		/// @return *this
		RoomCreateOption& isOpen(bool isOpen);

		/// @brief ルーム内のプレイヤーのユーザ ID を公開するかを設定します。
		/// @param publishUserId ルーム内のプレイヤーのユーザ ID を公開する場合 true, それ以外の場合は false
		/// @return *this
		RoomCreateOption& publishUserId(bool publishUserId);

		/// @brief ルームの最大人数を設定します。
		/// @param maxPlayers ルームの最大人数 (0の場合は指定なし)
		/// @return *this
		RoomCreateOption& maxPlayers(int32 maxPlayers);

		/// @brief ルームプロパティを設定します。
		/// @param properties ルームプロパティ
		/// @return *this
		RoomCreateOption& properties(const HashTable<String, String>& properties);

		/// @brief ロビーから参照可能なルームプロパティのキーリストを設定します。
		/// @param keys キーリスト
		/// @return *this
		RoomCreateOption& visibleRoomPropertyKeys(const Array<String>& visibleRoomPropertyKeys);

		/// @brief ルームから切断されたプレイヤーが再参加できる猶予時間を設定します。
		/// @param rejoinGracePeriod 再参加できる猶予時間 (0msの場合は再参加不可能、noneで無限)
		/// @return *this
		RoomCreateOption& rejoinGracePeriod(const Optional<Milliseconds>& rejoinGracePeriod = 0ms);

		/// @brief 誰もいないルームが破棄されるまでの猶予時間を設定します。
		/// @param roomDestroyGracePeriod 誰もいないルームが破棄されるまでの猶予時間。300000ms (5分) が最大値
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
		const HashTable<String, String>& properties() const noexcept;

		[[nodiscard]]
		const Array<String>& visibleRoomPropertyKeys() const noexcept;

		[[nodiscard]]
		Optional<Milliseconds> rejoinGracePeriod() const noexcept;

		[[nodiscard]]
		Milliseconds roomDestroyGracePeriod() const noexcept;

	private:

		bool m_isVisible = true;

		bool m_isOpen = true;

		bool m_publishUserId = true;

		int32 m_maxPlayers = 0;

		HashTable<String, String> m_properties{};

		Array<String> m_visibleRoomPropertyKeys{};

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
	enum class EventReceiverOption : uint8
	{
		/// @brief 自分以外のプレイヤーに送信
		Others,

		/// @brief 自分以外のプレイヤーに送信。送信者が部屋を離れるまでキャッシュされます。
		Others_CacheWithPlayer,

		/// @brief 自分以外のプレイヤーに送信。永続的にキャッシュされます。
		Others_CacheWithRoom,

		/// @brief 全員に送信
		All,

		/// @brief 全員に送信。送信者が部屋を離れるまでキャッシュされます。
		All_CacheWithPlayer,

		/// @brief 全員に送信。永続的にキャッシュされます。
		All_CacheWithRoom,

		/// @brief ホストに送信
		Host,
	};

	/// @brief ターゲットグループ
	class TargetGroup
	{
	public:
		[[nodiscard]]
		constexpr TargetGroup() = default;

		/// @brief ターゲットグループを作成します。
		/// @param targetGroup 1以上255以下の整数 (0の場合は全てのプレイヤー)
		[[nodiscard]]
		explicit TargetGroup(uint8 targetGroup) noexcept;

		[[nodiscard]]
		uint8 value() const noexcept;

	private:

		uint8 m_targetGroup = 0;
	};

	/// @brief 送信するイベントの情報
	class MultiplayerEvent
	{
	public:

		[[nodiscard]]
		MultiplayerEvent() = default;

		/// @brief 送信するイベントの情報を作成します。
		/// @param eventCode イベントコード (1~199)
		/// @param receiverOption ターゲット指定オプション
		/// @param priorityIndex プライオリティインデックス　0に近いほど優先的に処理される
		/// @remark Web 版では priorityIndex は無視されます。
		[[nodiscard]]
		explicit MultiplayerEvent(uint8 eventCode, EventReceiverOption receiverOption = EventReceiverOption::Others, uint8 priorityIndex = 0);

		/// @brief 送信するイベントの情報を作成します。
		/// @param eventCode イベントコード (1~199)
		/// @param targetList ターゲットリスト
		/// @param priorityIndex プライオリティインデックス　0に近いほど優先的に処理される
		/// @remark Web 版では priorityIndex は無視されます。
		[[nodiscard]]
		MultiplayerEvent(uint8 eventCode, Array<LocalPlayerID> targetList, uint8 priorityIndex = 0);

		/// @brief 送信するイベントの情報を作成します。
		/// @param eventCode イベントコード (1~199)
		/// @param targetGroup ターゲットグループ (1以上255以下の整数)
		/// @param priorityIndex プライオリティインデックス　0に近いほど優先的に処理される
		/// @remark Web 版では priorityIndex は無視されます。
		[[nodiscard]]
		MultiplayerEvent(uint8 eventCode, TargetGroup targetGroup, uint8 priorityIndex = 0);

		[[nodiscard]]
		uint8 eventCode() const noexcept;

		[[nodiscard]]
		uint8 priorityIndex() const noexcept;

		[[nodiscard]]
		uint8 targetGroup() const noexcept;

		[[nodiscard]]
		EventReceiverOption receiverOption() const noexcept;

		[[nodiscard]]
		const Optional<Array<LocalPlayerID>>& targetList() const noexcept;

	private:

		uint8 m_eventCode = 0;

		uint8 m_priorityIndex = 0;

		uint8 m_targetGroup = 0;

		EventReceiverOption m_receiverOption = EventReceiverOption::Others;

		Optional<Array<LocalPlayerID>> m_targetList;
	};

	/// @brief クライアントの状態
	enum class ClientState : uint8 {
		Disconnected,
		ConnectingToLobby,
		InLobby,
		JoiningRoom,
		InRoom,
		LeavingRoom,
		Disconnecting,
	};

	/// @brief マルチプレイヤー用クラス (Photon バックエンド)
	class Multiplayer_Photon
	{
	public:
		/// @brief デフォルトコンストラクタ
		SIV3D_NODISCARD_CXX20
			Multiplayer_Photon() = default;

		/// @brief マルチプレイヤー用クラスを作成します。
		/// @param secretPhotonAppID Photon アプリケーション ID
		/// @param photonAppVersion アプリケーションのバージョン
		/// @param デバッグ用の Print 出力をする場合 Verbose::Yes, それ以外の場合は Verbose::No
		/// @param protocol 通信に用いるプロトコル
		/// @remark アプリケーションバージョンが異なるプレイヤーとの通信はできません。
		SIV3D_NODISCARD_CXX20
			Multiplayer_Photon(std::string_view secretPhotonAppID, StringView photonAppVersion, Verbose verbose = Verbose::Yes, ConnectionProtocol protocol = ConnectionProtocol::Default);

		SIV3D_NODISCARD_CXX20
			Multiplayer_Photon(std::string_view secretPhotonAppID, StringView photonAppVersion, const std::function<void(StringView)>& logger = {},const Verbose verbose = Verbose::Yes, ConnectionProtocol protocol = ConnectionProtocol::Default);


		/// @brief デストラクタ
		virtual ~Multiplayer_Photon();

		/// @brief マルチプレイヤー用クラスを作成します。
		/// @param secretPhotonAppID Photon アプリケーション ID
		/// @param photonAppVersion アプリケーションのバージョン
		/// @param verbose デバッグ用の Print 出力をする場合 Verbose::Yes, それ以外の場合は Verbose::No
		/// @param protocol 通信に用いるプロトコル
		/// @remark アプリケーションバージョンが異なるプレイヤーとの通信はできません。
		void init(StringView secretPhotonAppID, StringView photonAppVersion, Verbose verbose = Verbose::Yes, ConnectionProtocol protocol = ConnectionProtocol::Default);


		/// @brief マルチプレイヤー用クラスを作成します。
		/// @param secretPhotonAppID Photon アプリケーション ID
		/// @param photonAppVersion アプリケーションのバージョン
		/// @param logger ログ出力関数
		/// @param verbose デバッグ用の logger 出力をする場合 Verbose::Yes, それ以外の場合は Verbose::No
		/// @param protocol 通信に用いるプロトコル
		/// @remark アプリケーションバージョンが異なるプレイヤーとの通信はできません。
		void init(StringView secretPhotonAppID, StringView photonAppVersion, const std::function<void(StringView)>& logger = {}, const Verbose verbose = Verbose::Yes, ConnectionProtocol protocol = ConnectionProtocol::Default);

		/// @brief 自身のユーザ名を設定します。
		/// @param name 自身のユーザ名
		void setUserName(StringView name);

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

		/// @brief サーバのタイムスタンプ（ミリ秒）を返します。
		/// @return サーバのタイムスタンプ（ミリ秒）
		[[nodiscard]]
		int32 getServerTimeMillisec() const;

		/// @brief サーバのタイムスタンプとクライアントのシステムのタイムスタンプのオフセット（ミリ秒）を返します。
		/// @return サーバのタイムスタンプとクライアントのシステムのタイムスタンプのオフセット（ミリ秒）
		/// @remark Multiplayer_Photon::GetSystemTimeMillisec() の戻り値と足した値がサーバのタイムスタンプと一致します。
		[[nodiscard]]
		int32 getServerTimeOffsetMillisec() const;

		/// @brief サーバーとのラウンドトリップタイムを取得します。
		/// @return サーバーとのラウンドトリップタイム
		[[nodiscard]]
		int32 getPingMillisec() const;

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
		/// @brief 受信したデータのサイズ（バイト）を返します。
		/// @return 受信したデータのサイズ（バイト）
		/// @remark この関数は Web 版では利用できません。
		[[nodiscard]]
		int32 getBytesIn() const = delete;

		/// @brief 送信したデータのサイズ（バイト）を返します。
		/// @return 送信したデータのサイズ（バイト）
		/// @remark この関数は Web 版では利用できません。
		[[nodiscard]]
		int32 getBytesOut() const = delete;
# endif

		/// @brief ランダムなルームに参加を試みます。
		/// @param expectedMaxPlayers 最大人数が指定されたものと一致するルームにのみ参加を試みます。（0の場合は指定なし）
		/// @param matchmakingMode マッチメイキングモード
		/// @return リクエストに成功してコールバックが呼ばれる場合 true、それ以外の場合は false
		/// @remark maxPlayers は 最大 255, 無料の Photon アカウントの場合は 20
		bool joinRandomRoom(int32 expectedMaxPlayers = 0, MatchmakingMode matchmakingMode = MatchmakingMode::FillOldestRoom);

		/// @brief ランダムなルームに参加を試みます。
		/// @param propertyFilter ルームプロパティのフィルタ
		/// @param expectedMaxPlayers 最大人数が指定されたものと一致するルームにのみ参加を試みます。（0の場合は指定なし）
		/// @param matchmakingMode マッチメイキングモード
		/// @return リクエストに成功してコールバックが呼ばれる場合 true、それ以外の場合は false
		/// @remark maxPlayers は 最大 255, 無料の Photon アカウントの場合は 20
		bool joinRandomRoom(const HashTable<String, String>& propertyFilter, int32 expectedMaxPlayers = 0, MatchmakingMode matchmakingMode = MatchmakingMode::FillOldestRoom);

		/// @brief ランダムなルームに参加を試み、参加できるルームが無かった場合にルームの作成を試みます。
		/// @param expectedMaxPlayers 最大人数が指定されたものと一致するルームにのみ参加を試みます。（0の場合は指定なし）
		/// @param roomName ルーム名
		/// @return リクエストに成功してコールバックが呼ばれる場合 true、それ以外の場合は false
		[[deprecated("This overload has been deprecated.")]]
		bool joinRandomOrCreateRoom(int32 expectedMaxPlayers, RoomNameView roomName);

		/// @brief ランダムなルームに参加を試み、参加できるルームが無かった場合にルームの作成を試みます。
		/// @param roomName ルーム名
		/// @param roomCreateOption ルーム作成オプション
		/// @param propertyFilter ルームプロパティのフィルタ
		/// @param expectedMaxPlayers 最大人数が指定されたものと一致するルームにのみ参加を試みます。（0の場合は指定なし）
		/// @param matchmakingMode マッチメイキングモード
		/// @return リクエストに成功してコールバックが呼ばれる場合 true、それ以外の場合は false
		bool joinRandomOrCreateRoom(RoomNameView roomName, const RoomCreateOption& roomCreateOption = {}, const HashTable<String, String>& propertyFilter = {}, int32 expectedMaxPlayers = 0, MatchmakingMode matchmakingMode = MatchmakingMode::FillOldestRoom);

		/// @brief 指定したルームに参加を試みます。
		/// @param roomName ルーム名
		/// @return リクエストに成功してコールバックが呼ばれる場合 true、それ以外の場合は false
		bool joinRoom(RoomNameView roomName);

		/// @brief ルームの作成を試みます。
		/// @param roomName ルーム名
		/// @param maxPlayers ルームの最大人数（0の場合は指定なし）
		/// @return リクエストに成功してコールバックが呼ばれる場合 true、それ以外の場合は false
		/// @remark maxPlayers は 最大 255, 無料の Photon アカウントの場合は 20
		bool createRoom(RoomNameView roomName, int32 maxPlayers = 0);

		/// @brief ルームの作成を試みます。
		/// @param roomName ルーム名
		/// @param option ルーム作成オプション
		/// @return リクエストに成功してコールバックが呼ばれる場合 true、それ以外の場合は false
		bool createRoom(RoomNameView roomName, const RoomCreateOption& option);

		/// @brief 指定した名前のルームに参加を試み、無かった場合にルームの作成を試みます。
		/// @param roomName ルーム名
		/// @param option ルーム作成オプション
		/// @return リクエストに成功してコールバックが呼ばれる場合 true、それ以外の場合は false
		bool joinOrCreateRoom(RoomNameView roomName, const RoomCreateOption& option);

		/// @brief 切断状態から、以前に参加していたルームに再参加を試みます。再参加可能な時間を過ぎている場合は失敗します。
		/// @return リクエストに成功してコールバックが呼ばれる場合 true、それ以外の場合は false
		bool reconnectAndRejoin();

		/// @brief ルームからの退出を試みます。
		/// @param willComeBack 退出後にreconnectAndRejoin()で再参加する場合 true
		/// @return リクエストに成功してコールバックが呼ばれる場合 true、それ以外の場合は false
		bool leaveRoom(bool willComeBack = false);

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

# if SIV3D_MULTIPLAYER_PHOTON_LAGACY == 1

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
		void removeEventCache(uint8 eventCode = 0);

		/// @brief キャッシュされたイベントを削除します。targetsで指定したプレイヤーに紐づくイベントのみ削除します。
		/// @param eventCode 削除するイベントコード, 0 の場合は全てのイベントを削除
		/// @param targets 対象のプレイヤーのローカル ID 0 の場合はRoomCache, それ以外の場合は対応したPlayerCacheを指します。
		/// @remark プレイヤーに紐づくイベントとは、EventReceiverOption::○○○_CacheWithPlayerによってキャッシュされたイベントのことです。○○○_CacheWithRoomによってキャッシュされたイベントはtargetsに0を指定することで削除できます。
		void removeEventCache(uint8 eventCode, const Array<LocalPlayerID>& targets);

		/// @brief 自身のプレイヤー情報を返します。
		LocalPlayer getLocalPlayer() const;

		/// @brief 自身のユーザ名を返します。
		/// @return 自身のユーザ名
		[[nodiscard]]
		String getUserName() const;

		/// @brief 自身のユーザ ID を取得します。
		/// @return 自身のユーザ ID
		/// @remark ユーザ ID はconnect()を呼びだした後は変更することができません。
		/// @remark ユーザー ID が未指定の場合にはユーザー名から自動的に生成されます。
		[[nodiscard]]
		String getUserID() const;

		/// @brief ルーム内でのプレイヤー ID を返します。
		/// @return ルーム内でのプレイヤー ID, ルームに参加していない場合は -1
		[[nodiscard]]
		LocalPlayerID getLocalPlayerID() const;

		/// @brief ホストプレイヤーのローカルプレイヤー ID を返します。
		/// @return ホストプレイヤーのローカルプレイヤー ID, ルームに参加していない場合は -1
		[[nodiscard]]
		LocalPlayerID getHostLocalPlayerID() const;

		/// @brief 存在するルームの情報リストを返します。 
		/// @return 存在するルームの情報リスト
		[[nodiscard]]
		Array<RoomInfo> getRoomList() const;

		/// @brief 存在するルームの名前一覧を返します。
		/// @return 存在するルームの名前一覧
		[[nodiscard]]
		Array<RoomName> getRoomNameList() const;

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

		/// @brief クライアントの状態を返します。
		/// @return 現在のクライアントの状態
		[[nodiscard]]
		ClientState getClientState() const;

		/// @brief 現在参加しているルームの情報を返します。
		/// @return 現在のルームの情報。
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

		/// @brief プレイヤープロパティを取得します。
		/// @param localPlayerID ルーム内のローカルプレイヤー ID
		/// @param key キー
		String getPlayerProperty(LocalPlayerID localPlayerID, StringView key) const;

		/// @brief プレイヤープロパティを取得します。
		/// @param localPlayerID ルーム内のローカルプレイヤー ID
		HashTable<String, String> getPlayerProperties(LocalPlayerID localPlayerID) const;

		/// @brief 自身のプレイヤープロパティを追加します。
		/// @param key キー
		/// @param value 値
		void addPlayerProperty(StringView key, StringView value);

		/// @brief 自身のプレイヤープロパティを削除します。
		/// @param key キー
		void removePlayerProperty(StringView key);

		/// @brief 自身のプレイヤープロパティを削除します。
		/// @param keys キーリスト
		void removePlayerProperty(const Array<String>& keys);

		/// @brief ルームプロパティを取得します。
		/// @param key キー
		String getRoomProperty(StringView key) const;

		/// @brief ルームプロパティを取得します。
		HashTable<String, String> getRoomProperties() const;

		/// @brief ルームプロパティを追加します。
		/// @param key キー
		/// @param value 値
		void addRoomProperty(StringView key, StringView value);

		/// @brief ルームプロパティを削除します。
		/// @param key キー
		void removeRoomProperty(StringView key);

		/// @brief ルームプロパティを削除します。
		/// @param keys キーリスト
		void removeRoomProperty(const Array<String>& keys);

		/// @brief ロビーから参照可能なルームプロパティのキーリストを返します。
		/// @return ロビーから参照可能なルームプロパティのキーリスト
		Array<String> getVisibleRoomPropertyKeys() const;

		/// @brief ロビーから参照可能なルームプロパティのキーリストを設定します。
		/// @param keys キーリスト
		void setVisibleRoomPropertyKeys(const Array<String>& keys);

		/// @brief 新たなルームのホストを設定します。
		/// @param playerID 新たなルームのホストのローカルプレイヤー ID
		void setHost(LocalPlayerID playerID);

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

		/// @brief 自分が現在のルームのホストであるかを返します。
		/// @return 自分が現在のルームのホストである場合 true, それ以外の場合は false
		[[nodiscard]]
		bool isHost() const;

		/// @brief update() を呼ぶ必要がある状態であるかを返します。
		/// @return  update() を呼ぶ必要がある状態である場合 true, それ以外の場合は false
		[[nodiscard]]
		bool isActive() const noexcept;

		/// @brief サーバへの接続に失敗したときに呼ばれます。
		/// @param errorCode エラーコード
		virtual void connectionErrorReturn(int32 errorCode);

		/// @brief サーバに接続を試みた結果が通知されるときに呼ばれます。
		/// @param errorCode エラーコード。0 の場合には成功
		/// @param errorString エラー文字列
		/// @param region 接続した地域
		/// @param cluster クラスター
		/// @remark Web 版では region は常に connect 時に設定された文字列で、cluster は常に空文字列です。
		virtual void connectReturn(int32 errorCode, const String& errorString, const String& region, const String& cluster);

		/// @brief サーバから切断したときに呼ばれます。
		virtual void disconnectReturn();

		/// @brief 自身がルームから退出したときに呼ばれます。
		/// @param errorCode エラーコード。0 の場合には成功
		/// @param errorString エラー文字列
		virtual void leaveRoomReturn(int32 errorCode, const String& errorString);

		/// @brief ランダムなルームへの参加を試みた結果が通知されるときに呼ばれます。
		/// @param playerID ルーム内のローカルプレイヤー ID
		/// @param errorCode エラーコード。0 の場合には成功
		/// @param errorString エラー文字列
		virtual void joinRandomRoomReturn(LocalPlayerID playerID, int32 errorCode, const String& errorString);

		/// @brief ルームへの参加を試みた結果が通知されるときに呼ばれます。
		/// @param playerID ルーム内のローカルプレイヤー ID
		/// @param errorCode エラーコード。0 の場合には成功
		/// @param errorString エラー文字列
		virtual void joinRoomReturn(LocalPlayerID playerID, int32 errorCode, const String& errorString);

		/// @brief 誰か（自分を含む）が現在のルームに参加したときに呼ばれます。
		/// @param newPlayer 参加者の情報
		/// @param playerIDs ルーム内のプレイヤー全員のローカルプレイヤー ID
		/// @param isSelf 参加したのが自分である場合 true, それ以外の場合は false
		virtual void joinRoomEventAction(const LocalPlayer& newPlayer, const Array<LocalPlayerID>& playerIDs, bool isSelf);

		/// @brief 現在参加しているルームから誰かが退出したときに呼ばれます。
		/// @param playerID 退出者のローカルプレイヤー ID
		/// @param isInactive 退出者が再参加できる場合 true, それ以外の場合は false
		virtual void leaveRoomEventAction(LocalPlayerID playerID, bool isInactive);

		/// @brief ルームの作成を試みた結果が通知されるときに呼ばれます。
		/// @param playerID 自身のローカルプレイヤー ID
		/// @param errorCode エラーコード。0 の場合には成功
		/// @param errorString エラー文字列
		virtual void createRoomReturn(LocalPlayerID playerID, int32 errorCode, const String& errorString);

		/// @brief ルームへの参加またはルームの作成を試みた結果が通知されるときに呼ばれます。
		/// @param playerID 自身のローカルプレイヤー ID
		/// @param errorCode エラーコード
		/// @param errorString エラー文字列
		virtual void joinOrCreateRoomReturn(LocalPlayerID playerID, int32 errorCode, const String& errorString);

		/// @brief ランダムなルームへの参加またはルームの作成を試みた結果が通知されるときに呼ばれます。
		/// @param playerID 自身のローカルプレイヤー ID
		/// @param errorCode エラーコード。0 の場合には成功
		/// @param errorString エラー文字列
		virtual void joinRandomOrCreateRoomReturn(LocalPlayerID playerID, int32 errorCode, const String& errorString);

		/// @brief ルームのリストが更新されたときに呼ばれます。
		virtual void onRoomListUpdate();

		/// @brief ルームのプロパティが変更されたときに呼ばれます。
		/// @param changes 変更されたプロパティのキーと値
		virtual void onRoomPropertiesChange(const HashTable<String, String>& changes);

		/// @brief プレイヤーのプロパティが変更されたときに呼ばれます。
		/// @param playerID 変更されたプレイヤーのローカルプレイヤー ID
		/// @param changes 変更されたプロパティのキーと値
		virtual void onPlayerPropertiesChange(LocalPlayerID playerID, const HashTable<String, String>& changes);

		/// @brief ホストが変更されたときに呼ばれます。
		/// @param newHostPlayerID 新しいホストのローカルプレイヤー ID
		/// @param oldHostPlayerID 古いホストのローカルプレイヤー ID
		virtual void onHostChange(LocalPlayerID newHostPlayerID, LocalPlayerID oldHostPlayerID);

		/// @brief ルームのイベントを受信した際に呼ばれます。
		/// @param playerID 送信者のローカルプレイヤー ID
		/// @param eventCode イベントコード
		/// @param data 受信したデータ
		/// @remark ユーザ定義型を受信する際に利用します。
		virtual void customEventAction(LocalPlayerID playerID, uint8 eventCode, Deserializer<MemoryViewReader>& reader);

# if SIV3D_MULTIPLAYER_PHOTON_LAGACY == 1

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

		template<class T, class... Args>
		void RegisterEventCallback(uint8 eventCode, EventCallbackType<T, Args...> callback);

		template<class... Args>
		void logger(Args&&... args) const
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
# else
		std::shared_ptr<PhotonDetail> m_detail;
# endif

		String m_secretPhotonAppID;

		String m_photonAppVersion;

		Optional<String> m_requestedRegion;

		ConnectionProtocol m_connectionProtocol = ConnectionProtocol::UDP;

		String m_lastJoinedRoomName;

		bool m_isActive = false;

		HashTable<uint8, detail::CustomEventReceiver> table;

		std::function<void(StringView)> m_logger;

		void sendEventImpl(const MultiplayerEvent& event, const Serializer<MemoryWriter>& writer);
	};

	namespace detail
	{
		template<class T, class... Args>
		struct WrapperImpl
		{
			static void wrapper(Multiplayer_Photon& client, TypeErasedCallback callback, LocalPlayerID player, Deserializer<MemoryViewReader>& reader)
			{
				std::tuple<std::remove_cvref_t<Args>...> args{};
				impl(static_cast<T&>(client), callback, player, reader, args, std::make_index_sequence<std::tuple_size_v<std::tuple<Args...>>>());
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
		sendEventImpl(event, Serializer<MemoryWriter> {}(args...));
	}

	template<class T, class ...Args>
	void Multiplayer_Photon::RegisterEventCallback(uint8 eventCode, Multiplayer_Photon::EventCallbackType<T, Args...> callback)
	{
		table[eventCode] = detail::CustomEventReceiver(reinterpret_cast<detail::TypeErasedCallback>(callback), &detail::WrapperImpl<T, Args...>::wrapper);
	}
}
