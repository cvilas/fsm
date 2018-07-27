//=====================================================================================================================
// Copyright (C) 2018 Vilas Kumar Chitrakaran
//
// This file is part of project fsm (https://github.com/cvilas/fsm)
//
// Licensed under the MIT License. See LICENSE.md
//=====================================================================================================================

#include "motor_control_states.h"
#include <iostream>

//=====================================================================================================================
IdleState::IdleState(fsm::Fsm& fsm) : fsm::FsmState(fsm, "idle")
//=====================================================================================================================
{
}

//----------------------------------------------------------------------------------------------------------------------
void IdleState::onEntry()
//----------------------------------------------------------------------------------------------------------------------
{
  std::cout << "[" << getName() << "::onEntry]\n";
}

//----------------------------------------------------------------------------------------------------------------------
void IdleState::onExit()
//----------------------------------------------------------------------------------------------------------------------
{
  std::cout << "[" << getName() << "::onExit]\n";
}

//=====================================================================================================================
PowerUpState::PowerUpState(fsm::Fsm& ctx) : fsm::FsmState(ctx, "power_up")
//=====================================================================================================================
{
}

//----------------------------------------------------------------------------------------------------------------------
void PowerUpState::onEntry()
//----------------------------------------------------------------------------------------------------------------------
{
  std::cout << "[" << getName() << "::onEntry] entered\n";
  std::this_thread::sleep_for(std::chrono::seconds(2));
  getFsm().raise("maintain_speed");
  std::cout << "[" << getName() << "::onEntry] exited\n";
}

//----------------------------------------------------------------------------------------------------------------------
void PowerUpState::onExit()
//----------------------------------------------------------------------------------------------------------------------
{
  std::cout << "[" << getName() << "::onExit]\n";
}

//=====================================================================================================================
PowerDownState::PowerDownState(fsm::Fsm& ctx) : fsm::FsmState(ctx, "power_down")
//=====================================================================================================================
{
}

//----------------------------------------------------------------------------------------------------------------------
void PowerDownState::onEntry()
//----------------------------------------------------------------------------------------------------------------------
{
  std::cout << "[" << getName() << "::onEntry] entered\n";
  std::this_thread::sleep_for(std::chrono::seconds(2));
  getFsm().raise("has_shutdown");
  std::cout << "[" << getName() << "::onEntry] exited\n";
}

//----------------------------------------------------------------------------------------------------------------------
void PowerDownState::onExit()
//----------------------------------------------------------------------------------------------------------------------
{
  std::cout << "[" << getName() << "::onExit]\n";
}

//=====================================================================================================================
SpeedControlState::SpeedControlState(fsm::Fsm& ctx) : FsmState(ctx, "speed_control")
//=====================================================================================================================
{
}

//----------------------------------------------------------------------------------------------------------------------
void SpeedControlState::onEntry()
//----------------------------------------------------------------------------------------------------------------------
{
  std::cout << "[" << getName() << "::onEntry]\n";
}

//----------------------------------------------------------------------------------------------------------------------
void SpeedControlState::onExit()
//----------------------------------------------------------------------------------------------------------------------
{
  std::cout << "[" << getName() << "::onExit]\n";
}

//=====================================================================================================================
MotorController::MotorController()
//=====================================================================================================================
{
  controller_fsm_.addState(std::make_shared<IdleState>(controller_fsm_));
  controller_fsm_.addState(std::make_shared<PowerUpState>(controller_fsm_));
  controller_fsm_.addState(std::make_shared<PowerDownState>(controller_fsm_));
  controller_fsm_.addState(std::make_shared<SpeedControlState>(controller_fsm_));

  controller_fsm_.addTransitionRule("idle", "on", "power_up");
  controller_fsm_.addTransitionRule("power_up", "maintain_speed", "speed_control");
  controller_fsm_.addTransitionRule("speed_control", "off", "power_down");
  controller_fsm_.addTransitionRule("power_up", "off", "power_down");
  controller_fsm_.addTransitionRule("power_down", "on", "power_up");
  controller_fsm_.addTransitionRule("power_down", "has_shutdown", "idle");

  controller_fsm_.initialise("idle");
}

//----------------------------------------------------------------------------------------------------------------------
MotorController::~MotorController()
//----------------------------------------------------------------------------------------------------------------------
{
  try
  {
    std::cout << "State while exiting: " << getCurrentState() << "\n" << std::flush;
    controller_fsm_.raise("off");
    std::cout << "Waiting for pending triggers to flush..\n" << std::flush;
    while (controller_fsm_.isStateTransitionPending())
    {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "No pending triggers\n" << std::flush;
    std::cout << "Waiting for \"idle\" state..\n" << std::flush;
    while (getCurrentState() != "idle")
    {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "State \"idle\" reached\n" << std::flush;
  }
  catch (const std::exception& ex)
  {
    std::cerr << "[" << __FUNCTION__ << "] " << ex.what();
  }
}

//----------------------------------------------------------------------------------------------------------------------
void MotorController::trigger(const fsm::FsmSignal& signal)
//----------------------------------------------------------------------------------------------------------------------
{
  controller_fsm_.raise(signal);
}

//----------------------------------------------------------------------------------------------------------------------
fsm::FsmName MotorController::getCurrentState() const
//----------------------------------------------------------------------------------------------------------------------
{
  return controller_fsm_.getCurrentState()->getName();
}
