#pragma once

#include "ConstantBuffer.h"
#include "DX12ComputeShader.h"
#include "StructuredBuffer.h"
#include "Rendering/InstanceBuffer.h"
#include "SortComputeShader.reflection.h"

REFLECTED()
class SortComputeShader : public DX12ComputeShader
{
    GENERATED()

public:
    SortComputeShader() = default;

    SortComputeShader(const SortComputeShader& other);
    SortComputeShader& operator=(const SortComputeShader& other);

    void Run(DX12GraphicsCommandList& commandList, AppendStructuredBuffer<SMInstance>& instanceBuffer);

    // DX12ComputeShader
public:
    virtual bool Recompile(bool immediate) override;

    // DX12ComputeShader
protected:
    virtual void Dispatch(DX12GraphicsCommandList& commandList, uint32 threadGroupCountX, uint32 threadGroupCountY, uint32 threadGroupCountZ) const override;

private:
    static constexpr uint32 _blockSize = 128;
    static constexpr uint32 _transposeBlockSize = 16;
    static constexpr uint32 _matrixWidth = _blockSize;

    ComPtr<ID3D12PipelineState> _transposePSO;
    ComPtr<ID3D12RootSignature> _transposeRootSignature;
    ComPtr<IDxcBlobEncoding> _serializedTransposeRootSignature;
    ComPtr<IDxcBlobEncoding> _transposeShader;

    struct SMBitonicSortConstants
    {
        uint32 Level;
        uint32 LevelMask;
        uint32 Width;
        uint32 Height;
    };

    RWStructuredBuffer<SMInstance> _intermediateBuffer{};
    SMBitonicSortConstants _constants{};
};
