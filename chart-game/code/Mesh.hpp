#pragma once

#include <vector>

#include "Atrium_GraphicsBuffer.hpp"
#include "Atrium_GraphicsAPI.hpp"
#include "Atrium_GraphicsPipeline.hpp"

class Mesh
{
public:
	virtual ~Mesh() = default;

	virtual void DrawToFrame(Atrium::FrameGraphicsContext& aFrameContext) = 0;
	virtual void DrawInstancedToFrame(Atrium::FrameGraphicsContext& aFrameContext, unsigned int anInstanceCount, unsigned int anInstanceStart) = 0;

	virtual void SetName(const std::wstring& aName) = 0;
};

template <typename VertexType>
class MeshT : public Mesh
{
public:
	MeshT()
		: myManager(nullptr)
		, myVertexCount(0)
	{ }

	MeshT(Atrium::GraphicsAPI& aManager)
		: myManager(&aManager)
		, myVertexCount(0)
		, myName(L"Mesh")
	{ }

	void DrawToFrame(Atrium::FrameGraphicsContext& aFrameContext) override
	{
		aFrameContext.SetPrimitiveTopology(Atrium::PrimitiveTopology::TriangleList);
		aFrameContext.SetVertexBuffer(myVertexBuffer);
		aFrameContext.Draw(myVertexCount, 0);
	}

	void DrawInstancedToFrame(Atrium::FrameGraphicsContext& aFrameContext, unsigned int anInstanceCount, unsigned int anInstanceStart) override
	{
		aFrameContext.SetPrimitiveTopology(Atrium::PrimitiveTopology::TriangleList);
		aFrameContext.SetVertexBuffer(myVertexBuffer);
		aFrameContext.DrawInstanced(myVertexCount, anInstanceCount, 0, anInstanceStart);
	}

	void SetVertices(const std::vector<VertexType>& someVertices)
	{
		if (someVertices.empty())
			return;

		myVertexCount = static_cast<std::uint32_t>(someVertices.size());

		myVertexBuffer = myManager->GetResourceManager().CreateGraphicsBuffer(
			Atrium::GraphicsBuffer::Target::Vertex,
			myVertexCount,
			sizeof(VertexType)
		);
		myVertexBuffer->SetName((myName + L" vertices").c_str());
		myVertexBuffer->SetData(
			&someVertices.front(),
			static_cast<std::uint32_t>(myVertexCount * sizeof(VertexType))
		);
	}

	void SetName(const std::wstring& aName) override
	{
		myName = aName;
		if (myVertexBuffer)
			myVertexBuffer->SetName((aName + L" vertices").c_str());
	}

protected:
	Atrium::GraphicsAPI* myManager;
	std::shared_ptr<Atrium::GraphicsBuffer> myVertexBuffer;
	std::uint32_t myVertexCount;
	std::wstring myName;
};

struct ColoredVertex
{
	static inline std::vector<Atrium::PipelineStateDescription::InputLayoutEntry> GetInputLayout()
	{
		std::vector<Atrium::PipelineStateDescription::InputLayoutEntry> layout;
		layout.emplace_back("POSITION", Atrium::GraphicsFormat::R32G32B32_SFloat);
		layout.emplace_back("COLOR", Atrium::GraphicsFormat::R32G32B32A32_SFloat);
		return layout;
	}

	float Position[3];
	float Color[4];
};
using ColoredMesh = MeshT<ColoredVertex>;