//=====================================================================================================================
// Copyright (C) 2018 Vilas Kumar Chitrakaran
//
// This file is part of project fsm (https://github.com/cvilas/fsm)
//
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

//======================================================================================================================
FsmState::FsmState(Fsm& fsm, FsmName name) : fsm_(fsm), name_(std::move(name))
//======================================================================================================================
{
}

//----------------------------------------------------------------------------------------------------------------------
FsmState::~FsmState() = default;
//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------
FsmName FsmState::getName() const
//----------------------------------------------------------------------------------------------------------------------
{
  return name_;
}

//----------------------------------------------------------------------------------------------------------------------
Fsm& FsmState::getFsm()
//----------------------------------------------------------------------------------------------------------------------
{
  return fsm_;
}

//======================================================================================================================
Fsm::Fsm() : exit_flag_(false)
//======================================================================================================================
{
  transition_handler_ = std::async(std::launch::async, [this]() { this->transitionHandler(); });
}

//----------------------------------------------------------------------------------------------------------------------
Fsm::~Fsm()
//----------------------------------------------------------------------------------------------------------------------
{
  exit_flag_ = true;
  signal_condition_.notify_all();
  if (transition_handler_.valid())
  {
    transition_handler_.wait();
  }
}

//----------------------------------------------------------------------------------------------------------------------
void Fsm::addState(std::shared_ptr<FsmState> state)
//----------------------------------------------------------------------------------------------------------------------
{
  states_[state->getName()] = std::move(state);
}

//----------------------------------------------------------------------------------------------------------------------
bool Fsm::transitionRuleExists(const FsmName& state_name, const FsmSignal& signal)
//----------------------------------------------------------------------------------------------------------------------
{
  auto state_transitions = transitions_.equal_range(state_name);
  for (auto it = state_transitions.first; it != state_transitions.second; ++it)
  {
    if (it->second.signal == signal)
    {
      return true;
    }
  }
  return false;
}

//----------------------------------------------------------------------------------------------------------------------
void Fsm::addTransitionRule(const FsmName& from_state, const FsmSignal& signal, const FsmName& to_state)
//----------------------------------------------------------------------------------------------------------------------
{
  if (states_.find(from_state) == states_.end())
  {
    std::stringstream str;
    str << "[" << __FUNCTION__ << "] State \"" << from_state << "\" does not exit";  // NOLINT
    throw FsmException(str.str());
  }
  if (states_.find(to_state) == states_.end())
  {
    std::stringstream str;
    str << "[" << __FUNCTION__ << "] State \"" << to_state << "\" does not exit";  // NOLINT
    throw FsmException(str.str());
  }
  if (transitionRuleExists(from_state, signal))
  {
    std::stringstream str;
    str << "[" << __FUNCTION__ << "] Transition from \"" << from_state << "\" already exists for \""  // NOLINT
        << signal << "\"";
    throw FsmException(str.str());
  }
  FsmTransition tr;
  transitions_.emplace(std::make_pair(FsmName(from_state), FsmTransition{ from_state, to_state, signal }));
}

//----------------------------------------------------------------------------------------------------------------------
void Fsm::initialise(const FsmName& state)
//----------------------------------------------------------------------------------------------------------------------
{
  if (current_state_ != nullptr)
  {
    current_state_->onExit();
  }
  auto it = states_.find(state);
  if (it == states_.end())
  {
    std::stringstream str;
    str << "[" << __FUNCTION__ << "] State \"" << state << "\" does not exist";  // NOLINT
    throw FsmException(str.str());
  }
  current_state_ = it->second;
  current_state_->onEntry();
}

//----------------------------------------------------------------------------------------------------------------------
const std::shared_ptr<FsmState>& Fsm::getCurrentState() const
//----------------------------------------------------------------------------------------------------------------------
{
  if (current_state_ == nullptr)
  {
    std::stringstream str;
    str << "[" << __FUNCTION__ << "] FSM not initialised";  // NOLINT
    throw FsmException(str.str());
  }
  return current_state_;
}

//----------------------------------------------------------------------------------------------------------------------
void Fsm::raise(const FsmSignal& signal)
//----------------------------------------------------------------------------------------------------------------------
{
  std::lock_guard<std::mutex> lk(signal_guard_);
  signal_queue_.push(signal);
  signal_condition_.notify_one();
}

//----------------------------------------------------------------------------------------------------------------------
bool Fsm::isStateTransitionPending() const
//----------------------------------------------------------------------------------------------------------------------
{
  std::lock_guard<std::mutex> lk(signal_guard_);
  return !signal_queue_.empty();
}

//----------------------------------------------------------------------------------------------------------------------
void Fsm::changeState(const FsmSignal& signal)
//----------------------------------------------------------------------------------------------------------------------
{
  if (current_state_ == nullptr)
  {
    std::stringstream str;
    str << "[" << __FUNCTION__ << "] FSM not initialised";  // NOLINT
    throw FsmException(str.str());
  }

  auto all_transitions = transitions_.equal_range(current_state_->getName());
  for (auto it = all_transitions.first; it != all_transitions.second; ++it)
  {
    if (it->second.signal == signal)
    {
      // exit current state and bring up new state
      current_state_->onExit();
      current_state_ = states_.at(it->second.next_state);
      current_state_->onEntry();
      return;
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------
void Fsm::transitionHandler()
//----------------------------------------------------------------------------------------------------------------------
{
  while (true)
  {
    std::unique_lock<std::mutex> lk(signal_guard_);
    signal_condition_.wait(lk);

    while (!signal_queue_.empty())
    {
      const auto sig = signal_queue_.front();
      signal_queue_.pop();

      lk.unlock();
      changeState(sig);
      lk.lock();
    }

    if (exit_flag_)
    {
      return;
    }
  }
}
}  // namespace fsm
