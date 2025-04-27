// Filter "Chart/Rendering"

#include "Atrium_Color.hpp"
#include "Atrium_Math.hpp"

#include "ChartMeshes.hpp"

namespace NoteColor
{
	constexpr Atrium::Color32 Green(0xFF59D606);
	constexpr Atrium::Color32 Red(0xFFDE0000);
	constexpr Atrium::Color32 Yellow(0xFFEFE810);
	constexpr Atrium::Color32 Blue(0xFF1280ED);
	constexpr Atrium::Color32 Orange(0xFFEF9612);

	constexpr Atrium::Color32 Open(0xFF9225C5);
}

namespace FretAtlas
{
	constexpr Atrium::RectangleF ToUV(float u, float v, float width, float height)
	{
		return Atrium::RectangleF(
			Atrium::PointF((u + width) / 1024.f, (v + height) / 1024.f),
			Atrium::SizeF((-width) / 1024.f, (-height) / 1024.f)
		);
	}

	constexpr Atrium::RectangleF Note_Cap_Neutral = ToUV(0, 0, 128, 64);
	constexpr Atrium::RectangleF Note_Cap_HOPO = ToUV(128, 0, 128, 64);
	constexpr Atrium::RectangleF Note_Body = ToUV(0, 64, 128, 64);
	constexpr Atrium::RectangleF Note_Body_Tap = ToUV(128, 64, 128, 64);
	constexpr Atrium::RectangleF Note_Base = ToUV(0, 128, 128, 64);

	constexpr Atrium::RectangleF Note_Open_Cap_HOPO = ToUV(384, 0, 384, 64);
	constexpr Atrium::RectangleF Note_Open_Cap_Neutral = ToUV(384, 64, 384, 64);
	constexpr Atrium::RectangleF Note_Open_Body = ToUV(384, 128, 384, 64);
	constexpr Atrium::RectangleF Note_Open_Base = ToUV(384, 192, 384, 64);

	constexpr Atrium::RectangleF Sustain_Neutral = ToUV(0, 448, 128, 64);
	constexpr Atrium::RectangleF Sustain_Active = ToUV(128, 448, 128, 64);
	constexpr Atrium::RectangleF Sustain_Missed = ToUV(256, 448, 128, 64);
	constexpr Atrium::RectangleF Sustain_Open = ToUV(384, 448, 384, 64);

	constexpr Atrium::RectangleF Target_Cap_Active = ToUV(128, 128, 128, 64);
	constexpr Atrium::RectangleF Target_Cap_Neutral = ToUV(256, 128, 128, 64);
	constexpr Atrium::RectangleF Target_ColorRing = ToUV(256, 192, 128, 64);
	constexpr Atrium::RectangleF Target_Head = ToUV(256, 256, 128, 64);
	constexpr Atrium::RectangleF Target_Ring = ToUV(128, 256, 128, 64);
	constexpr Atrium::RectangleF Target_Base_0 = ToUV(0, 320, 128, 64);
	constexpr Atrium::RectangleF Target_Base_1 = ToUV(128, 320, 128, 64);
	constexpr Atrium::RectangleF Target_Base_2 = ToUV(256, 320, 128, 64);
	constexpr Atrium::RectangleF Target_Base_3 = ToUV(256, 320, -128, 64);
	constexpr Atrium::RectangleF Target_Base_4 = ToUV(128, 320, -128, 64);
};

namespace FretboardMatrices
{
	constexpr Atrium::Vector3 CameraPosition(0, 0.9f, -0.55f);
	constexpr Atrium::Matrix CameraTranslation = Atrium::Matrix::CreateTranslation(FretboardMatrices::CameraPosition.X, FretboardMatrices::CameraPosition.Y, FretboardMatrices::CameraPosition.Z);
	constexpr Atrium::Matrix CameraRotation = Atrium::Matrix::CreateRotationX(Atrium::Math::ToRadians(30.f));

	constexpr Atrium::Matrix CameraViewMatrix = (CameraRotation * CameraTranslation).Inverse();

	constexpr Atrium::Matrix CameraProjectionMatrix = Atrium::Matrix::CreatePerspectiveFieldOfView(Atrium::Math::ToRadians(55.f), 1.f, 0.0001f, 100.f);

	constexpr float String_Offset[] =
	{
		(-0.4803f) + (((0.4803f * 2) / 6) * 1.f),
		(-0.4803f) + (((0.4803f * 2) / 6) * 2.f),
		(-0.4803f) + (((0.4803f * 2) / 6) * 3.f),
		(-0.4803f) + (((0.4803f * 2) / 6) * 4.f),
		(-0.4803f) + (((0.4803f * 2) / 6) * 5.f)
	};

	constexpr Atrium::Matrix String_Base
		= Atrium::Matrix::CreateTranslation(-0.5f, 0, 0)
		* Atrium::Matrix::CreateRotationY(Atrium::Math::Pi)
		* Atrium::Matrix::CreateScale(0.120075f, 1.9212f, 0)
		* Atrium::Matrix::CreateRotationX(Atrium::Math::HalfPi)
		;

	constexpr Atrium::Matrix Strings[] = {
		String_Base * Atrium::Matrix::CreateTranslation(String_Offset[0], 0, 0),
		String_Base * Atrium::Matrix::CreateTranslation(String_Offset[1], 0, 0),
		String_Base * Atrium::Matrix::CreateTranslation(String_Offset[2], 0, 0),
		String_Base * Atrium::Matrix::CreateTranslation(String_Offset[3], 0, 0),
		String_Base * Atrium::Matrix::CreateTranslation(String_Offset[4], 0, 0)
	};

	constexpr Atrium::Matrix FaceCamera(Atrium::Vector3 aPosition, Atrium::Vector2 aSize = { 1, 1 }, Atrium::Vector2 anAnchor = { 0.5f, 0.5f })
	{
		return
			Atrium::Matrix::CreateTranslation(-anAnchor.X, -anAnchor.Y, 0)
			* Atrium::Matrix::CreateRotationY(Atrium::Math::Pi)
			* Atrium::Matrix::CreateScale(aSize.X, aSize.Y, 0)
			* Atrium::Matrix::CreateRotationX(Atrium::Math::ToRadians(30.f))
			* Atrium::Matrix::CreateTranslation(aPosition.X, aPosition.Y, aPosition.Z)
			;
	}

	constexpr float TargetOffset = 0.15f;

	constexpr Atrium::Matrix Targets[] = {
		FaceCamera({ String_Offset[0], 0, TargetOffset }, { 0.18f, 0.09f }, { 0.5f, 0.f }),
		FaceCamera({ String_Offset[1], 0, TargetOffset }, { 0.18f, 0.09f }, { 0.5f, 0.f }),
		FaceCamera({ String_Offset[2], 0, TargetOffset }, { 0.18f, 0.09f }, { 0.5f, 0.f }),
		FaceCamera({ String_Offset[3], 0, TargetOffset }, { 0.18f, 0.09f }, { 0.5f, 0.f }),
		FaceCamera({ String_Offset[4], 0, TargetOffset }, { 0.18f, 0.09f }, { 0.5f, 0.f })
	};

	constexpr Atrium::Matrix OpenTarget =
		FaceCamera({ String_Offset[2], 0, TargetOffset }, { 4*0.18f, 4*0.03f }, { 0.5f, 0.f })
		;

	constexpr Atrium::Matrix Sustain_Base
		= Atrium::Matrix::CreateTranslation(-0.5f, 0, 0)
		* Atrium::Matrix::CreateRotationY(Atrium::Math::Pi)
		* Atrium::Matrix::CreateScale(0.18f, 1.f, 0)
		* Atrium::Matrix::CreateRotationX(Atrium::Math::HalfPi)
		;

	constexpr Atrium::Matrix Sustain_Roots[] = {
		Sustain_Base * Atrium::Matrix::CreateTranslation(String_Offset[0], 0, 0),
		Sustain_Base * Atrium::Matrix::CreateTranslation(String_Offset[1], 0, 0),
		Sustain_Base * Atrium::Matrix::CreateTranslation(String_Offset[2], 0, 0),
		Sustain_Base * Atrium::Matrix::CreateTranslation(String_Offset[3], 0, 0),
		Sustain_Base * Atrium::Matrix::CreateTranslation(String_Offset[4], 0, 0)
	};

	constexpr Atrium::Matrix Sustain_Open
		= Atrium::Matrix::CreateTranslation(-0.5f, 0, 0)
		* Atrium::Matrix::CreateRotationY(Atrium::Math::Pi)
		* Atrium::Matrix::CreateScale(4*0.18f, 1.f, 0)
		* Atrium::Matrix::CreateRotationX(Atrium::Math::HalfPi)
		* Atrium::Matrix::CreateTranslation(String_Offset[2], 0, 0)
		;
}