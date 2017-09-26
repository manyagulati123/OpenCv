
#include "CameraGrabber.h"
#include "FFmpegOutput.h"

#define FRAMEBUFFER_SIZE 3

CameraGrabber::CameraGrabber() : frames(FRAMEBUFFER_SIZE)/*, camera(NULL) */ {
}
CameraGrabber::~CameraGrabber() {
  if (camera != NULL) {
    std::cout << "Deinitialize..." << std::endl;
    camera->EndCapture();
    camera->FlushQueue();
    camera->RevokeAllFrames();
    camera->Close();
  }
}


void CameraGrabber::init(AVT::VmbAPI::CameraPtr cameraPtr,
    std::string pixelFormat, VmbInt64_t width, VmbInt64_t height,
    float fps, AVT::VmbAPI::IFrameObserverPtr receiverPtr) {
  camera = cameraPtr;

  VmbErrorType result;


  std::string cameraId;
  if (camera->GetID(cameraId) != VmbErrorSuccess)
    throw std::runtime_error("Could not get the camera id!");
std::cout<<"Camera id: "<< cameraId;
std::cout<<"\n";

  std::string cameraName;
  if (camera->GetName(cameraName) != VmbErrorSuccess)
    throw std::runtime_error("Could not get the camera's name!");
std::cout<<"Camera name: "<< cameraName;
std::cout<<"\n";

  if (camera->Open(VmbAccessModeFull) != VmbErrorSuccess)
    throw std::runtime_error("Could not open the camera!");
std::cout<<"Camera open: "<< VmbAccessModeFull;
std::cout<<"\n";

  setFeatureToValue("PixelFormat", pixelFormat.c_str());


  if (width == 0) setMaxValueModulo2("Width");
  else setFeatureToValue("Width", width);

  if (height == 0) setMaxValueModulo2("Height");
  else setFeatureToValue("Height", height);

  setFeatureToValue("AcquisitionMode", "Continuous");
  setFeatureToValue("AcquisitionFrameRateAbs", (float) fps);
long long int xyz = 1LL;
setFeatureToValue("AcquisitionFrameCount", xyz);

  VmbInt64_t framesize;
  getFeature("PayloadSize", framesize);
std::cout<<"Frame size: "<< framesize;
std::cout<<"\n";

  for (AVT::VmbAPI::FramePtrVector::iterator f = frames.begin();
      f != frames.end(); f++) {
    // Allocate the frame.
    (*f).reset(new AVT::VmbAPI::Frame(framesize));

    // Register the observer for the frame.
    if ((*f)->RegisterObserver(receiverPtr) != VmbErrorSuccess)
      throw std::runtime_error("Could not register the frame observer!");

    // Assign the frame to the camera.
    if (camera->AnnounceFrame(*f) != VmbErrorSuccess)
      throw std::runtime_error("Could not announce the frame!");
  }

  // Start the camera's capture engine.
  if (camera->StartCapture() != VmbErrorSuccess)
    throw std::runtime_error("Could not start capturing!");

  // Queue all frames for data acquisition.
  for (AVT::VmbAPI::FramePtrVector::iterator f = frames.begin();
      f != frames.end(); f++)
    if (camera->QueueFrame(*f) != VmbErrorSuccess)
      throw std::runtime_error("Could not queue the frame!");

  // Get the feature pointers to start and stop the acquisition.
  if (
      camera->GetFeatureByName("AcquisitionStart", startAcquisitionF) != VmbErrorSuccess ||
      camera->GetFeatureByName("AcquisitionStop", stopAcquisitionF) != VmbErrorSuccess
     )
    throw std::runtime_error("Could not get acquisition start/stop features!");

  std::cout
    << "Opened camera " << cameraId << " (" << cameraName << ")"
    << std::endl;
}

// _____________________________________________________________________________
void CameraGrabber::setMaxValueModulo2(const char* const& featureName) {
  AVT::VmbAPI::FeaturePtr feature; VmbInt64_t min, max;

  if (
      camera->GetFeatureByName(featureName, feature) != VmbErrorSuccess ||
      feature->GetRange(min, max) != VmbErrorSuccess
     )
    throw std::runtime_error(featureName);

  max = (max >> 1) << 1;  // Erase the last bit.

  if (feature->SetValue(max) != VmbErrorSuccess)
    throw std::runtime_error(featureName);

  std::cout << "Set \"" << featureName << "\" to " << max << std::endl;
}

void CameraGrabber::setFeatureToValue(const char* const& featureName,
    const char* const& value) {
  AVT::VmbAPI::FeaturePtr feature;
  if (camera->GetFeatureByName(featureName, feature) != VmbErrorSuccess ||
      feature->SetValue(value) != VmbErrorSuccess)
    throw std::runtime_error(featureName);
  std::cout << "Set \"" << featureName << "\" to " << value << std::endl;
}

void CameraGrabber::setFeatureToValue(const char* const& featureName,
    VmbInt64_t value) {
  AVT::VmbAPI::FeaturePtr feature;
  if (camera->GetFeatureByName(featureName, feature) != VmbErrorSuccess ||
      feature->SetValue(value) != VmbErrorSuccess)
    throw std::runtime_error(featureName);
  std::cout << "Set \"" << featureName << "\" to " << value << std::endl;
}



void CameraGrabber::setFeatureToValue(const char* const& featureName,
    float value) {
  AVT::VmbAPI::FeaturePtr feature;
  if (camera->GetFeatureByName(featureName, feature) != VmbErrorSuccess ||
      feature->SetValue(value) != VmbErrorSuccess)
    throw std::runtime_error(featureName);
  std::cout << "Set \"" << featureName << "\" to " << value << std::endl;
}

void CameraGrabber::getFeature(const char* const& featureName, VmbInt64_t& val) const {
  AVT::VmbAPI::FeaturePtr feature;
  if (camera->GetFeatureByName(featureName, feature) != VmbErrorSuccess ||
      feature->GetValue(val) != VmbErrorSuccess)
    throw std::runtime_error(featureName);
}

void CameraGrabber::getFeature(const char* const& featureName,
    std::string& val) const {
  AVT::VmbAPI::FeaturePtr feature;
  if (camera->GetFeatureByName(featureName, feature) != VmbErrorSuccess ||
      feature->GetValue(val) != VmbErrorSuccess)
    throw std::runtime_error(featureName);
}
