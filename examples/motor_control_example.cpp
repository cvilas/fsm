//=====================================================================================================================
// This file is part of fsm (https://github.com/cvilas/fsm)
// Licensed under the MIT License. See LICENSE.md
//=====================================================================================================================

#include "motor_control_states.h"

#include <csignal>
#include <iostream>

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
