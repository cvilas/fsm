//=====================================================================================================================
// Copyright (C) 2018 Vilas Kumar Chitrakaran
//
// This file is part of project fsm (https://github.com/cvilas/fsm)
//
// Licensed under the MIT License. See LICENSE.md
//=====================================================================================================================

#ifndef FSM_H
#define FSM_H

#include <condition_variable>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <string>

namespace fsm
{
class Fsm;
class FsmState;
struct FsmTransition;
using FsmName = std::string;
using FsmSignal = std::string;

//====================================================================================================================
/// Exception raised by FSM
class FsmException : public std::runtime_error
{
  static constexpr char DEFAULT_MESSAGE[] = "Fsm error";

public:
  explicit FsmException(const std::string& message = DEFAULT_MESSAGE);
};

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
  FsmName name_;
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

  /// Add a state in the machine. Also see Fsm::addTransitionRule().
  void addState(std::shared_ptr<FsmState> state);

  /// Define state transition rule. The corresponding states must already exist See Fsm::addState().
  /// \param from_state The name of state to transition from
  /// \param signal The signal that causes the state transition
  /// \param to_state The name of state to transition to.
  void addTransitionRule(const FsmName& from_state, const FsmSignal& signal, const FsmName& to_state);

  /// Set the initial state and start the state machine
  void initialise(const FsmName& state);

  /// Raise a signal. This will kick of a state transition if one is defined for this signal and current state.
  /// The signal is quietly ignored otherwise.
  void raise(const FsmSignal& signal);

  /// \return true if we are busy processing state transitions.
  bool isStateTransitionPending() const;

  /// \return A pointer to current state. Use this do perform operations on this state.
  const std::shared_ptr<FsmState>& getCurrentState() const;

private:
  bool transitionRuleExists(const FsmName& state_name, const FsmSignal& signal);
  void transitionHandler();
  void changeState(const FsmSignal& signal);

private:
  std::map<FsmName, std::shared_ptr<FsmState>> states_;
  std::multimap<FsmName, FsmTransition> transitions_;
  std::shared_ptr<FsmState> current_state_;

  mutable std::mutex signal_guard_;
  bool exit_flag_;
  std::condition_variable signal_condition_;
  std::queue<FsmSignal> signal_queue_;
  std::future<void> transition_handler_;
};

}  // namespace fsm

#endif  // FMS_H
