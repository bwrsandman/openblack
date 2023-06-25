#ifndef BLACKHACK_CAMERAMODE_H_
#define BLACKHACK_CAMERAMODE_H_

#include "types.h"

#include "lhlib/LHPoint.h"

struct GameOSFile;

struct CameraModeVftable {
  struct CameraModeNew3*(__fastcall* __dt__)(void*, const void* edx, uint32_t);
  bool(__fastcall* CanPlayerGestureWhenCameraMoving)(void*);
  void(__fastcall* Update)(void*);
  void(__fastcall* Validate)(void*);
  void(__fastcall* Restart)(void*);
  bool(__fastcall* IsStillValid)(void*);
  void(__fastcall* Cleanup)(void*);
  bool(__fastcall* CanExist)(void*);
  bool(__fastcall* MouseIsLocked)(void*);
  uint32_t GetMousePos;
  void(__fastcall* ProcessKeyMovement)(void*, const void* edx, uint16_t);
  void(__fastcall* ProcessMouseMovement)(void*);
  void(__fastcall* Delete)(void*);
  bool(__fastcall* Arrived)(void*);
  uint32_t(__fastcall* GetSaveID)(void*);
  void(__fastcall* Load)(void*, const void* edx, struct GameOSFile*);
  void(__fastcall* Save)(void*, const void* edx, struct GameOSFile*);
  const char*(__fastcall* GetDebugName)(void*);
  void(__fastcall* Initialise)(void*);
  void(__fastcall* Reinitialise)(void*, const void* edx, bool);
  void(__fastcall* FlyToPosFoc)(void*, const void* edx, struct LHPoint*, struct LHPoint*, float);
  void(__fastcall* SetupVia)(void*, const void* edx, const struct LHPoint*, const struct LHPoint*, const struct LHPoint*, float);
  uint32_t(__fastcall* GetCameraFeatures)(void*);
  void(__fastcall* ForceRotateAboutPoint)(void*, const void* edx, struct LHPoint* point);
};

enum CameraModeVTableAddresses {
  CameraModeVTable__CameraModeFollow = 0x008c7884,
  CameraModeVTable__CameraModeFree = 0x008c7958,
  CameraModeVTable__CameraModeScript = 0x008c7d5c,
  CameraModeVTable__CameraModeNew3 = 0x008c7bfc,
};

struct CameraMode {
  uint32_t vftable;
  uint32_t camera;
};

static struct TypeDescriptor* p_class_CameraMode_RTTI_Type_Descriptor = (struct TypeDescriptor*)0x009cd390;

struct CameraExclusion {
  struct CameraExclusion* next;
  struct CameraExclusion* prev;
  uint32_t field_0x8;
  struct LHPoint field_0xc;
  float field_0x18;
  float field_0x1c;
  uint32_t type;
  uint32_t field_0x24;
};

// win1.41 00455d50 mac 10000050 CameraExclusion::InsideExclusion(LHPoint)
bool __cdecl InsideExclusion__15CameraExclusionF7LHPoint(struct LHPoint point);
// win1.41 00455e20 mac 1004f140 CameraExclusion::InsideInclusion(LHPoint, LHPoint, LHPoint *, LHPoint *)
bool __cdecl InsideInclusion__15CameraExclusionF7LHPoint7LHPointP7LHPointP7LHPoint(struct LHPoint param_1, struct LHPoint param_2, struct LHPoint* param_3, struct LHPoint* param_4);

enum CAMERA_MODE_HAND_STATUS {
  CAMERA_MODE_HAND_STATUS_NORMAL = 0x0,
  CAMERA_MODE_HAND_STATUS_ZOOMING = 0x1,
  CAMERA_MODE_HAND_STATUS_TILT_ON = 0x2,
  CAMERA_MODE_HAND_STATUS_GRABBING_LAND = 0x3,
  CAMERA_MODE_HAND_STATUS_PANNING = 0x4,
  CAMERA_MODE_HAND_STATUS_TILTING = 0x5,
  CAMERA_MODE_HAND_STATUS_0x6 = 0x6,
  CAMERA_MODE_HAND_STATUS_0x7 = 0x7,
  CAMERA_MODE_HAND_STATUS_0x8 = 0x8,
  CAMERA_MODE_HAND_STATUS_0x9 = 0x9,
  CAMERA_MODE_HAND_STATUS_0xa = 0xa,

  _CAMERA_MODE_HAND_STATUS_COUNT,

  _CAMERA_MODE_HAND_STATUS_FORCE_32_BIT = 0x7FFFFFFF,
};

static const char* CAMERA_MODE_HAND_STATUS_strs[_CAMERA_MODE_HAND_STATUS_COUNT] = {
  "CAMERA_MODE_HAND_STATUS_NORMAL",
  "CAMERA_MODE_HAND_STATUS_ZOOMING",
  "CAMERA_MODE_HAND_STATUS_TILT_ON",
  "CAMERA_MODE_HAND_STATUS_GRABBING_LAND",
  "CAMERA_MODE_HAND_STATUS_PANNING",
  "CAMERA_MODE_HAND_STATUS_TILTING",
  "CAMERA_MODE_HAND_STATUS_0x6",
  "CAMERA_MODE_HAND_STATUS_0x7",
  "CAMERA_MODE_HAND_STATUS_0x8",
  "CAMERA_MODE_HAND_STATUS_0x9",
  "CAMERA_MODE_HAND_STATUS_0xa",
};

enum CAMERA_MODE_MOUSE_STATUS {
  CAMERA_MODE_MOUSE_STATUS_NONE = 0,
  CAMERA_MODE_MOUSE_STATUS_LEFT = 1 << 0,
  CAMERA_MODE_MOUSE_STATUS_MIDDLE_OR_KEYBOARD = 1 << 1,
};

#endif // BLACKHACK_CAMERAMODE_H_
