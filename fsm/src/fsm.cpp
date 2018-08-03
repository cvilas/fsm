//=====================================================================================================================
// This file is part of project fsm (https://github.com/cvilas/fsm)
// (C) 2018 Vilas Kumar Chitrakaran
// Licensed under the MIT License. See LICENSE.md
//=====================================================================================================================

#include "fsm/fsm.h"
#include <algorithm>
#include <sstream>

namespace fsm
{
//=====================================================================================================================
FsmException::FsmException(const std::string& message) : std::runtime_error(message)
//=====================================================================================================================
{
}

constexpr char FsmException::DEFAULT_MESSAGE[];

//======================================================================================================================
State::State(Fsm& fsm, Id id) : fsm_(fsm), id_(std::move(id))
//======================================================================================================================
{
}

//----------------------------------------------------------------------------------------------------------------------
State::~State() = default;
//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------
State::Id State::getId() const
//----------------------------------------------------------------------------------------------------------------------
{
  return id_;
}

//----------------------------------------------------------------------------------------------------------------------
Fsm& State::getFsm()
//----------------------------------------------------------------------------------------------------------------------
{
  return fsm_;
}

//======================================================================================================================
Fsm::Fsm() : exit_flag_(false)
//======================================================================================================================
{
}

//----------------------------------------------------------------------------------------------------------------------
Fsm::~Fsm()
//----------------------------------------------------------------------------------------------------------------------
{
  stop();
}

//----------------------------------------------------------------------------------------------------------------------
void Fsm::addState(std::shared_ptr<State> state)
//----------------------------------------------------------------------------------------------------------------------
{
  states_[state->getId()] = std::move(state);
}

//----------------------------------------------------------------------------------------------------------------------
bool Fsm::transitionRuleExists(const State::Id& state_name, const Event& event)
//----------------------------------------------------------------------------------------------------------------------
{
  auto tr = transitions_.equal_range(state_name);
  return tr.second != std::find_if(tr.first, tr.second, [&event](const auto& p) { return p.second.event == event; });
}

//----------------------------------------------------------------------------------------------------------------------
void Fsm::addTransitionRule(const State::Id& from_state, const Event& event, const State::Id& to_state)
//----------------------------------------------------------------------------------------------------------------------
{
  if (states_.find(to_state) == states_.end())
  {
    std::stringstream str;
    str << "[" << __FUNCTION__ << "] State \"" << to_state << "\" does not exit";  // NOLINT
    throw FsmException(str.str());
  }
  addTransitionRule(from_state, event, [to_state]() -> State::Id { return to_state; });
}

//----------------------------------------------------------------------------------------------------------------------
void Fsm::addTransitionRule(const State::Id& from_state, const Event& event, TransitionFunction&& func)
//----------------------------------------------------------------------------------------------------------------------
{
  if (states_.find(from_state) == states_.end())
  {
    std::stringstream str;
    str << "[" << __FUNCTION__ << "] State \"" << from_state << "\" does not exit";  // NOLINT
    throw FsmException(str.str());
  }
  /*
  if (states_.find(to_state) == states_.end())
  {
    std::stringstream str;
    str << "[" << __FUNCTION__ << "] State \"" << to_state << "\" does not exit";  // NOLINT
    throw FsmException(str.str());
  }*/
  if (transitionRuleExists(from_state, event))
  {
    std::stringstream str;
    str << "[" << __FUNCTION__ << "] Transition from \"" << from_state << "\" already exists for \""  // NOLINT
        << event << "\"";
    throw FsmException(str.str());
  }
  transitions_.emplace(
      std::make_pair(State::Id(from_state), Transition{ from_state, event, std::forward<TransitionFunction>(func) }));
}

//----------------------------------------------------------------------------------------------------------------------
void Fsm::start(const State::Id& state)
//----------------------------------------------------------------------------------------------------------------------
{
  if (isRunning())
  {
    std::stringstream str;
    str << "[" << __FUNCTION__ << "] Re-initialising is forbidden";  // NOLINT
    throw FsmException(str.str());
  }

  auto it = states_.find(state);
  if (it == states_.end())
  {
    std::stringstream str;
    str << "[" << __FUNCTION__ << "] State \"" << state << "\" does not exist";  // NOLINT
    throw FsmException(str.str());
  }
  active_state_ = it->second;
  active_state_->onEntry();

  exit_flag_ = false;
  event_handler_ = std::async(std::launch::async, [this]() { this->eventHandler(); });
}

//----------------------------------------------------------------------------------------------------------------------
void Fsm::stop()
//----------------------------------------------------------------------------------------------------------------------
{
  exit_flag_ = true;
  event_condition_.notify_all();
  if (event_handler_.valid())
  {
    event_handler_.wait();
  }
}

//----------------------------------------------------------------------------------------------------------------------
const std::shared_ptr<State>& Fsm::getActiveState() const
//----------------------------------------------------------------------------------------------------------------------
{
  if (active_state_ == nullptr)
  {
    std::stringstream str;
    str << "[" << __FUNCTION__ << "] FSM not initialised";  // NOLINT
    throw FsmException(str.str());
  }
  return active_state_;
}

//----------------------------------------------------------------------------------------------------------------------
void Fsm::raise(const Event& event)
//----------------------------------------------------------------------------------------------------------------------
{
  if (!event_handler_.valid())
  {
    std::stringstream str;
    str << "[" << __FUNCTION__ << "] Got event \"" << event << "\" when FSM is not running";  // NOLINT
    throw FsmException(str.str());
  }
  std::lock_guard<std::recursive_mutex> lk(event_guard_);
  event_queue_.push(event);
  event_condition_.notify_one();
}

//----------------------------------------------------------------------------------------------------------------------
bool Fsm::isRunning() const
//----------------------------------------------------------------------------------------------------------------------
{
  return event_handler_.valid();
}

//----------------------------------------------------------------------------------------------------------------------
bool Fsm::hasPendingEvents() const
//----------------------------------------------------------------------------------------------------------------------
{
  std::lock_guard<std::recursive_mutex> lk(event_guard_);
  return !event_queue_.empty();
}

//----------------------------------------------------------------------------------------------------------------------
void Fsm::changeState(const Event& event)
//----------------------------------------------------------------------------------------------------------------------
{
  const auto all_transitions = transitions_.equal_range(active_state_->getId());
  const auto it = std::find_if(all_transitions.first, all_transitions.second,
                               [&event](const auto& keyval) { return keyval.second.event == event; });

  if (it != all_transitions.second)
  {
    // exit current state and bring up new state
    active_state_->onExit();
    active_state_ = states_.at(it->second.transit());  /// \todo catch non-existent output state
    active_state_->onEntry();
  }
}

//----------------------------------------------------------------------------------------------------------------------
void Fsm::eventHandler()
//----------------------------------------------------------------------------------------------------------------------
{
  while (true)
  {
    std::unique_lock<std::recursive_mutex> lk(event_guard_);
    event_condition_.wait(lk);

    while (!event_queue_.empty())
    {
      const auto sig = event_queue_.front();
      event_queue_.pop();

      changeState(sig);
    }

    if (exit_flag_)
    {
      return;
    }
  }
}
}  // namespace fsm
