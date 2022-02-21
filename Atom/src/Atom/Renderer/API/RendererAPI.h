#pragma once

namespace Atom
{
    class RendererAPI
    {
    public:
        enum class API
        {
            None      = 0,
            DirectX12 = 1
        };
    public:
        virtual ~RendererAPI() = default;

        inline static API GetAPI() { return ms_API; }
    private:
        static API ms_API;
    };
}