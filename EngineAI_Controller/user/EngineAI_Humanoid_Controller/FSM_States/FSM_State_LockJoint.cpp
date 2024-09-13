//
// Created by engineai on 2024/07/03.
//
/*============================= Joint PD ==============================*/
/**
 * FSM State that allows PD control of the joints.
 */

#include "FSM_State_LockJoint.h"
#include <Configuration.h>

/**
 * Constructor for the FSM State that passes in state specific info to
 * the generic FSM State constructor.
 *
 * @param _controlFSMData holds all of the relevant control data
 */
template <typename T>
FSM_State_LockJoint<T>::FSM_State_LockJoint(ControlFSMData<T> *_controlFSMData)
    : FSM_State<T>(_controlFSMData, FSM_StateName::LOCK_JOINT, "LOCK_JOINT"),
      _ini_jpos(12)
{
    // Do nothing here yet
}

template <typename T>
void FSM_State_LockJoint<T>::onEnter()
{
    // Default is to not transition
    this->nextStateName = this->stateName;

    // Reset the transition data
    this->transitionData.zero();

    // Reset counter
    iter = 0;

    for (int i = 0; i < 12; i++)
    {
        _ini_jpos[i] = FSM_State<T>::_data->_legController->leg_control_data_biped.q[i];
    }

    printf("[FSM LOCKJOINT] On Enter\n");
    std::cout << "[FSM LOCKJOINT] joint damping:" << this->_data->controlParameters->safety_damper_kd[0] << " " << this->_data->controlParameters->safety_damper_kd[1] << " " << this->_data->controlParameters->safety_damper_kd[2] << std::endl;
}

/**
 * Calls the functions to be executed on each control loop iteration.
 */
template <typename T>
void FSM_State_LockJoint<T>::run()
{
    // This is just a test, should be running whatever other code you want
    Vec12<T> qDes;
    qDes << 0.0, 0.0, 0.24, -0.48, 0.24, -0.24, 0.0, 0.0, 0.24, -0.48, -0.24, 0.24;

    Vec12<T> kp_stance = Vec12<T>::Zero();

    Vec12<T> kd_stance;
    kd_stance << this->_data->controlParameters->safety_damper_kd[0], this->_data->controlParameters->safety_damper_kd[0], this->_data->controlParameters->safety_damper_kd[0],
        this->_data->controlParameters->safety_damper_kd[1], this->_data->controlParameters->safety_damper_kd[2], this->_data->controlParameters->safety_damper_kd[2],
        this->_data->controlParameters->safety_damper_kd[0], this->_data->controlParameters->safety_damper_kd[0], this->_data->controlParameters->safety_damper_kd[0],
        this->_data->controlParameters->safety_damper_kd[1], this->_data->controlParameters->safety_damper_kd[2], this->_data->controlParameters->safety_damper_kd[2];

    for (int i = 0; i < 12; i++)
    {
        FSM_State<T>::_data->_legController->leg_control_data_biped.q_des[i] = qDes[i];
        FSM_State<T>::_data->_legController->leg_control_data_biped.qd_des[i] = 0.0;
        FSM_State<T>::_data->_legController->leg_control_data_biped.tau_ff[i] = 0.0;
        FSM_State<T>::_data->_legController->leg_control_data_biped.kp[i] = kp_stance[i];
        FSM_State<T>::_data->_legController->leg_control_data_biped.kd[i] = kd_stance[i];
    }
}

/**
 * Manages which states can be transitioned into either by the user
 * commands or state event triggers.
 *
 * @return the enumerated FSM state name to transition into
 */
template <typename T>
FSM_StateName FSM_State_LockJoint<T>::checkTransition()
{
    this->nextStateName = this->stateName;

    iter++;

    // Switch FSM control mode
    switch ((int)this->_data->controlParameters->control_mode)
    {
    case K_LOCK_JOINT:
        // Normal operation for state based transitions
        break;

    case K_BALANCE_STAND:
        this->nextStateName = FSM_StateName::BALANCE_STAND;
        break;

    case K_JOINT_PD:
        this->nextStateName = FSM_StateName::JOINT_PD;
        break;

    case K_PASSIVE:
        this->nextStateName = FSM_StateName::PASSIVE;
        // Transition time is immediate
        this->transitionDuration = 0.0;

        break;

        // case K_RL_LOCOMOTION:
        //     this->nextStateName = FSM_StateName::RL_LOCOMOTION;
        //     // Transition time is immediate
        //     this->transitionDuration = 0.0;
        //     break;

    default:
        std::cout << "[CONTROL FSM] Bad Request: Cannot transition from "
                  << K_LOCK_JOINT << " to "
                  << this->_data->controlParameters->control_mode << std::endl;
    }

    // Get the next state
    return this->nextStateName;
}

/**
 * Handles the actual transition for the robot between states.
 * Returns true when the transition is completed.
 *
 * @return true if transition is complete
 */
template <typename T>
TransitionData<T> FSM_State_LockJoint<T>::transition()
{
    // Switch FSM control mode
    switch (this->nextStateName)
    {

    case FSM_StateName::JOINT_PD:
        this->transitionData.done = true;
        break;

    case FSM_StateName::BALANCE_STAND:
        this->transitionData.done = true;
        break;

    case FSM_StateName::PASSIVE:
        this->turnOffAllSafetyChecks();

        this->transitionData.done = true;
        break;

    default:
        std::cout << "[CONTROL FSM] Bad Request: Cannot transition from "
                  << K_LOCK_JOINT << " to "
                  << this->_data->controlParameters->control_mode << std::endl;
    }
    // Finish transition
    this->transitionData.done = true;

    // Return the transition data to the FSM
    return this->transitionData;
}

/**
 * Cleans up the state information on exiting the state.
 */
template <typename T>
void FSM_State_LockJoint<T>::onExit()
{
    // Nothing to clean up when exiting
}

// template class FSM_State_LockJoint<double>;
template class FSM_State_LockJoint<float>;
