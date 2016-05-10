#ifndef RAYCASTER_ENGINE_HPP
#define RAYCASTER_ENGINE_HPP

#include <vector>
#include <cmath>
#include <SDL.h>
#include <iostream>

namespace Raycaster
{
    class RaycasterEngine
    {
    public:
        RaycasterEngine();
        ~RaycasterEngine();

        void Init();
        void Cleanup();

        void Run();

        bool IsRunning() const noexcept { return m_isRunning; }
        void Quit() noexcept { m_isRunning = false; }

        template <typename T>
        struct Point {
            T x;
            T y;
        };

        typedef struct {
            int height;
            unsigned int colour;
        } Wall;

        enum class MovementDirection {
            FORWARD, BACKWARD, LEFT, RIGHT
        };

        inline SDL_Surface* GetScreen() const { return m_screen; }
        inline void SetPixel(Point<int> coordinates, const unsigned int pixel);
        inline unsigned int GetWallColour(const Point<double> mapCell) noexcept;
        inline int GetHeightForWallDistance(const double distance) const;

        inline int GetWorldMapCell(const Point<double> cell) const noexcept { return m_worldMap[static_cast<int>(cell.x)][static_cast<int>(cell.y)]; }
        inline void SetPlayerPosition(const Point<double> newPosition) noexcept;
        inline Point<double> GetPlayerPosition() const noexcept { return m_playerPosition; }
        inline void SetPlayerDirection(const Point<double> newDirection) noexcept;
        inline Point<double> GetPlayerDirection() const noexcept { return m_playerDirection; }
        inline void SetCameraPlane(const Point<double> newPlane) noexcept;
        inline Point<double> GetCameraPlane() const noexcept { return m_cameraPlane; }
        inline bool IsPlayerInWall() noexcept { return GetWorldMapCell(GetPlayerPosition()); }

        inline bool GetTexturesEnabled() const noexcept { return m_texturesEnabled; }
        inline void SetTexturesEnabled(const bool enable) noexcept { m_texturesEnabled = enable; }

        inline void MovePlayer(const MovementDirection direction, const float speed) noexcept;
        inline void StrafePlayer(const MovementDirection direction, const float speed) noexcept;
        inline void TurnPlayer(const MovementDirection direction, const float angle) noexcept;
        inline void RotateCamera(const MovementDirection direction, const float angle) noexcept;

    private:
        Point<double> m_playerPosition;
        Point<double> m_playerDirection;
        Point<double> m_cameraPlane;

        SDL_Surface *m_screen;

        bool m_isRunning;
        bool m_rotateCamera;
        bool m_texturesEnabled;

        double m_curFrameTime;
        double m_prevFrameTime;
        double m_movementSpeed;
        double m_rotateSpeed;

        static const int PROJ_PLANE_WIDTH{800};
        static const int PROJ_PLANE_HEIGHT{600};
        static const int TEXTURE_WIDTH{64};
        static const int TEXTURE_HEIGHT{64};
        static const int WORLD_MAP_COLS{20};
        static const int WORLD_MAP_ROWS{20};
        static const int BACKGROUND_COLOUR;
        static const int NUMBER_OF_TEXTURES{3};
        static const float WALL_SIDE_COLOUR_MULTIPLIER;
        static const float MOVEMENT_SPEED;
        static const float TURN_ANGLE;
        static const float ROTATE_CAMERA_ANGLE;

        unsigned int m_texture[NUMBER_OF_TEXTURES][TEXTURE_WIDTH * TEXTURE_HEIGHT];
        unsigned int m_screenBuffer[PROJ_PLANE_WIDTH][PROJ_PLANE_HEIGHT];

        int m_worldMap[WORLD_MAP_COLS][WORLD_MAP_ROWS] =
        {
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 3, 0, 3, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 3, 0, 3, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1},
            {1, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1},
            {1, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 3, 3, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 0, 0, 0, 2, 2, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 2, 2, 0, 2, 0, 0, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 2, 2, 0, 2, 0, 0, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 2, 0, 0, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
        };
    };
}

#endif // RAYCASTER_ENGINE_HPP
