// Filter "Chart/Rendering"

#include "Core_Color.hpp"
#include "Core_Math.hpp"

#include "ChartMeshes.hpp"

namespace NoteColor
{
	constexpr Atrium::Color32 Green(0xFF427D05);
	constexpr Atrium::Color32 Red(0xFFDE0000);
	constexpr Atrium::Color32 Yellow(0xFFD6D30D);
	constexpr Atrium::Color32 Blue(0xFF106ED0);
	constexpr Atrium::Color32 Orange(0xFFEF9612);
}

namespace FretAtlas
{
	constexpr Atrium::Math::Rectangle ToUV(float u, float v, float width, float height)
	{
		return Atrium::Math::Rectangle(
			Atrium::PointT<float>((u + width) / 1024.f, (v + height) / 1024.f),
			Atrium::SizeT<float>((-width) / 1024.f, (-height) / 1024.f)
		);
	}

	constexpr Atrium::Math::Rectangle White = ToUV(256, 330, 4, 4);

	constexpr Atrium::Math::Rectangle Note_Shell_Strum = ToUV(0, 256, 128, 64);
	constexpr Atrium::Math::Rectangle Note_Shell_Tap = ToUV(0, 192, 128, 64);
	constexpr Atrium::Math::Rectangle Note_Shell_HOPO = ToUV(0, 128, 128, 64);

	constexpr Atrium::Math::Rectangle Note_Color = ToUV(128, 256, 128, 64);
	constexpr Atrium::Math::Rectangle Note_Color_Tap = ToUV(128, 192, 128, 64);

	constexpr Atrium::Math::Rectangle Note_Target_L2_Head = ToUV(0, 0, 128, 64);
	constexpr Atrium::Math::Rectangle Note_Target_L2_Base = ToUV(0, 64, 128, 64);
	constexpr Atrium::Math::Rectangle Note_Target_L1_Head = ToUV(128, 0, 128, 64);
	constexpr Atrium::Math::Rectangle Note_Target_L1_Base = ToUV(128, 64, 128, 64);
	constexpr Atrium::Math::Rectangle Note_Target_C0_Head = ToUV(256, 0, 128, 64);
	constexpr Atrium::Math::Rectangle Note_Target_C0_Base = ToUV(256, 64, 128, 64);
	constexpr Atrium::Math::Rectangle Note_Target_R1_Head = ToUV(256, 0, -128, 64);
	constexpr Atrium::Math::Rectangle Note_Target_R1_Base = ToUV(256, 64, -128, 64);
	constexpr Atrium::Math::Rectangle Note_Target_R2_Head = ToUV(128, 0, -128, 64);
	constexpr Atrium::Math::Rectangle Note_Target_R2_Base = ToUV(128, 64, -128, 64);
}

namespace FretboardMatrices
{
	constexpr Atrium::Math::Vector3 CameraPosition(0, 0.9f, -0.55f);
	constexpr Atrium::Math::Matrix CameraTranslation = Atrium::Math::Matrix::CreateTranslation(FretboardMatrices::CameraPosition.X, FretboardMatrices::CameraPosition.Y, FretboardMatrices::CameraPosition.Z);
	constexpr Atrium::Math::Matrix CameraRotation = Atrium::Math::Matrix::CreateRotationX(Atrium::Math::ToRadians(30.f));

	constexpr Atrium::Math::Matrix CameraViewMatrix = (CameraRotation * CameraTranslation).Inverse();

	constexpr Atrium::Math::Matrix CameraProjectionMatrix = Atrium::Math::Matrix::CreatePerspectiveFieldOfView(Atrium::Math::ToRadians(55.f), 1.f, 0.0001f, 100.f);

	constexpr float String_Offset[] =
	{
		(-0.4803f) + (((0.4803f * 2) / 6) * 1.f),
		(-0.4803f) + (((0.4803f * 2) / 6) * 2.f),
		(-0.4803f) + (((0.4803f * 2) / 6) * 3.f),
		(-0.4803f) + (((0.4803f * 2) / 6) * 4.f),
		(-0.4803f) + (((0.4803f * 2) / 6) * 5.f)
	};

	constexpr Atrium::Math::Matrix String_Translations[] = {
		Atrium::Math::Matrix::CreateTranslation(String_Offset[0], 0, 0),
		Atrium::Math::Matrix::CreateTranslation(String_Offset[1], 0, 0),
		Atrium::Math::Matrix::CreateTranslation(String_Offset[2], 0, 0),
		Atrium::Math::Matrix::CreateTranslation(String_Offset[3], 0, 0),
		Atrium::Math::Matrix::CreateTranslation(String_Offset[4], 0, 0)
	};

	constexpr Atrium::Math::Matrix String_Base
		= Atrium::Math::Matrix::CreateTranslation(-0.5f, 0, 0)
		* Atrium::Math::Matrix::CreateScale(0.120075f, 1.9212f, 0)
		* Atrium::Math::Matrix::CreateRotationX(Atrium::Math::HalfPi)
		;

	constexpr Atrium::Math::Matrix Strings[] = {
		String_Base * String_Translations[0],
		String_Base * String_Translations[1],
		String_Base * String_Translations[2],
		String_Base * String_Translations[3],
		String_Base * String_Translations[4]
	};

	constexpr Atrium::Math::Matrix FaceCamera(Atrium::Math::Vector3 aPosition, Atrium::Math::Vector2 aSize = { 1, 1 }, Atrium::Math::Vector2 anAnchor = { 0.5f, 0.5f })
	{
		return
			Atrium::Math::Matrix::CreateTranslation(-anAnchor.X, -anAnchor.Y, 0)
			* Atrium::Math::Matrix::CreateRotationY(Atrium::Math::Pi)
			* Atrium::Math::Matrix::CreateScale(aSize.X, aSize.Y, 0)
			* Atrium::Math::Matrix::CreateRotationX(Atrium::Math::ToRadians(30.f))
			* Atrium::Math::Matrix::CreateTranslation(aPosition.X, aPosition.Y, aPosition.Z)
			;
	}

	constexpr float TargetOffset = 0.15f;

	constexpr Atrium::Math::Matrix Targets[] = {
		FaceCamera({ String_Offset[0], 0, TargetOffset }, { 0.18f, 0.09f }, { 0.5f, 0.f }),
		FaceCamera({ String_Offset[1], 0, TargetOffset }, { 0.18f, 0.09f }, { 0.5f, 0.f }),
		FaceCamera({ String_Offset[2], 0, TargetOffset }, { 0.18f, 0.09f }, { 0.5f, 0.f }),
		FaceCamera({ String_Offset[3], 0, TargetOffset }, { 0.18f, 0.09f }, { 0.5f, 0.f }),
		FaceCamera({ String_Offset[4], 0, TargetOffset }, { 0.18f, 0.09f }, { 0.5f, 0.f })
	};
}