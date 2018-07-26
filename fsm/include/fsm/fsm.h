//=====================================================================================================================
// Copyright (C) 2018 Vilas Kumar Chitrakaran
//
// This file is part of project fsm (https://github.com/cvilas/fsm)
//
// Licensed under the MIT License. See LICENSE.md
//=====================================================================================================================

#ifndef FSM_H
#define FSM_H

#include <algorithm>
#include <condition_variable>
#include <deque>
#include <future>
#include <memory>
#include <mutex>
#include <vector>

namespace fsm
{
class Fsm;
class FsmState;
struct FsmTransition;
using FsmSignal = std::string;

//======================================================================================================================
class FsmState
{
public:
  virtual ~FsmState()
  {
  }
  virtual void onEntry() = 0;
  virtual void onExit() = 0;
  std::string getName() const
  {
    return name_;
  }
  Fsm& getFsm()
  {
    return fsm_;
  }

protected:
  FsmState(Fsm& fsm, const std::string& name) : fsm_(fsm), name_(name)
  {
  }

protected:
  Fsm& fsm_;
  std::string name_;
};

//======================================================================================================================
struct FsmTransition
{
  std::string current_state;
  std::string new_state;
  FsmSignal signal;
};

//======================================================================================================================
class Fsm
{
public:
  Fsm();
  ~Fsm();
  void addState(std::shared_ptr<FsmState> state);
  void addTransitionRule(const std::string& from_state, const FsmSignal& signal, const std::string& to_state);
  void initialise(const std::string& state);
  void raise(const FsmSignal& signal);
  bool isStateTransitionPending() const;
  const std::shared_ptr<FsmState>& getCurrentState() const;

private:
  bool stateExists(const std::string& name);
  bool transitionExists(const std::string& state_name, const FsmSignal& signal);
  void signalHandler();
  void changeState(const FsmSignal& signal);

private:
  std::vector<std::shared_ptr<FsmState>> states_;
  std::vector<FsmTransition> transitions_;
  std::shared_ptr<FsmState> current_state_;

  mutable std::mutex signal_guard_;
  bool exit_flag_;
  std::condition_variable signal_condition_;
  std::deque<FsmSignal> signal_queue_;
  std::future<void> signal_handler_;
};

}  // namespace fsm

#endif  // FMS_H
