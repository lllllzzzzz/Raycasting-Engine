#include "raycasterEngine.hpp"

#include <stdexcept>

#define DEBUG_MODE

using namespace Raycaster;

//const int RaycasterEngine::PROJ_PLANE_WIDTH = 800;
//const int RaycasterEngine::PROJ_PLANE_HEIGHT = 600;
//const int RaycasterEngine::WORLD_MAP_COLS = 20;
//const int RaycasterEngine::WORLD_MAP_ROWS = 20;
const int RaycasterEngine::BACKGROUND_COLOUR = 0x000000;
const float RaycasterEngine::WALL_SIDE_COLOUR_MULTIPLIER = .75;
const float RaycasterEngine::MOVEMENT_SPEED = .4;
const float RaycasterEngine::TURN_ANGLE = .08;
const float RaycasterEngine::ROTATE_CAMERA_ANGLE = .0008;

RaycasterEngine::RaycasterEngine() :
    m_playerPosition{9, 8},
    m_playerDirection{-1, 0},
    m_cameraPlane{0, 0.66},
    m_isRunning{true},
    m_rotateCamera{true},
    m_texturesEnabled{true},
    m_curFrameTime{0},
    m_prevFrameTime{0},
    m_movementSpeed{MOVEMENT_SPEED},
    m_rotateSpeed{TURN_ANGLE}
{
    #ifdef DEBUG_MODE
    std::cout << "RaycasterEngine constructor" << std::endl;
    #endif
}

RaycasterEngine::~RaycasterEngine()
{

}

void RaycasterEngine::Init()
{
    #ifdef DEBUG_MODE
    std::cout << "RaycasterEngine::Init()" << std::endl;
    #endif

    SDL_Init(SDL_INIT_EVERYTHING);
    m_screen = SDL_SetVideoMode(PROJ_PLANE_WIDTH, PROJ_PLANE_HEIGHT, 32, SDL_SWSURFACE/* | SDL_FULLSCREEN*/);
    SDL_WM_SetCaption("Raycaster Engine", NULL);
    SDL_EnableKeyRepeat(100, SDL_DEFAULT_REPEAT_INTERVAL);

    // Generate textures
    for (int x{0}; x < TEXTURE_WIDTH; x++) {
        for (int y{0}; y < TEXTURE_HEIGHT; y++) {
            // XOR
            int xorColour = (x * 256 / TEXTURE_WIDTH) ^ (y * 256 / TEXTURE_HEIGHT);
            m_texture[0][TEXTURE_HEIGHT * y + x] = xorColour + 256 * xorColour + 65536 + xorColour;
            // Red with black cross
            //m_texture[0][TEXTURE_HEIGHT * y + x] = 65536 * 254 * (x != y && x != TEXTURE_WIDTH - y);
            // Red with black rectangles
            m_texture[1][TEXTURE_HEIGHT * y + x] = 65536 * 192 * (x % 16 && y % 16);
            // Vertical plasma lines
            m_texture[2][TEXTURE_HEIGHT * y + x] = static_cast<int>(128.f + (128.f * sin(x / 8.f)));
        }
    }
}

void RaycasterEngine::Cleanup()
{
    #ifdef DEBUG_MODE
    std::cout << "RaycasterEngine::Init()" << std::endl;
    #endif
}

void RaycasterEngine::Run()
{
    #ifdef DEBUG_MODE
    std::cout << "RaycasterEngine::Run()" << std::endl;
    #endif

    SDL_Event event;

    while (IsRunning()) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_UP:
                        #ifdef DEBUG_MODE
                        std::cout << "Up key pressed" << std::endl;
                        #endif

                        MovePlayer(MovementDirection::FORWARD, MOVEMENT_SPEED);

                        if (IsPlayerInWall()) {
                            MovePlayer(MovementDirection::BACKWARD, MOVEMENT_SPEED);
                        }

                        break;
                    case SDLK_DOWN:
                        #ifdef DEBUG_MODE
                        std::cout << "Down key pressed" << std::endl;
                        #endif

                        MovePlayer(MovementDirection::BACKWARD, MOVEMENT_SPEED);

                        if (IsPlayerInWall()) {
                            MovePlayer(MovementDirection::FORWARD, MOVEMENT_SPEED);
                        }
                        break;
                    case SDLK_LEFT:
                        #ifdef DEBUG_MODE
                        std::cout << "Left key pressed" << std::endl;
                        #endif

                        StrafePlayer(MovementDirection::LEFT, MOVEMENT_SPEED);

                        if (IsPlayerInWall()) {
                            StrafePlayer(MovementDirection::RIGHT, MOVEMENT_SPEED);
                        }
                        break;
                    case SDLK_RIGHT:
                        #ifdef DEBUG_MODE
                        std::cout << "Right key pressed" << std::endl;
                        #endif

                        StrafePlayer(MovementDirection::RIGHT, MOVEMENT_SPEED);

                        if (IsPlayerInWall()) {
                            StrafePlayer(MovementDirection::LEFT, MOVEMENT_SPEED);
                        }
                        break;
                    case SDLK_PAGEDOWN:
                        #ifdef DEBUG_MODE
                        std::cout << "Page Down key pressed" << std::endl;
                        #endif

                        TurnPlayer(MovementDirection::RIGHT, TURN_ANGLE);
                        break;
                    case SDLK_DELETE:
                        #ifdef DEBUG_MODE
                        std::cout << "Delete key pressed" << std::endl;
                        #endif

                        TurnPlayer(MovementDirection::LEFT, TURN_ANGLE);
                        break;
                    case SDLK_ESCAPE:
                        #ifdef DEBUG_MODE
                        std::cout << "ESC key pressed" << std::endl;
                        #endif
                        Quit();
                        break;
                    default:
                        break;
                }
            } else if (event.type == SDL_QUIT) {
                #ifdef DEBUG_MODE
                std::cout << "Close button pressed" << std::endl;
                #endif
                Quit();
            }
        }

        if (m_rotateCamera) {
            RotateCamera(MovementDirection::RIGHT, ROTATE_CAMERA_ANGLE);
        }

        //std::cout << "x: " << GetPlayerPosition().x << "    y: " << GetPlayerPosition().y << std::endl << "x: " << GetPlayerDirection().x << "    y: " << GetPlayerDirection().y << std::endl << std::endl;

        SDL_FillRect(GetScreen(), NULL, BACKGROUND_COLOUR);
        if (SDL_MUSTLOCK(GetScreen())) {
            SDL_LockSurface(GetScreen());
        }

        for (int i{0}; i < GetScreen()->w; i++) {
            // Calculate camera offset on x axis
            // This value is always between 1 and -1
            double cameraOffsetX = 2 * i / static_cast<double>(GetScreen()->w) - 1;

            // Ray is emitted from player position
            Point<double> rayPosition = GetPlayerPosition();
            // Calculate ray direction
            Point<double> rayDirection;
            rayDirection.x = GetPlayerDirection().x + GetCameraPlane().x * cameraOffsetX;
            rayDirection.y = GetPlayerDirection().y + GetCameraPlane().y * cameraOffsetX;

            //Wall curWall = GetWallForRay(rayPosition, rayDirection);

            // Cell on 2D map grid where is the origin of the ray
            // This is the position of the ray, which is the player position
            Point<double> mapCell;
            mapCell.x = static_cast<int>(rayPosition.x);
            mapCell.y = static_cast<int>(rayPosition.y);

            // Delta distance is the distance the ray needs to travel to get
            // from one x-side to the next x-side, or from one y-side to the
            // next y-side
            Point<double> deltaDistance;
            deltaDistance.x = sqrt(1 + (rayDirection.y * rayDirection.y) / (rayDirection.x * rayDirection.x));
            deltaDistance.y = sqrt(1 + (rayDirection.x * rayDirection.x) / (rayDirection.y * rayDirection.y));

            // Distance the ray must travel from its starting position
            // (current player position on 2D map grid) to the first x-side and
            // the first y-side
            Point<double> sideDistance;

            // Offset that specifies the direction the ray will travel (+1 or -1)
            Point<double> step;

            // If ray direction has a negative x-component,
            // sideDistance.x = distance from starting position to first side on
            // the left of the starting position.
            if (rayDirection.x < 0) {
                step.x = -1;
                sideDistance.x = (rayPosition.x - mapCell.x) * deltaDistance.x;
                // If ray direction has a positive x-component,
                // sideDistance.x = distance from starting position to first side on
                // the right of the starting position.
            } else {
                step.x = 1;
                sideDistance.x = ((mapCell.x + 1.0f) - rayPosition.x) * deltaDistance.x;
            }

            // If ray direction has a negative y-component,
            // sideDistance.y = distance from starting position to first side
            // above the starting position.
            if (rayDirection.y < 0) {
                step.y = -1;
                sideDistance.y = (rayPosition.y - mapCell.y) * deltaDistance.y;
                // If ray direction has a positive y-component,
                // sideDistance.y = distance from starting position to first side
                // below the starting position.
            } else {
                step.y = 1;
                sideDistance.y = ((mapCell.y + 1.0f) - rayPosition.y) * deltaDistance.y;
            }

            bool wallHit = false;
            bool sideHit = false;

            double perpWallDistance;

            // Perform DDA algorithm

            // The ray has hit a wall
            while (!wallHit) {
                if (sideDistance.x < sideDistance.y) {
                    sideDistance.x += deltaDistance.x;
                    mapCell.x += step.x;
                    sideHit = false;
                } else {
                    sideDistance.y += deltaDistance.y;
                    mapCell.y += step.y;
                    sideHit = true;
                }

                wallHit = GetWorldMapCell(mapCell);
            }

            // Check if the ray has hit the side of a wall
            if (!sideHit) {
                perpWallDistance = std::abs((mapCell.x - rayPosition.x + (1 - step.x) / 2) / rayDirection.x);
            } else {
                perpWallDistance = std::abs((mapCell.y - rayPosition.y + (1 - step.y) / 2) / rayDirection.y);
            }

            Wall curWall;
            curWall.height = GetHeightForWallDistance(perpWallDistance);

            if (!GetTexturesEnabled()) {
                curWall.colour = GetWallColour(mapCell);

                if (sideHit) {
                    curWall.colour *= WALL_SIDE_COLOUR_MULTIPLIER;
                }
            }

            //int numTexture = GetWorldMapCell(mapCell) - 1;

            double wallX;
            if (sideHit) {
                wallX = rayPosition.x + ((mapCell.y - rayPosition.y + (1 - step.y) / 2) / rayDirection.y) * rayDirection.x;
            } else {
                wallX = rayPosition.y + ((mapCell.x - rayPosition.x + (1 - step.x) / 2) / rayDirection.x) * rayDirection.y;
            }
            wallX -= floor(wallX);

            int textureX = static_cast<int>(wallX * static_cast<double>(TEXTURE_WIDTH));
            if ((!sideHit && rayDirection.x > 0) || (sideHit && rayDirection.x < 0)) {
                // Comment out this line - fix texture glitch?
                //textureX = TEXTURE_WIDTH - textureX - 1;
            }

            const int OFFSET = (GetScreen()->h - curWall.height) / 2;

            for (int j{OFFSET}; j < OFFSET + curWall.height; j++) {
                if (GetTexturesEnabled()) {
                    int wallDistance = j * 256 - PROJ_PLANE_HEIGHT * 128 + curWall.height * 128;
                    int textureY = ((wallDistance * TEXTURE_HEIGHT) / curWall.height) / 256;
                    unsigned int colour = m_texture[GetWorldMapCell(mapCell) - 1][TEXTURE_HEIGHT * textureY + textureX];

                    if (sideHit) {
                        colour = (colour >> 1) & 8355711;
                    }

                    SetPixel({i, j}, colour);
                    //m_screenBuffer[i][j] = colour;
                } else {
                    SetPixel({i, j}, curWall.colour);
                }
            }
        }

        if (SDL_MUSTLOCK(GetScreen())) {
            SDL_LockSurface(GetScreen());
        }

        SDL_Flip(GetScreen());

        m_prevFrameTime = m_curFrameTime;
        m_curFrameTime = SDL_GetTicks();
        double frameTime = (m_curFrameTime - m_prevFrameTime) / 1000.0f;

        std::cout << frameTime << std::endl;

        m_movementSpeed = frameTime * 5.0f;
        m_rotateSpeed = frameTime * 3.0f;
    }

    SDL_Quit();
}

void RaycasterEngine::SetPixel(const Point<int> coordinates, const unsigned int pixel)
{
    unsigned int *pixels = reinterpret_cast<unsigned int*>(GetScreen()->pixels);

    if (!pixels) {
        #ifdef DEBUG_MODE
        std::cerr << "RaycasterEngine::SetPixel(): Error retrieving SDL screen (null pointer)" << std::endl;
        #endif
        return;

        //throw std::runtime_error("Error accessing SDL screen pixels");
    }

    pixels[(coordinates.y * GetScreen()->w) + coordinates.x] = pixel;
}

unsigned int RaycasterEngine::GetWallColour(const Point<double> mapCell) noexcept
{
    int cellValue = GetWorldMapCell(mapCell);
    unsigned int wallColour;

    switch (cellValue) {
        case 1:
            wallColour = SDL_MapRGB(GetScreen()->format, 0xFF, 0, 0);
            break;
        case 2:
            wallColour = SDL_MapRGB(GetScreen()->format, 0, 0xFF, 0);
            break;
        default:
            wallColour = SDL_MapRGB(GetScreen()->format, 0, 0, 0xFF);
            break;
    }

    return wallColour;
}

int RaycasterEngine::GetHeightForWallDistance(const double distance) const
{
    if (!distance) {
        #ifdef DEBUG_MODE
        std::cerr << "RaycasterEngine::GetHeightForWallDistance(): distance cannot be zero (division be zero)" << std::endl;
        #endif
        return 0; // TODO: return error code
    }

    const int WALL_HEIGHT = std::abs(PROJ_PLANE_HEIGHT / distance);
    return std::min(WALL_HEIGHT, PROJ_PLANE_HEIGHT);
}

void RaycasterEngine::SetPlayerPosition(const Point<double> newPosition) noexcept
{
    m_playerPosition = newPosition;
}

void RaycasterEngine::SetPlayerDirection(const Point<double> newDirection) noexcept
{
    m_playerDirection = newDirection;
}

void RaycasterEngine::SetCameraPlane(const Point<double> newPlane) noexcept
{
    m_cameraPlane = newPlane;
}

void RaycasterEngine::MovePlayer(const MovementDirection direction, const float speed) noexcept
{
    if (direction != MovementDirection::FORWARD && direction != MovementDirection::BACKWARD) {
        #ifdef DEBUG_MODE
        std::cerr << "Bad arg(s) to MovePlayer(): can only move forward or backward" << std::endl;
        #endif
        return;
    }

    if (!speed) {
        return;
    }

    const float SPEED = (direction == MovementDirection::FORWARD) ? speed : -speed;
    //const float SPEED = m_movementSpeed;

    Point<double> predictedPosition;
    predictedPosition.x = static_cast<int>(GetPlayerPosition().x - GetPlayerDirection().x * SPEED);
    predictedPosition.y = static_cast<int>(GetPlayerPosition().y);

    Point<double> newPlayerPosition = GetPlayerPosition();

    if (!GetWorldMapCell(predictedPosition)) {
        newPlayerPosition.x = GetPlayerPosition().x + GetPlayerDirection().x * SPEED;
    }

    if (!GetWorldMapCell({static_cast<int>(GetPlayerPosition().x), static_cast<int>(GetPlayerPosition().x - GetPlayerDirection().x * SPEED)})) {
        newPlayerPosition.y = GetPlayerPosition().y + GetPlayerDirection().y * SPEED;
    }

    SetPlayerPosition(newPlayerPosition);
}

void RaycasterEngine::StrafePlayer(const MovementDirection direction, const float speed) noexcept
{
    if (direction != MovementDirection::LEFT && direction != MovementDirection::RIGHT) {
        #ifdef DEBUG_MODE
        std::cerr << "Bad arg(s) to StrafePlayer(): can only strafe left or right" << std::endl;
        #endif
        return;
    }

    if (!speed) {
        #ifdef DEBUG_MODE
        std::cerr << "Bad arg(s) to StrafePlayer(): speed must be greater than zero" << std::endl;
        #endif
        return;
    }

    const float SPEED = (direction == MovementDirection::RIGHT) ? speed : -speed;

    Point<double> newPlayerPosition;
    newPlayerPosition.x = GetPlayerPosition().x + GetCameraPlane().x * SPEED;
    newPlayerPosition.y = GetPlayerPosition().y + GetCameraPlane().y * SPEED;

    SetPlayerPosition(newPlayerPosition);
}

void RaycasterEngine::TurnPlayer(const MovementDirection direction, const float angle) noexcept
{
    if (direction != MovementDirection::LEFT && direction != MovementDirection::RIGHT) {
        #ifdef DEBUG_MODE
        std::cerr << "Bad arg(s) to TurnPlayer(): can only turn left or right" << std::endl;
        #endif
        return;
    }

    if (!angle) {
        return;
    }

    const float ANGLE = (direction == MovementDirection::LEFT) ? angle : -angle;

    double oldPlayerDirectionX = GetPlayerDirection().x;

    Point<double> newPlayerDirection;
    newPlayerDirection.x = GetPlayerDirection().x * cos(ANGLE) - GetPlayerDirection().y * sin(ANGLE);
    newPlayerDirection.y = oldPlayerDirectionX * sin(ANGLE) + GetPlayerDirection().y * cos(ANGLE);
    SetPlayerDirection(newPlayerDirection);

    double oldCameraPlaneX = GetCameraPlane().x;
    Point<double> newCameraPlane;

    newCameraPlane.x = GetCameraPlane().x * cos(ANGLE) - GetCameraPlane().y * sin(ANGLE);
    newCameraPlane.y = oldCameraPlaneX * sin(ANGLE) + GetCameraPlane().y * cos(ANGLE);
    SetCameraPlane(newCameraPlane);
}

void RaycasterEngine::RotateCamera(const MovementDirection direction, const float angle) noexcept
{
    if (direction != MovementDirection::LEFT && direction != MovementDirection::RIGHT) {
        #ifdef DEBUG_MODE
        std::cerr << "Bad arg(s) to RotateCamera(): can only rotate camera left or right" << std::endl;
        #endif
        return;
    }

    TurnPlayer(direction, angle);
}
