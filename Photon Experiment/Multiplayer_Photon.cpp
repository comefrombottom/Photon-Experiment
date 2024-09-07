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

# define NOMINMAX
# include <LoadBalancing-cpp/inc/Client.h>
# include "Multiplayer_Photon.hpp"

namespace s3d::detail {
	static void LogIfError(const Multiplayer_Photon& photon, const int32 errorCode, const StringView errorString)
	{
		if (errorCode)
		{
			photon.debugLog(U"- [Multiplayer_Photon] errorCode: ", errorCode);
			photon.debugLog(U"- [Multiplayer_Photon] errorString: ", errorString);
		}
	}
}

// detail, CustomType_Photon 
namespace s3d
{
	namespace detail
	{
		[[nodiscard]]
		static String ToString(const ExitGames::Common::JString& s)
		{
			return Unicode::FromWstring(std::wstring_view{ s.cstr(), s.length() });
		}

		[[nodiscard]]
		static ExitGames::Common::JString ToJString(const StringView s)
		{
			return ExitGames::Common::JString{ Unicode::ToWstring(s).c_str() };
		}

		[[nodiscard]]
		static ExitGames::Common::JString ObjectToJString(const ExitGames::Common::Object& obj)
		{
			return ExitGames::Common::ValueObject<ExitGames::Common::JString>(obj).getDataCopy();
		}

		[[nodiscard]]
		static HashTable<uint8, String> PhotonHashtableToStringHashTable(const ExitGames::Common::Hashtable& data)
		{
			HashTable<uint8, String> result {};

			const auto& keys = data.getKeys();

			for (uint32 i = 0; i < keys.getSize(); ++i)
			{
				const ExitGames::Common::JString key = ObjectToJString(keys[i]);
				const ExitGames::Common::JString value = ObjectToJString(*data.getValue(key));
				result[static_cast<uint8>(key.charAt(0))] = detail::ToString(value);
			}

			return result;
		}

		[[nodiscard]]
		static ExitGames::Common::Hashtable ToPhotonHashtable(const HashTable<uint8, String>& table)
		{
			ExitGames::Common::Hashtable result;

			for (const auto& [key, value] : table)
			{
				result.put(ToJString(String(1, static_cast<char32>(key))), ToJString(value));
			}

			return result;
		}

		[[nodiscard]]
		static Array<String> ToStringArray(const ExitGames::Common::JVector<ExitGames::Common::JString>& data)
		{
			Array<String> result(data.getSize());

			for (uint32 i = 0; i < data.getSize(); ++i)
			{
				result[i] = ToString(data[i]);
			}

			return result;
		}

		[[nodiscard]]
		static ExitGames::Common::JVector<ExitGames::Common::JString> ToJStringJVector(const Array<String>& data)
		{
			ExitGames::Common::JVector<ExitGames::Common::JString> result(static_cast<uint32>(data.size()));

			for (size_t i = 0; i < data.size(); ++i)
			{
				result.addElement(ToJString(data[i]));
			}

			return result;
		}

		[[nodiscard]]
		static ExitGames::LoadBalancing::RoomOptions ToRoomOptions(const RoomCreateOption& option)
		{
			ExitGames::LoadBalancing::RoomOptions roomOptions;
			roomOptions.setMaxPlayers(static_cast<uint8>(option.maxPlayers()));
			roomOptions.setIsVisible(option.isVisible());
			roomOptions.setIsOpen(option.isOpen());
			roomOptions.setCustomRoomProperties(ToPhotonHashtable(option.properties()));
			roomOptions.setPublishUserID(option.publishUserId());
			if (option.rejoinGracePeriod())
			{
				roomOptions.setPlayerTtl(static_cast<int32>(option.rejoinGracePeriod().value().count()));
			}
			else
			{
				roomOptions.setPlayerTtl(-1);
			}
			roomOptions.setEmptyRoomTtl(static_cast<int32>(option.roomDestroyGracePeriod().count()));
			return roomOptions;
		}

		static void LogIfError(const Multiplayer_Photon& photon, const int32 errorCode, const ExitGames::Common::JString& errorString)
		{
			LogIfError(photon, errorCode, ToString(errorString));
		}
	}

# if SIV3D_MULTIPLAYER_PHOTON_LEGACY
	template <class Type, uint8 customTypeIndex>
	class CustomType_Photon : public ExitGames::Common::CustomType<CustomType_Photon<Type, customTypeIndex>, customTypeIndex>
	{
	public:

		SIV3D_NODISCARD_CXX20
			CustomType_Photon() = default;

		SIV3D_NODISCARD_CXX20
			explicit CustomType_Photon(const Type& value)
			: ExitGames::Common::CustomType<CustomType_Photon<Type, customTypeIndex>, customTypeIndex>{}
			, m_value{ value } {}

		SIV3D_NODISCARD_CXX20
			CustomType_Photon(const CustomType_Photon& toCopy)
			: ExitGames::Common::CustomType<CustomType_Photon<Type, customTypeIndex>, customTypeIndex>{}
			, m_value{ toCopy.m_value } {}

		virtual ~CustomType_Photon() = default;

		CustomType_Photon& operator =(const CustomType_Photon& toCopy)
		{
			m_value = toCopy.m_value;
			return *this;
		}

		void cleanup() {}

		bool compare(const ExitGames::Common::CustomTypeBase& other) const override
		{
			return (m_value == (static_cast<const CustomType_Photon&>(other)).m_value);
		}

		void duplicate(ExitGames::Common::CustomTypeBase* pRetVal) const override
		{
			*reinterpret_cast<CustomType_Photon*>(pRetVal) = *this;
		}

		void deserialize(const nByte* pData, [[maybe_unused]] const short length) override
		{
			std::memcpy(&m_value, pData, sizeof(Type));
		}

		short serialize(nByte* pRetVal) const override
		{
			if (pRetVal)
			{
				Type* data = reinterpret_cast<Type*>(pRetVal);
				std::memcpy(data, &m_value, sizeof(Type));
				pRetVal = reinterpret_cast<nByte*>(data);
			}

			return sizeof(Type);
		}

		ExitGames::Common::JString& toString(ExitGames::Common::JString& retStr, [[maybe_unused]] const bool withTypes = false) const override
		{
			return (retStr = detail::ToJString(Format(m_value)));
		}

		const Type& getValue() const noexcept
		{
			return m_value;
		}

	private:

		Type m_value{};
	};

	using PhotonColor = CustomType_Photon<Color, 0>;
	using PhotonColorF = CustomType_Photon<ColorF, 1>;
	using PhotonHSV = CustomType_Photon<HSV, 2>;
	using PhotonPoint = CustomType_Photon<Point, 3>;
	using PhotonVec2 = CustomType_Photon<Vec2, 4>;
	using PhotonVec3 = CustomType_Photon<Vec3, 5>;
	using PhotonVec4 = CustomType_Photon<Vec4, 6>;
	using PhotonFloat2 = CustomType_Photon<Float2, 7>;
	using PhotonFloat3 = CustomType_Photon<Float3, 8>;
	using PhotonFloat4 = CustomType_Photon<Float4, 9>;
	using PhotonMat3x2 = CustomType_Photon<Mat3x2, 10>;
	using PhotonRect = CustomType_Photon<Rect, 11>;
	using PhotonCircle = CustomType_Photon<Circle, 12>;
	using PhotonLine = CustomType_Photon<Line, 13>;
	using PhotonTriangle = CustomType_Photon<Triangle, 14>;
	using PhotonRectF = CustomType_Photon<RectF, 15>;
	using PhotonQuad = CustomType_Photon<Quad, 16>;
	using PhotonEllipse = CustomType_Photon<Ellipse, 17>;
	using PhotonRoundRect = CustomType_Photon<RoundRect, 18>;

	static void RegisterTypes()
	{
		PhotonColor::registerType();
		PhotonColorF::registerType();
		PhotonHSV::registerType();
		PhotonPoint::registerType();
		PhotonVec2::registerType();
		PhotonVec3::registerType();
		PhotonVec4::registerType();
		PhotonFloat2::registerType();
		PhotonFloat3::registerType();
		PhotonFloat4::registerType();
		PhotonMat3x2::registerType();
		PhotonRect::registerType();
		PhotonCircle::registerType();
		PhotonLine::registerType();
		PhotonTriangle::registerType();
		PhotonRectF::registerType();
		PhotonQuad::registerType();
		PhotonEllipse::registerType();
		PhotonRoundRect::registerType();
	}

	static void UnregisterTypes()
	{
		PhotonColor::unregisterType();
		PhotonColorF::unregisterType();
		PhotonHSV::unregisterType();
		PhotonPoint::unregisterType();
		PhotonVec2::unregisterType();
		PhotonVec3::unregisterType();
		PhotonVec4::unregisterType();
		PhotonFloat2::unregisterType();
		PhotonFloat3::unregisterType();
		PhotonFloat4::unregisterType();
		PhotonMat3x2::unregisterType();
		PhotonRect::unregisterType();
		PhotonCircle::unregisterType();
		PhotonLine::unregisterType();
		PhotonTriangle::unregisterType();
		PhotonRectF::unregisterType();
		PhotonQuad::unregisterType();
		PhotonEllipse::unregisterType();
		PhotonRoundRect::unregisterType();
	}
# endif
}

// PhotonDetail
namespace s3d
{
	class Multiplayer_Photon::PhotonDetail : public ExitGames::LoadBalancing::Listener
	{
	public:

		explicit PhotonDetail(Multiplayer_Photon& context)
			: m_context{ context }
		{
# if SIV3D_MULTIPLAYER_PHOTON_LEGACY
			m_receiveEventFunctions.emplace(uint8{ 0 }, [this](const int playerID, const nByte eventCode, const ExitGames::Common::Object& data) { receivedCustomType<Color, 0>(playerID, eventCode, data); });
			m_receiveEventFunctions.emplace(uint8{ 1 }, [this](const int playerID, const nByte eventCode, const ExitGames::Common::Object& data) { receivedCustomType<ColorF, 1>(playerID, eventCode, data); });
			m_receiveEventFunctions.emplace(uint8{ 2 }, [this](const int playerID, const nByte eventCode, const ExitGames::Common::Object& data) { receivedCustomType<HSV, 2>(playerID, eventCode, data); });
			m_receiveEventFunctions.emplace(uint8{ 3 }, [this](const int playerID, const nByte eventCode, const ExitGames::Common::Object& data) { receivedCustomType<Point, 3>(playerID, eventCode, data); });
			m_receiveEventFunctions.emplace(uint8{ 4 }, [this](const int playerID, const nByte eventCode, const ExitGames::Common::Object& data) { receivedCustomType<Vec2, 4>(playerID, eventCode, data); });
			m_receiveEventFunctions.emplace(uint8{ 5 }, [this](const int playerID, const nByte eventCode, const ExitGames::Common::Object& data) { receivedCustomType<Vec3, 5>(playerID, eventCode, data); });
			m_receiveEventFunctions.emplace(uint8{ 6 }, [this](const int playerID, const nByte eventCode, const ExitGames::Common::Object& data) { receivedCustomType<Vec4, 6>(playerID, eventCode, data); });
			m_receiveEventFunctions.emplace(uint8{ 7 }, [this](const int playerID, const nByte eventCode, const ExitGames::Common::Object& data) { receivedCustomType<Float2, 7>(playerID, eventCode, data); });
			m_receiveEventFunctions.emplace(uint8{ 8 }, [this](const int playerID, const nByte eventCode, const ExitGames::Common::Object& data) { receivedCustomType<Float3, 8>(playerID, eventCode, data); });
			m_receiveEventFunctions.emplace(uint8{ 9 }, [this](const int playerID, const nByte eventCode, const ExitGames::Common::Object& data) { receivedCustomType<Float4, 9>(playerID, eventCode, data); });
			m_receiveEventFunctions.emplace(uint8{ 10 }, [this](const int playerID, const nByte eventCode, const ExitGames::Common::Object& data) { receivedCustomType<Mat3x2, 10>(playerID, eventCode, data); });
			m_receiveEventFunctions.emplace(uint8{ 11 }, [this](const int playerID, const nByte eventCode, const ExitGames::Common::Object& data) { receivedCustomType<Rect, 11>(playerID, eventCode, data); });
			m_receiveEventFunctions.emplace(uint8{ 12 }, [this](const int playerID, const nByte eventCode, const ExitGames::Common::Object& data) { receivedCustomType<Circle, 12>(playerID, eventCode, data); });
			m_receiveEventFunctions.emplace(uint8{ 13 }, [this](const int playerID, const nByte eventCode, const ExitGames::Common::Object& data) { receivedCustomType<Line, 13>(playerID, eventCode, data); });
			m_receiveEventFunctions.emplace(uint8{ 14 }, [this](const int playerID, const nByte eventCode, const ExitGames::Common::Object& data) { receivedCustomType<Triangle, 14>(playerID, eventCode, data); });
			m_receiveEventFunctions.emplace(uint8{ 15 }, [this](const int playerID, const nByte eventCode, const ExitGames::Common::Object& data) { receivedCustomType<RectF, 15>(playerID, eventCode, data); });
			m_receiveEventFunctions.emplace(uint8{ 16 }, [this](const int playerID, const nByte eventCode, const ExitGames::Common::Object& data) { receivedCustomType<Quad, 16>(playerID, eventCode, data); });
			m_receiveEventFunctions.emplace(uint8{ 17 }, [this](const int playerID, const nByte eventCode, const ExitGames::Common::Object& data) { receivedCustomType<Ellipse, 17>(playerID, eventCode, data); });
			m_receiveEventFunctions.emplace(uint8{ 18 }, [this](const int playerID, const nByte eventCode, const ExitGames::Common::Object& data) { receivedCustomType<RoundRect, 18>(playerID, eventCode, data); });
# endif
		}

		void onAvailableRegions(const ExitGames::Common::JVector<ExitGames::Common::JString>& availableRegions, [[maybe_unused]] const ExitGames::Common::JVector<ExitGames::Common::JString>& availableRegionServers) override
		{
			const String target = m_context.m_requestedRegion->lowercased();

			for (unsigned i = 0; i < availableRegions.getSize(); ++i)
			{
				if (detail::ToString(availableRegions[i]) == target)
				{
					m_context.m_client->selectRegion(availableRegions[i]);
					return;
				}
			}

			m_context.m_client->selectRegion(availableRegions[0]);
		}

		void debugReturn([[maybe_unused]] const int debugLevel, [[maybe_unused]] const ExitGames::Common::JString& string) override
		{

		}

		void connectionErrorReturn(const int errorCode) override
		{
			m_context.debugLog(U"[Multiplayer_Photon] Multiplayer_Photon::connectionErrorReturn() [サーバへの接続が失敗したときに呼ばれる]");
			m_context.debugLog(U"- [Multiplayer_Photon] errorCode: ", errorCode);
			m_context.connectionErrorReturn(errorCode);
		}

		void clientErrorReturn([[maybe_unused]] const int errorCode) override
		{

		}

		void warningReturn([[maybe_unused]] const int warningCode) override
		{

		}

		void serverErrorReturn([[maybe_unused]] const int errorCode) override
		{

		}

		// 誰か（自分を含む）がルームに参加したら呼ばれるコールバック
		void joinRoomEventAction([[maybe_unused]] const int playerID, const ExitGames::Common::JVector<int>& playerIDs, const ExitGames::LoadBalancing::Player& player) override
		{
			Array<LocalPlayerID> ids(playerIDs.getSize());
			{
				for (unsigned i = 0; i < playerIDs.getSize(); ++i)
				{
					ids[i] = playerIDs[i];
				}
			}

			assert(playerID == player.getNumber());

			const LocalPlayer localPlayer
			{
				.localID = playerID,
				.userName = detail::ToString(player.getName()),
				.userID = detail::ToString(player.getUserID()),
				.isHost = player.getIsMasterClient(),
				.isActive = (not player.getIsInactive()),
			};

			const bool isSelf = (playerID == m_context.getLocalPlayerID());

			if (isSelf)
			{
				m_context.m_lastJoinedRoomName = m_context.getCurrentRoomName();
			}

			m_context.debugLog(U"[Multiplayer_Photon] Multiplayer_Photon::joinRoomEventAction() [誰か（自分を含む）が現在のルームに参加したときに呼ばれる]");
			m_context.debugLog(U"- [Multiplayer_Photon] playerID [参加した人の ID]: ", playerID);
			m_context.debugLog(U"- [Multiplayer_Photon] isSelf [自分自身の参加？]: ", isSelf);
			m_context.debugLog(U"- [Multiplayer_Photon] playerIDs [ルームの参加者一覧]: ", ids);

			m_context.joinRoomEventAction(localPlayer, ids, isSelf);
		}

		// 誰か（自分を含む）がルームから退出したら呼ばれるコールバック
		void leaveRoomEventAction(const int playerID, const bool isInactive) override
		{
			m_context.debugLog(U"[Multiplayer_Photon] Multiplayer_Photon::leaveRoomEventAction()");
			m_context.debugLog(U"- [Multiplayer_Photon] playerID: ", playerID);
			m_context.debugLog(U"- [Multiplayer_Photon] isInactive: ", isInactive);

			m_context.leaveRoomEventAction(playerID, isInactive);
		}

		// ルームで他人が sendEvent したら呼ばれるコールバック
		void customEventAction(const int playerID, const nByte eventCode, const ExitGames::Common::Object& _data) override
		{
# if not SIV3D_MULTIPLAYER_PHOTON_LEGACY
			const ExitGames::Common::ValueObject<uint8*> data{ _data };
			const auto size = data.getSizes()[0];

			m_context.logger(U"[Multiplayer_Photon] Multiplayer_Photon::customEventAction(Deserializer<MemoryReader>)");
			m_context.logger(U"[Multiplayer_Photon] playerID: ", playerID);
			m_context.logger(U"[Multiplayer_Photon] eventCode: ", eventCode);
			m_context.logger(U"[Multiplayer_Photon] data: ", size, U" bytes (serialized)");

			Deserializer<MemoryViewReader> reader{ data.getDataCopy(), size };
			if (m_context.m_table.contains(eventCode)) {
				auto& receiver = m_context.m_table[eventCode];
				(receiver.second)(m_context, receiver.first, playerID, reader);
			}
			else {
				m_context.customEventAction(playerID, eventCode, reader);
			}
# else
			const uint8 type = _data.getType();

			if (type == ExitGames::Common::TypeCode::CUSTOM)
			{
				const uint8 customType = _data.getCustomType();
				m_receiveEventFunctions[customType](playerID, eventCode, _data);
			}
			else if (type == ExitGames::Common::TypeCode::HASHTABLE)
			{
				const ExitGames::Common::Hashtable eventDataContent = ExitGames::Common::ValueObject<ExitGames::Common::Hashtable>(_data).getDataCopy();
				const ExitGames::Common::JString mainType = ExitGames::Common::ValueObject<ExitGames::Common::JString>(eventDataContent.getValue(L"Type")).getDataCopy();

				if (mainType == L"Array")
				{
					switch (eventDataContent.getValue(L"values")->getType())
					{
					case ExitGames::Common::TypeCode::BOOLEAN:
					{
						const auto p = ExitGames::Common::ValueObject<bool*>(eventDataContent.getValue(L"values")).getDataCopy();
						const auto length = *(ExitGames::Common::ValueObject<bool*>(eventDataContent.getValue(L"values"))).getSizes();
						m_context.customEventAction(playerID, eventCode, Array<bool>(p, (p + length)));
						break;
					}
					case ExitGames::Common::TypeCode::BYTE:
					{
						const auto p = ExitGames::Common::ValueObject<uint8*>(eventDataContent.getValue(L"values")).getDataCopy();
						const auto length = *(ExitGames::Common::ValueObject<uint8*>(eventDataContent.getValue(L"values"))).getSizes();
						m_context.customEventAction(playerID, eventCode, Array<uint8>(p, (p + length)));
						break;
					}
					case ExitGames::Common::TypeCode::SHORT:
					{
						const auto p = ExitGames::Common::ValueObject<int16*>(eventDataContent.getValue(L"values")).getDataCopy();
						const auto length = *(ExitGames::Common::ValueObject<int16*>(eventDataContent.getValue(L"values"))).getSizes();
						m_context.customEventAction(playerID, eventCode, Array<int16>(p, (p + length)));
						break;
					}
					case ExitGames::Common::TypeCode::INTEGER:
					{
						const auto p = ExitGames::Common::ValueObject<int32*>(eventDataContent.getValue(L"values")).getDataCopy();
						const auto length = *(ExitGames::Common::ValueObject<int32*>(eventDataContent.getValue(L"values"))).getSizes();
						m_context.customEventAction(playerID, eventCode, Array<int32>(p, (p + length)));
						break;
					}
					case ExitGames::Common::TypeCode::LONG:
					{
						const auto p = ExitGames::Common::ValueObject<int64*>(eventDataContent.getValue(L"values")).getDataCopy();
						const auto length = *(ExitGames::Common::ValueObject<int64*>(eventDataContent.getValue(L"values"))).getSizes();
						m_context.customEventAction(playerID, eventCode, Array<int64>(p, (p + length)));
						break;
					}
					case ExitGames::Common::TypeCode::FLOAT:
					{
						const auto p = ExitGames::Common::ValueObject<float*>(eventDataContent.getValue(L"values")).getDataCopy();
						const auto length = *(ExitGames::Common::ValueObject<float*>(eventDataContent.getValue(L"values"))).getSizes();
						m_context.customEventAction(playerID, eventCode, Array<float>(p, (p + length)));
						break;
					}
					case ExitGames::Common::TypeCode::DOUBLE:
					{
						const auto p = ExitGames::Common::ValueObject<double*>(eventDataContent.getValue(L"values")).getDataCopy();
						const auto length = *(ExitGames::Common::ValueObject<double*>(eventDataContent.getValue(L"values"))).getSizes();
						m_context.customEventAction(playerID, eventCode, Array<double>(p, (p + length)));
						break;
					}
					case ExitGames::Common::TypeCode::STRING:
					{
						const auto values = ExitGames::Common::ValueObject<ExitGames::Common::JString*>(eventDataContent.getValue(L"values")).getDataCopy();
						const auto length = *(ExitGames::Common::ValueObject<ExitGames::Common::JString*>(eventDataContent.getValue(L"values"))).getSizes();
						Array<String> data(length);
						for (short i = 0; i < length; ++i)
						{
							data[i] = detail::ToString(values[i]);
						}
						m_context.customEventAction(playerID, eventCode, data);
						break;
					}
					default:
						break;
					}
				}
				else if (mainType == L"Blob")
				{
					switch (eventDataContent.getValue(L"values")->getType())
					{
					case ExitGames::Common::TypeCode::BYTE:
					{
						const auto values = ExitGames::Common::ValueObject<uint8*>(eventDataContent.getValue(L"values")).getDataCopy();
						const auto length = *(ExitGames::Common::ValueObject<uint8*>(eventDataContent.getValue(L"values"))).getSizes();
						Deserializer<MemoryViewReader> reader{ values, length };
						if (m_context.m_table.contains(eventCode)) {
							auto& receiver = m_context.m_table[eventCode];
							(receiver.second)(m_context, receiver.first, playerID, reader);
						}
						else {
							m_context.customEventAction(playerID, eventCode, reader);
						}
						break;
					}
					default:
						break;
					}
				}
			}
			else
			{
				switch (type)
				{
				case ExitGames::Common::TypeCode::BOOLEAN:
					m_context.customEventAction(playerID, eventCode, ExitGames::Common::ValueObject<bool>(_data).getDataCopy());
					return;
				case ExitGames::Common::TypeCode::BYTE:
					m_context.customEventAction(playerID, eventCode, ExitGames::Common::ValueObject<uint8>(_data).getDataCopy());
					return;
				case ExitGames::Common::TypeCode::SHORT:
					m_context.customEventAction(playerID, eventCode, ExitGames::Common::ValueObject<int16>(_data).getDataCopy());
					return;
				case ExitGames::Common::TypeCode::INTEGER:
					m_context.customEventAction(playerID, eventCode, ExitGames::Common::ValueObject<int32>(_data).getDataCopy());
					return;
				case ExitGames::Common::TypeCode::LONG:
					m_context.customEventAction(playerID, eventCode, ExitGames::Common::ValueObject<int64>(_data).getDataCopy());
					return;
				case ExitGames::Common::TypeCode::FLOAT:
					m_context.customEventAction(playerID, eventCode, ExitGames::Common::ValueObject<float>(_data).getDataCopy());
					return;
				case ExitGames::Common::TypeCode::DOUBLE:
					m_context.customEventAction(playerID, eventCode, ExitGames::Common::ValueObject<double>(_data).getDataCopy());
					return;
				case ExitGames::Common::TypeCode::STRING:
					m_context.customEventAction(playerID, eventCode, detail::ToString(ExitGames::Common::ValueObject<ExitGames::Common::JString>(_data).getDataCopy()));
					return;
				default:
					break;
				}
			}
# endif
		}

		// connect() の結果を通知するコールバック
		void connectReturn(const int errorCode, const ExitGames::Common::JString& errorString, const ExitGames::Common::JString& region, const ExitGames::Common::JString& cluster) override
		{
			m_context.debugLog(U"[Multiplayer_Photon] Multiplayer_Photon::connectReturn()");
			m_context.debugLog(U"[Multiplayer_Photon] region: ", detail::ToString(region));
			m_context.debugLog(U"- [Multiplayer_Photon] cluster: ", detail::ToString(cluster));

			detail::LogIfError(m_context, errorCode, detail::ToString(errorString));

			m_context.connectReturn(errorCode, detail::ToString(errorString), detail::ToString(region), detail::ToString(cluster));
		}

		// disconnect() の結果を通知するコールバック
		void disconnectReturn() override
		{
			m_context.debugLog(U"[Multiplayer_Photon] Multiplayer_Photon::disconnectReturn() [サーバから切断されたときに呼ばれる]");

			m_context.disconnectReturn();
		}

		void leaveRoomReturn(const int errorCode, const ExitGames::Common::JString& errorString) override
		{
			m_context.debugLog(U"[Multiplayer_Photon] Multiplayer_Photon::leaveRoomReturn() [ルームから退出した結果を処理する]");

			detail::LogIfError(m_context, errorCode, errorString);

			m_context.leaveRoomReturn(errorCode, detail::ToString(errorString));
		}

		void joinRoomReturn(const int playerID, [[maybe_unused]] const ExitGames::Common::Hashtable& roomProperties, [[maybe_unused]] const ExitGames::Common::Hashtable& playerProperties, const int errorCode, const ExitGames::Common::JString& errorString) override
		{
			m_context.debugLog(U"[Multiplayer_Photon] Multiplayer_Photon::joinRoomReturn()");
			m_context.debugLog(U"- [Multiplayer_Photon] playerID: ", playerID);

			detail::LogIfError(m_context, errorCode, errorString);

			m_context.joinRoomReturn(playerID, errorCode, detail::ToString(errorString));
		}

		void joinRandomRoomReturn(const int playerID, [[maybe_unused]] const ExitGames::Common::Hashtable& roomProperties, [[maybe_unused]] const ExitGames::Common::Hashtable& playerProperties, const int errorCode, const ExitGames::Common::JString& errorString) override
		{
			m_context.debugLog(U"[Multiplayer_Photon] Multiplayer_Photon::joinRandomRoomReturn()");
			m_context.debugLog(U"- [Multiplayer_Photon] playerID: ", playerID);

			detail::LogIfError(m_context, errorCode, errorString);

			m_context.joinRandomRoomReturn(playerID, errorCode, detail::ToString(errorString));
		}

		void createRoomReturn(const int playerID, [[maybe_unused]] const ExitGames::Common::Hashtable& roomProperties, [[maybe_unused]] const ExitGames::Common::Hashtable& playerProperties, const int errorCode, const ExitGames::Common::JString& errorString) override
		{
			m_context.debugLog(U"[Multiplayer_Photon] Multiplayer_Photon::createRoomReturn() [ルームを新規作成した結果を処理する]");
			m_context.debugLog(U"- [Multiplayer_Photon] playerID: ", playerID);

			detail::LogIfError(m_context, errorCode, errorString);

			m_context.createRoomReturn(playerID, errorCode, detail::ToString(errorString));
		}

		void joinOrCreateRoomReturn(const int playerID, [[maybe_unused]] const ExitGames::Common::Hashtable& roomProperties, [[maybe_unused]] const ExitGames::Common::Hashtable& playerProperties, const int errorCode, const ExitGames::Common::JString& errorString) override
		{
			m_context.debugLog(U"[Multiplayer_Photon] Multiplayer_Photon::joinOrCreateRoomReturn()");
			m_context.debugLog(U"- [Multiplayer_Photon] playerID: ", playerID);

			detail::LogIfError(m_context, errorCode, errorString);

			m_context.joinOrCreateRoomReturn(playerID, errorCode, detail::ToString(errorString));
		}

		void joinRandomOrCreateRoomReturn(const int playerID, [[maybe_unused]] const ExitGames::Common::Hashtable& roomProperties, [[maybe_unused]] const ExitGames::Common::Hashtable& playerProperties, const int errorCode, const ExitGames::Common::JString& errorString) override
		{
			m_context.debugLog(U"[Multiplayer_Photon] Multiplayer_Photon::joinRandomOrCreateRoomReturn()");
			m_context.debugLog(U"- [Multiplayer_Photon] playerID: ", playerID);

			detail::LogIfError(m_context, errorCode, errorString);

			m_context.joinRandomOrCreateRoomReturn(playerID, errorCode, detail::ToString(errorString));
		}

		void onRoomListUpdate() override
		{
			m_context.debugLog(U"[Multiplayer_Photon] Multiplayer_Photon::onRoomListUpdate()");

			m_context.onRoomListUpdate();
		}

		void onRoomPropertiesChange(const ExitGames::Common::Hashtable& changes_) override
		{
			auto changes = detail::PhotonHashtableToStringHashTable(changes_);

			if (changes.size() == 0)
			{
				return;
			}

			m_context.debugLog(U"[Multiplayer_Photon] Multiplayer_Photon::onRoomPropertiesChange()");
			m_context.debugLog(U"[Multiplayer_Photon] - changes: {}"_fmt(Format(changes)));

			m_context.onRoomPropertiesChange(changes);
		}

		void onMasterClientChanged(const int newHostID, const int oldHostID) override
		{
			m_context.debugLog(U"[Multiplayer_Photon] Multiplayer_Photon::onHostChange()");
			m_context.debugLog(U"[Multiplayer_Photon] - netHostID: {}"_fmt(newHostID));
			m_context.debugLog(U"[Multiplayer_Photon] - oldHostID: {}"_fmt(oldHostID));

			m_context.onHostChange(newHostID, oldHostID);
		}

	private:

		Multiplayer_Photon& m_context;

		HashTable<uint8, std::function<void(const int, const nByte, const ExitGames::Common::Object&)>> m_receiveEventFunctions;

# if SIV3D_MULTIPLAYER_PHOTON_LEGACY
		template <class Type, uint8 N>
		void receivedCustomType(const int playerID, const nByte eventCode, const ExitGames::Common::Object& eventContent)
		{
			const auto value = ExitGames::Common::ValueObject<CustomType_Photon<Type, N>>(eventContent).getDataCopy().getValue();
			m_context.customEventAction(playerID, eventCode, value);
		}
# endif
	};
}

// RoomCreateOption, TargetGroup, MultiplayerEvent
namespace s3d {

	// RoomCreateOption

	RoomCreateOption& RoomCreateOption::isVisible(bool isVisible)
	{
		m_isVisible = isVisible;
		return *this;
	}

	RoomCreateOption& RoomCreateOption::isOpen(bool isOpen)
	{
		m_isOpen = isOpen;
		return *this;
	}

	RoomCreateOption& RoomCreateOption::publishUserId(bool publishUserId)
	{
		m_publishUserId = publishUserId;
		return *this;
	}

	RoomCreateOption& RoomCreateOption::maxPlayers(int32 maxPlayers)
	{
		if (not InRange(maxPlayers, 0, 255))
		{
			throw Error{ U"[Multiplayer_Photon] MaxPlayers must be in a range of 0 to 255" };
		}
		m_maxPlayers = maxPlayers;
		return *this;
	}

	RoomCreateOption& RoomCreateOption::properties(const HashTable<uint8, String>& properties)
	{
		m_properties = properties;
		return *this;
	}

	RoomCreateOption& RoomCreateOption::rejoinGracePeriod(const Optional<Milliseconds>& rejoinGracePeriod)
	{
		m_rejoinGracePeriod = rejoinGracePeriod;
		return *this;
	}

	RoomCreateOption& RoomCreateOption::roomDestroyGracePeriod(Milliseconds roomDestroyGracePeriod)
	{
		m_roomDestroyGracePeriod = roomDestroyGracePeriod;
		return *this;
	}

	bool RoomCreateOption::isVisible() const noexcept
	{
		return m_isVisible;
	}

	bool RoomCreateOption::isOpen() const noexcept
	{
		return m_isOpen;
	}

	bool RoomCreateOption::publishUserId() const noexcept
	{
		return m_publishUserId;
	}

	int32 RoomCreateOption::maxPlayers() const noexcept
	{
		return m_maxPlayers;
	}

	const HashTable<uint8, String>& RoomCreateOption::properties() const noexcept
	{
		return m_properties;
	}

	const Optional<Milliseconds>& RoomCreateOption::rejoinGracePeriod() const noexcept
	{
		return m_rejoinGracePeriod;
	}

	Milliseconds RoomCreateOption::roomDestroyGracePeriod() const noexcept
	{
		return m_roomDestroyGracePeriod;
	}

	// TargetGroup

	TargetGroup::TargetGroup(uint8 targetGroup) noexcept
		: m_targetGroup(targetGroup)
	{
	}

	uint8 TargetGroup::value() const noexcept
	{
		return m_targetGroup;
	}

	// MultiplayerEvent

	uint8 MultiplayerEvent::eventCode() const noexcept
	{
		return m_eventCode;
	}

	uint8 MultiplayerEvent::priorityIndex() const noexcept
	{
		return m_priorityIndex;
	}

	uint8 MultiplayerEvent::targetGroup() const noexcept
	{
		return m_targetGroup;
	}

	ReceiverOption MultiplayerEvent::receiverOption() const noexcept
	{
		return m_receiverOption;
	}

	const Optional<Array<LocalPlayerID>>& MultiplayerEvent::targetList() const noexcept
	{
		return m_targetList;
	}
}

// Multiplayer_Photon
namespace s3d {
	Multiplayer_Photon::Multiplayer_Photon() = default;

	Multiplayer_Photon::Multiplayer_Photon(std::string_view secretPhotonAppID, const StringView photonAppVersion, const Verbose verbose, const ConnectionProtocol protocol)
	{
		init(Unicode::WidenAscii(secretPhotonAppID), photonAppVersion, verbose, protocol);
	}

	Multiplayer_Photon::Multiplayer_Photon(StringView secretPhotonAppID, const StringView photonAppVersion, const Verbose verbose, const ConnectionProtocol protocol)
	{
		init(secretPhotonAppID, photonAppVersion, verbose, protocol);
	}

	Multiplayer_Photon::Multiplayer_Photon(std::string_view secretPhotonAppID, StringView photonAppVersion, const std::function<void(StringView)>& logger, const Verbose verbose, ConnectionProtocol protocol)
	{
		init(Unicode::WidenAscii(secretPhotonAppID), photonAppVersion, logger, verbose, protocol);
	}

	Multiplayer_Photon::Multiplayer_Photon(StringView secretPhotonAppID, StringView photonAppVersion, const std::function<void(StringView)>& logger, const Verbose verbose, ConnectionProtocol protocol)
	{
		init(secretPhotonAppID, photonAppVersion, logger, verbose, protocol);
	}

	Multiplayer_Photon::~Multiplayer_Photon()
	{
		disconnect();

# if SIV3D_MULTIPLAYER_PHOTON_LEGACY
		UnregisterTypes();
# endif

# if SIV3D_PLATFORM(WEB)
		g_detail = nullptr;
# endif
	}

	void Multiplayer_Photon::init(const std::string_view secretPhotonAppID, const StringView photonAppVersion, const Verbose verbose, const ConnectionProtocol protocol)
	{
		init(Unicode::WidenAscii(secretPhotonAppID), photonAppVersion, Print, verbose, protocol);
	}

	void Multiplayer_Photon::init(const StringView secretPhotonAppID, const StringView photonAppVersion, const Verbose verbose, const ConnectionProtocol protocol)
	{
		init(secretPhotonAppID, photonAppVersion, Print, verbose, protocol);
	}

	void Multiplayer_Photon::init(const std::string_view secretPhotonAppID, const StringView photonAppVersion, const std::function<void(StringView)>& logger, const Verbose verbose, const ConnectionProtocol protocol)
	{
		init(Unicode::WidenAscii(secretPhotonAppID), photonAppVersion, logger, verbose, protocol);
	}
}

/// Multiplayer_Photon
namespace s3d
{
	void Multiplayer_Photon::init(StringView secretPhotonAppID, StringView photonAppVersion, const std::function<void(StringView)>& logger, const Verbose verbose, ConnectionProtocol protocol)
	{
		if (m_listener) // すでに初期化済みであれば何もしない
		{
			return;
		}

		m_secretPhotonAppID = secretPhotonAppID;
		m_photonAppVersion = photonAppVersion;
		m_listener = std::make_unique<PhotonDetail>(*this);
		m_connectionProtocol = protocol;
		m_logger = logger;
		m_verbose = verbose.getBool();

# if SIV3D_MULTIPLAYER_PHOTON_LEGACY
		RegisterTypes();
# endif
	}

	bool Multiplayer_Photon::connect(const StringView userName_, const Optional<String>& region)
	{
		m_requestedRegion = region;

		m_client.reset();

		m_client = std::make_unique<ExitGames::LoadBalancing::Client>(*m_listener, detail::ToJString(m_secretPhotonAppID), detail::ToJString(m_photonAppVersion),
		  ExitGames::LoadBalancing::ClientConstructOptions{ FromEnum(m_connectionProtocol), false, (m_requestedRegion ? ExitGames::LoadBalancing::RegionSelectionMode::SELECT : ExitGames::LoadBalancing::RegionSelectionMode::BEST) });

		const auto userName = detail::ToJString(userName_);
		const auto userID = ExitGames::LoadBalancing::AuthenticationValues{}.setUserID(userName + static_cast<uint32>(Time::GetMillisecSinceEpoch()));

		if (not m_client->connect({ userID, userName }))
		{
			debugLog(U"[Multiplayer_Photon] ExitGmae::LoadBalancing::Client::connect() failed.");
			return false;
		}

		m_client->fetchServerTimestamp();

		return true;
	}

	void Multiplayer_Photon::disconnect()
	{
		if (not m_client)
		{
			return;
		}

		m_client->disconnect();

		m_client->service();
	}

	void Multiplayer_Photon::update()
	{
		if (not m_client)
		{
			return;
		}

		m_client->service();
	}

	bool Multiplayer_Photon::isActive() const noexcept
	{
		return getClientState() != ClientState::Disconnected;
	}

	ClientState Multiplayer_Photon::getClientState() const
	{
		if (not m_client)
		{
			return ClientState::Disconnected;
		}

		using namespace ExitGames::LoadBalancing::PeerStates;

		switch (m_client->getState()) {
		case Uninitialized:
		case PeerCreated:
			return ClientState::Disconnected;
		case ConnectingToNameserver:
		case ConnectedToNameserver:
		case DisconnectingFromNameserver:
		case Connecting:
		case Connected:
		case WaitingForCustomAuthenticationNextStepCall:
		case Authenticated:
			return ClientState::ConnectingToLobby;
		case JoinedLobby:
			return ClientState::InLobby;
		case DisconnectingFromMasterserver:
		case ConnectingToGameserver:
		case ConnectedToGameserver:
		case AuthenticatedOnGameServer:
		case Joining:
			return ClientState::JoiningRoom;
		case Joined:
			return ClientState::InRoom;
		case Leaving:
		case Left:
		case DisconnectingFromGameserver:
		case ConnectingToMasterserver:
		case ConnectedComingFromGameserver:
		case AuthenticatedComingFromGameserver:
			return ClientState::LeavingRoom;
		case Disconnecting:
		case Disconnected:
			return ClientState::Disconnecting;
		default:
			return ClientState::Disconnected;
		}
	}

	bool Multiplayer_Photon::isInLobby() const
	{
		if (not m_client)
		{
			return false;
		}

		return m_client->getIsInLobby();
	}

	bool Multiplayer_Photon::isInLobbyOrInRoom() const
	{
		if (not m_client)
		{
			return false;
		}

		return m_client->getIsInRoom();
	}

	bool Multiplayer_Photon::isInRoom() const
	{
		if (not m_client)
		{
			return false;
		}

		return m_client->getIsInGameRoom();
	}

	Array<RoomInfo> Multiplayer_Photon::getRoomList() const
	{
		if (not m_client)
		{
			return{};
		}

		const auto& roomList = m_client->getRoomList();

		Array<RoomInfo> results(roomList.getSize());

		for (uint32 i = 0; i < roomList.getSize(); ++i)
		{
			const auto room = roomList[i];

			RoomInfo roomInfo
			{
				.name = detail::ToString(room->getName()),
				.playerCount = room->getPlayerCount(),
				.maxPlayers = room->getMaxPlayers(),
				.isOpen = room->getIsOpen(),
				.properties = detail::PhotonHashtableToStringHashTable(room->getCustomProperties()),
			};

			results[i] = std::move(roomInfo);
		}

		return results;
	}

	Array<RoomName> Multiplayer_Photon::getRoomNameList() const
	{
		if (not m_client)
		{
			return{};
		}

		const auto roomNameList = m_client->getRoomNameList();

		Array<RoomName> results(roomNameList.getSize());

		for (uint32 i = 0; i < roomNameList.getSize(); ++i)
		{
			results[i] = detail::ToString(roomNameList[i]);
		}

		return results;
	}

	int32 Multiplayer_Photon::getServerTimeMillisec() const
	{
		if (not m_client)
		{
			return 0;
		}

		return m_client->getServerTime();
	}

	int32 Multiplayer_Photon::getServerTimeOffsetMillisec() const
	{
		if (not m_client)
		{
			return 0;
		}

		return m_client->getServerTimeOffset();
	}

	int32 Multiplayer_Photon::getPingMillisec() const
	{
		if (not m_client)
		{
			return 0;
		}

		return m_client->getRoundTripTime();
	}

	int32 Multiplayer_Photon::getPingIntervalMillisec() const
	{
		if (not m_client)
		{
			return 0;
		}

		return m_client->getTimePingInterval();
	}

	void Multiplayer_Photon::setPingIntervalMillisec(int32 intervalMillisec)
	{
		if (not m_client)
		{
			return;
		}

		return m_client->setTimePingInterval(intervalMillisec);
	}

	int32 Multiplayer_Photon::getBytesIn() const
	{
		if (not m_client)
		{
			return 0;
		}

		return m_client->getBytesIn();
	}

	int32 Multiplayer_Photon::getBytesOut() const
	{
		if (not m_client)
		{
			return 0;
		}

		return m_client->getBytesOut();
	}

	int32 Multiplayer_Photon::getCountGamesRunning() const
	{
		if (not m_client)
		{
			return 0;
		}

		return m_client->getCountGamesRunning();
	}

	int32 Multiplayer_Photon::getCountPlayersIngame() const
	{
		if (not m_client)
		{
			return 0;
		}

		return m_client->getCountPlayersIngame();
	}

	int32 Multiplayer_Photon::getCountPlayersOnline() const
	{
		if (not m_client)
		{
			return 0;
		}

		return m_client->getCountPlayersOnline();
	}

	bool Multiplayer_Photon::joinRandomRoom(int32 expectedMaxPlayers, MatchmakingMode matchmakingMode)
	{
		if (not m_client)
		{
			return false;
		}

		if (not InRange(expectedMaxPlayers, 0, 255))
		{
			return false;
		}

		return m_client->opJoinRandomRoom({}, static_cast<uint8>(expectedMaxPlayers), static_cast<nByte>(matchmakingMode));
	}

	bool Multiplayer_Photon::joinRandomRoom(const HashTable<uint8, String>& propertyFilter, int32 expectedMaxPlayers, MatchmakingMode matchmakingMode)
	{
		if (not m_client)
		{
			return false;
		}

		if (not InRange(expectedMaxPlayers, 0, 255))
		{
			return false;
		}

		return m_client->opJoinRandomRoom(detail::ToPhotonHashtable(propertyFilter), static_cast<uint8>(expectedMaxPlayers), static_cast<nByte>(matchmakingMode));
	}

	bool Multiplayer_Photon::joinRandomOrCreateRoom(const int32 maxPlayers, const RoomNameView roomName)
	{
		if (not m_client)
		{
			return false;
		}

		if (not InRange(maxPlayers, 0, 255))
		{
			return false;
		}

		return m_client->opJoinRandomOrCreateRoom(detail::ToJString(roomName), {}, {}, static_cast<uint8>(maxPlayers));
	}

	bool Multiplayer_Photon::joinRandomOrCreateRoom(RoomNameView roomName, const RoomCreateOption& roomCreateOption, const HashTable<uint8, String>& propertyFilter, int32 expectedMaxPlayers, MatchmakingMode matchmakingMode)
	{
		if (not m_client)
		{
			return false;
		}

		if (not InRange(expectedMaxPlayers, 0, 255))
		{
			return false;
		}

		return m_client->opJoinRandomOrCreateRoom(detail::ToJString(roomName), detail::ToRoomOptions(roomCreateOption), detail::ToPhotonHashtable(propertyFilter), static_cast<uint8>(expectedMaxPlayers), static_cast<nByte>(matchmakingMode));
	}

	bool Multiplayer_Photon::joinOrCreateRoom(RoomNameView roomName, const RoomCreateOption& option)
	{
		if (not m_client)
		{
			return false;
		}

		return m_client->opJoinOrCreateRoom(detail::ToJString(roomName), detail::ToRoomOptions(option));
	}

	bool Multiplayer_Photon::joinRoom(const RoomNameView roomName)
	{
		if (not m_client)
		{
			return false;
		}

		constexpr bool rejoin = false;
		return m_client->opJoinRoom(detail::ToJString(roomName), rejoin);
	}

	bool Multiplayer_Photon::createRoom(const RoomNameView roomName, const int32 maxPlayers)
	{
		if (not m_client)
		{
			return false;
		}

		if (not InRange(maxPlayers, 0, 255))
		{
			return false;
		}

		const auto roomOption = ExitGames::LoadBalancing::RoomOptions()
			.setMaxPlayers(static_cast<uint8>(maxPlayers))
			.setPublishUserID(true);

		return m_client->opCreateRoom(detail::ToJString(roomName), roomOption);
	}

	bool Multiplayer_Photon::createRoom(RoomNameView roomName, const RoomCreateOption& option)
	{
		if (not m_client)
		{
			return false;
		}

		return m_client->opCreateRoom(detail::ToJString(roomName), detail::ToRoomOptions(option));
	}

	void Multiplayer_Photon::leaveRoom(bool willComeBack)
	{
		if (not m_client)
		{
			return;
		}

		m_client->opLeaveRoom(willComeBack);
	}

	bool Multiplayer_Photon::reconnectAndRejoin()
	{
		if (not m_client)
		{
			return false;
		}

		auto state = getClientState();

		if (state == ClientState::InLobby)
		{
			constexpr bool rejoin = true;
			return m_client->opJoinRoom(detail::ToJString(m_lastJoinedRoomName), rejoin);
		}

		if (state == ClientState::Disconnected)
		{
			return m_client->reconnectAndRejoin();
		}

		return false;
	}

	void Multiplayer_Photon::joinEventTargetGroup(uint8 targetGroup)
	{
		joinEventTargetGroup(Array<uint8>{ targetGroup });
	}

	void Multiplayer_Photon::joinEventTargetGroup(const Array<uint8>& targetGroups)
	{
		if (not m_client)
		{
			return;
		}

		if (targetGroups.isEmpty())
		{
			return;
		}

		for (const auto targetGroup : targetGroups)
		{
			if (not InRange(static_cast<int>(targetGroup), 1, 255)) {
				throw Error{ U"[Multiplayer_Photon] targetGroups must be in a range of 1 to 255" };
			}
		}

		auto joinGroups = ExitGames::Common::JVector<nByte>(targetGroups.data(), static_cast<uint32>(targetGroups.size()));
		m_client->opChangeGroups(nullptr, &joinGroups);
	}

	void Multiplayer_Photon::joinAllEventTargetGroups()
	{
		if (not m_client)
		{
			return;
		}

		auto emptyJVector = ExitGames::Common::JVector<nByte>();

		m_client->opChangeGroups(nullptr, &emptyJVector);
	}

	void Multiplayer_Photon::leaveEventTargetGroup(const uint8 targetGroup)
	{
		leaveEventTargetGroup(Array<uint8>{ targetGroup });
	}

	void Multiplayer_Photon::leaveEventTargetGroup(const Array<uint8>& targetGroups)
	{
		if (not m_client)
		{
			return;
		}

		if (targetGroups.isEmpty())
		{
			return;
		}

		for (const auto targetGroup : targetGroups)
		{
			if (not InRange(static_cast<int>(targetGroup), 1, 255)) {
				throw Error{ U"[Multiplayer_Photon] targetGroups must be in a range of 1 to 255" };
			}
		}

		auto leaveGroups = ExitGames::Common::JVector<nByte>(targetGroups.data(), static_cast<uint32>(targetGroups.size()));
		m_client->opChangeGroups(&leaveGroups, nullptr);
	}

	void Multiplayer_Photon::leaveAllEventTargetGroups()
	{
		if (not m_client)
		{
			return;
		}

		auto emptyJVector = ExitGames::Common::JVector<nByte>();

		m_client->opChangeGroups(&emptyJVector, nullptr);
	}
}

/// Multiplayer_Photon::sendEvent
namespace s3d
{
	namespace detail
	{
		[[nodiscard]]
		static ExitGames::LoadBalancing::RaiseEventOptions MakeRaiseEventOptions(const Optional<Array<LocalPlayerID>>& targets)
		{
			ExitGames::LoadBalancing::RaiseEventOptions options{};

			if (targets)
			{
				options.setTargetPlayers(targets->data(), static_cast<short>(targets->size()));
			}

			return options;
		}
	}

	static constexpr bool Reliable = true;

	void Multiplayer_Photon::sendEvent(const MultiplayerEvent& eventInfo, const Serializer<MemoryWriter>& writer)
	{
		if (not m_client)
		{
			return;
		}

		uint8 receiver = ExitGames::Lite::ReceiverGroup::OTHERS;
		uint8 caching = ExitGames::Lite::EventCache::DO_NOT_CACHE;

		switch (eventInfo.receiverOption())
		{
		case ReceiverOption::Others:
			break;
		case ReceiverOption::Others_CacheUntilLeaveRoom:
			caching = ExitGames::Lite::EventCache::ADD_TO_ROOM_CACHE;
			break;
		case ReceiverOption::Others_CacheForever:
			caching = ExitGames::Lite::EventCache::ADD_TO_ROOM_CACHE_GLOBAL;
			break;
		case ReceiverOption::All:
			receiver = ExitGames::Lite::ReceiverGroup::ALL;
			break;
		case ReceiverOption::All_CacheUntilLeaveRoom:
			receiver = ExitGames::Lite::ReceiverGroup::ALL;
			caching = ExitGames::Lite::EventCache::ADD_TO_ROOM_CACHE;
			break;
		case ReceiverOption::All_CacheForever:
			receiver = ExitGames::Lite::ReceiverGroup::ALL;
			caching = ExitGames::Lite::EventCache::ADD_TO_ROOM_CACHE_GLOBAL;
			break;
		case ReceiverOption::Host:
			receiver = ExitGames::Lite::ReceiverGroup::MASTER_CLIENT;
			break;
		}

		const auto eventOptions = detail::MakeRaiseEventOptions(eventInfo.targetList())
			.setChannelID(eventInfo.priorityIndex())
			.setInterestGroup(eventInfo.targetGroup())
			.setReceiverGroup(receiver)
			.setEventCaching(caching);

		const auto& blob = writer->getBlob();
		const size_t size = blob.size();
		const uint8* src = static_cast<const uint8*>(static_cast<const void*>(blob.data()));

# if not SIV3D_MULTIPLAYER_PHOTON_LEGACY
		m_client->opRaiseEvent(Reliable, src, static_cast<unsigned int>(size), eventInfo.eventCode(), eventOptions);
# else
		ExitGames::Common::Hashtable ev;
		ev.put(L"Type", L"Blob");
		ev.put(L"values", src, static_cast<int16>(size));

		m_client->opRaiseEvent(Reliable, ev, eventInfo.eventCode(), eventOptions);
# endif
	}
}

/// [LEGACY] Multiplayer_Photon::sendEvent
# 	if SIV3D_MULTIPLAYER_PHOTON_LEGACY
namespace s3d
{
	void Multiplayer_Photon::sendEvent(const uint8 eventCode, const bool value, const Optional<Array<LocalPlayerID>>& targets)
	{
		if (not m_client)
		{
			return;
		}

		m_client->opRaiseEvent(Reliable, value, eventCode, detail::MakeRaiseEventOptions(targets));
	}

	void Multiplayer_Photon::sendEvent(const uint8 eventCode, const uint8 value, const Optional<Array<LocalPlayerID>>& targets)
	{
		if (not m_client)
		{
			return;
		}

		m_client->opRaiseEvent(Reliable, value, eventCode, detail::MakeRaiseEventOptions(targets));
	}

	void Multiplayer_Photon::sendEvent(const uint8 eventCode, const int16 value, const Optional<Array<LocalPlayerID>>& targets)
	{
		if (not m_client)
		{
			return;
		}

		m_client->opRaiseEvent(Reliable, value, eventCode, detail::MakeRaiseEventOptions(targets));
	}

	void Multiplayer_Photon::sendEvent(const uint8 eventCode, const int32 value, const Optional<Array<LocalPlayerID>>& targets)
	{
		if (not m_client)
		{
			return;
		}

		m_client->opRaiseEvent(Reliable, value, eventCode, detail::MakeRaiseEventOptions(targets));
	}

	void Multiplayer_Photon::sendEvent(const uint8 eventCode, const int64 value, const Optional<Array<LocalPlayerID>>& targets)
	{
		if (not m_client)
		{
			return;
		}

		m_client->opRaiseEvent(Reliable, value, eventCode, detail::MakeRaiseEventOptions(targets));
	}

	void Multiplayer_Photon::sendEvent(const uint8 eventCode, const float value, const Optional<Array<LocalPlayerID>>& targets)
	{
		if (not m_client)
		{
			return;
		}

		m_client->opRaiseEvent(Reliable, value, eventCode, detail::MakeRaiseEventOptions(targets));
	}

	void Multiplayer_Photon::sendEvent(const uint8 eventCode, const double value, const Optional<Array<LocalPlayerID>>& targets)
	{
		if (not m_client)
		{
			return;
		}

		m_client->opRaiseEvent(Reliable, value, eventCode, detail::MakeRaiseEventOptions(targets));
	}

	void Multiplayer_Photon::sendEvent(const uint8 eventCode, const char32* value, const Optional<Array<LocalPlayerID>>& targets)
	{
		sendEvent(eventCode, StringView{ value }, targets);
	}

	void Multiplayer_Photon::sendEvent(const uint8 eventCode, const StringView value, const Optional<Array<LocalPlayerID>>& targets)
	{
		if (not m_client)
		{
			return;
		}

		m_client->opRaiseEvent(Reliable, detail::ToJString(value), eventCode, detail::MakeRaiseEventOptions(targets));
	}

	void Multiplayer_Photon::sendEvent(const uint8 eventCode, const String& value, const Optional<Array<LocalPlayerID>>& targets)
	{
		sendEvent(eventCode, StringView{ value }, targets);
	}

	void Multiplayer_Photon::sendEvent(const uint8 eventCode, const Array<bool>& values, const Optional<Array<LocalPlayerID>>& targets)
	{
		if (not m_client)
		{
			return;
		}

		ExitGames::Common::Hashtable ev;
		ev.put(L"Type", L"Array");
		ev.put(L"values", values.data(), static_cast<int16>(values.size()));
		m_client->opRaiseEvent(Reliable, ev, eventCode, detail::MakeRaiseEventOptions(targets));
	}

	void Multiplayer_Photon::sendEvent(const uint8 eventCode, const Array<uint8>& values, const Optional<Array<LocalPlayerID>>& targets)
	{
		if (not m_client)
		{
			return;
		}

		ExitGames::Common::Hashtable ev;
		ev.put(L"Type", L"Array");
		ev.put(L"values", values.data(), static_cast<int16>(values.size()));
		m_client->opRaiseEvent(Reliable, ev, eventCode, detail::MakeRaiseEventOptions(targets));
	}

	void Multiplayer_Photon::sendEvent(const uint8 eventCode, const Array<int16>& values, const Optional<Array<LocalPlayerID>>& targets)
	{
		if (not m_client)
		{
			return;
		}

		ExitGames::Common::Hashtable ev;
		ev.put(L"Type", L"Array");
		ev.put(L"values", values.data(), static_cast<int16>(values.size()));
		m_client->opRaiseEvent(Reliable, ev, eventCode, detail::MakeRaiseEventOptions(targets));
	}

	void Multiplayer_Photon::sendEvent(const uint8 eventCode, const Array<int32>& values, const Optional<Array<LocalPlayerID>>& targets)
	{
		if (not m_client)
		{
			return;
		}

		ExitGames::Common::Hashtable ev;
		ev.put(L"Type", L"Array");
		ev.put(L"values", values.data(), static_cast<int16>(values.size()));
		m_client->opRaiseEvent(Reliable, ev, eventCode, detail::MakeRaiseEventOptions(targets));
	}

	void Multiplayer_Photon::sendEvent(const uint8 eventCode, const Array<int64>& values, const Optional<Array<LocalPlayerID>>& targets)
	{
		if (not m_client)
		{
			return;
		}

		ExitGames::Common::Hashtable ev;
		ev.put(L"Type", L"Array");
		ev.put(L"values", values.data(), static_cast<int16>(values.size()));
		m_client->opRaiseEvent(Reliable, ev, eventCode, detail::MakeRaiseEventOptions(targets));
	}

	void Multiplayer_Photon::sendEvent(const uint8 eventCode, const Array<float>& values, const Optional<Array<LocalPlayerID>>& targets)
	{
		if (not m_client)
		{
			return;
		}

		ExitGames::Common::Hashtable ev;
		ev.put(L"Type", L"Array");
		ev.put(L"values", values.data(), static_cast<int16>(values.size()));
		m_client->opRaiseEvent(Reliable, ev, eventCode, detail::MakeRaiseEventOptions(targets));
	}

	void Multiplayer_Photon::sendEvent(const uint8 eventCode, const Array<double>& values, const Optional<Array<LocalPlayerID>>& targets)
	{
		if (not m_client)
		{
			return;
		}

		ExitGames::Common::Hashtable ev;
		ev.put(L"Type", L"Array");
		ev.put(L"values", values.data(), static_cast<int16>(values.size()));
		m_client->opRaiseEvent(Reliable, ev, eventCode, detail::MakeRaiseEventOptions(targets));
	}

	void Multiplayer_Photon::sendEvent(const uint8 eventCode, const Array<String>& values, const Optional<Array<LocalPlayerID>>& targets)
	{
		if (not m_client)
		{
			return;
		}

		Array<ExitGames::Common::JString> data(Arg::reserve = values.size());
		for (const auto& value : values)
		{
			data << detail::ToJString(value);
		}

		ExitGames::Common::Hashtable ev;
		ev.put(L"Type", L"Array");
		ev.put(L"values", data.data(), static_cast<int16>(data.size()));
		m_client->opRaiseEvent(Reliable, ev, eventCode, detail::MakeRaiseEventOptions(targets));
	}

	void Multiplayer_Photon::sendEvent(const uint8 eventCode, const Color& value, const Optional<Array<LocalPlayerID>>& targets)
	{
		if (not m_client)
		{
			return;
		}

		m_client->opRaiseEvent(Reliable, PhotonColor{ value }, eventCode, detail::MakeRaiseEventOptions(targets));
	}

	void Multiplayer_Photon::sendEvent(const uint8 eventCode, const ColorF& value, const Optional<Array<LocalPlayerID>>& targets)
	{
		if (not m_client)
		{
			return;
		}

		m_client->opRaiseEvent(Reliable, PhotonColorF{ value }, eventCode, detail::MakeRaiseEventOptions(targets));
	}

	void Multiplayer_Photon::sendEvent(const uint8 eventCode, const HSV& value, const Optional<Array<LocalPlayerID>>& targets)
	{
		if (not m_client)
		{
			return;
		}

		m_client->opRaiseEvent(Reliable, PhotonHSV{ value }, eventCode, detail::MakeRaiseEventOptions(targets));
	}

	void Multiplayer_Photon::sendEvent(const uint8 eventCode, const Point& value, const Optional<Array<LocalPlayerID>>& targets)
	{
		if (not m_client)
		{
			return;
		}

		m_client->opRaiseEvent(Reliable, PhotonPoint{ value }, eventCode, detail::MakeRaiseEventOptions(targets));
	}

	void Multiplayer_Photon::sendEvent(const uint8 eventCode, const Vec2& value, const Optional<Array<LocalPlayerID>>& targets)
	{
		if (not m_client)
		{
			return;
		}

		m_client->opRaiseEvent(Reliable, PhotonVec2{ value }, eventCode, detail::MakeRaiseEventOptions(targets));
	}

	void Multiplayer_Photon::sendEvent(const uint8 eventCode, const Vec3& value, const Optional<Array<LocalPlayerID>>& targets)
	{
		if (not m_client)
		{
			return;
		}

		m_client->opRaiseEvent(Reliable, PhotonVec3{ value }, eventCode, detail::MakeRaiseEventOptions(targets));
	}

	void Multiplayer_Photon::sendEvent(const uint8 eventCode, const Vec4& value, const Optional<Array<LocalPlayerID>>& targets)
	{
		if (not m_client)
		{
			return;
		}

		m_client->opRaiseEvent(Reliable, PhotonVec4{ value }, eventCode, detail::MakeRaiseEventOptions(targets));
	}

	void Multiplayer_Photon::sendEvent(const uint8 eventCode, const Float2& value, const Optional<Array<LocalPlayerID>>& targets)
	{
		if (not m_client)
		{
			return;
		}

		m_client->opRaiseEvent(Reliable, PhotonFloat2{ value }, eventCode, detail::MakeRaiseEventOptions(targets));
	}

	void Multiplayer_Photon::sendEvent(const uint8 eventCode, const Float3& value, const Optional<Array<LocalPlayerID>>& targets)
	{
		if (not m_client)
		{
			return;
		}

		m_client->opRaiseEvent(Reliable, PhotonFloat3{ value }, eventCode, detail::MakeRaiseEventOptions(targets));
	}

	void Multiplayer_Photon::sendEvent(const uint8 eventCode, const Float4& value, const Optional<Array<LocalPlayerID>>& targets)
	{
		if (not m_client)
		{
			return;
		}

		m_client->opRaiseEvent(Reliable, PhotonFloat4{ value }, eventCode, detail::MakeRaiseEventOptions(targets));
	}

	void Multiplayer_Photon::sendEvent(const uint8 eventCode, const Mat3x2& value, const Optional<Array<LocalPlayerID>>& targets)
	{
		if (not m_client)
		{
			return;
		}

		m_client->opRaiseEvent(Reliable, PhotonMat3x2{ value }, eventCode, detail::MakeRaiseEventOptions(targets));
	}

	void Multiplayer_Photon::sendEvent(const uint8 eventCode, const Rect& value, const Optional<Array<LocalPlayerID>>& targets)
	{
		if (not m_client)
		{
			return;
		}

		m_client->opRaiseEvent(Reliable, PhotonRect{ value }, eventCode, detail::MakeRaiseEventOptions(targets));
	}

	void Multiplayer_Photon::sendEvent(const uint8 eventCode, const Circle& value, const Optional<Array<LocalPlayerID>>& targets)
	{
		if (not m_client)
		{
			return;
		}

		m_client->opRaiseEvent(Reliable, PhotonCircle{ value }, eventCode, detail::MakeRaiseEventOptions(targets));
	}

	void Multiplayer_Photon::sendEvent(const uint8 eventCode, const Line& value, const Optional<Array<LocalPlayerID>>& targets)
	{
		if (not m_client)
		{
			return;
		}

		m_client->opRaiseEvent(Reliable, PhotonLine{ value }, eventCode, detail::MakeRaiseEventOptions(targets));
	}

	void Multiplayer_Photon::sendEvent(const uint8 eventCode, const Triangle& value, const Optional<Array<LocalPlayerID>>& targets)
	{
		if (not m_client)
		{
			return;
		}

		m_client->opRaiseEvent(Reliable, PhotonTriangle{ value }, eventCode, detail::MakeRaiseEventOptions(targets));
	}

	void Multiplayer_Photon::sendEvent(const uint8 eventCode, const RectF& value, const Optional<Array<LocalPlayerID>>& targets)
	{
		if (not m_client)
		{
			return;
		}

		m_client->opRaiseEvent(Reliable, PhotonRectF{ value }, eventCode, detail::MakeRaiseEventOptions(targets));
	}

	void Multiplayer_Photon::sendEvent(const uint8 eventCode, const Quad& value, const Optional<Array<LocalPlayerID>>& targets)
	{
		if (not m_client)
		{
			return;
		}

		m_client->opRaiseEvent(Reliable, PhotonQuad{ value }, eventCode, detail::MakeRaiseEventOptions(targets));
	}

	void Multiplayer_Photon::sendEvent(const uint8 eventCode, const Ellipse& value, const Optional<Array<LocalPlayerID>>& targets)
	{
		if (not m_client)
		{
			return;
		}

		m_client->opRaiseEvent(Reliable, PhotonEllipse{ value }, eventCode, detail::MakeRaiseEventOptions(targets));
	}

	void Multiplayer_Photon::sendEvent(const uint8 eventCode, const RoundRect& value, const Optional<Array<LocalPlayerID>>& targets)
	{
		if (not m_client)
		{
			return;
		}

		m_client->opRaiseEvent(Reliable, PhotonRoundRect{ value }, eventCode, detail::MakeRaiseEventOptions(targets));
	}

	void Multiplayer_Photon::sendEvent(const uint8 eventCode, const Serializer<MemoryWriter>& writer, const Optional<Array<LocalPlayerID>>& targets)
	{
		if (not m_client)
		{
			return;
		}

		const auto& blob = writer->getBlob();
		const uint8* src = static_cast<const uint8*>(static_cast<const void*>(blob.data()));
		const size_t size = blob.size();

		ExitGames::Common::Hashtable ev;
		ev.put(L"Type", L"Blob");
		ev.put(L"values", src, static_cast<int16>(size));
		m_client->opRaiseEvent(Reliable, ev, eventCode, detail::MakeRaiseEventOptions(targets));
	}
}
# 	endif

/// Multiplayer_Photon
namespace s3d
{
	template<>
	void Multiplayer_Photon::removeEventCache(uint8 eventCode)
	{
		if (not m_client)
		{
			return;
		}

		if (not InRange(static_cast<int>(eventCode), 1, 199))
		{
			throw Error{ U"[Multiplayer_Photon] EventCode must be in a range of 1 to 199" };
		}

		m_client->opRaiseEvent(Reliable, ExitGames::Common::Hashtable(), eventCode, ExitGames::LoadBalancing::RaiseEventOptions().setEventCaching(ExitGames::Lite::EventCache::REMOVE_FROM_ROOM_CACHE));
	}

	template<>
	void Multiplayer_Photon::removeEventCache(uint8 eventCode, const Array<LocalPlayerID>& targets)
	{
		if (not m_client)
		{
			return;
		}

		if (not InRange(static_cast<int>(eventCode), 1, 199))
		{
			throw Error{ U"[Multiplayer_Photon] EventCode must be in a range of 1 to 199" };
		}

		auto option = detail::MakeRaiseEventOptions(targets).setEventCaching(ExitGames::Lite::EventCache::REMOVE_FROM_ROOM_CACHE);

		m_client->opRaiseEvent(Reliable, ExitGames::Common::Hashtable(), eventCode, option);
	}

	LocalPlayer Multiplayer_Photon::getLocalPlayer() const
	{
		if (not m_client)
		{
			return{};
		}

		if (not m_client->getIsInGameRoom())
		{
			return{};
		}

		const auto& player = m_client->getLocalPlayer();

		LocalPlayer localPlayer
		{
			.localID = player.getNumber(),
			.userName = detail::ToString(player.getName()),
			.userID = detail::ToString(player.getUserID()),
			.isHost = player.getIsMasterClient(),
			.isActive = (not player.getIsInactive()),
		};

		return localPlayer;
	}

	LocalPlayer Multiplayer_Photon::getLocalPlayer(LocalPlayerID localPlayerID) const
	{
		if (not m_client)
		{
			return{};
		}

		if (not m_client->getIsInGameRoom())
		{
			return{};
		}

		const auto player = m_client->getCurrentlyJoinedRoom().getPlayerForNumber(localPlayerID);

		if (not player)
		{
			return{};
		}

		LocalPlayer localPlayer
		{
			.localID = player->getNumber(),
			.userName = detail::ToString(player->getName()),
			.userID = detail::ToString(player->getUserID()),
			.isHost = player->getIsMasterClient(),
			.isActive = (not player->getIsInactive()),
		};

		return localPlayer;
	}

	String Multiplayer_Photon::getUserName() const
	{
		if (not m_client)
		{
			return{};
		}

		return detail::ToString(m_client->getLocalPlayer().getName());
	}

	String Multiplayer_Photon::getUserName(LocalPlayerID localPlayerID) const
	{
		if (not m_client)
		{
			return{};
		}

		if (not m_client->getIsInGameRoom())
		{
			return{};
		}

		const auto player = m_client->getCurrentlyJoinedRoom().getPlayerForNumber(localPlayerID);

		if (not player)
		{
			return{};
		}

		return detail::ToString(player->getName());
	}

	String Multiplayer_Photon::getUserID() const
	{
		if (not m_client)
		{
			return{};
		}

		return detail::ToString(m_client->getLocalPlayer().getUserID());
	}

	String Multiplayer_Photon::getUserID(LocalPlayerID localPlayerID) const
	{
		if (not m_client)
		{
			return{};
		}

		if (not m_client->getIsInGameRoom())
		{
			return{};
		}

		const auto player = m_client->getCurrentlyJoinedRoom().getPlayerForNumber(localPlayerID);

		return detail::ToString(player->getUserID());
	}

	bool Multiplayer_Photon::isHost() const
	{
		if (not m_client)
		{
			return false;
		}

		return m_client->getLocalPlayer().getIsMasterClient();
	}

	LocalPlayerID Multiplayer_Photon::getLocalPlayerID() const
	{
		if (not m_client)
		{
			return -1;
		}

		const LocalPlayerID localPlayerID = m_client->getLocalPlayer().getNumber();

		if (localPlayerID < 0)
		{
			return -1;
		}

		return localPlayerID;
	}

	LocalPlayerID Multiplayer_Photon::getHostLocalPlayerID() const
	{
		if (not m_client)
		{
			return -1;
		}

		const LocalPlayerID hostID = m_client->getCurrentlyJoinedRoom().getMasterClientID();

		if (hostID < 0)
		{
			return -1;
		}

		return hostID;
	}

	void Multiplayer_Photon::setUserName(StringView userName)
	{
		if (not m_client)
		{
			return;
		}

		m_client->getLocalPlayer().setName(detail::ToJString(userName));
	}

	void Multiplayer_Photon::setHost(LocalPlayerID localPlayerID)
	{
		if (not m_client)
		{
			return;
		}

		if (not m_client->getIsInGameRoom())
		{
			return;
		}

		m_client->getCurrentlyJoinedRoom().setMasterClient(*m_client->getCurrentlyJoinedRoom().getPlayerForNumber(localPlayerID));
	}

	RoomInfo Multiplayer_Photon::getCurrentRoom() const
	{
		if (not m_client)
		{
			return{};
		}

		if (not m_client->getIsInGameRoom())
		{
			return{};
		}

		const auto& room = m_client->getCurrentlyJoinedRoom();

		RoomInfo roomInfo
		{
			.name = detail::ToString(room.getName()),
			.playerCount = room.getPlayerCount(),
			.maxPlayers = room.getMaxPlayers(),
			.isOpen = room.getIsOpen(),
			.properties = detail::PhotonHashtableToStringHashTable(room.getCustomProperties()),
		};

		return roomInfo;
	}

	String Multiplayer_Photon::getCurrentRoomName() const
	{
		if (not m_client)
		{
			return{};
		}

		if (not m_client->getIsInGameRoom())
		{
			return{};
		}

		return detail::ToString(m_client->getCurrentlyJoinedRoom().getName());
	}

	Array<LocalPlayer> Multiplayer_Photon::getLocalPlayers() const
	{
		if (not m_client)
		{
			return{};
		}

		if (not m_client->getIsInGameRoom())
		{
			return{};
		}

		Array<LocalPlayer> results;

		const auto& players = m_client->getCurrentlyJoinedRoom().getPlayers();

		for (uint32 i = 0; i < players.getSize(); ++i)
		{
			const auto& player = players[i];

			LocalPlayer localPlayer
			{
				.localID = player->getNumber(),
				.userName = detail::ToString(player->getName()),
				.userID = detail::ToString(player->getUserID()),
				.isHost = player->getIsMasterClient(),
				.isActive = (not player->getIsInactive()),
			};

			results << std::move(localPlayer);
		}

		return results;
	}

	int32 Multiplayer_Photon::getPlayerCountInCurrentRoom() const
	{
		if (not m_client)
		{
			return 0;
		}

		if (not m_client->getIsInGameRoom())
		{
			return 0;
		}

		return m_client->getCurrentlyJoinedRoom().getPlayerCount();
	}

	int32 Multiplayer_Photon::getMaxPlayersInCurrentRoom() const
	{
		if (not m_client)
		{
			return 0;
		}

		if (not m_client->getIsInGameRoom())
		{
			return 0;
		}

		return m_client->getCurrentlyJoinedRoom().getMaxPlayers();
	}

	bool Multiplayer_Photon::getIsOpenInCurrentRoom() const
	{
		if (not m_client)
		{
			return false;
		}

		return m_client->getCurrentlyJoinedRoom().getIsOpen();
	}

	bool Multiplayer_Photon::getIsVisibleInCurrentRoom() const
	{
		if (not m_client)
		{
			return false;
		}

		return m_client->getCurrentlyJoinedRoom().getIsVisible();
	}

	void Multiplayer_Photon::setIsOpenInCurrentRoom(const bool isOpen)
	{
		if (not m_client)
		{
			return;
		}

		m_client->getCurrentlyJoinedRoom().setIsOpen(isOpen);
	}

	void Multiplayer_Photon::setIsVisibleInCurrentRoom(const bool isVisible)
	{
		if (not m_client)
		{
			return;
		}

		m_client->getCurrentlyJoinedRoom().setIsVisible(isVisible);
	}

	String Multiplayer_Photon::getRoomProperty(uint8 key) const
	{
		if (not m_client)
		{
			return {};
		}

		if (not m_client->getIsInGameRoom())
		{
			return {};
		}

		const auto& properties = m_client->getCurrentlyJoinedRoom().getCustomProperties();

		if (auto object = properties.getValue(detail::ToJString(String(1, static_cast<char32>(key))))) {
			return detail::ToString(detail::ObjectToJString(*object));
		}

		return {};
	}

	HashTable<uint8, String> Multiplayer_Photon::getRoomProperties() const
	{
		if (not m_client)
		{
			return{};
		}

		if (not m_client->getIsInGameRoom())
		{
			return{};
		}

		return detail::PhotonHashtableToStringHashTable(m_client->getCurrentlyJoinedRoom().getCustomProperties());
	}

	void Multiplayer_Photon::setRoomProperty(uint8 key, StringView value)
	{
		if (not m_client)
		{
			return;
		}

		if (not m_client->getIsInGameRoom())
		{
			return;
		}

		if (key < 0 or 255 < key)
		{
			throw Error{ U"[Multiplayer_Photon] KeyCode must be in a range of 0 to 255" };
		}

		bool result = m_client->getCurrentlyJoinedRoom().addCustomProperty(detail::ToJString(String(1, static_cast<char32>(key))), detail::ToJString(value));

		if (not result)
		{
			return;
		}

		auto jKeys = m_client->getCurrentlyJoinedRoom().getPropsListedInLobby();

		jKeys.addElement(detail::ToJString(String(1, static_cast<char32>(key))));

		m_client->getCurrentlyJoinedRoom().setPropsListedInLobby(jKeys);
	}

	int32 Multiplayer_Photon::GetSystemTimeMillisec()
	{
		return GETTIMEMS();
	}
}

# if SIV3D_MULTIPLAYER_PHOTON_LEGACY
/// [LEGACY] Multiplayer_Photon::customEventAction
namespace s3d
{
	namespace detail
	{
		template <class Type>
		void PrintCustomEventAction(const Multiplayer_Photon& photon, const StringView type, const LocalPlayerID playerID, const uint8 eventCode, const Type& data)
		{
			photon.debugLog(U"[Multiplayer_Photon] Multiplayer_Photon::customEventAction(", type, U")");
			photon.debugLog(U"- [Multiplayer_Photon] playerID: ", playerID);
			photon.debugLog(U"- [Multiplayer_Photon] eventCode: ", eventCode);
			photon.debugLog(U"- [Multiplayer_Photon] data: ", data);
		}
	}

	void Multiplayer_Photon::customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const bool data)
	{
		detail::PrintCustomEventAction(*this, U"bool", playerID, eventCode, data);
	}

	void Multiplayer_Photon::customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const uint8 data)
	{
		detail::PrintCustomEventAction(*this, U"uint8", playerID, eventCode, data);
	}

	void Multiplayer_Photon::customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const int16 data)
	{
		detail::PrintCustomEventAction(*this, U"int16", playerID, eventCode, data);
	}

	void Multiplayer_Photon::customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const int32 data)
	{
		detail::PrintCustomEventAction(*this, U"int32", playerID, eventCode, data);
	}

	void Multiplayer_Photon::customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const int64 data)
	{
		detail::PrintCustomEventAction(*this, U"int64", playerID, eventCode, data);
	}

	void Multiplayer_Photon::customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const float data)
	{
		detail::PrintCustomEventAction(*this, U"float", playerID, eventCode, data);
	}

	void Multiplayer_Photon::customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const double data)
	{
		detail::PrintCustomEventAction(*this, U"double", playerID, eventCode, data);
	}

	void Multiplayer_Photon::customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const String& data)
	{
		detail::PrintCustomEventAction(*this, U"String", playerID, eventCode, data);
	}

	void Multiplayer_Photon::customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const Array<bool>& data)
	{
		detail::PrintCustomEventAction(*this, U"Array<bool>", playerID, eventCode, data);
	}

	void Multiplayer_Photon::customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const Array<uint8>& data)
	{
		detail::PrintCustomEventAction(*this, U"Array<uint8>", playerID, eventCode, data);
	}

	void Multiplayer_Photon::customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const Array<int16>& data)
	{
		detail::PrintCustomEventAction(*this, U"Array<int16>", playerID, eventCode, data);
	}

	void Multiplayer_Photon::customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const Array<int32>& data)
	{
		detail::PrintCustomEventAction(*this, U"Array<int32>", playerID, eventCode, data);
	}

	void Multiplayer_Photon::customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const Array<int64>& data)
	{
		detail::PrintCustomEventAction(*this, U"Array<int64>", playerID, eventCode, data);
	}

	void Multiplayer_Photon::customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const Array<float>& data)
	{
		detail::PrintCustomEventAction(*this, U"Array<float>", playerID, eventCode, data);
	}

	void Multiplayer_Photon::customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const Array<double>& data)
	{
		detail::PrintCustomEventAction(*this, U"Array<double>", playerID, eventCode, data);
	}

	void Multiplayer_Photon::customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const Array<String>& data)
	{
		detail::PrintCustomEventAction(*this, U"Array<String>", playerID, eventCode, data);
	}

	void Multiplayer_Photon::customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const Color& data)
	{
		detail::PrintCustomEventAction(*this, U"Color", playerID, eventCode, data);
	}

	void Multiplayer_Photon::customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const ColorF& data)
	{
		detail::PrintCustomEventAction(*this, U"ColorF", playerID, eventCode, data);
	}

	void Multiplayer_Photon::customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const HSV& data)
	{
		detail::PrintCustomEventAction(*this, U"HSV", playerID, eventCode, data);
	}

	void Multiplayer_Photon::customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const Point& data)
	{
		detail::PrintCustomEventAction(*this, U"Point", playerID, eventCode, data);
	}

	void Multiplayer_Photon::customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const Vec2& data)
	{
		detail::PrintCustomEventAction(*this, U"Vec2", playerID, eventCode, data);
	}

	void Multiplayer_Photon::customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const Vec3& data)
	{
		detail::PrintCustomEventAction(*this, U"Vec3", playerID, eventCode, data);
	}

	void Multiplayer_Photon::customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const Vec4& data)
	{
		detail::PrintCustomEventAction(*this, U"Vec4", playerID, eventCode, data);
	}

	void Multiplayer_Photon::customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const Float2& data)
	{
		detail::PrintCustomEventAction(*this, U"Float2", playerID, eventCode, data);
	}

	void Multiplayer_Photon::customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const Float3& data)
	{
		detail::PrintCustomEventAction(*this, U"Float3", playerID, eventCode, data);
	}

	void Multiplayer_Photon::customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const Float4& data)
	{
		detail::PrintCustomEventAction(*this, U"Float4", playerID, eventCode, data);
	}

	void Multiplayer_Photon::customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const Mat3x2& data)
	{
		detail::PrintCustomEventAction(*this, U"Mat3x2", playerID, eventCode, data);
	}

	void Multiplayer_Photon::customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const Rect& data)
	{
		detail::PrintCustomEventAction(*this, U"Rect", playerID, eventCode, data);
	}

	void Multiplayer_Photon::customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const Circle& data)
	{
		detail::PrintCustomEventAction(*this, U"Circle", playerID, eventCode, data);
	}

	void Multiplayer_Photon::customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const Line& data)
	{
		detail::PrintCustomEventAction(*this, U"Line", playerID, eventCode, data);
	}

	void Multiplayer_Photon::customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const Triangle& data)
	{
		detail::PrintCustomEventAction(*this, U"Triangle", playerID, eventCode, data);
	}

	void Multiplayer_Photon::customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const RectF& data)
	{
		detail::PrintCustomEventAction(*this, U"RectF", playerID, eventCode, data);
	}

	void Multiplayer_Photon::customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const Quad& data)
	{
		detail::PrintCustomEventAction(*this, U"Quad", playerID, eventCode, data);
	}

	void Multiplayer_Photon::customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const Ellipse& data)
	{
		detail::PrintCustomEventAction(*this, U"Ellipse", playerID, eventCode, data);
	}

	void Multiplayer_Photon::customEventAction(const LocalPlayerID playerID, const uint8 eventCode, const RoundRect& data)
	{
		detail::PrintCustomEventAction(*this, U"RoundRect", playerID, eventCode, data);
	}
}
# endif

namespace s3d
{
	template<>
	void Multiplayer_Photon::sendEvent<>(const MultiplayerEvent& event)
	{
		sendEvent(event, Serializer<MemoryWriter> {});
	}
}
