#pragma once

#include <vector>

#define ASSETS_FOLDER "vRenderer\\assets\\"
#define MODEL_ASSETS_FOLDER "vRenderer\\assets\\models\\"
#define MODEL_ASSETS(asset) concat(MODEL_ASSETS_FOLDER, asset)

#define CAMERA_ZOOM_STEP 2.0f
#define CAMERA_ROTATION_SPEED 120.0f

const std::vector<const char*> c_supportedFormats = { ".obj" };