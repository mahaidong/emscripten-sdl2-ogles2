//
// Window and input event handling
//
#include "camera.h"

class EventHandler
{
public:
    EventHandler(const char *windowTitle);

    void processEvents();

    Camera &camera() { return mCamera; }

    void swapWindow();

private:
    // Camera
    Camera mCamera;

    // Window
    SDL_Window *mpWindow;

    Uint32 mWindowID;

    void windowResizeEvent(int width, int height);

    void initWindow(const char *title);

    // Mouse input
    const float cMouseWheelZoomDelta;
    bool mMouseButtonDown;
    int mMouseButtonDownX, mMouseButtonDownY;
    int mMousePositionX, mMousePositionY;

    // Finger input
    bool mFingerDown;
    float mFingerDownX, mFingerDownY;
    long long mFingerDownId;

    // Pinch input
    const float cPinchZoomThreshold, cPinchScale;
    bool mPinch;

    // Events
    void zoomEventMouse(bool mouseWheelDown, int x, int y);
    void zoomEventPinch(float pinchDist, float pinchX, float pinchY);
    void panEventMouse(int x, int y);
    void panEventFinger(float x, float y);
};

inline EventHandler::EventHandler(const char *windowTitle)
    : mpWindow(nullptr), mWindowID(0), // Window
      cMouseWheelZoomDelta(0.05f),     // mouse
      mMouseButtonDown(false),
      mMouseButtonDownX(0),
      mMouseButtonDownY(0),
      mMousePositionX(0),
      mMousePositionY(0),

      mFingerDown(false), // Finger input
      mFingerDownX(0.0f),
      mFingerDownY(0.0f),
      mFingerDownId(0),

      cPinchZoomThreshold(0.001f), // Pinch input
      cPinchScale(8.0f),
      mPinch(false)
{
    initWindow(windowTitle);
}