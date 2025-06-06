// Filter "Chart/Rendering"
#include "ChartRenderer.hpp"

#include "ChartData.hpp"
#include "ChartMeshes.hpp"
#include "ChartPlayer.hpp"
#include "ChartTrack.hpp"

#include "Atrium_GUI.hpp"

#include "FretAtlas.hpp"

static std::chrono::microseconds LookAhead = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::seconds(3));
static float NotePositionAdjustment = 0.015f;
static float SustainPositionAdjustment = 0.0f;

ChartRenderer::ChartRenderer(ChartPlayer& aPlayer)
	: myPlayer(aPlayer)
{ }

#if IS_IMGUI_ENABLED
void ChartRenderer::ImGui()
{
	ImGui::TextUnformatted("Adjust");
	
	{
		int lookaheadMicroseconds = static_cast<int>(LookAhead.count());
		const auto lookaheadSeconds = std::chrono::duration_cast<std::chrono::seconds>(LookAhead);
		if (ImGui::SliderInt("Debug look-ahead", &lookaheadMicroseconds, 0, static_cast<int>(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::seconds(15)).count()), std::format("{} seconds", lookaheadSeconds.count()).c_str()))
			LookAhead = std::chrono::microseconds(lookaheadMicroseconds);
	}

	ImGui::DragFloat("Note position:", &NotePositionAdjustment, 0.0001f);
	ImGui::DragFloat("Sustain position:", &SustainPositionAdjustment, 0.0001f);
}
#endif

void ChartRenderer::SetupResources(Atrium::GraphicsAPI& aGraphicsAPI, Atrium::GraphicsFormat aColorTargetFormat)
{
	ZoneScoped;

	std::unique_ptr<Atrium::RootSignatureBuilder> builder = aGraphicsAPI.GetResourceManager().CreateRootSignature();

	builder->SetVisibility(Atrium::Shader::Type::Vertex);

	// Model, view, projection.
	builder->AddTable().AddCBVRange(1, 0, Atrium::ResourceUpdateFrequency::PerObject);
	builder->AddTable().AddCBVRange(1, 0, Atrium::ResourceUpdateFrequency::PerFrame);

	builder->SetVisibility(Atrium::Shader::Type::Pixel);

	builder->AddTable().AddSRVRange(4, 0, Atrium::ResourceUpdateFrequency::PerMaterial);

	builder->AddSampler(0) // Clamping point
		.Filter(Atrium::FilterMode::Point)
		.Address(Atrium::TextureWrapMode::Clamp)
		;

	builder->AddSampler(1) // Clamping linear
		.Filter(Atrium::FilterMode::Bilinear)
		.Address(Atrium::TextureWrapMode::Clamp)
		;

	std::shared_ptr<Atrium::RootSignature> rootSignature = builder->Finalize();

	myQuadRenderer.Setup(aGraphicsAPI, rootSignature, aColorTargetFormat);
	myQuadRenderer.SetTexture(aGraphicsAPI.GetResourceManager().LoadTexture("fretatlas.dds"));

	myFretboardRenderer.Setup(aGraphicsAPI, rootSignature, aColorTargetFormat);
	myFretboardRenderer.SetTexture(aGraphicsAPI.GetResourceManager().LoadTexture("fretboard.dds"));
}

void ChartRenderer::Render(Atrium::FrameGraphicsContext& aContext, const std::shared_ptr<Atrium::RenderTexture>& aTarget)
{
	ZoneScoped;
	CONTEXT_ZONE(aContext, "Chart");

	aContext.SetRenderTargets({ aTarget }, nullptr);
	aContext.SetViewportAndScissorRect(Atrium::Size(aTarget->GetWidth(), aTarget->GetHeight()));

	const std::vector<std::unique_ptr<ChartController>>& controllers = myPlayer.GetControllers();
	const std::vector<Atrium::RectangleF> controllerRects = GetControllerRectangles(
		Atrium::RectangleF(
			Atrium::PointF(0, 0),
			Atrium::SizeF(static_cast<float>(aTarget->GetWidth()), static_cast<float>(aTarget->GetHeight()))
		), controllers.size());

	for (unsigned int i = 0; i < controllers.size(); ++i)
	{
		ZoneScopedN("Controller");
		aContext.SetViewport(controllerRects[i]);

		myFretboardRenderer.Render(aContext);

		QueueTargets(*controllers.at(i));
		RenderController(*controllers.at(i));
		myQuadRenderer.Flush(i);
	}

	myQuadRenderer.Render(
		aContext,
		[&](std::size_t aGroup)
		{
			aContext.SetViewport(controllerRects[aGroup]);
		}
	);
}

std::pair<int, int> ChartRenderer::GetControllerRectanglesGrid(const Atrium::RectangleF& aTotalRectangle, float aGridCellAspectRatio, std::size_t aControllerCount) const
{
	if (aControllerCount == 0)
		return { 0, 0 };

	const float normalizedAspectRatio = (aTotalRectangle.Width / aTotalRectangle.Height) * aGridCellAspectRatio;

	const float columns = std::sqrt(aControllerCount * normalizedAspectRatio);
	const float rows = std::sqrt(aControllerCount / normalizedAspectRatio);

	std::optional<int> integerColumns, integerRows;
	auto pass =
		[aControllerCount, &integerColumns, &integerRows](int aColumnCount, int aRowCount)
		{
			if (aColumnCount * aRowCount < aControllerCount)
				return;

			if (!integerColumns.has_value() || aColumnCount * aRowCount < integerColumns.value() * integerRows.value())
			{
				integerColumns = aColumnCount;
				integerRows = aRowCount;
			}
		};

	pass(Atrium::Math::FloorTo<int>(columns), Atrium::Math::CeilingTo<int>(rows));
	pass(Atrium::Math::CeilingTo<int>(columns), Atrium::Math::FloorTo<int>(rows));
	pass(Atrium::Math::CeilingTo<int>(columns), Atrium::Math::CeilingTo<int>(rows));

	return { integerColumns.value(), integerRows.value() };
}

std::vector<Atrium::RectangleF> ChartRenderer::GetControllerRectangles(const Atrium::RectangleF& aTotalRectangle, std::size_t aControllerCount) const
{
	constexpr float CellAspectRatio = 1.f;

	const std::pair<int, int> gridCells = GetControllerRectanglesGrid(aTotalRectangle, CellAspectRatio, aControllerCount);

	if ((gridCells.first * gridCells.second) == 0)
		return { };

	const float gridAspectRatio = static_cast<float>(gridCells.first) / static_cast<float>(gridCells.second);
	const float gridWidth = Atrium::Math::Min<float>(aTotalRectangle.Width, (aTotalRectangle.Height * gridAspectRatio));
	const Atrium::SizeF gridSize(gridWidth, gridWidth / gridAspectRatio);
	const Atrium::RectangleF gridRect(
		aTotalRectangle.Center() - Atrium::Math::Vector2(gridSize.Width / 2, gridSize.Height / 2),
		gridSize);

	const Atrium::Math::Vector2 gridCellSize(gridRect.Width / gridCells.first, gridRect.Height / gridCells.second);

	std::vector<Atrium::RectangleF> rectsOut;

	for (std::size_t i = 0; i < aControllerCount; ++i)
	{
		const std::size_t cellX = i % gridCells.first;
		const std::size_t cellY = (i - cellX) / gridCells.first;

		const Atrium::Math::Vector2 position(
			(gridRect.Center().X - (gridRect.Width / 2)) + (cellX * gridCellSize.X),
			(gridRect.Center().Y + (gridRect.Height / 2)) - ((cellY + 1) * gridCellSize.Y)
		);

		rectsOut.push_back(Atrium::RectangleF(Atrium::PointF(position), Atrium::SizeF(gridCellSize)));
	}

	return rectsOut;
}

void ChartRenderer::RenderController(ChartController& aController)
{
	ZoneScoped;

	if (myPlayer.GetChartData() == nullptr)
		return;

	if (!myPlayer.GetChartData()->GetTracks().contains(aController.GetTrackType()))
		return;

	const ChartTrack& track = *myPlayer.GetChartData()->GetTracks().at(aController.GetTrackType()).get();

	switch (aController.GetTrackType())
	{
		case ChartTrackType::Drums:
			break;
		case ChartTrackType::LeadGuitar:
		case ChartTrackType::RhythmGuitar:
		case ChartTrackType::BassGuitar:
			RenderNotes(aController, static_cast<const ChartGuitarTrack&>(track));
			break;
		case ChartTrackType::Vocal_Main:
		case ChartTrackType::Vocal_Harmony:
			break;
	}
}

void ChartRenderer::RenderNotes(ChartController& aController, const ChartGuitarTrack& aTrack)
{
	const std::vector<ChartNoteRange>& difficultyNotes = aTrack.GetNoteRanges().at(aController.GetTrackDifficulty());
	for (const ChartNoteRange& note : difficultyNotes)
	{
		if (!note.IsSustain())
			continue;

		const std::optional<std::chrono::microseconds> sustainHitEnd = aController.GetNoteHitEnd(note);

		if (note.CanBeOpen && aController.AllowOpenNotes())
		{
			RenderNote_GuitarOpenSustain(note, sustainHitEnd);
		}
		else
		{
			SustainState state = SustainState::Neutral;
			if (aController.IsSustainActive(note))
				state = SustainState::Active;
			else if (aController.IsNoteMissed(note))
				state = SustainState::Missed;

			RenderNote_GuitarSustain(note, state, sustainHitEnd);
		}
	}

	for (auto note = difficultyNotes.crbegin(); note != difficultyNotes.crend(); ++note)
	{
		if (aController.GetNoteHitEnd(*note).has_value())
			continue;

		if (note->CanBeOpen && aController.AllowOpenNotes())
			RenderNote_GuitarOpen(*note);
		else
			RenderNote_Guitar(*note);
	}
}

void ChartRenderer::RenderNote_Guitar(const ChartNoteRange& aNote)
{
	const float notePosition = TimeToPositionOffset(aNote.Start) + NotePositionAdjustment;

	if (notePosition < -FretboardMatrices::TargetOffset || (FretboardLength - FretboardMatrices::TargetOffset) < notePosition)
		return;

	Atrium::Color32 noteColor;
	switch (aNote.Lane)
	{
		case 0:
			noteColor = NoteColor::Green;
			break;
		case 1:
			noteColor = NoteColor::Red;
			break;
		case 2:
			noteColor = NoteColor::Yellow;
			break;
		case 3:
			noteColor = NoteColor::Blue;
			break;
		case 4:
			noteColor = NoteColor::Orange;
			break;
	}

	Atrium::Matrix noteTransform = FretboardMatrices::Targets[aNote.Lane];

	noteTransform.SetTranslation4(
		noteTransform.GetTranslation4() +
		Atrium::Vector4::UnitZ() * notePosition
	);

	myQuadRenderer.Queue(noteTransform, noteColor,
		aNote.Type == ChartNoteType::Tap ? FretAtlas::Note_Body_Tap : FretAtlas::Note_Body
	);
	myQuadRenderer.Queue(noteTransform, {},
		FretAtlas::Note_Base
	);
	myQuadRenderer.Queue(noteTransform, {},
		aNote.Type == ChartNoteType::HOPO ? FretAtlas::Note_Cap_HOPO : FretAtlas::Note_Cap_Neutral
	);
}

void ChartRenderer::RenderNote_GuitarOpen(const ChartNoteRange& aNote)
{
	const float notePosition = TimeToPositionOffset(aNote.Start) + NotePositionAdjustment;

	if (notePosition < -FretboardMatrices::TargetOffset || (FretboardLength - FretboardMatrices::TargetOffset) < notePosition)
		return;

	if (aNote.Type != ChartNoteType::Strum)
		Atrium::Debug::LogWarning("Open notes that aren't of type strum? How does that make sense?");

	Atrium::Matrix noteTransform = FretboardMatrices::OpenTarget;

	noteTransform.SetTranslation4(
		noteTransform.GetTranslation4() +
		Atrium::Vector4::UnitZ() * notePosition
	);

	myQuadRenderer.Queue(noteTransform, NoteColor::Open, FretAtlas::Note_Open_Body);
	myQuadRenderer.Queue(noteTransform, {}, FretAtlas::Note_Open_Base);
	myQuadRenderer.Queue(noteTransform, {}, FretAtlas::Note_Open_Cap_Neutral);
}

void ChartRenderer::RenderNote_GuitarSustain(const ChartNoteRange& aNote, SustainState aState, std::optional<std::chrono::microseconds> anOverrideStart)
{
	const float sustainStart = TimeToPositionOffset(anOverrideStart.value_or(aNote.Start)) + SustainPositionAdjustment;
	const float sustainEnd = TimeToPositionOffset(aNote.End) + SustainPositionAdjustment;

	if (sustainEnd < -FretboardMatrices::TargetOffset || (FretboardLength - FretboardMatrices::TargetOffset) < sustainStart)
		return;

	Atrium::Color32 noteColor;
	switch (aNote.Lane)
	{
		case 0:
			noteColor = NoteColor::Green;
			break;
		case 1:
			noteColor = NoteColor::Red;
			break;
		case 2:
			noteColor = NoteColor::Yellow;
			break;
		case 3:
			noteColor = NoteColor::Blue;
			break;
		case 4:
			noteColor = NoteColor::Orange;
			break;
	}

	Atrium::Matrix sustainTransform
		= FretboardMatrices::Sustain_Roots[aNote.Lane]
		* Atrium::Matrix::CreateScale(1, 1, sustainEnd - sustainStart)
		;

	sustainTransform.SetTranslation4(
		sustainTransform.GetTranslation4() +
		Atrium::Vector4::UnitZ() * (FretboardMatrices::TargetOffset + 0.04f + sustainStart)
	);

	Atrium::RectangleF sustainRect = FretAtlas::Sustain_Neutral;

	switch (aState)
	{
		case SustainState::Missed:
			sustainRect = FretAtlas::Sustain_Missed;
			break;
		case SustainState::Neutral:
			sustainRect = FretAtlas::Sustain_Neutral;
			break;
		case SustainState::Active:
			sustainRect = FretAtlas::Sustain_Active;
			break;
	}

	myQuadRenderer.Queue(sustainTransform, noteColor, sustainRect);
}

void ChartRenderer::RenderNote_GuitarOpenSustain(const ChartNoteRange& aNote, std::optional<std::chrono::microseconds> anOverrideStart)
{
	const float sustainStart = TimeToPositionOffset(anOverrideStart.value_or(aNote.Start)) + SustainPositionAdjustment;
	const float sustainEnd = TimeToPositionOffset(aNote.End) + SustainPositionAdjustment;

	if (sustainEnd < -FretboardMatrices::TargetOffset || (FretboardLength - FretboardMatrices::TargetOffset) < sustainStart)
		return;

	Atrium::Matrix sustainTransform
		= FretboardMatrices::Sustain_Open
		* Atrium::Matrix::CreateScale(1, 1, sustainEnd - sustainStart)
		;

	sustainTransform.SetTranslation4(
		sustainTransform.GetTranslation4() +
		Atrium::Vector4::UnitZ() * (FretboardMatrices::TargetOffset + 0.04f + sustainStart)
	);

	myQuadRenderer.Queue(sustainTransform, NoteColor::Open, FretAtlas::Sustain_Open);
}

void ChartRenderer::QueueTargets(ChartController& aController)
{
	const std::span<const bool> laneStates = aController.GetLaneStates();

	auto drawTarget = [&](const int anIndex, const Atrium::Color32 aColor)
		{
			switch (anIndex)
			{
				case 0:
					myQuadRenderer.Queue(FretboardMatrices::Targets[0], {}, FretAtlas::Target_Base_0);
					break;
				case 1:
					myQuadRenderer.Queue(FretboardMatrices::Targets[1], {}, FretAtlas::Target_Base_1);
					break;
				case 2:
					myQuadRenderer.Queue(FretboardMatrices::Targets[2], {}, FretAtlas::Target_Base_2);
					break;
				case 3:
					myQuadRenderer.Queue(FretboardMatrices::Targets[3], {}, FretAtlas::Target_Base_3);
					break;
				case 4:
					myQuadRenderer.Queue(FretboardMatrices::Targets[4], {}, FretAtlas::Target_Base_4);
					break;
			}

			const bool isActive =
				laneStates.size() > anIndex
				? laneStates.data()[anIndex]
				: false;

			float heightAdjustment = (isActive ? -0.1f : 0.f);

			static constexpr std::chrono::microseconds StrumAnimationLength = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(150));

			const std::chrono::microseconds timeSinceStrum = aController.GetLastStrum() ? myPlayer.GetPlayhead() - aController.GetLaneLastStrum().data()[anIndex] : StrumAnimationLength;
			const float stateFade = Atrium::Math::Max(1.f - (static_cast<float>(timeSinceStrum.count()) / static_cast<float>(StrumAnimationLength.count())), 0.f);

			heightAdjustment = Atrium::Math::Lerp(heightAdjustment, 0.5f, stateFade);

			const Atrium::Matrix targetHeadTransform
				= (isActive ? Atrium::Matrix::CreateTranslation(0, heightAdjustment, 0) : Atrium::Matrix::Identity())
				* FretboardMatrices::Targets[anIndex]
				;

			myQuadRenderer.Queue(targetHeadTransform, {}, FretAtlas::Target_Head);
			myQuadRenderer.Queue(targetHeadTransform, aColor, FretAtlas::Target_ColorRing);
			myQuadRenderer.Queue(targetHeadTransform, {}, isActive ? FretAtlas::Target_Cap_Active : FretAtlas::Target_Cap_Neutral);

			myQuadRenderer.Queue(FretboardMatrices::Targets[anIndex], {}, FretAtlas::Target_Ring);
		};

	drawTarget(0, NoteColor::Green);
	drawTarget(1, NoteColor::Red);
	drawTarget(2, NoteColor::Yellow);
	drawTarget(3, NoteColor::Blue);
	drawTarget(4, NoteColor::Orange);
}

float ChartRenderer::TimeToPositionOffset(std::chrono::microseconds aTime) const
{
	const auto relativeToPlayhead = aTime - myPlayer.GetPlayhead();
	const float playheadToLookahead = static_cast<float>(relativeToPlayhead.count()) / static_cast<float>(LookAhead.count());

	return Atrium::Math::Lerp(
		0.f,
		FretboardLength - FretboardMatrices::TargetOffset,
		playheadToLookahead);
}
