/*################################################################
Pandrift
Copyright (c) 2013 Warren Moore

This software may be redistributed under the terms of the MIT License.
See the file LICENSE for details.
################################################################*/

#include "pandrift_rift_manager.hh"
#include <iostream>

using namespace std;
using namespace OVR;
using namespace OVR::Util::Render;

namespace
{

const float cDefaultIPD = 0.0655;
const float cDistortionFitPoint[2] = { -0.75, 0.0 };

}

namespace pandrift
{

RiftManager::RiftManager()
{
  System::Init(Log::ConfigureDefaultLog(LogMask_All));

  // Quick and dirty!
  device_manager_ptr_ = *DeviceManager::Create();
  device_ptr_ = *device_manager_ptr_->EnumerateDevices<HMDDevice>().CreateDevice();

  HMDInfo rift_info;
  if (device_ptr_ && device_ptr_->GetDeviceInfo(&rift_info))
  {
    Ptr<SensorDevice> sensor_ptr = device_ptr_->GetSensor();
    sensor_fusion_.AttachToSensor(sensor_ptr);

    stereo_config_.SetHMDInfo(rift_info);
  }

  stereo_config_.SetIPD(cDefaultIPD);
  stereo_config_.SetDistortionFitPointVP(cDistortionFitPoint[0],
                                         cDistortionFitPoint[1]);

  // No difference in the parameters I'm using between each eye
  eye_params_ = stereo_config_.GetEyeRenderParams(StereoEye_Left);
}

RiftManager::~RiftManager()
{

}

int RiftManager::get_display_width_pixels()
{
  return stereo_config_.GetHMDInfo().HResolution;
}

int RiftManager::get_display_height_pixels()
{
  return stereo_config_.GetHMDInfo().VResolution;
}

float RiftManager::get_display_width_metres()
{
  return stereo_config_.GetHMDInfo().HScreenSize;
}

float RiftManager::get_display_height_metres()
{
  return stereo_config_.GetHMDInfo().VScreenSize;
}

float RiftManager::get_lens_separation()
{
  return stereo_config_.GetHMDInfo().LensSeparationDistance;
}

float RiftManager::get_eye_screen_distance()
{
  return stereo_config_.GetHMDInfo().EyeToScreenDistance;
}

float RiftManager::get_y_fov_radians()
{
  return stereo_config_.GetYFOVRadians();
}

float RiftManager::get_display_aspect_ratio()
{
  return (float(stereo_config_.GetHMDInfo().HResolution) * 0.5) / float(stereo_config_.GetHMDInfo().VResolution);
}

float RiftManager::get_interpupillary_distance()
{
  return stereo_config_.GetIPD();
}

float RiftManager::get_projection_centre_offset()
{
  return stereo_config_.GetProjectionCenterOffset();
}

float RiftManager::get_distortion_scale()
{
  return eye_params_.pDistortion->Scale;
}

float RiftManager::get_distortion_centre_offset()
{
  return eye_params_.pDistortion->XCenterOffset;
}

LVector4f RiftManager::get_distortion_coefficients()
{
  return LVector4f(eye_params_.pDistortion->K[0],
                   eye_params_.pDistortion->K[1],
                   eye_params_.pDistortion->K[2],
                   eye_params_.pDistortion->K[3]);
}

LVector4f RiftManager::get_chromatic_aberration_coefficients()
{
  return LVector4f(eye_params_.pDistortion->ChromaticAberration[0],
                   eye_params_.pDistortion->ChromaticAberration[1],
                   eye_params_.pDistortion->ChromaticAberration[2],
                   eye_params_.pDistortion->ChromaticAberration[3]);
}

bool RiftManager::get_sensor_euler_angles(float &yaw, float &pitch, float &roll)
{
  if (!sensor_fusion_.IsAttachedToSensor())
    return false;

  Quatf value = sensor_fusion_.GetOrientation();
  value.GetEulerAngles<Axis_Y, Axis_X, Axis_Z>(&yaw, &pitch, &roll);

  return true;
}

}
