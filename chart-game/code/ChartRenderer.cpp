// Filter "Chart/Rendering"
#include "ChartRenderer.hpp"

#include "ChartData.hpp"
#include "ChartMeshes.hpp"
#include "ChartPlayer.hpp"
#include "ChartTrack.hpp"

#include "FretAtlas.hpp"

struct ModelViewProjection
{
	Atrium::Math::Matrix Model;
	Atrium::Math::Matrix View;
	Atrium::Math::Matrix Projection;
};

ChartRenderer::ChartRenderer(ChartPlayer& aPlayer)
	: myPlayer(aPlayer)
{ }

void ChartRenderer::SetupResources(Atrium::Core::GraphicsAPI& aGraphicsAPI, Atrium::Core::GraphicsFormat aColorTargetFormat)
{
	ZoneScoped;

	myAtlas = aGraphicsAPI.GetResourceManager().LoadTexture("fretatlas.dds");
	myFretboardTexture = aGraphicsAPI.GetResourceManager().LoadTexture("fretboard.dds");

	std::unique_ptr<Atrium::Core::RootSignatureBuilder> builder = aGraphicsAPI.GetResourceManager().CreateRootSignature();

	builder->SetVisibility(Atrium::Core::ShaderVisibility::Vertex);

	// Model, view, projection.
	builder->AddTable().AddCBVRange(1, 0, Atrium::Core::ResourceUpdateFrequency::PerObject);
	builder->AddTable().AddCBVRange(1, 0, Atrium::Core::ResourceUpdateFrequency::PerFrame);

	builder->SetVisibility(Atrium::Core::ShaderVisibility::Pixel);

	builder->AddTable().AddSRVRange(4, 0, Atrium::Core::ResourceUpdateFrequency::PerMaterial);

	builder->AddSampler(0) // Clamping point
		.Filter(Atrium::Core::FilterMode::Point)
		.Address(Atrium::Core::TextureWrapMode::Clamp)
		;

	builder->AddSampler(1) // Clamping linear
		.Filter(Atrium::Core::FilterMode::Bilinear)
		.Address(Atrium::Core::TextureWrapMode::Clamp)
		;

	std::shared_ptr<Atrium::Core::RootSignature> rootSignature = builder->Finalize();

	SetupQuadResources(aGraphicsAPI, rootSignature, aColorTargetFormat);
	SetupFretboardResources(aGraphicsAPI, rootSignature, aColorTargetFormat);
}

void ChartRenderer::Render(Atrium::Core::FrameContext& aContext, const std::shared_ptr<Atrium::Core::RenderTexture>& aTarget)
{
	ZoneScoped;
	CONTEXT_ZONE(aContext, "Chart");

	myLastQuadFlush = 0;
	myQuadInstanceData.clear();

	aContext.SetRenderTargets({ aTarget }, nullptr);
	Atrium::Size targetSize(aTarget->GetWidth(), aTarget->GetHeight());
	aContext.SetViewportAndScissorRect(targetSize);

	const std::vector<std::unique_ptr<ChartController>>& controllers = myPlayer.GetControllers();
	const std::vector<Atrium::Math::Rectangle> controllerRects = GetControllerRectangles(Atrium::Math::Rectangle::FromExtents({ 0, 0 }, targetSize), controllers.size());
	for (unsigned int i = 0; i < controllers.size(); ++i)
	{
		ZoneScopedN("Controller");
		aContext.SetViewport(controllerRects[i]);

		aContext.SetPipelineState(myFretboardPipelineState);
		aContext.SetPipelineResource(Atrium::Core::ResourceUpdateFrequency::PerObject, 0, myFretboardModelViewProjection);
		aContext.SetPipelineResource(Atrium::Core::ResourceUpdateFrequency::PerMaterial, 0, myFretboardTexture);
		myFretboardMesh->DrawToFrame(aContext);

		aContext.SetPipelineResource(Atrium::Core::ResourceUpdateFrequency::PerMaterial, 0, myAtlas);

		QueueFretboardQuads();
		RenderController(*controllers.at(i));
		FlushQuads(aContext);
	}

	myQuadInstanceBuffer->SetData<ChartQuadInstance>(myQuadInstanceData);
}

void ChartRenderer::SetupQuadResources(Atrium::Core::GraphicsAPI& aGraphicsAPI, const std::shared_ptr<Atrium::Core::RootSignature>& aRootSignature, Atrium::Core::GraphicsFormat aColorTargetFormat)
{
	ZoneScoped;

	myQuadMesh = CreateQuadMesh(aGraphicsAPI);
	myQuadMesh->SetName(L"Quad");

	Atrium::Core::PipelineStateDescription pipelineDescription;
	pipelineDescription.RootSignature = aRootSignature;
	pipelineDescription.InputLayout = ChartQuadVertex::GetInputLayout();
	const std::filesystem::path shaderPath = "ChartQuad.hlsl";
	pipelineDescription.VertexShader = aGraphicsAPI.GetResourceManager().CreateShader(shaderPath, Atrium::Core::Shader::Type::Vertex, "vertexShader");
	pipelineDescription.PixelShader = aGraphicsAPI.GetResourceManager().CreateShader(shaderPath, Atrium::Core::Shader::Type::Pixel, "pixelShader");
	pipelineDescription.OutputFormats = { aColorTargetFormat };
	pipelineDescription.BlendMode.BlendFactors[0].Enabled = true;

	myQuadPipelineState = aGraphicsAPI.GetResourceManager().CreatePipelineState(pipelineDescription);

	struct CameraMatrices
	{
		Atrium::Math::Matrix View;
		Atrium::Math::Matrix Projection;
	} cameraMatrices;
	cameraMatrices.View = FretboardMatrices::CameraViewMatrix;
	cameraMatrices.Projection = FretboardMatrices::CameraProjectionMatrix;

	myCameraMatrices = aGraphicsAPI.GetResourceManager().CreateGraphicsBuffer(Atrium::Core::GraphicsBuffer::Target::Constant, 1, sizeof(CameraMatrices));
	myCameraMatrices->SetData(&cameraMatrices, sizeof(CameraMatrices));

	myQuadInstanceBuffer = aGraphicsAPI.GetResourceManager().CreateGraphicsBuffer(Atrium::Core::GraphicsBuffer::Target::Vertex, 512, sizeof(ChartQuadInstance));
}

void ChartRenderer::SetupFretboardResources(Atrium::Core::GraphicsAPI& aGraphicsAPI, const std::shared_ptr<Atrium::Core::RootSignature>& aRootSignature, Atrium::Core::GraphicsFormat aColorTargetFormat)
{
	ZoneScoped;

	myFretboardMesh = CreateFretboardMesh(aGraphicsAPI);
	myFretboardMesh->SetName(L"Fretboard vertices");

	Atrium::Core::PipelineStateDescription fretboardPipelineDescription;
	fretboardPipelineDescription.RootSignature = aRootSignature;
	fretboardPipelineDescription.InputLayout = ChartFretboardVertex::GetInputLayout();
	const std::filesystem::path shaderPath = "ChartFretboard.hlsl";
	fretboardPipelineDescription.VertexShader = aGraphicsAPI.GetResourceManager().CreateShader(shaderPath, Atrium::Core::Shader::Type::Vertex, "vertexShader");
	fretboardPipelineDescription.PixelShader = aGraphicsAPI.GetResourceManager().CreateShader(shaderPath, Atrium::Core::Shader::Type::Pixel, "pixelShader");
	fretboardPipelineDescription.OutputFormats = { aColorTargetFormat };
	myFretboardPipelineState = aGraphicsAPI.GetResourceManager().CreatePipelineState(fretboardPipelineDescription);

	ModelViewProjection fretboardMatrices;
	fretboardMatrices.Model = Atrium::Math::Matrix::Identity();
	fretboardMatrices.View = FretboardMatrices::CameraViewMatrix;
	fretboardMatrices.Projection = FretboardMatrices::CameraProjectionMatrix;

	myFretboardModelViewProjection = aGraphicsAPI.GetResourceManager().CreateGraphicsBuffer(Atrium::Core::GraphicsBuffer::Target::Constant, 1, sizeof(ModelViewProjection));
	myFretboardModelViewProjection->SetData(&fretboardMatrices, sizeof(ModelViewProjection));
}

std::pair<int, int> ChartRenderer::GetControllerRectanglesGrid(Atrium::Math::Rectangle aTotalRectangle, float aGridCellAspectRatio, std::size_t aControllerCount) const
{
	if (aControllerCount == 0)
		return { 0, 0 };

	const float normalizedAspectRatio = (aTotalRectangle.Size.X / aTotalRectangle.Size.Y) * aGridCellAspectRatio;

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

	pass(Atrium::Math::FloorTo<int>(columns), Atrium::Math::CeilTo<int>(rows));
	pass(Atrium::Math::CeilTo<int>(columns), Atrium::Math::FloorTo<int>(rows));
	pass(Atrium::Math::CeilTo<int>(columns), Atrium::Math::CeilTo<int>(rows));

	return { integerColumns.value(), integerRows.value() };
}

std::vector<Atrium::Math::Rectangle> ChartRenderer::GetControllerRectangles(Atrium::Math::Rectangle aTotalRectangle, std::size_t aControllerCount) const
{
	constexpr float CellAspectRatio = 1.f;

	const std::pair<int, int> gridCells = GetControllerRectanglesGrid(aTotalRectangle, CellAspectRatio, aControllerCount);

	if ((gridCells.first * gridCells.second) == 0)
		return { };

	const float gridAspectRatio = static_cast<float>(gridCells.first) / static_cast<float>(gridCells.second);
	const float gridWidth = Atrium::Math::Min<float>(aTotalRectangle.Size.X, (aTotalRectangle.Size.Y * gridAspectRatio));
	const Atrium::Math::Rectangle gridRect(aTotalRectangle.Center, { gridWidth, gridWidth / gridAspectRatio });
	const Atrium::Math::Vector2 gridCellSize(
		gridRect.Size.X / gridCells.first,
		gridRect.Size.Y / gridCells.second
	);

	std::vector<Atrium::Math::Rectangle> rectsOut;

	for (std::size_t i = 0; i < aControllerCount; ++i)
	{
		const std::size_t cellX = i % gridCells.first;
		const std::size_t cellY = (i - cellX) / gridCells.first;

		const Atrium::Math::Vector2 position(
			(gridRect.Center.X - (gridRect.Size.X / 2)) + (cellX * gridCellSize.X),
			(gridRect.Center.Y + (gridRect.Size.Y / 2)) - ((cellY + 1) * gridCellSize.Y)
		);

		rectsOut.push_back(Atrium::Math::Rectangle::FromExtents(position, position + gridCellSize));
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
	auto timeToTrackPosition =
		[&](std::chrono::microseconds aTime) -> float
		{
			constexpr std::chrono::microseconds LookAhead(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::seconds(2)));
			const auto relativeToPlayhead = aTime - myPlayer.GetPlayhead();
			return static_cast<float>(relativeToPlayhead.count()) / static_cast<float>(LookAhead.count());
		};

	const std::vector<ChartNoteRange>& difficultyNotes = aTrack.GetNoteRanges().at(aController.GetTrackDifficulty());
	for (const ChartNoteRange& note : difficultyNotes)
	{
		const float noteStartFretboardFraction = timeToTrackPosition(note.Start);
		const float noteEndFretboardFraction = timeToTrackPosition(note.End);

		if (noteEndFretboardFraction < -0.5f || noteStartFretboardFraction > 2.f)
			continue;

		Atrium::Math::Rectangle noteFill = FretAtlas::Note_Color;
		Atrium::Math::Rectangle noteShell = FretAtlas::Note_Shell_Strum;

		switch (note.Type)
		{
		case ChartNoteType::Strum:
			break;
		case ChartNoteType::Tap:
			noteFill = FretAtlas::Note_Color_Tap;
			noteShell = FretAtlas::Note_Shell_Tap;
			break;
		case ChartNoteType::HOPO:
			noteShell = FretAtlas::Note_Shell_HOPO;
			break;
		}

		Atrium::Color32 noteColor;
		switch (note.Lane)
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

		Atrium::Math::Matrix noteTransform
			= FretboardMatrices::Targets[note.Lane]
			* Atrium::Math::Matrix::CreateTranslation(
				0,
				0,
				Atrium::Math::Lerp<float>(
					0,
					FretboardLength - FretboardMatrices::TargetOffset,
					noteStartFretboardFraction
				)
			);

		QueueQuad(noteTransform, noteColor, noteFill);
		QueueQuad(noteTransform, {}, noteShell);
	}
}

void ChartRenderer::QueueFretboardQuads()
{
	// Lane 1
	QueueQuad(FretboardMatrices::Targets[0], {}, FretAtlas::Note_Target_L2_Base);
	QueueQuad(FretboardMatrices::Targets[0], NoteColor::Green, FretAtlas::Note_Target_L2_Head);

	// Lane 2
	QueueQuad(FretboardMatrices::Targets[1], {}, FretAtlas::Note_Target_L1_Base);
	QueueQuad(FretboardMatrices::Targets[1], NoteColor::Red, FretAtlas::Note_Target_L1_Head);

	// Lane 3
	QueueQuad(FretboardMatrices::Targets[2], {}, FretAtlas::Note_Target_C0_Base);
	QueueQuad(FretboardMatrices::Targets[2], NoteColor::Yellow, FretAtlas::Note_Target_C0_Head);

	// Lane 4
	QueueQuad(FretboardMatrices::Targets[3], {}, FretAtlas::Note_Target_R1_Base);
	QueueQuad(FretboardMatrices::Targets[3], NoteColor::Blue, FretAtlas::Note_Target_R1_Head);

	// Lane 5
	QueueQuad(FretboardMatrices::Targets[4], {}, FretAtlas::Note_Target_R2_Base);
	QueueQuad(FretboardMatrices::Targets[4], NoteColor::Orange, FretAtlas::Note_Target_R2_Head);
}

void ChartRenderer::QueueQuad(Atrium::Math::Matrix aTransform, std::optional<Atrium::Color32> aColor, std::optional<Atrium::Math::Rectangle> aUVRectangle)
{
	ChartQuadInstance& instance = myQuadInstanceData.emplace_back();
	instance.Transform = aTransform;

	const Atrium::Color32 color = aColor.value_or(Atrium::Color32::Predefined::White);
	instance.Color[0] = static_cast<float>(color.R) / 255.f;
	instance.Color[1] = static_cast<float>(color.G) / 255.f;
	instance.Color[2] = static_cast<float>(color.B) / 255.f;
	instance.Color[3] = static_cast<float>(color.A) / 255.f;

	instance.UVMin = { 0, 0 };
	instance.UVMax = { 1, 1 };
	if (aUVRectangle.has_value())
		aUVRectangle.value().ToExtents(instance.UVMin, instance.UVMax);
}

void ChartRenderer::FlushQuads(Atrium::Core::FrameContext& aContext)
{
	CONTEXT_ZONE(aContext, "Render quads");

	const std::size_t queuedSinceLastFlush = myQuadInstanceData.size() - myLastQuadFlush;
	if (queuedSinceLastFlush == 0)
		return;

	aContext.SetPipelineState(myQuadPipelineState);
	aContext.SetPipelineResource(Atrium::Core::ResourceUpdateFrequency::PerFrame, 0, myCameraMatrices);

	aContext.SetVertexBuffer(myQuadInstanceBuffer, 1);
	myQuadMesh->DrawInstancedToFrame(aContext, static_cast<unsigned int>(queuedSinceLastFlush), static_cast<unsigned int>(myLastQuadFlush));

	myLastQuadFlush = myQuadInstanceData.size();
}
