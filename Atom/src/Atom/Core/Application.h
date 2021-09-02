#pragma once

namespace Atom
{
    class Application
    {
    public:
        Application() = default;
        virtual ~Application() = default;

        void Run();
    };

}

