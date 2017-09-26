
#ifndef CAMERAGRABBER_H_
#define CAMERAGRABBER_H_

#include <iostream>

#include "Include/VimbaCPP.h"

class CameraGrabber {
 public:
  // ___________________________________________________________________________
  CameraGrabber();
  ~CameraGrabber();
void init(AVT::VmbAPI::CameraPtr cam, std::string pixelFormat,
      VmbInt64_t width, VmbInt64_t height, float fps,
      AVT::VmbAPI::IFrameObserverPtr rec);

  
  inline void startAcquisition() const {
    if (startAcquisitionF->RunCommand() != VmbErrorSuccess)
      throw std::runtime_error("Couldn't start acquisition!");
  }
  inline void stopAcquisition() const {
    if (stopAcquisitionF->RunCommand() != VmbErrorSuccess)
      throw std::runtime_error("Couldn't stop acquisition!");
  }

 private:
  
  void setMaxValueModulo2(const char* const& featureName);

  void setFeatureToValue(const char* const& featureName, const char* const& value);
  void setFeatureToValue(const char* const& featureName, VmbInt64_t value);
  void setFeatureToValue(const char* const& featureName, float value);
  void getFeature(const char* const& featureName, VmbInt64_t& val) const;
  void getFeature(const char* const& featureName, std::string& val) const;

    AVT::VmbAPI::CameraPtr camera;  // The camera pointer.
  AVT::VmbAPI::FramePtrVector frames;  // Frame buffers.
  AVT::VmbAPI::FeaturePtr startAcquisitionF;  // Run this to start acquisition.
  AVT::VmbAPI::FeaturePtr stopAcquisitionF;  // Run this to stop acquisition.
};

#endif  
