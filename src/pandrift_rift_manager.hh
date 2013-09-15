/*################################################################
Pandrift
Copyright (c) 2013 Warren Moore

This software may be redistributed under the terms of the MIT License.
See the file LICENSE for details.
################################################################*/

#ifndef PANDRIFT_DISPLAY_RIFT_HEADER
#define PANDRIFT_DISPLAY_RIFT_HEADER

#include "pandrift.hh"
#include "OVR.h"
#include "lvector4.h"

namespace pandrift
{

class RiftManager
{
public:
  RiftManager();

  ~RiftManager();

  int get_display_width();

  int get_display_height();

  float get_y_fov_radians();

  float get_display_aspect_ratio();

  float get_interpupillary_distance();

  float get_projection_centre_offset();

  float get_distortion_scale();

  float get_distortion_centre_offset();

  LVector4f get_distortion_coefficients();

  LVector4f get_chromatic_aberration_coefficients();

  bool get_sensor_euler_angles(float &yaw, float &pitch, float &roll);

private:
  OVR::Ptr<OVR::DeviceManager> device_manager_ptr_;
  OVR::Ptr<OVR::HMDDevice> device_ptr_;
  OVR::SensorFusion sensor_fusion_;
  OVR::Util::Render::StereoConfig stereo_config_;
  OVR::Util::Render::StereoEyeParams eye_params_;
};

}

#endif
