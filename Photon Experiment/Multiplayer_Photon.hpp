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

// Photono SDK クラスの前方宣言
namespace ExitGames::LoadBalancing
{
	class Listener;
	class Client;
	class RoomOptions;
}

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

	class Multiplayer_Photon;


	namespace detail
	{
		using TypeErasedCallback = void(Multiplayer_Photon::*)();
		using CallbackWrapper = void(*)(Multiplayer_Photon&, TypeErasedCallback, LocalPlayerID, Deserializer<MemoryViewReader>&);

		using CustomEventReceiver = std::pair<TypeErasedCallback, CallbackWrapper>;
	}

	class RoomCreateOption
	{
	public:
		[[nodiscard]]
		RoomCreateOption() = default;

		[[nodiscard]]
		explicit RoomCreateOption(int32 maxPlayers, bool isVisible = true, bool isOpen = true, const HashTable<String, String>& properties = {}, const Array<String>& visibleRoomPropertyKeys = {}, int32 reconnectableGraceMilliseconds = 0, int32 emptyRoomLifeMilliseconds = 0, bool publishUserId = true);

		RoomCreateOption& isVisible(bool isVisible);

		RoomCreateOption& isOpen(bool isOpen);

		RoomCreateOption& publishUserId(bool publishUserId);

		RoomCreateOption& maxPlayers(int32 maxPlayers);

		RoomCreateOption& properties(const HashTable<String, String>& properties);

		RoomCreateOption& visibleRoomPropertyKeys(const Array<String>& visibleRoomPropertyKeys);

		RoomCreateOption& reconnectableGraceMilliseconds(int32 reconnectableGraceMilliseconds);

		RoomCreateOption& emptyRoomLifeMilliseconds(int32 emptyRoomLifeMilliseconds);

	private:
		bool m_isVisible = true;

		bool m_isOpen = true;

		bool m_publishUserId = true;

		int32 m_maxPlayers = 0;

		HashTable<String, String> m_properties;

		Array<String> m_visibleRoomPropertyKeys;

		int32 m_reconnectableGraceMilliseconds = 0;

		int32 m_emptyRoomLifeMilliseconds = 0;

		ExitGames::LoadBalancing::RoomOptions toRoomOptions() const;

		friend class Multiplayer_Photon;
	};

	enum class MatchmakingMode : uint8
	{
		FillOldestRoom,
		Serial,
		Random,
	};

	/// @brief ターゲット指定オプション。キャッシュを利用すると以降に入室するプレイヤーにも送信されます。
	enum class EventReceiverOption : uint8
	{
		/// @brief 自分以外のプレイヤーに送信
		Others,

		/// @brief 自分以外のプレイヤーに送信。送信者が部屋を離れるまでキャッシュされます。
		Others_CacheUntilLeaveRoom,

		/// @brief 自分以外のプレイヤーに送信。永続的にキャッシュされます。
		Others_CacheForever,

		/// @brief 全員に送信
		All,

		/// @brief 全員に送信。送信者が部屋を離れるまでキャッシュされます。
		All_CacheUntilLeaveRoom,

		/// @brief 全員に送信。永続的にキャッシュされます。
		All_CacheForever,

		/// @brief ホストに送信
		Host,
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
		[[nodiscard]]
		explicit MultiplayerEvent(uint8 eventCode, EventReceiverOption receiverOption = EventReceiverOption::Others, uint8 priorityIndex = 0);

		/// @brief 送信するイベントの情報を作成します。
		/// @param eventCode イベントコード (1~199)
		/// @param targetList ターゲットリスト
		/// @param priorityIndex プライオリティインデックス　0に近いほど優先的に処理される
		[[nodiscard]]
		MultiplayerEvent(uint8 eventCode, Array<LocalPlayerID> targetList, uint8 priorityIndex = 0);

		/// @brief 送信するイベントの情報を作成します。
		/// @param eventCode イベントコード (1~199)
		/// @param targetGroup ターゲットグループ (1以上255以下の整数)
		/// @param priorityIndex プライオリティインデックス　0に近いほど優先的に処理される
		[[nodiscard]]
		MultiplayerEvent(uint8 eventCode, uint8 targetGroup, uint8 priorityIndex = 0);

	private:
		/// @brief イベントコード (1~199)
		uint8 m_eventCode = 0;

		/// @brief プライオリティインデックス　0に近いほど優先的に処理される
		uint8 m_priorityIndex = 0;

		/// @brief ターゲットグループ (1以上255以下の整数)
		uint8 m_targetGroup = 0;

		/// @brief ターゲット指定オプション
		EventReceiverOption m_receiverOption = EventReceiverOption::Others;

		/// @brief ターゲットリスト
		Optional<Array<LocalPlayerID>> m_targetList;

		friend class Multiplayer_Photon;
	};

	/// @brief マルチプレイヤー用クラス (Photon バックエンド)
	class Multiplayer_Photon
	{
	public:

		enum class NetworkState : uint8 {
			Disconnected,
			ConnectingToLobby,
			InLobby,
			JoiningRoom,
			InRoom,
			LeavingRoom,
			Disconnecting,
		};

		/// @brief デフォルトコンストラクタ
		SIV3D_NODISCARD_CXX20
			Multiplayer_Photon() = default;

		/// @brief マルチプレイヤー用クラスを作成します。
		/// @param secretPhotonAppID Photon アプリケーション ID
		/// @param photonAppVersion アプリケーションのバージョン
		/// @param デバッグ用の Print 出力をする場合 Verbose::Yes, それ以外の場合は Verbose::No
		/// @remark アプリケーションバージョンが異なるプレイヤーとの通信はできません。
		SIV3D_NODISCARD_CXX20
			Multiplayer_Photon(std::string_view secretPhotonAppID, StringView photonAppVersion, Verbose verbose = Verbose::Yes);

		/// @brief デストラクタ
		virtual ~Multiplayer_Photon();

		/// @brief マルチプレイヤー用クラスを作成します。
		/// @param secretPhotonAppID Photon アプリケーション ID
		/// @param photonAppVersion アプリケーションのバージョン
		/// @param verbose デバッグ用の Print 出力をする場合 Verbose::Yes, それ以外の場合は Verbose::No
		/// @remark アプリケーションバージョンが異なるプレイヤーとの通信はできません。
		void init(StringView secretPhotonAppID, StringView photonAppVersion, Verbose verbose = Verbose::Yes);

		/// @brief Photon サーバへの接続を試みます。
		/// @param userName ユーザ名
		/// @param region 接続するサーバのリージョン。unspecified の場合は利用可能なサーバのうち最速のものが選択されます
		/// @remark リージョンは https://doc.photonengine.com/en-us/pun/current/connection-and-authentication/regions を参照してください。
		void connect(StringView userName, const Optional<String>& region = unspecified);

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

		/// @brief 受信したデータのサイズ（バイト）を返します。
		/// @return 受信したデータのサイズ（バイト）
		[[nodiscard]]
		int32 getBytesIn() const;

		/// @brief 送信したデータのサイズ（バイト）を返します。
		/// @return 送信したデータのサイズ（バイト）
		[[nodiscard]]
		int32 getBytesOut() const;

		/// @brief ランダムなルームに参加を試みます。
		/// @param expectedMaxPlayers 最大人数が指定されたものと一致するルームにのみ参加を試みます。（0の場合は指定なし）
		/// @param matchmakingMode マッチメイキングモード
		/// @remark maxPlayers は 最大 255, 無料の Photon アカウントの場合は 20
		void joinRandomRoom(int32 expectedMaxPlayers = 0, MatchmakingMode matchmakingMode = MatchmakingMode::FillOldestRoom);

		/// @brief ランダムなルームに参加を試みます。
		/// @param propertyFilter ルームプロパティのフィルタ
		/// @param expectedMaxPlayers 最大人数が指定されたものと一致するルームにのみ参加を試みます。（0の場合は指定なし）
		/// @param matchmakingMode マッチメイキングモード
		/// @remark maxPlayers は 最大 255, 無料の Photon アカウントの場合は 20
		void joinRandomRoom(const HashTable<String,String>& propertyFilter, int32 expectedMaxPlayers = 0, MatchmakingMode matchmakingMode = MatchmakingMode::FillOldestRoom);

		/// @brief ランダムなルームに参加を試み、参加できるルームが無かった場合にルームの作成を試みます。
		/// @param expectedMaxPlayers 最大人数が指定されたものと一致するルームにのみ参加を試みます。（0の場合は指定なし）
		/// @param roomName ルーム名
		[[deprecated("This overload has been deprecated. Use another overload instead.")]]
		void joinRandomOrCreateRoom(int32 expectedMaxPlayers, RoomNameView roomName);

		/// @brief ランダムなルームに参加を試み、参加できるルームが無かった場合にルームの作成を試みます。
		/// @param roomName ルーム名
		/// @param roomCreateOption ルーム作成オプション
		/// @param propertyFilter ルームプロパティのフィルタ
		/// @param expectedMaxPlayers 最大人数が指定されたものと一致するルームにのみ参加を試みます。（0の場合は指定なし）
		/// @param matchmakingMode マッチメイキングモード
		void joinRandomOrCreateRoom(RoomNameView roomName, const RoomCreateOption& roomCreateOption = {}, const HashTable<String, String>& propertyFilter = {}, int32 expectedMaxPlayers = 0, MatchmakingMode matchmakingMode = MatchmakingMode::FillOldestRoom);

		/// @brief 指定したルームに参加を試みます。
		/// @param roomName ルーム名
		void joinRoom(RoomNameView roomName);

		/// @brief ルームの作成を試みます。
		/// @param roomName ルーム名
		/// @param maxPlayers ルームの最大人数（0の場合は指定なし）
		/// @remark maxPlayers は 最大 255, 無料の Photon アカウントの場合は 20
		void createRoom(RoomNameView roomName, int32 maxPlayers = 0);

		/// @brief ルームの作成を試みます。
		/// @param roomName ルーム名
		/// @param option ルーム作成オプション
		void createRoom(RoomNameView roomName, const RoomCreateOption& option);

		/// @brief 指定した名前のルームに参加を試み、無かった場合にルームの作成を試みます。
		/// @param roomName ルーム名
		/// @param option ルーム作成オプション
		void joinOrCreateRoom(RoomNameView roomName, const RoomCreateOption& option);

		/// @brief ルームからの退出を試みます。
		void leaveRoom();

		/// @brief 指定したイベントターゲットグループに参加します。
		/// @param targetGroup ターゲットグループ　(1以上255以下の整数)
		void joinEventTargetGroup(const uint8 targetGroup);

		/// @brief 指定したイベントターゲットグループに参加します。
		/// @param targetGroups ターゲットグループの配列　(1以上255以下の整数)
		void joinEventTargetGroup(const Array<uint8>& targetGroups);

		/// @brief 指定したイベントターゲットグループから退出します。
		/// @param targetGroup ターゲットグループ　(1以上255以下の整数)
		void leaveEventTargetGroup(const uint8 targetGroup);

		/// @brief 指定したイベントターゲットグループから退出します。
		/// @param targetGroups ターゲットグループの配列　(1以上255以下の整数)
		void leaveEventTargetGroup(const Array<uint8>& targetGroups);

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

		/*template<class... Args>
		void sendEvent(uint8 eventCode, const Optional<Array<LocalPlayerID>>& targets = unspecified, Args... args)
		{
			sendEvent(eventCode, Serializer<MemoryWriter>{}(args...), targets);
		}*/

		template<class... Args>
		void sendEvent(const MultiplayerEvent& event, Args... args)
		{
			sendEventImpl(event, Serializer<MemoryWriter>{}(args...));
		}
	private:
		void sendEventImpl(const MultiplayerEvent& event, const Serializer<MemoryWriter>& writer);
	public:

		/// @brief キャッシュされたイベントを削除します。
		/// @param eventCode 削除するイベントコード, 0 の場合は全てのイベントを削除
		void removeEventCache(uint8 eventCode = 0);

		/// @brief キャッシュされたイベントを削除します。targetsで指定したプレイヤーに紐づくイベントのみ削除します。
		/// @param eventCode 削除するイベントコード, 0 の場合は全てのイベントを削除
		/// @param targets 対象のプレイヤーのローカル ID
		/// @remark プレイヤーに紐づくイベントとは、EventReceiverOption::○○○_CacheUntilLeaveRoomによってキャッシュされたイベントのことです。
		void removeEventCache(uint8 eventCode, Array<LocalPlayerID> targets);

		/// @brief 自身のユーザ名を返します。
		/// @return 自身のユーザ名
		[[nodiscard]]
		String getUserName() const;

		/// @brief 自身のユーザ名を設定します。
		/// @param userName ユーザ名
		void setUserName(StringView userName);

		/// @brief 自動生成された自身のユーザ ID を取得します。
		/// @return 自身のユーザ ID
		[[nodiscard]]
		String getUserID() const;

		/// @brief ルーム内でのプレイヤー ID を返します。
		/// @return ルーム内でのプレイヤー ID, ルームに参加していない場合は -1
		[[nodiscard]]
		LocalPlayerID getLocalPlayerID() const;

		/// @brief ルーム内のホストのプレイヤー ID を返します。
		/// @return ルーム内のホストのプレイヤー ID, ルームに参加していない場合は -1
		[[nodiscard]]
		LocalPlayerID getHostLocalPlayerID() const;

		/// @brief 存在するルームの名前一覧を返します。
		/// @return 存在するルームの名前一覧
		[[nodiscard]]
		Array<RoomName> getRoomNameList() const;

		/// @brief 存在するルームの情報一覧を返します。
		/// @return 存在するルームの情報一覧
		[[nodiscard]]
		Array<RoomInfo> getRoomInfoList() const;

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

		NetworkState getNetworkState() const;

		int32 getState() const;

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

		/// @brief 自身のプレイヤープロパティを設定します。
		/// @param key キー
		/// @param value 値
		void setPlayerProperty(StringView key, StringView value);

		/// @brief 自身のプレイヤープロパティを削除します。
		/// @param key キー
		void removePlayerProperty(StringView key);

		/// @brief ルームプロパティを取得します。
		/// @param key キー
		String getRoomProperty(StringView key) const;

		/// @brief ルームプロパティを設定します。
		/// @param key キー
		/// @param value 値
		void setRoomProperty(StringView key, StringView value);

		/// @brief ルームプロパティを削除します。
		/// @param key キー
		void removeRoomProperty(StringView key);

		/// @brief ロビーから参照可能なルームプロパティのキーリストを返します。
		/// @return ロビーから参照可能なルームプロパティのキーリスト
		Array<String> getVisibleRoomPropertyKeys() const;

		/// @brief ロビーから参照可能なルームプロパティのキーリストを設定します。
		/// @param keys キーリスト
		void setVisibleRoomPropertyKeys(const Array<String>& keys);

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
		/// @param errorCode エラーコード
		/// @param errorString エラー文字列
		/// @param region 接続した地域
		/// @param cluster クラスター
		virtual void connectReturn(int32 errorCode, const String& errorString, const String& region, const String& cluster);

		/// @brief サーバから切断したときに呼ばれます。
		virtual void disconnectReturn();

		/// @brief 自身がルームから退出したときに呼ばれます。
		/// @param errorCode エラーコード
		/// @param errorString エラー文字列
		virtual void leaveRoomReturn(int32 errorCode, const String& errorString);

		/// @brief ランダムなルームへの参加を試みた結果が通知されるときに呼ばれます。
		/// @param playerID ルーム内のローカルプレイヤー ID
		/// @param errorCode エラーコード
		/// @param errorString エラー文字列
		virtual void joinRandomRoomReturn(LocalPlayerID playerID, int32 errorCode, const String& errorString);

		/// @brief ルームへの参加を試みた結果が通知されるときに呼ばれます。
		/// @param playerID ルーム内のローカルプレイヤー ID
		/// @param errorCode エラーコード
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
		/// @param errorCode エラーコード
		/// @param errorString エラー文字列
		virtual void createRoomReturn(LocalPlayerID playerID, int32 errorCode, const String& errorString);

		/// @brief ランダムなルームへの参加またはルームの作成を試みた結果が通知されるときに呼ばれます。
		/// @param playerID 自身のローカルプレイヤー ID
		/// @param errorCode エラーコード
		/// @param errorString エラー文字列
		virtual void joinRandomOrCreateRoomReturn(LocalPlayerID playerID, int32 errorCode, const String& errorString);

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

		/// @brief ルームのイベントを受信した際に呼ばれます。
		/// @param playerID 送信者のローカルプレイヤー ID
		/// @param eventCode イベントコード
		/// @param data 受信したデータ
		/// @remark ユーザ定義型を受信する際に利用します。
		virtual void customEventAction(LocalPlayerID playerID, uint8 eventCode, Deserializer<MemoryViewReader>& reader);

		/// @brief クライアントのシステムのタイムスタンプ（ミリ秒）を返します。
		/// @return クライアントのシステムのタイムスタンプ（ミリ秒）
		/// @remark この値に getServerTimeOffsetMillisec() の戻り値と足した値がサーバのタイムスタンプと一致します。
		[[nodiscard]]
		static int32 GetSystemTimeMillisec();

		template<class T, class... Args>
		using EventCallbackType = void (T::*)(LocalPlayerID, Args...);

		template<class T, class... Args>
		void RegisterEventCallback(uint8 eventCode, EventCallbackType<T, Args...> callback);

	protected:

		/// @brief 既存のランダムマッチが見つからなかった時のエラーコード
		/// @remark `joinRandomRoomReturn()` で使います。
		static constexpr int32 NoRandomMatchFound = (0x7FFF - 7);

		/// @brief Verbose モード (Print による詳細なデバッグ出力をする場合 true)
		bool m_verbose = true;

	private:

		class PhotonDetail;

		std::unique_ptr<ExitGames::LoadBalancing::Listener> m_listener;

		std::unique_ptr<ExitGames::LoadBalancing::Client> m_client;

		String m_secretPhotonAppID;

		String m_photonAppVersion;

		Optional<String> m_requestedRegion;

		HashTable<uint8, detail::CustomEventReceiver> table;

		bool m_isActive = false;
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

	template<class T, class ...Args>
	void Multiplayer_Photon::RegisterEventCallback(uint8 eventCode, Multiplayer_Photon::EventCallbackType<T, Args...> callback)
	{

		table[eventCode] = detail::CustomEventReceiver(reinterpret_cast<detail::TypeErasedCallback>(callback), &detail::WrapperImpl<T, Args...>::wrapper);
	}
}
