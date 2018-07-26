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
using FsmName = std::string;
using FsmSignal = std::string;

//======================================================================================================================
/// Represents a single state within an Fsm
class FsmState
{
public:
  virtual ~FsmState();

  /// Get identifier
  FsmName getName() const;

  /// Get reference to FSM to which this state belongs
  Fsm& getFsm();

  /// Method gets called on entry into state.
  /// \note: This method should not block or else state transitions will not occur
  virtual void onEntry() = 0;

  /// Method gets called on exit from state
  /// \note: This method should not block or else state transitions will not occur
  virtual void onExit() = 0;

protected:
  /// Only derived classes can be instantiated directly
  FsmState(Fsm& fsm, FsmName name);

protected:
  Fsm& fsm_;
  std::string name_;
};

//======================================================================================================================
/// A finite state machine.
///
/// Steps to use the class
/// - Define and add states (see Fsm::addState)
/// - Define and add state transition rules (see Fsm::addTransitionRule)
/// - Initialise with a starting state (see Fsm::initialise)
/// - Raise signals to change state (see Fsm::raise)
class Fsm
{
public:
  /// Defines an FSM transition from one state to another.
  struct FsmTransition
  {
    FsmName current_state;
    FsmName next_state;
    FsmSignal signal;
  };

public:
  Fsm();
  ~Fsm();
  void addState(std::shared_ptr<FsmState> state);
  void addTransitionRule(const FsmName& from_state, const FsmSignal& signal, const FsmName& to_state);
  void initialise(const FsmName& state);
  void raise(const FsmSignal& signal);
  bool isStateTransitionPending() const;
  const std::shared_ptr<FsmState>& getCurrentState() const;

private:
  bool stateExists(const FsmName& name);
  bool transitionRuleExists(const FsmName& state_name, const FsmSignal& signal);
  void transitionHandler();
  void changeState(const FsmSignal& signal);

private:
  std::vector<std::shared_ptr<FsmState>> states_;
  std::vector<FsmTransition> transitions_;
  std::shared_ptr<FsmState> current_state_;

  mutable std::mutex signal_guard_;
  bool exit_flag_;
  std::condition_variable signal_condition_;
  std::deque<FsmSignal> signal_queue_;
  std::future<void> transition_handler_;
};

}  // namespace fsm

#endif  // FMS_H
