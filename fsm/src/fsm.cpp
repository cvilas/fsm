//=====================================================================================================================
// Copyright (C) 2018 Vilas Kumar Chitrakaran
//
// This file is part of project fsm (https://github.com/cvilas/fsm)
//
// Licensed under the MIT License. See LICENSE.md
//=====================================================================================================================

#include "fsm/fsm.h"
#include <sstream>

namespace fsm
{
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
  states_.emplace_back(std::move(state));
}

//----------------------------------------------------------------------------------------------------------------------
bool Fsm::stateExists(const FsmName& name)
//----------------------------------------------------------------------------------------------------------------------
{
  return states_.end() != std::find_if(states_.begin(), states_.end(),
                                       [&name](const std::shared_ptr<FsmState>& s) { return s->getName() == name; });
}

//----------------------------------------------------------------------------------------------------------------------
bool Fsm::transitionRuleExists(const FsmName& state_name, const FsmSignal& signal)
//----------------------------------------------------------------------------------------------------------------------
{
  return transitions_.end() !=
         std::find_if(transitions_.begin(), transitions_.end(), [&state_name, &signal](const FsmTransition& t) {
           return ((t.current_state == state_name) && (t.signal == signal));
         });
}

//----------------------------------------------------------------------------------------------------------------------
void Fsm::addTransitionRule(const FsmName& from_state, const FsmSignal& signal, const FsmName& to_state)
//----------------------------------------------------------------------------------------------------------------------
{
  if (!stateExists(from_state))
  {
    std::stringstream str;
    str << "[" << __FUNCTION__ << "] 'From' state \"" << from_state << "\" does not exit";  // NOLINT
    throw std::runtime_error(str.str());
  }
  if (!stateExists(to_state))
  {
    std::stringstream str;
    str << "[" << __FUNCTION__ << "] 'To' state \"" << to_state << "\" does not exit";  // NOLINT
    throw std::runtime_error(str.str());
  }
  if (transitionRuleExists(from_state, signal))
  {
    std::stringstream str;
    str << "[" << __FUNCTION__ << "] Transition from \"" << from_state << "\" already exists for \""  // NOLINT
        << signal << "\"";
    throw std::runtime_error(str.str());
  }
  transitions_.push_back({ from_state, to_state, signal });
}

//----------------------------------------------------------------------------------------------------------------------
void Fsm::initialise(const FsmName& state)
//----------------------------------------------------------------------------------------------------------------------
{
  if (current_state_ != nullptr)
  {
    current_state_->onExit();
  }
  auto it = std::find_if(states_.begin(), states_.end(),
                         [&state](const std::shared_ptr<FsmState>& s) { return s->getName() == state; });
  if (it == states_.end())
  {
    std::stringstream str;
    str << "[" << __FUNCTION__ << "] State \"" << state << "\" does not exist";  // NOLINT
    throw std::runtime_error(str.str());
  }
  current_state_ = *it;
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
    throw std::runtime_error(str.str());
  }
  return current_state_;
}

//----------------------------------------------------------------------------------------------------------------------
void Fsm::raise(const FsmSignal& signal)
//----------------------------------------------------------------------------------------------------------------------
{
  std::lock_guard<std::mutex> lk(signal_guard_);
  signal_queue_.push_back(signal);
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
    throw std::runtime_error(str.str());
  }

  // find a valid transition
  const auto it = std::find_if(transitions_.begin(), transitions_.end(), [this, signal](const FsmTransition& t) {
    return (t.current_state == current_state_->getName()) && (t.signal == signal);
  });

  if (it == transitions_.end())
  {
    // std::cerr << "[" << __FUNCTION__ << "] No transition from \"" << current_state_->getName() << "\" defined for \""
    // << signal << "\". Ignored.\n";
    return;
  }

  // find next state
  auto next_state_it = std::find_if(states_.begin(), states_.end(), [it](const std::shared_ptr<FsmState>& state) {
    return state->getName() == (*it).next_state;
  });

  // exit current state and bring up new state
  current_state_->onExit();
  current_state_ = *next_state_it;
  current_state_->onEntry();
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
      signal_queue_.pop_front();

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
