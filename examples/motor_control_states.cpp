//=====================================================================================================================
// This file is part of fsm (https://github.com/cvilas/fsm)
// Licensed under the MIT License. See LICENSE.md
//=====================================================================================================================

#include "motor_control_states.h"
#include <functional>
#include <iostream>

//=====================================================================================================================
IdleState::IdleState(fsm::Fsm& fsm) : fsm::State(fsm, "idle")
//=====================================================================================================================
{
}

//----------------------------------------------------------------------------------------------------------------------
void IdleState::onEntry()
//----------------------------------------------------------------------------------------------------------------------
{
  std::cout << "[" << getId() << "::onEntry]\n";
}

//----------------------------------------------------------------------------------------------------------------------
void IdleState::onExit()
//----------------------------------------------------------------------------------------------------------------------
{
  std::cout << "[" << getId() << "::onExit]\n";
}

//=====================================================================================================================
PowerUpState::PowerUpState(fsm::Fsm& ctx) : fsm::State(ctx, "power_up")
//=====================================================================================================================
{
}

//----------------------------------------------------------------------------------------------------------------------
void PowerUpState::onEntry()
//----------------------------------------------------------------------------------------------------------------------
{
  std::cout << "[" << getId() << "::onEntry] entered\n";
  std::this_thread::sleep_for(std::chrono::seconds(2));
  getFsm().raise("maintain_speed");
  std::cout << "[" << getId() << "::onEntry] exited\n";
}

//----------------------------------------------------------------------------------------------------------------------
void PowerUpState::onExit()
//----------------------------------------------------------------------------------------------------------------------
{
  std::cout << "[" << getId() << "::onExit]\n";
}

//=====================================================================================================================
PowerDownState::PowerDownState(fsm::Fsm& ctx) : fsm::State(ctx, "power_down")
//=====================================================================================================================
{
}

//----------------------------------------------------------------------------------------------------------------------
void PowerDownState::onEntry()
//----------------------------------------------------------------------------------------------------------------------
{
  std::cout << "[" << getId() << "::onEntry] entered\n";
  std::this_thread::sleep_for(std::chrono::seconds(2));
  getFsm().raise("has_shutdown");
  std::cout << "[" << getId() << "::onEntry] exited\n";
}

//----------------------------------------------------------------------------------------------------------------------
void PowerDownState::onExit()
//----------------------------------------------------------------------------------------------------------------------
{
  std::cout << "[" << getId() << "::onExit]\n";
}

//=====================================================================================================================
SpeedControlState::SpeedControlState(fsm::Fsm& ctx) : State(ctx, "speed_control")
//=====================================================================================================================
{
}

//----------------------------------------------------------------------------------------------------------------------
void SpeedControlState::onEntry()
//----------------------------------------------------------------------------------------------------------------------
{
  std::cout << "[" << getId() << "::onEntry]\n";
}

//----------------------------------------------------------------------------------------------------------------------
void SpeedControlState::onExit()
//----------------------------------------------------------------------------------------------------------------------
{
  std::cout << "[" << getId() << "::onExit]\n";
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

  controller_fsm_.start("idle");
}

//----------------------------------------------------------------------------------------------------------------------
MotorController::~MotorController()
//----------------------------------------------------------------------------------------------------------------------
{
  try
  {
    if (!controller_fsm_.isRunning())
    {
      return;
    }
    controller_fsm_.raise("off");
    std::cout << "Waiting for \"idle\" state..\n" << std::flush;
    while (getActiveState() != "idle")
    {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "State \"idle\" reached\n" << std::flush;
  }
  catch (const std::exception& ex)
  {
    std::cerr << "[" << __FUNCTION__ << "] " << ex.what();  // NOLINT
  }
}

//----------------------------------------------------------------------------------------------------------------------
void MotorController::trigger(const fsm::Fsm::Event& signal)
//----------------------------------------------------------------------------------------------------------------------
{
  controller_fsm_.raise(signal);
}

//----------------------------------------------------------------------------------------------------------------------
fsm::State::Id MotorController::getActiveState() const
//----------------------------------------------------------------------------------------------------------------------
{
  return controller_fsm_.getActiveState()->getId();
}
