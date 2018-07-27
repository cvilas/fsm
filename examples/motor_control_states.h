//=====================================================================================================================
// Copyright (C) 2018 Vilas Kumar Chitrakaran
//
// This file is part of project fsm (https://github.com/cvilas/fsm)
//
// Licensed under the MIT License. See LICENSE.md
//=====================================================================================================================

#include "fsm/fsm.h"

//=====================================================================================================================
/// do nothing state
class IdleState : public fsm::FsmState
{
public:
  explicit IdleState(fsm::Fsm& fsm);
  void onEntry() final;
  void onExit() final;
};

//=====================================================================================================================
/// powering up
class PowerUpState : public fsm::FsmState
{
public:
  explicit PowerUpState(fsm::Fsm& ctx);
  void onEntry() final;
  void onExit() final;
};

//=====================================================================================================================
/// powering down
class PowerDownState : public fsm::FsmState
{
public:
  explicit PowerDownState(fsm::Fsm& ctx);
  void onEntry() final;
  void onExit() final;
};

//=====================================================================================================================
/// maintain speed
class SpeedControlState : public fsm::FsmState
{
public:
  explicit SpeedControlState(fsm::Fsm& ctx);
  void onEntry() final;
  void onExit() final;
};

//=====================================================================================================================
/// The automatic motor controller
class MotorController
{
public:
  MotorController();
  ~MotorController();
  void trigger(const fsm::FsmSignal& signal);
  fsm::FsmName getCurrentState() const;

  MotorController(const MotorController&) = delete;
  MotorController(const MotorController&&) = delete;
  MotorController& operator=(const MotorController&) = delete;
  MotorController& operator=(const MotorController&&) = delete;

private:
  fsm::Fsm controller_fsm_;
};
