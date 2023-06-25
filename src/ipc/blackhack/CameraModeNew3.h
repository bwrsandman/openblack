#ifndef BLACKHACK_CAMERAMODENEW2_H_
#define BLACKHACK_CAMERAMODENEW2_H_

#include "CameraMode.h"

#include "lhlib/LHCoord.h"
#include "lhlib/LHMatrix.h"
#include "lhlib/Zoomer.h"

enum CameraModeNew3_fight_status_t {
  CameraModeNew3_fight_status_t_0x0 = 0,
  CameraModeNew3_fight_status_t_0x1 = 1,
  CameraModeNew3_fight_status_t_0x2 = 2,

  _CameraModeNew3_fight_status_t_COUNT,
  _CameraModeNew3_fight_status_t_FORCE_32_BITS = 0x7FFFFFFF,
};

static const char* CameraModeNew3_fight_status_t_strs[_CameraModeNew3_fight_status_t_COUNT] = {
  "CameraModeNew3_fight_status_t_0x0",
  "CameraModeNew3_fight_status_t_0x1",
  "CameraModeNew3_fight_status_t_0x2",
};

enum CameraModeNew3_Tricons {
  CameraModeNew3_Tricons_None = 0,
  CameraModeNew3_Tricons_Rotate = 1 << 0,
  CameraModeNew3_Tricons_Pitch = 1 << 1,
  CameraModeNew3_Tricons_Top = 1 << 2,
  CameraModeNew3_Tricons_0x8 = 1 << 3,
  CameraModeNew3_Tricons_0x10 = 1 << 4,
  CameraModeNew3_Tricons_0x20 = 1 << 5,
  CameraModeNew3_Tricons_0x40 = 1 << 6,
  CameraModeNew3_Tricons_Inactive = 1 << 7,

  _CameraModeNew3_Tricons_FORCE_32_BITS = 0x7FFFFFFF,
};


struct CameraModeNew3 {
  struct CameraMode super;
  float field0x8;
  struct LHPoint origin;
  struct LHPoint heading;
  struct LHPoint field0x24;
  struct LHPoint field0x30;
  uint32_t field0x3c;
  uint32_t field0x40;
  bool32_t hasFight;
  uint32_t arena;
  bool field0x4c;
  bool field0x4d;
  bool field0x4e;
  bool field0x4f;
  float yaw0;
  float pitch0;
  float fightDistance;
  int fightTimeLeft;
  int timeInArena;
  enum CameraModeNew3_fight_status_t fightStatus;
  struct LHPoint field0x68;
  float field0x74;
  float elapsedTime;
  struct LHPoint rotatePoint;
  bool rotateAroundPoint;
  enum CAMERA_MODE_MOUSE_STATUS mouseButtons;
  enum CameraModeNew3_Tricons tricon0x90;
  enum CameraModeNew3_Tricons tricon0x94;
  struct LHPoint2D fromScreenCentre;
  struct LHPoint2D fromScreenCentreAbs;
  struct LHCoord mouseDelta;
  struct LHCoord mousePosCurrent;
  struct LHCoord rotateAroundMousePos;
  struct LHCoord mousePos1;
  bool screenCentreHit;
  struct LHPoint mouseHitPoint;
  struct LHPoint lastGrabMouseHitPoint;
  float yaw1;
  float pitch1;
  float perpDistance0xec;
  struct LHPoint screenCentreHitPoint;
  float originCentreYDiff;
  bool field0x100;
  bool field0x101;
  bool field0x102;
  bool field0x103;
  struct LHPoint field0x104;
  struct LHPoint field0x110;
  struct LHPoint field0x11c;
  int timeAtClick;
  struct LHPoint headingAtClick;
  struct LHPoint originAtClick;
  bool handHit;
  bool field0x145;
  bool field0x146;
  bool field0x147;
  bool lastHandHit;
  struct LHPoint handHeadingNormalAtInteractionStart;
  double mouseHitPointDot;
  struct LHPoint originAtInteractionStart;
  struct LHPoint headingAtInteractionStart;
  struct LHMatrix matrixAtInteractionStart;
  float originHeadingDistanceAtInteractionStart;
  enum CAMERA_MODE_HAND_STATUS handStatus;
  struct LHPoint2D fromScreenCentreAtInteractionStart;
  struct LHPoint2D mouseClickDelta;
  float averageIslandDistance;
  float verticalDistance;
  struct LHPoint fallbackOrigin;
  struct LHPoint fallbackHeading;
  struct LHPoint field0x1e0;
  struct LHPoint origin0x1ec;
  struct LHPoint field0x1f8;
  struct LHPoint heading0x204;
  struct Zoomer field0x210;
  struct Zoomer3d field0x240;
  bool field0x2d0;
  uint8_t field0x2d1;
  uint8_t field0x2d2;
  uint8_t field0x2d3;
  struct LHCoord mousePosPrevious;
  struct LHCoord mousePosWithDelta;
  float headingDistance;
  float idleTime;
  bool32_t field0x2ec;
  bool32_t interactionThisFrame;
  uint32_t cameraExclusion;
  uint32_t field0x2f8;
  uint32_t field0x2fc;
};
static_assert(offsetof(struct CameraModeNew3, field0x74) == 0x74, "Struct offset is wrong");
static_assert(offsetof(struct CameraModeNew3, fromScreenCentreAtInteractionStart) == 0x1b0, "Struct is of wrong size");
static_assert(sizeof(struct CameraModeNew3) == 0x300, "Struct is of wrong size");

#endif // BLACKHACK_CAMERAMODENEW2_H_
