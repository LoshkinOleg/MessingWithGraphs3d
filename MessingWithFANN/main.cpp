#include <iostream>
// #include <stdexcept>
#include <string>
// #include <array>
#include <vector>
// #include <cassert>
#include <limits>

#include <raylib.h>

constexpr const int WINDOW_WIDTH = 600;
constexpr const int WINDOW_HEIGHT = 600;
constexpr const size_t RESOLUTION_X = 50;
constexpr const size_t RESOLUTION_Y = 50;
constexpr const size_t RESOLUTION_Z = 50;
constexpr const char* WINDOW_TITLE = "Messing with graphs.";

constexpr const float PARAMETERS_ADJUSTEMENT_MULTIPLIER = 0.1f;
constexpr const float PARAMETERS_ADJUSTEMENT_MULTIPLIER_C2 = PARAMETERS_ADJUSTEMENT_MULTIPLIER * 1.0f;
constexpr const float PARAMETERS_ADJUSTEMENT_MULTIPLIER_C1 = PARAMETERS_ADJUSTEMENT_MULTIPLIER * 1.0f;
constexpr const float PARAMETERS_ADJUSTEMENT_MULTIPLIER_C0 = PARAMETERS_ADJUSTEMENT_MULTIPLIER * 1.0f;

constexpr inline const float RemapToRange(const float inRangeMin, const float inRangeMax, const float outRangeMin, const float outRangeMax, const float value)
{
    return outRangeMin + (value - inRangeMin) * (outRangeMax - outRangeMin) / (inRangeMax - inRangeMin);
}

void PrintVector3(const Vector3 v)
{
    std::cout << std::to_string(v.x) << ";" << std::to_string(v.y) << std::to_string(v.z) << "\n";
}

void PrintFloat(const float f)
{
    std::cout << std::to_string(f) << "\n";
}

struct Polynomial4x4
{
    float SampleFunctionAt(const float x, const float y) const
    {
        return  (a3 * x * x * x) + (a2 * x * x) + (a1 * x) + a0 +
            (b3 * y * y * y) + (b2 * y * y) + (b1 * y) + b0;
    }

    Polynomial4x4 Derivative()
    {
        return {
            0, 3.0f * a3, 2.0f * a2, a1,
            0, 3.0f * b3, 2.0f * b2, b1
        };
    }

    Polynomial4x4 PartialDerivativeA()
    {
        return {
            0, 3.0f * a3, 2.0f * a2, a1,
            0, 0, 0, 0
        };
    }
    Polynomial4x4 PartialDerivativeB()
    {
        return {
            0, 0,0,0,
            0, 3.0f * b3, 2.0f * b2, b1
        };
    }

    float a3 = 0;
    float a2 = 0;
    float a1 = 0;
    float a0 = 0;

    float b3 = 0;
    float b2 = 0;
    float b1 = 0;
    float b0 = 0;
};

void SamplePolynomial4AtInterval(
    const Polynomial4x4 p,
    const float intervalBeginX, const float intervalEndX, const float stepSizeX,
    const float intervalBeginY, const float intervalEndY, const float stepSizeY,
    std::vector<std::vector<float>>& outSamples)
{
    size_t idxX = 0;
    for (float x = intervalBeginX; x < intervalEndX - stepSizeX; x += stepSizeX)
    {
        size_t idxY = 0;
        for (float y = intervalBeginY; y < intervalEndY - stepSizeY; y += stepSizeY)
        {
            outSamples[idxX][idxY++] = p.SampleFunctionAt(x, y);
        }
        idxX++;
    }
}

float DotProduct(const Vector3 a, const Vector3 b)
{
    return {a.x * b.x + a.y * b.y + a.z + b.z};
}

float EulerLength(const Vector3 v)
{
    return std::sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

Vector3 Normalize(const Vector3 v)
{
    return {
        v.x / EulerLength(v),
        v.y / EulerLength(v),
        v.z / EulerLength(v)
    };
}

struct Plane
{
    float GetZAtXY(const float x, const float y) const
    {
        return (-a * x - b * y - d) / c;
    }

    void SetSlopeOnXZ(const float m)
    {
        a = -m; // TODO: why this? supposed to be 1 / m I think...
    }

    void SetSlopeOnYZ(const float m)
    {
        b = -m;
    }

    float a = 0;
    float b = 0;
    float c = 0;
    float d = 0;
};

Color MixColors(const Color a, const Color b, const float mix)
{
    return
    {
        (unsigned char)((float)a.r * mix + (float)b.r * (1.0f - mix)),
        (unsigned char)((float)a.g * mix + (float)b.g * (1.0f - mix)),
        (unsigned char)((float)a.b * mix + (float)b.b * (1.0f - mix)),
        255
    };
}

void SamplePlaneAtInterval(
    const Plane p,
    const float intervalBeginX, const float intervalEndX, const float stepSizeX,
    const float intervalBeginY, const float intervalEndY, const float stepSizeY,
    std::vector<std::vector<float>>& outSamples)
{
    size_t idxX = 0;
    for (float x = intervalBeginX; x < intervalEndX - stepSizeX; x += stepSizeX)
    {
        size_t idxY = 0;
        for (float y = intervalBeginY; y < intervalEndY - stepSizeY; y += stepSizeY)
        {
            outSamples[idxX][idxY++] = p.GetZAtXY(x, y);
        }
        idxX++;
    }
}

void DrawSamples(
    const std::vector<std::vector<float>> z,
    const float intervalBeginX, const float intervalEndX, const float stepSizeX,
    const float intervalBeginY, const float intervalEndY, const float stepSizeY,
    const Vector3 normalizedOffset = { 0,0,0 })
{
    float min = std::numeric_limits<float>::max();
    float max = -std::numeric_limits<float>::max();
    for (size_t x = 0; x < z.size(); x++)
    {
        for (size_t y = 0; y < z[0].size(); y++)
        {
            if (z[x][y] < min) min = z[x][y];
            if (z[x][y] > max) max = z[x][y];
        }
    }

    size_t idxX = 0;
    for (float x = intervalBeginX; x < intervalEndX - stepSizeX; x += stepSizeX)
    {
        size_t idxY = 0;
        for (float y = intervalBeginY; y < intervalEndY - stepSizeY; y += stepSizeY)
        {
            const Color c0 = MixColors(RED, { 0, RED.g, RED.b, RED.a }, RemapToRange(min, max, 0.0f, 1.0f, z[idxX][idxY]));
            const Color c1 = MixColors(GREEN, { GREEN.r, 0, GREEN.b, GREEN.a }, x / (intervalEndX - intervalBeginX) - 0.1f); // 0.1 to compensate for some unsigned char arithemtic inaccuracies I think?
            const Color c2 = MixColors(BLUE, { BLUE.r, BLUE.g, 0, GREEN.a }, y / (intervalEndY - intervalBeginY) + 0.1f);
            const Color c =
            {
                c0.r + c1.r + c2.r,
                c0.g + c1.g + c2.g,
                c0.b + c1.b + c2.b,
                125
            };
            DrawSphere(
                {
                    x + normalizedOffset.x,
                    y + normalizedOffset.y,
                    z[idxX][idxY++] + normalizedOffset.z
                },
                0.01f,
                c);
        }
        idxX++;
    }
}

int main()
{
    const Vector3 normal = Normalize({0,0,1});
    Plane p = { normal.x,normal.y,normal.z,0};

    Polynomial4x4 p0 =
    {
        -1,1,0,0,
        0,-1,0,0
    };
    Polynomial4x4 da0 =
    {
        0,0,0,0,
        0,0,0,0
    };
    Polynomial4x4 db0 =
    {
        0,0,0,0,
        0,0,0,0
    };
    Polynomial4x4 d0 =
    {
        0,0,0,0,
        0,0,0,0
    };
    Polynomial4x4 tangent =
    {
        0,0,0,0,
        0,0,0,0
    };

    float seekX = 0.0f;
    float seekY = 0.0f;
    float yaw = 0.0f;

    constexpr const float intervalBegin = -0.8f;
    constexpr const float intervalEnd = 0.8f;
    constexpr const Vector3 DEFAULT_VIEW = { 1.0f, -1.0f, 1.0f };
    constexpr const Vector3 DEFAULT_VIEW_UP = { 0, 0, 1 };
    constexpr const Vector3 RIGHT_VIEW = { 1.0f, 0, 0 };
    constexpr const Vector3 FRONT_VIEW = { 0, 1, 0 };
    constexpr const Vector3 BACK_VIEW = { 0, -1, 0 };
    constexpr const Vector3 TOP_VIEW = { 0, 0, 1 };
    constexpr const Vector3 TOP_VIEW_UP = { 0, 1, 0 };

    std::vector<std::vector<float>> samples(RESOLUTION_X, std::vector<float>(RESOLUTION_Y, 0.0f));

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);

    Camera camera = { 0 };
    camera.position = DEFAULT_VIEW;
    camera.target = Vector3{ 0.0f, 0.0f, 0.0f };
    camera.up = DEFAULT_VIEW_UP;
    camera.fovy = 2.0f;
    camera.projection = CAMERA_ORTHOGRAPHIC;

    SetCameraMode(camera, CAMERA_FREE);

    SetTargetFPS(10);

    while (!WindowShouldClose())
    {
        const bool increaseA = IsKeyDown(KEY_KP_6);
        seekX += PARAMETERS_ADJUSTEMENT_MULTIPLIER * increaseA;
        const bool decreaseA = IsKeyDown(KEY_KP_4);
        seekX -= PARAMETERS_ADJUSTEMENT_MULTIPLIER * decreaseA;

        const bool increaseB = IsKeyDown(KEY_KP_8);
        seekY += PARAMETERS_ADJUSTEMENT_MULTIPLIER * increaseB;
        const bool decreaseB = IsKeyDown(KEY_KP_2);
        seekY -= PARAMETERS_ADJUSTEMENT_MULTIPLIER * decreaseB;

        const bool increaseYaw = IsKeyDown(KEY_KP_9);
        yaw += PARAMETERS_ADJUSTEMENT_MULTIPLIER * increaseYaw;
        const bool decreaseYaw = IsKeyDown(KEY_KP_7);
        yaw -= PARAMETERS_ADJUSTEMENT_MULTIPLIER * decreaseYaw;

        if (IsKeyDown(KEY_R))
        {
            p = { normal.x,normal.y,normal.z,0 };
        }

        da0 = p0.PartialDerivativeA();
        db0 = p0.PartialDerivativeB();
        d0 = p0.Derivative();

        p.SetSlopeOnXZ(da0.SampleFunctionAt(seekX, 0));
        p.SetSlopeOnYZ(db0.SampleFunctionAt(0, seekY));

        camera.position.x = std::cosf(yaw);
        camera.position.y = std::sinf(yaw);

        BeginDrawing();

        ClearBackground(RAYWHITE);

        BeginMode3D(camera);

        // X axis
        DrawLine3D(
            {
                0.0f, 0.0f, 0.0f
            },
            {
                1.0f, 0.0f, 0.0f
            },
            RED
        );
        // Y axis
        DrawLine3D(
            {
                0.0f, 0.0f, 0.0f
            },
            {
                0.0f, 1.0f, 0.0f
            },
            GREEN
        );
        // Z axis
        DrawLine3D(
            {
                0.0f, 0.0f, 0.0f
            },
            {
                0.0f, 0.0f, 1.0f
            },
            BLUE
        );

        // Polynomial.
        SamplePolynomial4AtInterval(p0, intervalBegin, intervalEnd, 2.0f / RESOLUTION_X, intervalBegin, intervalEnd, 2.0f / RESOLUTION_Y, samples);
        DrawSamples(samples, intervalBegin, intervalEnd, 2.0f / RESOLUTION_X, intervalBegin, intervalEnd, 2.0f / RESOLUTION_Y);

        // Derivative.
        // SamplePolynomial4AtInterval(d0, intervalBegin, intervalEnd, 2.0f / RESOLUTION_X, intervalBegin, intervalEnd, 2.0f / RESOLUTION_Y, z);
        // DrawSamples(z, intervalBegin, intervalEnd, 2.0f / RESOLUTION_X, intervalBegin, intervalEnd, 2.0f / RESOLUTION_Y);
        
        // Tangent.
        SamplePlaneAtInterval(p, intervalBegin / 4.0f, intervalEnd / 4.0f, 2.0f / RESOLUTION_X, intervalBegin / 4.0f, intervalEnd / 4.0f, 2.0f / RESOLUTION_Y, samples);
        DrawSamples(samples, intervalBegin / 4.0f, intervalEnd / 4.0f, 2.0f / RESOLUTION_X, intervalBegin / 4.0f, intervalEnd / 4.0f, 2.0f / RESOLUTION_Y, {seekX, seekY, p0.SampleFunctionAt(seekX, seekY)});

        EndMode3D();

        EndDrawing();
    }

    CloseWindow();

    return 0;
}