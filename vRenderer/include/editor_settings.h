#pragma once

#include <vector>

#define ASSETS_FOLDER "vRenderer\\assets\\"
#define MODEL_ASSETS_FOLDER "vRenderer\\assets\\models\\"
#define MODEL_ASSETS(asset) concat(MODEL_ASSETS_FOLDER, asset)

#define CAMERA_ZOOM_STEP 2.0f
#define CAMERA_ROTATION_SPEED 0.5f
#define CAMERA_FPV_SENSETIVITY_HORIZONTAL 0.35f
#define CAMERA_FPV_SENSETIVITY_VERTICAL 0.3f
#define CAMERA_FPV_SPEED 8.0f
#define CAMERA_FPV_INCREASED_SPEED 16.0f
#define CAMERA_INITIAL_POSITION { 0, 0, 10.f}

enum CameraType
{
    ORBIT,
    FPV
};

const std::vector<const char*> c_supportedFormats = { ".obj" };