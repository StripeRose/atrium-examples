// Filter "Chart/Rendering"

#include "Common_Color.hpp"
#include "Common_Math.hpp"

#include "ChartMeshes.hpp"

namespace NoteColor
{
	constexpr RoseGold::Color32 Green(0xFF427D05);
	constexpr RoseGold::Color32 Red(0xFFDE0000);
	constexpr RoseGold::Color32 Yellow(0xFFD6D30D);
	constexpr RoseGold::Color32 Blue(0xFF106ED0);
	constexpr RoseGold::Color32 Orange(0xFFEF9612);
}

namespace FretAtlas
{
	constexpr RoseGold::Math::Rectangle ToUV(float u, float v, float width, float height)
	{
		return RoseGold::Math::Rectangle::FromExtents({ (u + width) / 1024.f, (v + height) / 1024.f }, { u / 1024.f, v / 1024.f });
	}

	constexpr RoseGold::Math::Rectangle White = ToUV(256, 330, 4, 4);

	constexpr RoseGold::Math::Rectangle Note_Shell_Strum = ToUV(0, 256, 128, 64);
	constexpr RoseGold::Math::Rectangle Note_Shell_Tap = ToUV(0, 192, 128, 64);
	constexpr RoseGold::Math::Rectangle Note_Shell_HOPO = ToUV(0, 128, 128, 64);

	constexpr RoseGold::Math::Rectangle Note_Color = ToUV(128, 256, 128, 64);
	constexpr RoseGold::Math::Rectangle Note_Color_Tap = ToUV(128, 192, 128, 64);

	constexpr RoseGold::Math::Rectangle Note_Target_L2_Head = ToUV(0, 0, 128, 64);
	constexpr RoseGold::Math::Rectangle Note_Target_L2_Base = ToUV(0, 64, 128, 64);
	constexpr RoseGold::Math::Rectangle Note_Target_L1_Head = ToUV(128, 0, 128, 64);
	constexpr RoseGold::Math::Rectangle Note_Target_L1_Base = ToUV(128, 64, 128, 64);
	constexpr RoseGold::Math::Rectangle Note_Target_C0_Head = ToUV(256, 0, 128, 64);
	constexpr RoseGold::Math::Rectangle Note_Target_C0_Base = ToUV(256, 64, 128, 64);
	constexpr RoseGold::Math::Rectangle Note_Target_R1_Head = ToUV(256, 0, -128, 64);
	constexpr RoseGold::Math::Rectangle Note_Target_R1_Base = ToUV(256, 64, -128, 64);
	constexpr RoseGold::Math::Rectangle Note_Target_R2_Head = ToUV(128, 0, -128, 64);
	constexpr RoseGold::Math::Rectangle Note_Target_R2_Base = ToUV(128, 64, -128, 64);
}

namespace FretboardMatrices
{
	constexpr RoseGold::Math::Vector3 CameraPosition(0, 0.9f, -0.55f);
	constexpr RoseGold::Math::Matrix CameraTranslation = RoseGold::Math::MakeMatrix::Translation(FretboardMatrices::CameraPosition.X, FretboardMatrices::CameraPosition.Y, FretboardMatrices::CameraPosition.Z);
	constexpr RoseGold::Math::Matrix CameraRotation = RoseGold::Math::MakeMatrix::RotationX(RoseGold::Math::ToRadians(30.f));

	constexpr RoseGold::Math::Matrix CameraViewMatrix = (CameraRotation * CameraTranslation).Invert().value();

	constexpr RoseGold::Math::Matrix CameraProjectionMatrix = RoseGold::Math::MakeMatrix::PerspectiveFieldOfView(RoseGold::Math::ToRadians(55.f), 1.f, 0.0001f, 100.f);

	constexpr float String_Offset[] =
	{
		(-0.4803f) + (((0.4803f * 2) / 6) * 1.f),
		(-0.4803f) + (((0.4803f * 2) / 6) * 2.f),
		(-0.4803f) + (((0.4803f * 2) / 6) * 3.f),
		(-0.4803f) + (((0.4803f * 2) / 6) * 4.f),
		(-0.4803f) + (((0.4803f * 2) / 6) * 5.f)
	};

	constexpr RoseGold::Math::Matrix String_Translations[] = {
		RoseGold::Math::MakeMatrix::Translation(String_Offset[0], 0, 0),
		RoseGold::Math::MakeMatrix::Translation(String_Offset[1], 0, 0),
		RoseGold::Math::MakeMatrix::Translation(String_Offset[2], 0, 0),
		RoseGold::Math::MakeMatrix::Translation(String_Offset[3], 0, 0),
		RoseGold::Math::MakeMatrix::Translation(String_Offset[4], 0, 0)
	};

	constexpr RoseGold::Math::Matrix String_Base
		= RoseGold::Math::MakeMatrix::Translation(-0.5f, 0, 0)
		* RoseGold::Math::MakeMatrix::Scale(0.120075f, 1.9212f, 0)
		* RoseGold::Math::MakeMatrix::RotationX(RoseGold::Math::HalfPi<float>)
		;

	constexpr RoseGold::Math::Matrix Strings[] = {
		String_Base * String_Translations[0],
		String_Base * String_Translations[1],
		String_Base * String_Translations[2],
		String_Base * String_Translations[3],
		String_Base * String_Translations[4]
	};

	constexpr RoseGold::Math::Matrix FaceCamera(RoseGold::Math::Vector3 aPosition, RoseGold::Math::Vector2 aSize = { 1, 1 }, RoseGold::Math::Vector2 anAnchor = { 0.5f, 0.5f })
	{
		return
			RoseGold::Math::MakeMatrix::Translation(-anAnchor.X, -anAnchor.Y, 0)
			* RoseGold::Math::MakeMatrix::RotationY(RoseGold::Math::Pi<float>)
			* RoseGold::Math::MakeMatrix::Scale(aSize.X, aSize.Y, 0)
			* CameraRotation
			* RoseGold::Math::MakeMatrix::Translation(aPosition.X, aPosition.Y, aPosition.Z)
			;
	}

	constexpr float TargetOffset = 0.15f;

	constexpr RoseGold::Math::Matrix Targets[] = {
		FaceCamera({ String_Offset[0], 0, TargetOffset }, { 0.18f, 0.09f }, { 0.5f, 0.f }),
		FaceCamera({ String_Offset[1], 0, TargetOffset }, { 0.18f, 0.09f }, { 0.5f, 0.f }),
		FaceCamera({ String_Offset[2], 0, TargetOffset }, { 0.18f, 0.09f }, { 0.5f, 0.f }),
		FaceCamera({ String_Offset[3], 0, TargetOffset }, { 0.18f, 0.09f }, { 0.5f, 0.f }),
		FaceCamera({ String_Offset[4], 0, TargetOffset }, { 0.18f, 0.09f }, { 0.5f, 0.f })
	};
}