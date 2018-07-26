//=====================================================================================================================
// Copyright (C) 2018 Vilas Kumar Chitrakaran
//
// This file is part of project fsm (https://github.com/cvilas/fsm)
//
// Licensed under the MIT License. See LICENSE.md
//=====================================================================================================================

#include "fsm/fsm.h"

#include <csignal>
#include <iostream>

/// do nothing state
class IdleState : public fsm::FsmState
{
public:
  explicit IdleState(fsm::Fsm& fsm) : fsm::FsmState(fsm, "idle")
  {
  }
  void onEntry() final
  {
    std::cout << "[" << getName() << "::onEntry]\n";
  }
  void onExit() final
  {
    std::cout << "[" << getName() << "::onExit]\n";
  }
};

/// powering up
class PowerUpState : public fsm::FsmState
{
public:
  explicit PowerUpState(fsm::Fsm& ctx) : fsm::FsmState(ctx, "power_up")
  {
  }
  void onEntry() final
  {
    std::cout << "[" << getName() << "::onEntry] entered\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
    getFsm().raise("maintain_speed");
    std::cout << "[" << getName() << "::onEntry] exited\n";
  }
  void onExit() final
  {
    std::cout << "[" << getName() << "::onExit]\n";
  }
};

/// powering down
class PowerDownState : public fsm::FsmState
{
public:
  explicit PowerDownState(fsm::Fsm& ctx) : fsm::FsmState(ctx, "power_down")
  {
  }
  void onEntry() final
  {
    std::cout << "[" << getName() << "::onEntry] entered\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
    getFsm().raise("has_shutdown");
    std::cout << "[" << getName() << "::onEntry] exited\n";
  }
  void onExit() final
  {
    std::cout << "[" << getName() << "::onExit]\n";
  }
};

/// maintain speed
class SpeedControlState : public fsm::FsmState
{
public:
  explicit SpeedControlState(fsm::Fsm& ctx) : FsmState(ctx, "speed_control")
  {
  }
  void onEntry() final
  {
    std::cout << "[" << getName() << "::onEntry] entered\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
    getFsm().raise("off");
    std::cout << "[" << getName() << "::onEntry] exited\n";
  }
  void onExit() final
  {
    std::cout << "[" << getName() << "::onExit]\n";
  }
};

/// The automatic motor controller
class MotorController
{
public:
  MotorController()
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

  ~MotorController()
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

  void trigger(const fsm::FsmSignal& signal)
  {
    controller_fsm_.raise(signal);
  }

  std::string getCurrentState() const
  {
    return controller_fsm_.getCurrentState()->getName();
  }

  MotorController(const MotorController&) = delete;
  MotorController(const MotorController&&) = delete;
  MotorController& operator=(const MotorController&) = delete;
  MotorController& operator=(const MotorController&&) = delete;

private:
  fsm::Fsm controller_fsm_;
};

static int s_stopFlag = 0;

//---------------------------------------------------------------------------------------------------------------------
static void onSignal(int signum)
//---------------------------------------------------------------------------------------------------------------------
{
  (void)signum;
  s_stopFlag = 1;
}

//=====================================================================================================================
int main(int argc, char** argv)
//=====================================================================================================================
{
  (void)argc;
  (void)argv;

  signal(SIGINT, &onSignal);

  try
  {
    MotorController controller;
    std::cout << "Starting state: " << controller.getCurrentState() << "\n" << std::flush;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    controller.trigger("on");

    while (0 == s_stopFlag)
    {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }
  catch (const std::exception& ex)
  {
    std::cerr << ex.what() << "\n";
  }
}
