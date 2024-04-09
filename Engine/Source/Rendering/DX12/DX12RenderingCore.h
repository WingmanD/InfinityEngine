#pragma once

#include "d3dx12/d3dx12.h"
#include <dxcapi.h>
#include <functional>

using Microsoft::WRL::ComPtr;

using DX12Device = ID3D12Device14;
using DX12GraphicsCommandList = ID3D12GraphicsCommandList10;
using DXCompiler = IDxcCompiler3;

struct DX12CommandList
{
public:
    enum ECommandListState : uint8
    {
        Ready,
        Closed,
        Executing
    };
    
    ComPtr<ID3D12CommandAllocator> CommandAllocator;
    ComPtr<DX12GraphicsCommandList> CommandList;
    ECommandListState State = Ready;

public:
    void Close()
    {
        CommandList->Close();
        State = Closed;    
    }
    
    void Reset()
    {
        CommandAllocator->Reset();
        CommandList->Reset(CommandAllocator.Get(), nullptr);
        State = Ready;
    }
};

struct DX12CopyCommandList : public DX12CommandList
{
    std::vector<std::function<void()>> OnCompletedCallbacks;
};
