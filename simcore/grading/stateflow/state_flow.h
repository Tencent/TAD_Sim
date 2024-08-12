// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace eval {
// type of state
typedef std::string StateType;

// io/parameters for state-flow
struct FLowInput {};
struct FlowOutput {};
struct FlowParams {};

typedef FLowInput *FLowInputPtr;
typedef FlowOutput *FlowOutputPtr;
typedef FlowParams *FlowParamsPtr;

// state-flow interface
template <class Input, class Output, class Params>
class I_Flow;

template <class StateFlowCls>
class I_State;
template <class StateFlowCls>
using I_StatePtr = std::shared_ptr<I_State<StateFlowCls>>;

// interface for state-flow
template <class Input, class Output, class Params>
class I_Flow {
 public:
  // operation
  virtual bool Switch(const StateType &state_type) = 0;
  virtual bool Init() = 0;
  virtual bool Update(const Input &input) = 0;
  virtual bool Stop() = 0;

  // IO/Params, overload functions
  Output *MutableOutput() { return &m_output; }

  // output of state-flow, overload functions
  const Params &GetParams() const { return m_param; }
  const Input &GetInput() const { return m_input; }
  const Output &GetOutput() const { return m_output; }

 protected:
  Input m_input;
  Output m_output;
  Params m_param;
};

// interface for state in state-flow
template <class StateFlowCls>
class I_State {
 public:
  /*entry of state, do not set output in this state*/
  virtual bool OnEntry() = 0;
  /*update of state, set output in this state*/
  virtual bool Update() = 0;
  /*exit of state, set output in this state*/
  virtual bool OnExit() = 0;

  const StateType &GetStateType() { return m_state_type; }

 protected:
  StateType m_state_type;
  StateFlowCls *m_flow;
};

}  // namespace eval
