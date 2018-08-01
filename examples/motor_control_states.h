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
class IdleState : public fsm::State
{
public:
  explicit IdleState(fsm::Fsm& fsm);
  void onEntry() final;
  void onExit() final;
};

//=====================================================================================================================
/// powering up
class PowerUpState : public fsm::State
{
public:
  explicit PowerUpState(fsm::Fsm& ctx);
  void onEntry() final;
  void onExit() final;
};

//=====================================================================================================================
/// powering down
class PowerDownState : public fsm::State
{
public:
  explicit PowerDownState(fsm::Fsm& ctx);
  void onEntry() final;
  void onExit() final;
};

//=====================================================================================================================
/// maintain speed
class SpeedControlState : public fsm::State
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
  void trigger(const fsm::Fsm::Event& signal);
  fsm::State::Id getActiveState() const;

  MotorController(const MotorController&) = delete;
  MotorController(const MotorController&&) = delete;
  MotorController& operator=(const MotorController&) = delete;
  MotorController& operator=(const MotorController&&) = delete;

private:
  fsm::Fsm controller_fsm_;
};
