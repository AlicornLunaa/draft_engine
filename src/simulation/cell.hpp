#pragma once

namespace Krypton {
    namespace Simulation {
        enum class CellType {
            STONE,
            SAND,
            WATER,
            SMOKE,
            EMPTY
        };

        struct Cell {
            CellType type;
            unsigned int x;
            unsigned int y;

            Cell(CellType type, unsigned int x, unsigned int y) : type(type), x(x), y(y) {}
        };
    }
}