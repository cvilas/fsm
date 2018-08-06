//=====================================================================================================================
// This file is part of fsm (https://github.com/cvilas/fsm)
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

//====================================================================================================================
/// Exception raised by FSM
class FsmException : public std::runtime_error
{
  static constexpr char DEFAULT_MESSAGE[] = "FSM error";

public:
  explicit FsmException(const std::string& message = DEFAULT_MESSAGE);
};

//======================================================================================================================
/// Represents a single state within an Fsm
class State
{
public:
  using Id = std::string;

public:
  virtual ~State();

  /// Get identifier
  Id getId() const;

  /// Get reference to FSM to which this state belongs
  Fsm& getFsm();

  /// Method gets called on entry into state.
  /// \note This method should not block or else state transitions will not occur
  virtual void onEntry() = 0;

  /// Method gets called on exit from state
  /// \note This method should not block or else state transitions will not occur
  virtual void onExit() = 0;

protected:
  /// Only derived classes can be instantiated directly
  State(Fsm& fsm, Id id);

protected:
  Fsm& fsm_;
  Id id_;
};

//======================================================================================================================
/// A finite state machine.
///
/// Steps to use the class
/// - Define and add states (see Fsm::addState)
/// - Define and add state transition rules (see Fsm::addTransitionRule)
/// - Initialise and start (see Fsm::start)
/// - Raise event to change state (see Fsm::raise)
class Fsm
{
public:
  using Event = std::string;

public:
  Fsm();
  virtual ~Fsm();

  /// Add a state in the machine. Also see Fsm::addTransitionRule().
  void addState(std::shared_ptr<State> state);

  /// Define state transition rule. The corresponding states must already exist. See Fsm::addState().
  /// \param from_state The name of state to transition from
  /// \param event The signal that causes the state transition
  /// \param to_state The name of state to transition to.
  void addTransitionRule(const State::Id& from_state, const Event& event, const State::Id& to_state);

  /// Signature for state transition function. See addTransitionRule().
  using TransitionFunction = std::function<State::Id()>;

  /// Define state transition rule as a function. Allows implementation of conditional state
  /// transitions or user-defined processing steps in response to an event. The corresponding states
  /// must already exist. See Fsm::addState().
  /// \param from_state The name of state to transition from
  /// \param event The signal that causes the state transition
  /// \param func Function returns ID of resulting state.
  void addTransitionRule(const State::Id& from_state, const Event& event, TransitionFunction&& func);

  /// Set the initial state and start the state machine
  void start(const State::Id& state);

  /// Raise an event. This will kick of a state transition if one is defined for this event and current state.
  /// The event is quietly ignored otherwise.
  void raise(const Event& event);

  /// \return true if not all events have been processed yet.
  bool hasPendingEvents() const;

  /// \return A pointer to current state. Use this do perform operations on this state.
  const std::shared_ptr<State>& getActiveState() const;

  /// \return true if the FSM is running
  bool isRunning() const;

private:
  void stop();
  bool hasTransitionRule(const State::Id& state_name, const Event& event);
  void eventHandler();
  void changeState(const Event& event);

private:
  /// Defines an FSM transition from one state to another.
  struct Transition
  {
    State::Id from_state;        //!< state active at the time of event
    Event event;                 //!< signal that triggers state transition
    TransitionFunction transit;  //!< Functional that returns state to transition into next
  };

private:
  std::map<State::Id, std::shared_ptr<State>> states_;
  std::multimap<State::Id, Transition> transitions_;
  std::shared_ptr<State> active_state_;

  mutable std::recursive_mutex event_guard_;
  bool exit_flag_;
  std::condition_variable_any event_condition_;
  std::queue<Event> event_queue_;
  std::future<void> event_handler_;
};

}  // namespace fsm

#endif  // FMS_H
